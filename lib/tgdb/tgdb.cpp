/* Includes {{{ */
#if HAVE_CONFIG_H
#include "config.h"
#endif

#if HAVE_STDIO_H
#include <stdio.h>
#endif /* HAVE_STDIO_H */

#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */

#if HAVE_SIGNAL_H
#include <signal.h> /* sig_atomic_t */
#endif

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#if HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include "tgdb.h"
#include "fork_util.h"
#include "commands.h"
#include "annotations.h"
#include "ibuf.h"
#include "io.h"
#include "pseudo.h" /* SLAVE_SIZE constant */
#include "sys_util.h"
#include "cgdb_clog.h"

/* }}} */

/* struct tgdb {{{ */

/**
 * The TGDB context data structure.
 */
struct tgdb {
    /** This module is used for parsing the commands output of gdb */
    struct commands *c;

    /** The GDB annotations parser */
    struct annotations_parser *parser;

    /** Reading from this will read from the debugger's output */
    int debugger_stdout;

    /** Writing to this will write to the debugger's stdin */
    int debugger_stdin;

    /** Reading from this will read the stdout from the debugged program */
    int inferior_stdout;

    /** Writing to this will write to the stdin of the debugged program */
    int inferior_stdin;

    /** The master, slave and slavename of the pty device */
    pty_pair_ptr pty_pair;

    /** The pid of child process */
    pid_t debugger_pid;

    /** The list of command requests to process */
    tgdb_request_ptr_list *command_requests;

    /**
     * If set to 1, libtgdb thinks the lower level subsystem is capable of
     * receiving another command. It needs this so that it doesn't send 2
     * commands to the lower level before it can say it can't receive a command.
     * At some point, maybe this can be removed?
     * When its set to 0, libtgdb thinks it can not send the lower level another
     * command.
     *
     * Basically whether gdb is at prompt or not. */
    int is_gdb_ready_for_next_command;

    /**
     * Set to true if a console ready callback is required, otherwise false.
     *
     * Any command given to GDB, that impacts the console, should set this
     * to true when the command is run. Then when the next gdb prompt is
     * available, tgdb will tell cgdb the console is ready.
     */
    bool make_console_ready_callback;

    /** If ^c was hit by user */
    sig_atomic_t control_c;

    /**
     * When GDB dies (purposely or not), the SIGCHLD is sent to the
     * application controlling TGDB. This data structure represents the
     * fact that SIGCHLD has been sent.
     *
     * This currently does not need to track if more than 1 SIGCHLD has
     * been received. So no matter how many are receieved, this will only
     * be 1. Otherwise if none have been received this will be 0.  */
    int has_sigchld_recv;

    tgdb_callbacks callbacks;
};

/* }}} */

/* Temporary prototypes {{{ */
static void tgdb_run_request(struct tgdb *tgdb, struct tgdb_request *request);
static void tgdb_unqueue_and_deliver_command(struct tgdb *tgdb);
void tgdb_run_or_queue_request(struct tgdb *tgdb,
        struct tgdb_request *request, bool priority);

/* }}} */

static struct tgdb *initialize_tgdb_context(tgdb_callbacks callbacks)
{
    struct tgdb *tgdb = (struct tgdb *) cgdb_malloc(sizeof (struct tgdb));

    tgdb->c = 0;
    tgdb->parser = NULL;
    tgdb->control_c = 0;

    tgdb->debugger_stdout = -1;
    tgdb->debugger_stdin = -1;

    tgdb->inferior_stdout = -1;
    tgdb->inferior_stdin = -1;

    tgdb->pty_pair = NULL;

    tgdb->command_requests = new tgdb_request_ptr_list();

    tgdb->is_gdb_ready_for_next_command = 1;
    tgdb->make_console_ready_callback = true;

    tgdb->has_sigchld_recv = 0;

    tgdb->callbacks = callbacks;

    return tgdb;
}

/*******************************************************************************
 * This is the basic initialization
 ******************************************************************************/

/* 
 * Gets the users home dir and creates the config directory.
 *
 * Also returns the logs directory.
 *
 * @param tgdb
 * The tgdb context.
 *
 * @param logs_dir 
 * Should be FSUTIL_PATH_MAX in size on way in.
 * On way out, it will be the path to the logs dir
 *
 * \return
 * -1 on error, or 0 on success
 */
