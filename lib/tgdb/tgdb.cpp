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

struct tgdb_request *last_request = NULL;

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

  /***************************************************************************
   * All the queue's the clients can run commands through
   * The different queue's can be slightly confusing.
   **************************************************************************/

    /**
     * The commands that need to be run through GDB.
     *
     * This is a buffered queue that represents all of the commands that TGDB
     * needs to execute. These commands will be executed one at a time.
     * That is, a command will be issued, and TGDB will wait for the entire
     * response before issuing any more commands.
     *
     * While a command is executing, tgdb can add commands to the 
     * oob_command_queue. If this happens TGDB will execute all of the
     * commands in the oob_command_queue before executing the next
     * command in this queue. 
     */
    struct tgdb_command **gdb_input_queue;

    /** 
     * The priority queue.
     *
     * This has commands that tgdb requires to run, in order to provide
     * the front end with the proper resopnses. When the front end runs
     * a command, sometimes tgdb will discover that it needs to run other
     * commands in order to satisfy the functionality requested by the GUI.
     * 
     *
     * These commands should *always* be run first.
     */
    struct tgdb_command **priority_queue;


    /** 
     * The commands that the front end has requested to run.
     *
     * TGDB buffers all of the commands that the front end wants to run.
     * That is, if the front end is sending commands faster than TGDB can
     * pass the command to GDB and get a response, then the commands are
     * buffered until it is there turn to run. These commands are run
     * in the order that they are received.
     *
     * This is here as a convenience to the front end. TGDB currently does
     * not access these commands. It provides the push/pop functionality
     * and it erases the queue when a control_c is received.
     */
    tgdb_request_ptr *gdb_client_request_queue;

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

    /** The config directory that this context can write too. */
    char config_dir[FSUTIL_PATH_MAX];

    /** The init file for the debugger. */
    char gdb_init_file[FSUTIL_PATH_MAX];

};

/* }}} */

/* Temporary prototypes {{{ */
static void tgdb_deliver_command(struct tgdb *tgdb,
        struct tgdb_command *command);
static void tgdb_unqueue_and_deliver_command(struct tgdb *tgdb);
void tgdb_run_or_queue_command(struct tgdb *tgdb,
        struct tgdb_command *com);

/* }}} */

/* These functions are used to determine the state of libtgdb */

/**
 * Determines if tgdb should send data to gdb or put it in a buffer. This
 * is when the debugger is ready and there are no commands to run.
 *
 * \return
 * 1 if can issue directly to gdb. Otherwise 0.
 */
static int tgdb_can_issue_command(struct tgdb *tgdb)
{
    if (tgdb->is_gdb_ready_for_next_command &&
        annotations_parser_at_prompt(tgdb->parser) &&
        (sbcount(tgdb->gdb_input_queue) == 0))
        return 1;

    return 0;
}

/**
 * Determines if tgdb has commands it needs to run.
 *
 * \return
 * 1 if can issue directly to gdb. Otherwise 0.
 */
static int tgdb_has_command_to_run(struct tgdb *tgdb)
{
    if (annotations_parser_at_prompt(tgdb->parser) &&
        ((sbcount(tgdb->gdb_input_queue) > 0) ||
            (sbcount(tgdb->priority_queue) > 0)))
        return 1;

    return 0;
}

/**
 * If the TGDB instance is not busy, it will run the requested command.
 * Otherwise, the command will get queued to run later.
 *
 * \param tgdb_in
 * An instance of the tgdb library to operate on.
 *
 * \param request
 * The requested command to have TGDB process.
 *
 * \return
 * 0 on success or -1 on error
 */
static void handle_request(struct tgdb *tgdb_in, struct tgdb_request *request)
{
    if (tgdb_can_issue_command(tgdb_in)) {
        tgdb_process_command(tgdb_in, request);
    } else {
        sbpush(tgdb_in->gdb_client_request_queue, request);
    }
}

struct tgdb_request *tgdb_get_last_request()
{
    return last_request;
}