static int tgdb_initialize_config_dir(struct tgdb *tgdb, char *logs_dir)
{
    char config_dir[FSUTIL_PATH_MAX];
    char *home_dir = getenv("HOME");

    /* Make sure the toplevel .cgdb dir exists */
    snprintf(config_dir, FSUTIL_PATH_MAX, "%s/.cgdb", home_dir);
    if (!fs_util_create_dir(config_dir)) {
        clog_error(CLOG_CGDB, "Could not create dir %s", config_dir);
        return -1;
    }

    /* Try to create full .cgdb/logs directory */
    snprintf(logs_dir, FSUTIL_PATH_MAX, "%s/.cgdb/logs", home_dir);
    if (!fs_util_create_dir(logs_dir)) {
        clog_error(CLOG_CGDB, "Could not create dir %s", logs_dir);
        return -1;
    }

    return 0;
}

/**
 * Knowing the user's home directory, TGDB can initialize the logger interface
 *
 * \param tgdb
 * The tgdb context.
 *
 * \param logs_dir 
 * The path to the user's logging directory
 *
 * \return
 * -1 on error, or 0 on success
 */
static int tgdb_initialize_logger_interface(struct tgdb *tgdb, char *logs_dir)
{
    /* Open our cgdb and tgdb io logfiles */
    clog_open(CLOG_CGDB_ID, "%s/cgdb_log%d.txt", logs_dir);
    clog_open(CLOG_GDBIO_ID, "%s/cgdb_gdb_io_log%d.txt", logs_dir);

    /* Puts cgdb in a mode where it writes a debug log of everything
     * that is read from gdb. That is basically the entire session.
     * This info is useful in determining what is going on under tgdb
     * since the gui is good at hiding that info from the user.
     *
     * Change level to CLOG_ERROR to write only error messages.
     *   clog_set_level(CLOG_GDBIO, CLOG_ERROR);
     */
    clog_set_level(CLOG_GDBIO_ID, CLOG_DEBUG);
    clog_set_fmt(CLOG_GDBIO_ID, CGDB_CLOG_FORMAT);

    /* General cgdb logging. Only logging warnings and debug messages
       by default. */
    clog_set_level(CLOG_CGDB_ID, CLOG_WARN);
    clog_set_fmt(CLOG_CGDB_ID, CGDB_CLOG_FORMAT);


    return 0;
}

static void tgdb_issue_request(struct tgdb *tgdb, enum tgdb_request_type type,
        bool priority)
{
    tgdb_request_ptr request_ptr;
    request_ptr = (tgdb_request_ptr)cgdb_malloc(sizeof (struct tgdb_request));
    request_ptr->header = type;
    tgdb_run_or_queue_request(tgdb, request_ptr, priority);
}
 
static void tgdb_breakpoints_changed(void *context)
{
    struct tgdb *tgdb = (struct tgdb*)context;
    tgdb_issue_request(tgdb, TGDB_REQUEST_BREAKPOINTS, true);
}

static void tgdb_source_location_changed(void *context)
{
    struct tgdb *tgdb = (struct tgdb*)context;
    tgdb_request_current_location(tgdb);
}

static void tgdb_prompt_changed(void *context, const std::string &prompt)
{
    struct tgdb *tgdb = (struct tgdb*)context;

    struct tgdb_response *response =
        tgdb_create_response(TGDB_UPDATE_CONSOLE_PROMPT_VALUE);
    response->choice.update_console_prompt_value.prompt_value =
            cgdb_strdup(prompt.c_str());
    tgdb_send_response(tgdb, response);
}

static void tgdb_console_output(void *context, const std::string &msg)
{
    struct tgdb *tgdb = (struct tgdb*)context;
    enum tgdb_request_type type = commands_get_current_request_type(tgdb->c);

    // Send output to the terminal if the current command is a console command
    if (type == TGDB_REQUEST_CONSOLE_COMMAND) {
        tgdb->callbacks.console_output_callback(tgdb->callbacks.context, msg);
    } else {
        commands_process(tgdb->c, msg);
    }

}

static void tgdb_command_error(void *context, const std::string &msg)
{
    struct tgdb *tgdb = (struct tgdb*)context;

    /* Let the command package no that it's command was interupted */
    commands_process_error(tgdb->c);

    /* Send cgdb the error message */
    tgdb->callbacks.console_output_callback(tgdb->callbacks.context, msg);
}

static void tgdb_console_at_prompt(void *context)
{
    struct tgdb *tgdb = (struct tgdb*)context;

    tgdb->is_gdb_ready_for_next_command = 1;

    if (tgdb->make_console_ready_callback &&
            tgdb->command_requests->size() == 0) {
        tgdb->callbacks.console_ready_callback(tgdb->callbacks.context);
        tgdb->make_console_ready_callback = false;
    }

    if (tgdb->command_requests->size() > 0) {
        tgdb_unqueue_and_deliver_command(tgdb);
    }
}

int tgdb_process_command(struct tgdb *tgdb, tgdb_request_ptr request)
{
    tgdb_run_or_queue_request(tgdb, request, false);
    return 0;
}

static int tgdb_open_new_tty(struct tgdb *tgdb, int *inferior_stdin,
    int *inferior_stdout)
{
    if (tgdb->pty_pair)
        pty_pair_destroy(tgdb->pty_pair);

    tgdb->pty_pair = pty_pair_create();
    if (!tgdb->pty_pair) {
        clog_error(CLOG_CGDB, "pty_pair_create failed");
        return -1;
    }

    *inferior_stdin = pty_pair_get_masterfd(tgdb->pty_pair);
    *inferior_stdout = pty_pair_get_masterfd(tgdb->pty_pair);

    tgdb_request_ptr request_ptr;
    request_ptr = (tgdb_request_ptr)cgdb_malloc(sizeof (struct tgdb_request));
    request_ptr->header = TGDB_REQUEST_TTY;
    request_ptr->choice.tty_command.slavename = 
        pty_pair_get_slavename(tgdb->pty_pair);
    tgdb_run_or_queue_request(tgdb, request_ptr, true);

    return 0;
}

/**
 * Free the tgdb request pointer data.
 *
 * @param request_ptr
 * The request pointer to destroy.
 */
static void tgdb_request_destroy(tgdb_request_ptr request_ptr)
{
    if (!request_ptr)
        return;

    switch (request_ptr->header) {
        case TGDB_REQUEST_CONSOLE_COMMAND:
            free((char *) request_ptr->choice.console_command.command);
            request_ptr->choice.console_command.command = NULL;
            break;
        case TGDB_REQUEST_INFO_SOURCES:
            break;
        case TGDB_REQUEST_DEBUGGER_COMMAND:
            break;
        case TGDB_REQUEST_MODIFY_BREAKPOINT:
            free((char *) request_ptr->choice.modify_breakpoint.file);
            request_ptr->choice.modify_breakpoint.file = NULL;
            break;
        case TGDB_REQUEST_COMPLETE:
            free((char *) request_ptr->choice.complete.line);
            request_ptr->choice.complete.line = NULL;
            break;
        case TGDB_REQUEST_DISASSEMBLE_PC:
        case TGDB_REQUEST_DISASSEMBLE_FUNC:
            break;
        default:
            break;
    }

    free(request_ptr);
    request_ptr = NULL;
}


/* Creating and Destroying a libtgdb context. {{{*/

struct tgdb *tgdb_initialize(const char *debugger,
        int argc, char **argv, int *debugger_fd, tgdb_callbacks callbacks)
{
    /* Initialize the libtgdb context */
    struct tgdb *tgdb = initialize_tgdb_context(callbacks);
    static struct annotations_parser_callbacks annotations_callbacks = {
        tgdb,
        tgdb_breakpoints_changed,
        tgdb_source_location_changed,
        tgdb_prompt_changed,
        tgdb_console_output,
        tgdb_command_error,
        tgdb_console_at_prompt  
    };
    char logs_dir[FSUTIL_PATH_MAX];

    /* Create config directory */
    if (tgdb_initialize_config_dir(tgdb, logs_dir) == -1) {
        clog_error(CLOG_CGDB, "tgdb_initialize error");
        return NULL;
    }

    if (tgdb_initialize_logger_interface(tgdb, logs_dir) == -1) {
        printf("Could not initialize logger interface\n");
        return NULL;
    }

    tgdb->debugger_pid = invoke_debugger(debugger, argc, argv,
            &tgdb->debugger_stdin, &tgdb->debugger_stdout, 0);

    /* Couldn't invoke process */
    if (tgdb->debugger_pid == -1)
        return NULL;

    tgdb->c = commands_initialize(tgdb);

    tgdb->parser = annotations_parser_initialize(annotations_callbacks);

    tgdb_open_new_tty(tgdb, &tgdb->inferior_stdin, &tgdb->inferior_stdout);

    /* Need to get source information before breakpoint information otherwise
     * the TGDB_UPDATE_BREAKPOINTS event will be ignored in process_commands()
     * because there are no source files to add the breakpoints to.
     */
    tgdb_request_current_location(tgdb);

    /* gdb may already have some breakpoints when it starts. This could happen
     * if the user puts breakpoints in there .gdbinit.
     * This makes sure that TGDB asks for the breakpoints on start up.
     */
    tgdb_issue_request(tgdb, TGDB_REQUEST_BREAKPOINTS, true);

    /**
     * Query if disassemble supports the /s flag
     */
    tgdb_issue_request(tgdb, TGDB_REQUEST_DATA_DISASSEMBLE_MODE_QUERY, true);

    *debugger_fd = tgdb->debugger_stdout;

    return tgdb;
}