void tgdb_set_last_request(struct tgdb_request *request)
{
    tgdb_request_destroy(last_request);
    last_request = request;
}

/**
 * This function looks at the request that CGDB has made and determines if it
 * effects the GDB console window. For instance, if the request makes output go
 * to that window, then the user would like to see another prompt there when the
 * command finishes. However, if the command is 'o', to get all the sources and
 * display them, then this doesn't effect the GDB console window and the window
 * should not redisplay the prompt.
 *
 * \param request
 * The request to analysize
 *
 * \param update
 * Will return as 1 if the console window should be updated, or 0 otherwise
 *
 * \return
 * 0 on success or -1 on error
 */
int
tgdb_does_request_require_console_update(struct tgdb_request *request)
{
    switch (request->header) {
        case TGDB_REQUEST_CONSOLE_COMMAND:
        case TGDB_REQUEST_DEBUGGER_COMMAND:
        case TGDB_REQUEST_MODIFY_BREAKPOINT:
        case TGDB_REQUEST_COMPLETE:
            return 1;
        case TGDB_REQUEST_INFO_SOURCES:
        case TGDB_REQUEST_CURRENT_LOCATION:
        case TGDB_REQUEST_DISASSEMBLE_FUNC:
        default:
            return 0;
    }
}

static struct tgdb *initialize_tgdb_context(void)
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

    tgdb->gdb_input_queue = NULL;
    tgdb->priority_queue = NULL;
    tgdb->gdb_client_request_queue = NULL;

    tgdb->is_gdb_ready_for_next_command = 1;

    tgdb->has_sigchld_recv = 0;

    tgdb->config_dir[0] = '\0';
    tgdb->gdb_init_file[0] = '\0';

    return tgdb;
}

/*******************************************************************************
 * This is the basic initialization
 ******************************************************************************/

/* 
 * Gets the users home dir and creates the config directory.
 *
 * \param tgdb
 * The tgdb context.
 *
 * \param config_dir 
 * Should be FSUTIL_PATH_MAX in size on way in.
 * On way out, it will be the path to the config dir
 *
 * \return
 * -1 on error, or 0 on success
 */
static int tgdb_initialize_config_dir(struct tgdb *tgdb,
    char *config_dir, char *logs_dir)
{
    char *home_dir = getenv("HOME");

    /* Make sure the toplevel .cgdb dir exists */
    snprintf(config_dir, FSUTIL_PATH_MAX, "%s/.cgdb", home_dir);
    fs_util_create_dir(config_dir);

    /* Try to create full .cgdb/logs directory */
    snprintf(logs_dir, FSUTIL_PATH_MAX, "%s/.cgdb/logs", home_dir);
    if (!fs_util_create_dir(logs_dir)) {
        clog_error(CLOG_CGDB, "fs_util_create_dir_in_base error");
        return -1;
    }

    return 0;
}

/**
 * Creates a config file for the user.
 *
 *  Pre: The directory already has read/write permissions. This should have
 *       been checked by tgdb-base.
 *
 *  Return: 1 on success or 0 on error
 */
static int tgdb_setup_config_file(struct tgdb *tgdb, const char *dir)
{
    FILE *fp;

    strncpy(tgdb->config_dir, dir, strlen(dir) + 1);

    fs_util_get_path(dir, "gdb_init_commands", tgdb->gdb_init_file);

    if ((fp = fopen(tgdb->gdb_init_file, "w"))) {
        fprintf(fp, "set annotate 2\n"
                    "set height 0\n");
        fclose(fp);
    } else {
        clog_error(CLOG_CGDB, "fopen error '%s'", tgdb->gdb_init_file);
        return 0;
    }

    return 1;
}

/**
 * Knowing the user's home directory, TGDB can initialize the logger interface
 *
 * \param tgdb
 * The tgdb context.
 *
 * \param config_dir 
 * The path to the user's config directory
 *
 * \return
 * -1 on error, or 0 on success
 */