int tgdb_shutdown(struct tgdb *tgdb)
{
    tgdb_request_ptr_list::iterator iter = tgdb->command_requests->begin();
    for (; iter != tgdb->command_requests->end(); ++iter) {
        tgdb_request_destroy(*iter);
    }

    delete tgdb->command_requests;
    tgdb->command_requests = 0;

    annotations_parser_shutdown(tgdb->parser);

    cgdb_close(tgdb->debugger_stdin);
    tgdb->debugger_stdin = -1;

    commands_shutdown(tgdb->c);

    return 0;
}

void tgdb_close_logfiles()
{
    clog_info(CLOG_CGDB, "Closing logfile.");
    clog_free(CLOG_CGDB_ID);

    clog_info(CLOG_GDBIO, "Closing logfile.");
    clog_free(CLOG_GDBIO_ID);
}

/* }}}*/

static const char *tgdb_get_client_command(struct tgdb *tgdb,
        enum tgdb_command_type c)
{
    switch (c) {
        case TGDB_CONTINUE:
            return "continue";
        case TGDB_FINISH:
            return "finish";
        case TGDB_NEXT:
            return "next";
        case TGDB_START:
            return "start";
        case TGDB_RUN:
            return "run";
        case TGDB_KILL:
            return "kill";
        case TGDB_STEP:
            return "step";
        case TGDB_UNTIL:
            return "until";
        case TGDB_UP:
            return "up";
        case TGDB_DOWN:
            return "down";
    }

    return NULL;
}

static char *tgdb_client_modify_breakpoint_call(struct tgdb *tgdb,
    const char *file, int line, uint64_t addr, enum tgdb_breakpoint_action b)
{
    const char *action;

    switch (b)
    {
    default:
    case TGDB_BREAKPOINT_ADD:
        action = "break";
        break;
    case TGDB_BREAKPOINT_DELETE:
        action ="clear";
        break;
    case TGDB_TBREAKPOINT_ADD:
        action = "tbreak";
        break;
    }

    if (file)
        return sys_aprintf("%s \"%s\":%d", action, file, line);

    return sys_aprintf("%s *0x%" PRIx64, action, addr);
}

/*******************************************************************************
 * This is the main_loop stuff for tgdb-base
 ******************************************************************************/

/**
 * Run a command request if gdb is idle, otherwise queue it.
 *
 * @param tgdb
 * The TGDB context to use.
 *
 * @param request
 * The command request
 *
 * @param priority
 * True if this is a priority request, false otherwise.
 */
void tgdb_run_or_queue_request(struct tgdb *tgdb,
        struct tgdb_request *request, bool priority)
{
    int can_issue = tgdb->is_gdb_ready_for_next_command &&
        annotations_parser_at_prompt(tgdb->parser);

    if (request->header == TGDB_REQUEST_CONSOLE_COMMAND) {
        request->choice.console_command.queued = !can_issue;
    }

    if (can_issue) {
        tgdb_run_request(tgdb, request);
    } else {
        if (priority) {
            tgdb->command_requests->push_front(request);
        } else {
            tgdb->command_requests->push_back(request);
        }
    }
}

int tgdb_get_gdb_command(struct tgdb *tgdb, tgdb_request_ptr request,
        std::string &command);

/**
 * Send a command to gdb.
 *
 * @param tgdb
 * An instance of tgdb
 *
 * @param request 
 * The command request to issue the command for
 */
static void tgdb_run_request(struct tgdb *tgdb, struct tgdb_request *request)
{
    std::string command;

    if (request->header == TGDB_REQUEST_CONSOLE_COMMAND ||
        request->header == TGDB_REQUEST_COMPLETE ||
        request->header == TGDB_REQUEST_DEBUGGER_COMMAND) {
        tgdb->make_console_ready_callback = true;
    }

    tgdb->is_gdb_ready_for_next_command = 0;

    tgdb_get_gdb_command(tgdb, request, command);

    /* Add a newline to the end of the command if it doesn't exist */
    if (*command.rbegin() != '\n') {
        command.push_back('\n');
    }

    /* Send what we're doing to log file */
    char *str = sys_quote_nonprintables(command.c_str(), -1);
    clog_debug(CLOG_GDBIO, "%s", str);
    sbfree(str);

    /* A command for the debugger */
    commands_set_current_request_type(tgdb->c, request->header);

    io_writen(tgdb->debugger_stdin, command.c_str(), command.size());

    // Alert the GUI a command was sent
    tgdb->callbacks.request_sent_callback(
            tgdb->callbacks.context, request, command);

    tgdb_request_destroy(request);
}

/**
 * TGDB will search it's command queue's and determine what the next command
 * to deliever to GDB should be.
 *
 * \return
 * 0 on success, -1 on error
 */
static void tgdb_unqueue_and_deliver_command(struct tgdb *tgdb)
{
    if (tgdb->command_requests->size() > 0) {
        struct tgdb_request *request = tgdb->command_requests->front();
        tgdb->command_requests->pop_front();
        tgdb_run_request(tgdb, request);
        // TODO: free request?
    }
}

/* These functions are used to communicate with the inferior */
int tgdb_send_inferior_char(struct tgdb *tgdb, char c)
{
    if (io_write_byte(tgdb->inferior_stdout, c) == -1) {
        clog_error(CLOG_CGDB, "io_write_byte failed");
        return -1;
    }

    return 0;
}

ssize_t tgdb_recv_inferior_data(struct tgdb * tgdb, char *buf, size_t n)
{
    ssize_t size;

    /* read all the data possible from the child that is ready. */
    size = io_read(tgdb->inferior_stdin, buf, n);
    if (size < 0) {
        clog_error(CLOG_CGDB, "inferior_fd read failed");
        return -1;
    }

    return size;
}

/**
 * TGDB is going to quit.
 *
 * \param tgdb
 * The tgdb context
 *
 * \return
 * 0 on success or -1 on error
 */
static int tgdb_add_quit_command(struct tgdb *tgdb)
{
    struct tgdb_response *response;
    response = tgdb_create_response(TGDB_QUIT);
    tgdb_send_response(tgdb, response);
    return 0;
}

/**
 * Called to process the sigchld signal, and clean up zombies!
 *
 * @param tgdb
 * The tgdb instance
 *
 * @return
 * 0 on success or -1 on error
 */
static int tgdb_handle_sigchld(struct tgdb *tgdb)
{
    int result = 0;
    int status;

    if (tgdb->has_sigchld_recv) {
        int waitpid_result;

        do {
            waitpid_result = waitpid(tgdb->debugger_pid, &status, WNOHANG);
            if (waitpid_result == -1) {
                result = -1;
                clog_error(CLOG_CGDB, "waitpid error");
                break;
            }
        } while (waitpid_result != 0);

        tgdb->has_sigchld_recv = 0;
    }

    return result;
}

/**
 * If the user typed control_c at the prompt, clear the queues.
 *
 * @param tgdb
 * The tgdb instance to work on
 */
static void tgdb_handle_control_c(struct tgdb *tgdb)
{
    if (tgdb->control_c) {
        tgdb_request_ptr_list::iterator iter = tgdb->command_requests->begin();
        for (; iter != tgdb->command_requests->end(); ++iter) {
            tgdb_request_destroy(*iter);
        }
        tgdb->command_requests->clear();

        tgdb->control_c = 0;

        tgdb->make_console_ready_callback = true;
    }
}

int tgdb_process(struct tgdb * tgdb)
{
    const int n = 4096;
    static char buf[n];
    ssize_t size;
    int result = 0;

    // Cleanup those zombies!
    tgdb_handle_sigchld(tgdb);

    // If ^c has been typed at the prompt, clear the queues
    tgdb_handle_control_c(tgdb);

    size = io_read(tgdb->debugger_stdout, buf, n);
    if (size < 0) {
        // Error reading from GDB
        clog_error(CLOG_CGDB, "Error reading from gdb's stdout, closing down");
        result = -1;
        tgdb_add_quit_command(tgdb);
    } else if (size == 0) {
        // Read EOF from GDB
        clog_info(CLOG_GDBIO, "read EOF from GDB, closing down");
        tgdb_add_quit_command(tgdb);
    } else {
        // Read some GDB console output, process it
        result = annotations_parser_io(tgdb->parser, buf, size);
    }

    return result;
}