static int tgdb_initialize_logger_interface(struct tgdb *tgdb, char *config_dir)
{
    /* Open our cgdb and tgdb io logfiles */
    clog_open(CLOG_CGDB_ID, "%s/cgdb_log%d.txt", config_dir);
    clog_open(CLOG_GDBIO_ID, "%s/cgdb_gdb_io_log%d.txt", config_dir);

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

static int tgdb_get_current_location(struct tgdb *tgdb)
{
    return commands_issue_command(tgdb->c, COMMAND_INFO_FRAME, NULL, 1);
}


static void tgdb_source_location_changed(void *context)
{
    struct tgdb *tgdb = (struct tgdb*)context;
    tgdb_get_current_location(tgdb);
}

static void tgdb_prompt_changed(void *context, const std::string &prompt)
{
    struct tgdb *tgdb = (struct tgdb*)context;

    struct tgdb_response *response =
        tgdb_create_response(TGDB_UPDATE_CONSOLE_PROMPT_VALUE);
    response->choice.update_console_prompt_value.prompt_value =
            cgdb_strdup(prompt.c_str());
    commands_add_response(tgdb->c, response);
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

    commands_issue_command(tgdb->c, COMMAND_TTY,
        pty_pair_get_slavename(tgdb->pty_pair), 1);

    return 0;
}

/* Creating and Destroying a libtgdb context. {{{*/

struct tgdb *tgdb_initialize(const char *debugger,
        int argc, char **argv, int *debugger_fd)
{
    /* Initialize the libtgdb context */
    struct tgdb *tgdb = initialize_tgdb_context();
    static struct annotations_parser_callbacks callbacks = {
        tgdb,
        tgdb_source_location_changed,
        tgdb_prompt_changed
    };
    char config_dir[FSUTIL_PATH_MAX];
    char logs_dir[FSUTIL_PATH_MAX];

    /* Create config directory */
    if (tgdb_initialize_config_dir(tgdb, config_dir, logs_dir) == -1) {
        clog_error(CLOG_CGDB, "tgdb_initialize error");
        return NULL;
    }

    if (tgdb_initialize_logger_interface(tgdb, logs_dir) == -1) {
        printf("Could not initialize logger interface\n");
        return NULL;
    }

    if (!tgdb_setup_config_file(tgdb, config_dir)) {
        clog_error(CLOG_CGDB,  "error setting up config file");
        return NULL;
    }

    tgdb->debugger_pid = invoke_debugger(debugger, argc, argv,
            &tgdb->debugger_stdin, &tgdb->debugger_stdout,
            0, tgdb->gdb_init_file);

    /* Couldn't invoke process */
    if (tgdb->debugger_pid == -1)
        return NULL;

    tgdb->c = commands_initialize(tgdb);

    tgdb->parser = annotations_parser_initialize(callbacks);

    tgdb_open_new_tty(tgdb, &tgdb->inferior_stdin, &tgdb->inferior_stdout);

    /* Need to get source information before breakpoint information otherwise
     * the TGDB_UPDATE_BREAKPOINTS event will be ignored in process_commands()
     * because there are no source files to add the breakpoints to.
     */
    tgdb_get_current_location(tgdb);

    /* gdb may already have some breakpoints when it starts. This could happen
     * if the user puts breakpoints in there .gdbinit.
     * This makes sure that TGDB asks for the breakpoints on start up.
     */
    if (commands_issue_command(tgdb->c, COMMAND_BREAKPOINTS, NULL, 0) == -1) {
        return NULL;
    }

    /**
     * Query if disassemble supports the /s flag
     */
    if (commands_issue_command(tgdb->c,
            COMMAND_DATA_DISASSEMBLE_MODE_QUERY, NULL, 1) == -1) {
        return NULL;
    }

    *debugger_fd = tgdb->debugger_stdout;

    return tgdb;
}

int tgdb_shutdown(struct tgdb *tgdb)
{
    int i;
    for (i = 0; i < sbcount(tgdb->gdb_input_queue); i++) {
        struct tgdb_command *tc = tgdb->gdb_input_queue[i];
        tgdb_command_destroy(tc);
    }
    sbfree(tgdb->gdb_input_queue);

    for (i = 0; i < sbcount(tgdb->gdb_client_request_queue); i++) {
        tgdb_request_ptr tr = tgdb->gdb_client_request_queue[i];
        tgdb_request_destroy(tr);
    }
    sbfree(tgdb->gdb_client_request_queue);

    for (i = 0; i < sbcount(tgdb->priority_queue); i++) {
        struct tgdb_command *tc = tgdb->priority_queue[i];
        tgdb_command_destroy(tc);
    }
    sbfree(tgdb->priority_queue);

    annotations_parser_shutdown(tgdb->parser);

    cgdb_close(tgdb->debugger_stdin);
    tgdb->debugger_stdin = -1;

    commands_delete_responses(tgdb->c);

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

static int tgdb_disassemble_pc(struct commands *c, int lines)
{
    int ret;
    char *data = NULL;

    data = lines ? sys_aprintf("%d", lines) : NULL;
    ret = commands_issue_command(c, COMMAND_DISASSEMBLE_PC, data, 0);

    free(data);
    return ret;
}

static int tgdb_disassemble_func(struct commands *c, int raw, int source)
{
    /* GDB 7.11 adds /s command to disassemble

    https://sourceware.org/git/gitweb.cgi?p=binutils-gdb.git;a=commit;h=6ff0ba5f7b8a2b10642bbb233a32043595c55670
        The "source centric" /m option to the disassemble command is often
        unhelpful, e.g., in the presence of optimized code.
        This patch adds a /s modifier that is better.
        For one, /m only prints instructions from the originating source file,
        leaving out instructions from e.g., inlined functions from other files.

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
    int ret;
    char *data = NULL;

    if (raw) {
        data = sys_aprintf("%s", "/r");
    } else if (source && commands_disassemble_supports_s_mode(c)) {
        data = sys_aprintf("%s", "/s");
    }

    ret = commands_issue_command(c, COMMAND_DISASSEMBLE_FUNC, data, 0);

    free(data);
    return ret;
}

void tgdb_request_destroy(tgdb_request_ptr request_ptr)
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

static void tgdb_request_destroy_func(void *item)
{
    tgdb_request_destroy((tgdb_request_ptr) item);
}

/* tgdb_handle_signals
 */
static int tgdb_handle_signals(struct tgdb *tgdb)
{
    if (tgdb->control_c) {
        sbsetcount(tgdb->gdb_input_queue, 0);
        sbsetcount(tgdb->gdb_client_request_queue, 0);
        tgdb->control_c = 0;
    }

    return 0;
}

/*******************************************************************************
 * This is the main_loop stuff for tgdb-base
 ******************************************************************************/

/* 
 * Sends a command to the debugger. This function gets called when the GUI
 * wants to run a command.
 *
 * \param tgdb
 * An instance of the tgdb library to operate on.
 *
 * \param command
 * This is the command that should be sent to the debugger.
 *
 * \param command_choice
 * This tells tgdb_send who is sending the command.
 */
static void
tgdb_send(struct tgdb *tgdb, const char *command,
        enum tgdb_command_choice command_choice)
{
    struct tgdb_command *tc;
    char *temp_command = NULL;
    int length = strlen(command);

    /* Add a newline to the end of the command if it doesn't exist */
    if (!length || (command[length - 1] != '\n')) {
        temp_command = sys_aprintf("%s\n", command);
        command = temp_command;
    }

    /* Create the client command */
    tc = tgdb_command_create(command, command_choice, COMMAND_USER_COMMAND);

    free(temp_command);
    temp_command = NULL;

    tgdb_run_or_queue_command(tgdb, tc);
    commands_user_ran_command(tgdb->c);
}

/**
 * If TGDB is ready to process another command, then this command will be
 * sent to the debugger. However, if TGDB is not ready to process another command,
 * then the command will be queued and run when TGDB is ready.
 *
 * \param tgdb
 * The TGDB context to use.
 *
 * \param command
 * The command to run or queue.
 *
 * \return
 * 0 on success or -1 on error
 */
void tgdb_run_or_queue_command(struct tgdb *tgdb, struct tgdb_command *command)
{
    int can_issue = tgdb_can_issue_command(tgdb);

    if (can_issue) {
        tgdb_deliver_command(tgdb, command);
        tgdb_command_destroy(command);
    } else {
        /* Make sure to put the command into the correct queue. */
        switch (command->command_choice) {
            case TGDB_COMMAND_FRONT_END:
            case TGDB_COMMAND_TGDB_CLIENT:
                sbpush(tgdb->gdb_input_queue, command);
                break;
            case TGDB_COMMAND_TGDB_CLIENT_PRIORITY:
                sbpush(tgdb->priority_queue, command);
                break;
            case TGDB_COMMAND_CONSOLE:
            default:
                clog_error(CLOG_CGDB, "unimplemented command");
                break;
        }
    }
}

/**
 * Will send a command to the debugger immediately. No queueing will be done
 * at this point.
 *
 * \param tgdb
 * The TGDB context to use.
 *
 * \param command 
 * The command to run.
 *
 *  NOTE: This function assummes valid commands are being sent to it. 
 *        Error checking should be done before inserting into queue.
 */
static void tgdb_deliver_command(struct tgdb *tgdb, struct tgdb_command *command)
{
    tgdb->is_gdb_ready_for_next_command = 0;

    /* Send what we're doing to log file */
    char *str = sys_quote_nonprintables(command->gdb_command, -1);
    clog_debug(CLOG_GDBIO, "%s", str);
    sbfree(str);

    /* A command for the debugger */
    commands_prepare_for_command(tgdb->c, command);

    io_writen(tgdb->debugger_stdin, command->gdb_command,
            strlen(command->gdb_command));

    if (command->command_choice != TGDB_COMMAND_CONSOLE) {
        char *s = command->gdb_command;
        struct tgdb_response *response = (struct tgdb_response *)
                cgdb_malloc(sizeof (struct tgdb_response));
        response->header = TGDB_DEBUGGER_COMMAND_DELIVERED;
        response->choice.debugger_command_delivered.debugger_command =
            (command->command_choice == TGDB_COMMAND_FRONT_END)?1:0;
        response->choice.debugger_command_delivered.command = cgdb_strdup(s);
        commands_add_response(tgdb->c, response);
    }
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
  tgdb_unqueue_and_deliver_command_tag:

    /* This will redisplay the prompt when a command is run
     * through the gui with data on the console.
     */

    /* The out of band commands should always be run first */
    if (sbcount(tgdb->priority_queue) > 0) {
        /* These commands are always run. 
         * However, if an assumption is made that a misc
         * prompt can never be set while in this spot.
         */
        struct tgdb_command *item = NULL;

        item = sbpopfront(tgdb->priority_queue);
        tgdb_deliver_command(tgdb, item);
        tgdb_command_destroy(item);
    }
    /* If the queue is not empty, run a command */
    else if (sbcount(tgdb->gdb_input_queue) > 0) {
        struct tgdb_command *item;

        item = sbpopfront(tgdb->gdb_input_queue);

        if (annotations_parser_at_miscellaneous_prompt(tgdb->parser)) {
            /* If at the misc prompt, don't run the internal tgdb commands,
             * In fact throw them out for now, since they are only 
             * 'info breakpoints' */
            if (item->command_choice != TGDB_COMMAND_CONSOLE) {
                tgdb_command_destroy(item);
                goto tgdb_unqueue_and_deliver_command_tag;
            }
        }

        tgdb_deliver_command(tgdb, item);
        tgdb_command_destroy(item);
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

/* returns to the caller data from the child */
/**
 * Returns output that the debugged program printed (the inferior).
 *
 * @param tgdb
 * The tgdb instance to act on.
 *
 * @param buf
 * The buffer to write the inferior data to.
 *
 * @param n
 * The number of bytes that buf can contain.
 *
 * @return
 * 0 on EOR, -1 on error, or the number of bytes written to buf.
 */
ssize_t tgdb_recv_inferior_data(struct tgdb * tgdb, char *buf, size_t n)
{
    char local_buf[n + 1];
    ssize_t size;

    /* read all the data possible from the child that is ready. */
    if ((size = io_read(tgdb->inferior_stdin, local_buf, n)) < 0) {
        clog_error(CLOG_CGDB, "inferior_fd read failed");
        return -1;
    }

    strncpy(buf, local_buf, size);
    buf[size] = '\0';

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
    response->choice.quit.exit_status = -1;
    response->choice.quit.return_value = 0;
    commands_add_response(tgdb->c, response);
    return 0;
}

/**
 * This is called when GDB has finished.
 * Its job is to add the type of QUIT command that is appropriate.
 *
 * \param tgdb
 * The tgdb context
 *
 * \param tgdb_will_quit
 * This will return as 1 if tgdb sent the TGDB_QUIT command. Otherwise 0.
 * 
 * \return
 * 0 on success or -1 on error
 */
static int tgdb_get_quit_command(struct tgdb *tgdb, int *tgdb_will_quit)
{
    int status = 0;
    pid_t ret;
    struct tgdb_response *response = tgdb_create_response(TGDB_QUIT);

    if (!tgdb_will_quit)
        return -1;

    *tgdb_will_quit = 0;

    ret = waitpid(tgdb->debugger_pid, &status, WNOHANG);

    if (ret == -1) {
        clog_error(CLOG_CGDB, "waitpid error");
        return -1;
    } else if (ret == 0) {
        /* This SIGCHLD wasn't for GDB */
        return 0;
    }

    if ((WIFEXITED(status)) == 0) {
        /* Child did not exit normally */
        response->choice.quit.exit_status = -1;
        response->choice.quit.return_value = 0;

    } else {
        response->choice.quit.exit_status = 0;
        response->choice.quit.return_value = WEXITSTATUS(status);

    }

    *tgdb_will_quit = 1;
    return 0;
}

size_t tgdb_process(struct tgdb * tgdb, char *buf, size_t n, int *is_finished)
{
    char local_buf[10 * n];
    ssize_t size;
    size_t buf_size = 0;
    std::string result;

    /* TODO: This is kind of a hack.
     * Since I know that I didn't do a read yet, the next select loop will
     * get me back here. This probably shouldn't return, however, I have to
     * re-write a lot of this function. Also, I think this function should
     * return a malloc'd string, not a static buffer.
     *
     * Currently, I see it as a bigger hack to try to just append this to the
     * beginning of buf.
     */
    if (tgdb->has_sigchld_recv) {
        int tgdb_will_quit;

        /* tgdb_get_quit_command will return right away, it's asynchrounous.
         * We call it to determine if it was GDB that died.
         * If GDB didn't die, things will work like normal. ignore this.
         * If GDB did die, this get's the quit command and add's it to the list. It's
         * OK that the rest of this function get's executed, since the read will simply
         * return EOF.
         */
        int val = tgdb_get_quit_command(tgdb, &tgdb_will_quit);

        if (val == -1) {
            clog_error(CLOG_CGDB, "tgdb_get_quit_command error");
            return -1;
        }

        tgdb->has_sigchld_recv = 0;
        if (tgdb_will_quit)
            goto tgdb_finish;
    }

    /* set buf to null for debug reasons */
    memset(buf, '\0', n);

    /* 1. read all the data possible from gdb that is ready. */
    if ((size = io_read(tgdb->debugger_stdout, local_buf, n)) < 0) {
        clog_error(CLOG_CGDB, "could not read from masterfd");
        buf_size = -1;
        tgdb_add_quit_command(tgdb);
        goto tgdb_finish;
    } else if (size == 0) {     /* EOF */
        tgdb_add_quit_command(tgdb);
        goto tgdb_finish;
    }

    /* 2. At this point local_buf has everything new from this read.
     * Basically this function is responsible for separating the annotations
     * that gdb writes from the data. 
     *
     * buf and buf_size are the data to be returned from the user.
     */

    local_buf[size] = '\0';

    result = annotations_parser_io(tgdb->parser, local_buf, size);

    if (commands_is_console_command(tgdb->c)) {
        strncpy(buf, result.c_str(), result.size());
        buf_size = result.size();
    } else {
        commands_process(tgdb->c, result);
    }

    if (annotations_parser_at_prompt(tgdb->parser)) {
        /* success, and finished command */
        tgdb->is_gdb_ready_for_next_command = 1;
    }

    /* 3. if ^c has been sent, clear the buffers.
     *        If a signal has been received, clear the queue and return
     */
    if (tgdb_handle_signals(tgdb) == -1) {
        clog_error(CLOG_CGDB, "tgdb_handle_signals failed");
        return -1;
    }

    /* 4. runs the users buffered command if any exists */
    if (tgdb_has_command_to_run(tgdb))
        tgdb_unqueue_and_deliver_command(tgdb);

tgdb_finish:
    *is_finished = tgdb_can_issue_command(tgdb);

    return buf_size;
}

/* Getting Data out of TGDB {{{*/

struct tgdb_response *tgdb_get_response(struct tgdb *tgdb, int index)
{
    return commands_get_response(tgdb->c, index);
}

struct tgdb_response *tgdb_create_response(enum tgdb_response_type header)
{
    struct tgdb_response *response;

    response = (struct tgdb_response *)cgdb_calloc(1, sizeof(struct tgdb_response));
    response->header = header;

    return response;
}

int tgdb_delete_response(struct tgdb_response *com)
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
        case TGDB_DEBUGGER_COMMAND_DELIVERED: {
            const char *value =
                com->choice.debugger_command_delivered.command;
            free((char*)value);
            break;
        }
        case TGDB_QUIT:
            break;
    }

    free(com);
    com = NULL;
    return 0;
}

void tgdb_delete_responses(struct tgdb *tgdb)
{
    commands_delete_responses(tgdb->c);
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

    handle_request(tgdb, request_ptr);
}

void tgdb_request_inferiors_source_files(struct tgdb * tgdb)
{
    tgdb_request_ptr request_ptr;

    request_ptr = (tgdb_request_ptr)cgdb_malloc(sizeof (struct tgdb_request));

    request_ptr->header = TGDB_REQUEST_INFO_SOURCES;

    handle_request(tgdb, request_ptr);
}

void tgdb_request_current_location(struct tgdb * tgdb)
{
    tgdb_request_ptr request_ptr;

    request_ptr = (tgdb_request_ptr)cgdb_malloc(sizeof (struct tgdb_request));

    request_ptr->header = TGDB_REQUEST_CURRENT_LOCATION;

    handle_request(tgdb, request_ptr);
}

void
tgdb_request_run_debugger_command(struct tgdb * tgdb, enum tgdb_command_type c)
{
    tgdb_request_ptr request_ptr;

    request_ptr = (tgdb_request_ptr)cgdb_malloc(sizeof (struct tgdb_request));

    request_ptr->header = TGDB_REQUEST_DEBUGGER_COMMAND;
    request_ptr->choice.debugger_command.c = c;

    handle_request(tgdb, request_ptr);
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

    handle_request(tgdb, request_ptr);
}

void tgdb_request_complete(struct tgdb * tgdb, const char *line)
{
    tgdb_request_ptr request_ptr;

    request_ptr = (tgdb_request_ptr)cgdb_malloc(sizeof (struct tgdb_request));
    request_ptr->header = TGDB_REQUEST_COMPLETE;
    request_ptr->choice.complete.line = (const char *)cgdb_strdup(line);

    handle_request(tgdb, request_ptr);
}

void tgdb_request_disassemble_pc(struct tgdb *tgdb, int lines)
{
    tgdb_request_ptr request_ptr;

    request_ptr = (tgdb_request_ptr)cgdb_malloc(sizeof (struct tgdb_request));
    request_ptr->header = TGDB_REQUEST_DISASSEMBLE_PC;

    request_ptr->choice.disassemble.lines = lines;

    handle_request(tgdb, request_ptr);
}

void tgdb_request_disassemble_func(struct tgdb *tgdb,
        enum disassemble_func_type type)
{
    tgdb_request_ptr request_ptr;

    request_ptr = (tgdb_request_ptr)cgdb_malloc(sizeof (struct tgdb_request));
    request_ptr->header = TGDB_REQUEST_DISASSEMBLE_FUNC;

    request_ptr->choice.disassemble_func.raw = (type == DISASSEMBLE_FUNC_RAW_INSTRUCTIONS);
    request_ptr->choice.disassemble_func.source = (type == DISASSEMBLE_FUNC_SOURCE_LINES);

    handle_request(tgdb, request_ptr);
}

/* }}}*/

/* Process {{{*/

int tgdb_process_command(struct tgdb *tgdb, tgdb_request_ptr request)
{
    if (!tgdb || !request)
        return -1;

    if (!tgdb_can_issue_command(tgdb))
        return -1;

    tgdb_set_last_request(request);

    if (request->header == TGDB_REQUEST_CONSOLE_COMMAND) {
        tgdb_send(tgdb, request->choice.console_command.command,
            TGDB_COMMAND_CONSOLE);
    } else if (request->header == TGDB_REQUEST_DEBUGGER_COMMAND) {
        tgdb_send(tgdb, tgdb_get_client_command(tgdb,
            request->choice.debugger_command.c), TGDB_COMMAND_FRONT_END);
    } else if (request->header == TGDB_REQUEST_MODIFY_BREAKPOINT) {
        char *val = tgdb_client_modify_breakpoint_call(tgdb,
            request->choice.modify_breakpoint.file,
            request->choice.modify_breakpoint.line,
            request->choice.modify_breakpoint.addr,
            request->choice.modify_breakpoint.b);
        if (val)
        {
            tgdb_send(tgdb, val, TGDB_COMMAND_FRONT_END);
            free(val);
        }
    } else {
        if (request->header == TGDB_REQUEST_INFO_SOURCES) {
            commands_issue_command(tgdb->c, COMMAND_INFO_SOURCES, NULL, 0);
        }
        else if (request->header == TGDB_REQUEST_CURRENT_LOCATION) {
            commands_issue_command(tgdb->c, COMMAND_INFO_FRAME, NULL, 0);
        }
        else if (request->header == TGDB_REQUEST_COMPLETE) {
            commands_issue_command(tgdb->c, COMMAND_COMPLETE,
                request->choice.complete.line, 1);
        }
        else if (request->header == TGDB_REQUEST_DISASSEMBLE_PC) {
            tgdb_disassemble_pc(tgdb->c, request->choice.disassemble.lines);
        }
        else if (request->header == TGDB_REQUEST_DISASSEMBLE_FUNC) {
            tgdb_disassemble_func(tgdb->c,
                  request->choice.disassemble_func.raw,
                  request->choice.disassemble_func.source);
        }
    }

    return 0;
}

/* }}}*/

/* }}} */

/* TGDB Queue commands {{{*/

tgdb_request_ptr tgdb_queue_pop(struct tgdb * tgdb)
{
    return sbpopfront(tgdb->gdb_client_request_queue);
}

int tgdb_queue_size(struct tgdb *tgdb)
{
    return sbcount(tgdb->gdb_client_request_queue);
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