/* Getting Data out of TGDB {{{*/

struct tgdb_response *tgdb_create_response(enum tgdb_response_type header)
{
    struct tgdb_response *response;

    response = (struct tgdb_response *)cgdb_calloc(1, sizeof(struct tgdb_response));
    response->header = header;

    return response;
}

static int tgdb_delete_response(struct tgdb_response *com)
{
    if (!com)
        return -1;

    switch (com->header) {
        case TGDB_UPDATE_BREAKPOINTS:
        {
            int i;
            struct tgdb_breakpoint *breakpoints =
                com->choice.update_breakpoints.breakpoints;

            for (i = 0; i < sbcount(breakpoints); i++) {

                struct tgdb_breakpoint *tb = &breakpoints[i];

                free(tb->path);
            }

            sbfree(breakpoints);
            com->choice.update_breakpoints.breakpoints = NULL;
            break;
        }
        case TGDB_UPDATE_FILE_POSITION:
        {
            struct tgdb_file_position *tfp =
                    com->choice.update_file_position.file_position;

            free(tfp->path);
            free(tfp->from);
            free(tfp->func);

            free(tfp);

            com->choice.update_file_position.file_position = NULL;
            break;
        }
        case TGDB_UPDATE_SOURCE_FILES:
        {
            int i;
            char **source_files = com->choice.update_source_files.source_files;

            for (i = 0; i < sbcount(source_files); i++) {
                free(source_files[i]);
            }
            sbfree(source_files);

            com->choice.update_source_files.source_files = NULL;
            break;
        }
        case TGDB_UPDATE_COMPLETIONS:
        {
            int i;
            char **completions = com->choice.update_completions.completions;

            for (i = 0; i < sbcount(completions); i++)
                free(completions[i]);
            sbfree(completions);

            com->choice.update_completions.completions = NULL;
            break;
        }
        case TGDB_DISASSEMBLE_PC:
        case TGDB_DISASSEMBLE_FUNC:
        {
            int i;
            char **disasm = com->choice.disassemble_function.disasm;

            for (i = 0; i < sbcount(disasm); i++) {
                free(disasm[i]);
            }
            sbfree(disasm);
            break;
        }
        case TGDB_UPDATE_CONSOLE_PROMPT_VALUE:
        {
            const char *value =
                    com->choice.update_console_prompt_value.prompt_value;

            free((char *) value);
            com->choice.update_console_prompt_value.prompt_value = NULL;
            break;
        }
        case TGDB_QUIT:
            break;
    }

    free(com);
    com = NULL;
    return 0;
}

void tgdb_send_response(struct tgdb *tgdb, struct tgdb_response *response)
{
    tgdb->callbacks.command_response_callback(
            tgdb->callbacks.context, response);
    tgdb_delete_response(response);
}

/* }}}*/

/* Inferior tty commands {{{*/

int tgdb_tty_new(struct tgdb *tgdb)
{
    int ret = tgdb_open_new_tty(tgdb,
            &tgdb->inferior_stdin,
            &tgdb->inferior_stdout);

    return ret;
}

int tgdb_get_inferior_fd(struct tgdb *tgdb)
{
    return tgdb->inferior_stdout;
}

const char *tgdb_tty_name(struct tgdb *tgdb)
{
    return pty_pair_get_slavename(tgdb->pty_pair);
}

/* }}}*/

/* Functional commands {{{ */

/* Request {{{*/

void
tgdb_request_run_console_command(struct tgdb *tgdb, const char *command)
{
    tgdb_request_ptr request_ptr;

    request_ptr = (tgdb_request_ptr)cgdb_malloc(sizeof (struct tgdb_request));

    request_ptr->header = TGDB_REQUEST_CONSOLE_COMMAND;
    request_ptr->choice.console_command.command = (const char *)
            cgdb_strdup(command);

    tgdb_run_or_queue_request(tgdb, request_ptr, false);
}

void tgdb_request_inferiors_source_files(struct tgdb * tgdb)
{
    tgdb_request_ptr request_ptr;

    request_ptr = (tgdb_request_ptr)cgdb_malloc(sizeof (struct tgdb_request));

    request_ptr->header = TGDB_REQUEST_INFO_SOURCES;

    tgdb_run_or_queue_request(tgdb, request_ptr, false);
}

void tgdb_request_current_location(struct tgdb * tgdb)
{
    tgdb_request_ptr request_ptr;

    request_ptr = (tgdb_request_ptr)cgdb_malloc(sizeof (struct tgdb_request));

    request_ptr->header = TGDB_REQUEST_INFO_FRAME;

    tgdb_run_or_queue_request(tgdb, request_ptr, true);
}

void
tgdb_request_run_debugger_command(struct tgdb * tgdb, enum tgdb_command_type c)
{
    tgdb_request_ptr request_ptr;

    request_ptr = (tgdb_request_ptr)cgdb_malloc(sizeof (struct tgdb_request));

    request_ptr->header = TGDB_REQUEST_DEBUGGER_COMMAND;
    request_ptr->choice.debugger_command.c = c;

    tgdb_run_or_queue_request(tgdb, request_ptr, false);
}

void
tgdb_request_modify_breakpoint(struct tgdb *tgdb, const char *file, int line,
    uint64_t addr, enum tgdb_breakpoint_action b)
{
    tgdb_request_ptr request_ptr;

    request_ptr = (tgdb_request_ptr)cgdb_malloc(sizeof (struct tgdb_request));

    request_ptr->header = TGDB_REQUEST_MODIFY_BREAKPOINT;
    request_ptr->choice.modify_breakpoint.file = file ? cgdb_strdup(file) : NULL;
    request_ptr->choice.modify_breakpoint.line = line;
    request_ptr->choice.modify_breakpoint.addr = addr;
    request_ptr->choice.modify_breakpoint.b = b;

    tgdb_run_or_queue_request(tgdb, request_ptr, false);
}

void tgdb_request_complete(struct tgdb * tgdb, const char *line)
{
    tgdb_request_ptr request_ptr;

    request_ptr = (tgdb_request_ptr)cgdb_malloc(sizeof (struct tgdb_request));
    request_ptr->header = TGDB_REQUEST_COMPLETE;
    request_ptr->choice.complete.line = (const char *)cgdb_strdup(line);

    tgdb_run_or_queue_request(tgdb, request_ptr, false);
}

void tgdb_request_disassemble_pc(struct tgdb *tgdb, int lines)
{
    tgdb_request_ptr request_ptr;

    request_ptr = (tgdb_request_ptr)cgdb_malloc(sizeof (struct tgdb_request));
    request_ptr->header = TGDB_REQUEST_DISASSEMBLE_PC;

    request_ptr->choice.disassemble.lines = lines;

    tgdb_run_or_queue_request(tgdb, request_ptr, false);
}

void tgdb_request_disassemble_func(struct tgdb *tgdb,
        enum disassemble_func_type type)
{
    tgdb_request_ptr request_ptr;

    request_ptr = (tgdb_request_ptr)cgdb_malloc(sizeof (struct tgdb_request));
    request_ptr->header = TGDB_REQUEST_DISASSEMBLE_FUNC;

    request_ptr->choice.disassemble_func.raw = (type == DISASSEMBLE_FUNC_RAW_INSTRUCTIONS);
    request_ptr->choice.disassemble_func.source = (type == DISASSEMBLE_FUNC_SOURCE_LINES);

    tgdb_run_or_queue_request(tgdb, request_ptr, false);
}

/* }}}*/

/* Process {{{*/

int tgdb_get_gdb_command(struct tgdb *tgdb, tgdb_request_ptr request,
        std::string &command)
{
    char *str;

    if (!tgdb || !request) {
        return -1;
    }

    switch (request->header) {
        case TGDB_REQUEST_CONSOLE_COMMAND:
            command = request->choice.console_command.command;
            break;
        case TGDB_REQUEST_INFO_SOURCES:
            command = "server interpreter-exec mi"
                        " \"-file-list-exec-source-files\"\n";
            break;
        case TGDB_REQUEST_INFO_SOURCE_FILE:
            command = "server interpreter-exec mi"
                    " \"-file-list-exec-source-file\"\n";
            break;
        case TGDB_REQUEST_BREAKPOINTS:
            command = "server interpreter-exec mi"
                    " \"-break-info\"\n";
            break;
        case TGDB_REQUEST_TTY:
            str = sys_aprintf("server interpreter-exec mi"
                    " \"-inferior-tty-set %s\"\n",
                    request->choice.tty_command.slavename);
            command = str;
            free(str);
            str = NULL;
            break;
        case TGDB_REQUEST_INFO_FRAME:
            command = "server interpreter-exec mi"
                    " \"-stack-info-frame\"\n";
            break;
        case TGDB_REQUEST_DATA_DISASSEMBLE_MODE_QUERY:
            command = "server interpreter-exec mi"
                    " \"-data-disassemble -s 0 -e 0 -- 4\"\n";
            break;
        case TGDB_REQUEST_DEBUGGER_COMMAND:
            command = tgdb_get_client_command(tgdb,
                    request->choice.debugger_command.c);
            break;
        case TGDB_REQUEST_MODIFY_BREAKPOINT:
            command = tgdb_client_modify_breakpoint_call(tgdb,
                    request->choice.modify_breakpoint.file,
                    request->choice.modify_breakpoint.line,
                    request->choice.modify_breakpoint.addr,
                    request->choice.modify_breakpoint.b);
            break;
        case TGDB_REQUEST_COMPLETE:
            str = sys_aprintf("server interpreter-exec mi"
                    " \"complete %s\"\n", request->choice.complete.line);
            command = str;
            free(str);
            str = NULL;
            break;
        case TGDB_REQUEST_DISASSEMBLE_PC:
            str = sys_aprintf("server interpreter-exec mi"
                    " \"x/%di $pc\"\n", request->choice.disassemble.lines);
            command = str;
            free(str);
            str = NULL;
            break;
        case TGDB_REQUEST_DISASSEMBLE_FUNC: {
            /* GDB 7.11 adds /s command to disassemble

            https://sourceware.org/git/gitweb.cgi?p=binutils-gdb.git;a=commit;h=6ff0ba5f7b8a2b10642bbb233a32043595c55670
                The "source centric" /m option to the disassemble command is
                often unhelpful, e.g., in the presence of optimized code.
                This patch adds a /s modifier that is better.
                For one, /m only prints instructions from the originating
                source file, leaving out instructions from
                e.g., inlined functions from other files.

            disassemble
                 /m: source lines included
                 /s: source lines included, output in pc order (7.10 and higher)
                 /r: raw instructions included in hex
                 single argument: function surrounding is dumped
                 two arguments: start,end or start,+length
                 disassemble 'driver.cpp'::main
                 interp mi "disassemble /s 'driver.cpp'::main,+10"
                 interp mi "disassemble /r 'driver.cpp'::main,+10"
             */
            const char *data = NULL;
            if (request->choice.disassemble_func.raw) {
                data = "/r";
            } else if (request->choice.disassemble_func.source &&
                    commands_disassemble_supports_s_mode(tgdb->c)) {
                data = "/s";
            }
            str = sys_aprintf("server interpreter-exec mi"
                    " \"disassemble%s%s\"\n",
                    data ? " " : "", data ? data : "");
            command = str;
            free(str);
            str = NULL;
            break;
        }
    }

    return 0;
}

/* }}}*/

/* }}} */

/* TGDB Queue commands {{{*/

tgdb_request_ptr tgdb_queue_pop(struct tgdb * tgdb)
{
    tgdb_request_ptr front = tgdb->command_requests->front();
    tgdb->command_requests->pop_front();
    return front;
}

int tgdb_queue_size(struct tgdb *tgdb)
{
    return tgdb->command_requests->size();
}

/* }}}*/

/* Signal Handling Support {{{*/

int tgdb_signal_notification(struct tgdb *tgdb, int signum)
{
    struct termios t;
    cc_t *sig_char = NULL;

    tcgetattr(tgdb->debugger_stdin, &t);

    if (signum == SIGINT) {     /* ^c */
        tgdb->control_c = 1;
        sig_char = &t.c_cc[VINTR];
        if (write(tgdb->debugger_stdin, sig_char, 1) < 1)
            return -1;
    } else if (signum == SIGQUIT) { /* ^\ */
        sig_char = &t.c_cc[VQUIT];
        if (write(tgdb->debugger_stdin, sig_char, 1) < 1)
            return -1;
    } else if (signum == SIGCHLD) {
        tgdb->has_sigchld_recv = 1;
    }

    return 0;
}

/* }}}*/
