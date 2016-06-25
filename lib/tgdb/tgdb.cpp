/* Includes {{{ */
#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif /* HAVE_STDLIB_H */

#if HAVE_SIGNAL_H
#include <signal.h>             /* sig_atomic_t */
#endif /* HAVE_SIGNAL_H */

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#if HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#include <inttypes.h>

#include "tgdb.h"
#include "commands.h"
#include "fs_util.h"
#include "ibuf.h"
#include "io.h"
#include "queue.h"
#include "a2-tgdb.h"
#include "annotate_two.h"
#include "state_machine.h"

#include "pseudo.h"             /* SLAVE_SIZE constant */
#include "fork_util.h"
#include "sys_util.h"
#include "tgdb_list.h"
#include "logger.h"

/* }}} */

static int num_loggers = 0;

struct tgdb_request *last_request = NULL;

/* struct tgdb {{{ */

/**
 * The TGDB context data structure.
 */
struct tgdb {

  /** A client context to abstract the debugger.  */
    struct annotate_two *tcc;

  /** Reading from this will read from the debugger's output */
    int debugger_stdout;

  /** Writing to this will write to the debugger's stdin */
    int debugger_stdin;

  /** Reading from this will read the stdout from the program being debugged */
    int inferior_stdout;

  /** Writing to this will write to the stdin of the program being debugged */
    int inferior_stdin;

  /***************************************************************************
   * All the queue's the clients can run commands through
   * The different queue's can be slightly confusing.
   **************************************************************************/

  /**
   * The commands that need to be run through GDB.
   *
   * This is a buffered queue that represents all of the commands that TGDB
   * needs to execute. These commands will be executed one at a time. That is,
   * 1 command will be issued, and TGDB will wait for the entire response before
   * issuing any more commands. It is even possible that while this command is
   * executing, that the client context will add commands to the 
   * oob_command_queue. If this happens TGDB will execute all of the commands
   * in the oob_command_queue before executing the next command in this queue. 
   */
    struct queue *gdb_input_queue;

  /** 
   * The commands that the client has requested to run.
   *
   * TGDB buffers all of the commands that the FE wants to run. That is,
   * if the FE is sending commands faster than TGDB can pass the command to
   * GDB and get a response, then the commands are buffered until it is there
   * turn to run. These commands are run in the order that they are received.
   *
   * This is here as a convenience to the FE. TGDB currently does not access 
   * these commands. It provides the push/pop functionality and it erases the
   * queue when a control_c is received.
   */
    struct queue *gdb_client_request_queue;

  /** 
   * The out of band input queue. 
   *
   * These commands are used by the client context. When a single client context 
   * command is run, sometimes it will discover that it needs to run other commands
   * in order to satisfy the functionality requested by the GUI. For instance, a 
   * single GUI function request could take N client context commands to generate
   * a response. Also, to make things worse, sometimes the client context doesn't 
   * know if it will take 1 or N commands to satisfy the request until after it has 
   * sent and received the information from the first command. Thus, while the 
   * client context is processing the first command, it may add more commands to 
   * this queue to specify that these commands need to be run before any other 
   * commands are sent to GDB.
   *
   * These commands should *always* be run first.
   */
    struct queue *oob_input_queue;

  /** These are 2 very important state variables.  */

  /**
   * If set to 1, libtgdb thinks the lower level subsystem is capable of 
   * receiving another command. It needs this so that it doesn't send 2
   * commands to the lower level before it can say it can't receive a command.
   * At some point, maybe this can be removed?
   * When its set to 0, libtgdb thinks it can not send the lower level another
   * command.  */
    int IS_SUBSYSTEM_READY_FOR_NEXT_COMMAND;

  /** If ^c was hit by user */
    sig_atomic_t control_c;

  /**
   * This is the queue of commands TGDB has currently made to give to the 
   * front end.  */
    struct tgdb_list *command_list;

  /** An iterator into command_list. */
    tgdb_list_iterator *command_list_iterator;

  /**
   * When GDB dies (purposely or not), the SIGCHLD is sent to the application controlling TGDB.
   * This data structure represents the fact that SIGCHLD has been sent.
   *
   * This currently does not need to track if more than 1 SIGCHLD has been received. So
   * no matter how many are receieved, this will only be 1. Otherwise if none have been
   * received this will be 0.  */
    int has_sigchld_recv;
};

/* }}} */

/* Temporary prototypes {{{ */
static int tgdb_deliver_command(struct tgdb *tgdb,
        struct tgdb_command *command);
static int tgdb_unqueue_and_deliver_command(struct tgdb *tgdb);
static int tgdb_run_or_queue_command(struct tgdb *tgdb,
        struct tgdb_command *com);

/* }}} */

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
    if (tgdb_is_busy(tgdb_in)) {
        tgdb_queue_append(tgdb_in, request);
    } else {
        tgdb_process_command(tgdb_in, request);
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

/**
 * Process the commands that were created by the client
 *
 * \param tgdb
 * The TGDB context
 *
 * \return
 * -1 on error, 0 on success
 */
static int tgdb_process_client_commands(struct tgdb *tgdb)
{
    int i;

    for (i = 0; i < sbcount(tgdb->tcc->client_commands); i++)
    {
        struct tgdb_command *command = tgdb->tcc->client_commands[i];

        if (tgdb_run_or_queue_command(tgdb, command) == -1) {
            logger_write_pos(logger, __FILE__, __LINE__,
                    "tgdb_run_or_queue_command failed");
            return -1;
        }
    }

    /* Clear the client command array */
    sbsetcount(tgdb->tcc->client_commands, 0);
    return 0;
}

static struct tgdb *initialize_tgdb_context(void)
{
    struct tgdb *tgdb = (struct tgdb *) cgdb_malloc(sizeof (struct tgdb));

    tgdb->tcc = NULL;
    tgdb->control_c = 0;

    tgdb->debugger_stdout = -1;
    tgdb->debugger_stdin = -1;

    tgdb->inferior_stdout = -1;
    tgdb->inferior_stdin = -1;

    tgdb->gdb_client_request_queue = NULL;
    tgdb->gdb_input_queue = NULL;
    tgdb->oob_input_queue = NULL;

    tgdb->IS_SUBSYSTEM_READY_FOR_NEXT_COMMAND = 1;

    tgdb->command_list = tgdb_list_init();
    tgdb->has_sigchld_recv = 0;

    logger = NULL;

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
static int tgdb_initialize_config_dir(struct tgdb *tgdb, char *config_dir)
{
    /* Get the home directory */
    char *home_dir = getenv("HOME");
    const char *tgdb_dir = ".tgdb";

    /* Create the config directory */
    if (!fs_util_create_dir_in_base(home_dir, tgdb_dir)) {
        logger_write_pos(logger, __FILE__, __LINE__,
                "fs_util_create_dir_in_base error");
        return -1;
    }

    fs_util_get_path(home_dir, tgdb_dir, config_dir);

    return 0;
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

    /* Get the home directory */
    const char *tgdb_log_file = "tgdb_log.txt";
    char tgdb_log_path[FSUTIL_PATH_MAX];

    fs_util_get_path(config_dir, tgdb_log_file, tgdb_log_path);

    /* Initialize the logger */
    if (num_loggers == 0) {
        logger = logger_create();

        if (!logger) {
            printf("Error: Could not create log file\n");
            return -1;
        }
    }

    ++num_loggers;

    if (logger_set_file(logger, tgdb_log_path) == -1) {
        printf("Error: Could not open log file\n");
        return -1;
    }

    return 0;
}

/* Createing and Destroying a libtgdb context. {{{*/

struct tgdb *tgdb_initialize(const char *debugger,
        int argc, char **argv, int *debugger_fd)
{
    /* Initialize the libtgdb context */
    struct tgdb *tgdb = initialize_tgdb_context();
    char config_dir[FSUTIL_PATH_MAX];

    /* Create config directory */
    if (tgdb_initialize_config_dir(tgdb, config_dir) == -1) {
        logger_write_pos(logger, __FILE__, __LINE__, "tgdb_initialize error");
        return NULL;
    }

    if (tgdb_initialize_logger_interface(tgdb, config_dir) == -1) {
        printf("Could not initialize logger interface\n");
        return NULL;
    }

    tgdb->gdb_client_request_queue = queue_init();
    tgdb->gdb_input_queue = queue_init();
    tgdb->oob_input_queue = queue_init();

    tgdb->tcc = a2_create_context(debugger, argc, argv, config_dir,
            logger);

    /* create an instance and initialize a tgdb_client_context */
    if (tgdb->tcc == NULL) {
        logger_write_pos(logger, __FILE__, __LINE__,
                "a2_create_context failed");
        return NULL;
    }

    if (a2_initialize(tgdb->tcc,
                    &(tgdb->debugger_stdin),
                    &(tgdb->debugger_stdout),
                    &(tgdb->inferior_stdin), &(tgdb->inferior_stdout)) == -1) {
        logger_write_pos(logger, __FILE__, __LINE__,
                "tgdb_client_initialize failed");
        return NULL;
    }

    tgdb_process_client_commands(tgdb);

    *debugger_fd = tgdb->debugger_stdout;

    return tgdb;
}

int tgdb_shutdown(struct tgdb *tgdb)
{
    /* Free the logger */
    if (num_loggers == 1) {
        if (logger_destroy(logger) == -1) {
            printf("Could not destroy logger interface\n");
            return -1;
        }
    }

    --num_loggers;

    return a2_shutdown(tgdb->tcc);
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

static int tgdb_disassemble_pc(struct annotate_two *a2, int lines)
{
    int ret;
    char *data = NULL;

    data = lines ? sys_aprintf("%d", lines) : NULL;
    ret = commands_issue_command(a2, ANNOTATE_DISASSEMBLE_PC, data, 0);

    free(data);
    return ret;
}

static int tgdb_disassemble_func(struct annotate_two *a2, int raw, int source)
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
    } else if (source && commands_disassemble_supports_s_mode(a2->c)) {
        data = sys_aprintf("%s", "/s");
    }

    ret = commands_issue_command(a2, ANNOTATE_DISASSEMBLE_FUNC, data, 0);

    free(data);
    return ret;
}

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
    if (tgdb->IS_SUBSYSTEM_READY_FOR_NEXT_COMMAND &&
            a2_is_client_ready(tgdb->tcc) &&
            (queue_size(tgdb->gdb_input_queue) == 0))
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
    if (a2_is_client_ready(tgdb->tcc) &&
            ((queue_size(tgdb->gdb_input_queue) > 0) ||
                    (queue_size(tgdb->oob_input_queue) > 0)))
        return 1;

    return 0;
}

int tgdb_is_busy(struct tgdb *tgdb)
{
    return !tgdb_can_issue_command(tgdb);
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
        queue_free_list(tgdb->gdb_input_queue, tgdb_command_destroy);
        queue_free_list(tgdb->gdb_client_request_queue, tgdb_request_destroy_func);
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
 *
 * \return
 * 0 on success, or -1 on error.
 */
static int
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
    tc = tgdb_command_create(command, command_choice,
        (enum annotate_commands)-1);

    free(temp_command);
    temp_command = NULL;

    if (tgdb_run_or_queue_command(tgdb, tc) == -1) {
        logger_write_pos(logger, __FILE__, __LINE__,
                "tgdb_run_or_queue_command failed");
        return -1;
    }

    if (a2_user_ran_command(tgdb->tcc) == -1) {
        logger_write_pos(logger, __FILE__, __LINE__,
                "tgdb_client_tgdb_ran_command failed");
        return -1;
    }

    return tgdb_process_client_commands(tgdb);
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
static int
tgdb_run_or_queue_command(struct tgdb *tgdb, struct tgdb_command *command)
{
    int  can_issue = tgdb_can_issue_command(tgdb);

    if (can_issue) {
        if (tgdb_deliver_command(tgdb, command) == -1)
            return -1;

        tgdb_command_destroy(command);
    } else {
        /* Make sure to put the command into the correct queue. */
        switch (command->command_choice) {
            case TGDB_COMMAND_FRONT_END:
            case TGDB_COMMAND_TGDB_CLIENT:
                queue_append(tgdb->gdb_input_queue, command);
                break;
            case TGDB_COMMAND_TGDB_CLIENT_PRIORITY:
                queue_append(tgdb->oob_input_queue, command);
                break;
            case TGDB_COMMAND_CONSOLE:
                logger_write_pos(logger, __FILE__, __LINE__,
                        "unimplemented command");
                return -1;
            default:
                logger_write_pos(logger, __FILE__, __LINE__,
                        "unimplemented command");
                return -1;
        }
    }

    return 0;
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
static int tgdb_deliver_command(struct tgdb *tgdb, struct tgdb_command *command)
{
    tgdb->IS_SUBSYSTEM_READY_FOR_NEXT_COMMAND = 0;

    /* Send what we're doing to log file */
    io_debug_write_fmt(" tgdb_deliver_command: <%s>", command->gdb_command);

    /* A command for the debugger */
    if (a2_prepare_for_command(tgdb->tcc, command) == -1)
        return -1;

    /* A regular command from the client */
    io_debug_write_fmt("<%s>", command->gdb_command);

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
        tgdb_types_append_command(tgdb->command_list, response);
    }

    return 0;
}

/**
 * TGDB will search it's command queue's and determine what the next command
 * to deliever to GDB should be.
 *
 * \return
 * 0 on success, -1 on error
 */
static int tgdb_unqueue_and_deliver_command(struct tgdb *tgdb)
{
  tgdb_unqueue_and_deliver_command_tag:

    /* This will redisplay the prompt when a command is run
     * through the gui with data on the console.
     */
    /* The out of band commands should always be run first */
    if (queue_size(tgdb->oob_input_queue) > 0) {
        /* These commands are always run. 
         * However, if an assumption is made that a misc
         * prompt can never be set while in this spot.
         */
        struct tgdb_command *item = NULL;

        item = (struct tgdb_command *)queue_pop(tgdb->oob_input_queue);
        tgdb_deliver_command(tgdb, item);
        tgdb_command_destroy(item);
    }
    /* If the queue is not empty, run a command */
    else if (queue_size(tgdb->gdb_input_queue) > 0) {
        struct tgdb_command *item = NULL;

        item = (struct tgdb_command *)queue_pop(tgdb->gdb_input_queue);

        /* If at the misc prompt, don't run the internal tgdb commands,
         * In fact throw them out for now, since they are only 
         * 'info breakpoints' */
        if (a2_is_misc_prompt(tgdb->tcc) == 1) {
            if (item->command_choice != TGDB_COMMAND_CONSOLE) {
                tgdb_command_destroy(item);
                goto tgdb_unqueue_and_deliver_command_tag;
            }
        }

        /* This happens when a command was skipped because the client no longer
         * needs the command to be run */
        if (tgdb_deliver_command(tgdb, item) == -1)
            goto tgdb_unqueue_and_deliver_command_tag;

        tgdb_command_destroy(item);
    }

    return 0;
}

/* These functions are used to communicate with the inferior */
int tgdb_send_inferior_char(struct tgdb *tgdb, char c)
{
    if (io_write_byte(tgdb->inferior_stdout, c) == -1) {
        logger_write_pos(logger, __FILE__, __LINE__, "io_write_byte failed");
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
        logger_write_pos(logger, __FILE__, __LINE__, "inferior_fd read failed");
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

    tgdb_types_append_command(tgdb->command_list, response);

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
    pid_t pid = a2_get_debugger_pid(tgdb->tcc);
    int status = 0;
    pid_t ret;
    struct tgdb_response *response = tgdb_create_response(TGDB_QUIT);

    if (!tgdb_will_quit)
        return -1;

    *tgdb_will_quit = 0;

    ret = waitpid(pid, &status, WNOHANG);

    if (ret == -1) {
        logger_write_pos(logger, __FILE__, __LINE__, "waitpid error");
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

    tgdb_types_append_command(tgdb->command_list, response);
    *tgdb_will_quit = 1;

    return 0;
}

size_t tgdb_process(struct tgdb * tgdb, char *buf, size_t n, int *is_finished)
{
    char local_buf[10 * n];
    ssize_t size;
    size_t buf_size = 0;

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
            logger_write_pos(logger, __FILE__, __LINE__,
                    "tgdb_get_quit_command error");
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
        logger_write_pos(logger, __FILE__, __LINE__,
                "could not read from masterfd");
        buf_size = -1;
        tgdb_add_quit_command(tgdb);
        goto tgdb_finish;
    } else if (size == 0) {     /* EOF */
        tgdb_add_quit_command(tgdb);
        goto tgdb_finish;
    }

    local_buf[size] = '\0';

    /* 2. At this point local_buf has everything new from this read.
     * Basically this function is responsible for separating the annotations
     * that gdb writes from the data. 
     *
     * buf and buf_size are the data to be returned from the user.
     */
    {
        /* unused for now */
        char *infbuf = NULL;
        size_t infbuf_size;
        int result;

        result = a2_parse_io(tgdb->tcc,
                local_buf, size,
                buf, &buf_size, infbuf, &infbuf_size, tgdb->command_list);

        tgdb_process_client_commands(tgdb);

        if (result == 0) {
            /* success, and more to parse, ss isn't done */
        } else if (result == 1) {
            /* success, and finished command */
            tgdb->IS_SUBSYSTEM_READY_FOR_NEXT_COMMAND = 1;
        } else if (result == -1) {
            logger_write_pos(logger, __FILE__, __LINE__,
                    "a2_parse_io failed");
        }
    }

    /* 3. if ^c has been sent, clear the buffers.
     *        If a signal has been received, clear the queue and return
     */
    if (tgdb_handle_signals(tgdb) == -1) {
        logger_write_pos(logger, __FILE__, __LINE__,
                "tgdb_handle_signals failed");
        return -1;
    }

    /* 4. runs the users buffered command if any exists */
    if (tgdb_has_command_to_run(tgdb))
        tgdb_unqueue_and_deliver_command(tgdb);

  tgdb_finish:

    /* Set the iterator to the beginning. So when the user
     * calls tgdb_get_command it, it will be in the right spot.
     */
    tgdb->command_list_iterator = tgdb_list_get_first(tgdb->command_list);

    *is_finished = !tgdb_is_busy(tgdb);

    return buf_size;
}

/* Getting Data out of TGDB {{{*/

struct tgdb_response *tgdb_get_response(struct tgdb *tgdb)
{
    struct tgdb_response *command;

    if (tgdb->command_list_iterator == NULL)
        return NULL;

    command = (struct tgdb_response *) tgdb_list_get_item(
                tgdb->command_list_iterator);

    tgdb->command_list_iterator = tgdb_list_next(tgdb->command_list_iterator);

    return command;
}

void tgdb_traverse_responses(struct tgdb *tgdb)
{
    tgdb_list_foreach(tgdb->command_list, tgdb_types_print_command);
}

struct tgdb_response *tgdb_create_response(enum tgdb_response_type header)
{
    struct tgdb_response *response;

    response = (struct tgdb_response *)cgdb_calloc(1, sizeof(struct tgdb_response));
    response->header = header;

    return response;
}

void tgdb_delete_responses(struct tgdb *tgdb)
{
    tgdb_list_free(tgdb->command_list, tgdb_types_free_command);
}

/* }}}*/

/* Inferior tty commands {{{*/

int tgdb_tty_new(struct tgdb *tgdb)
{
    int ret = a2_open_new_tty(tgdb->tcc,
            &tgdb->inferior_stdin,
            &tgdb->inferior_stdout);

    tgdb_process_client_commands(tgdb);

    return ret;
}

int tgdb_get_inferior_fd(struct tgdb *tgdb)
{
    return tgdb->inferior_stdout;
}

const char *tgdb_tty_name(struct tgdb *tgdb)
{
    return pty_pair_get_slavename(tgdb->tcc->pty_pair);
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
    int ret = -1;

    if (!tgdb || !request)
        return -1;

    if (!tgdb_can_issue_command(tgdb))
        return -1;

    tgdb_set_last_request(request);

    if (request->header == TGDB_REQUEST_CONSOLE_COMMAND) {
        ret = tgdb_send(tgdb, request->choice.console_command.command,
            TGDB_COMMAND_CONSOLE);
    }
    else if (request->header == TGDB_REQUEST_DEBUGGER_COMMAND) {
        ret = tgdb_send(tgdb, tgdb_get_client_command(tgdb,
                request->choice.debugger_command.c), TGDB_COMMAND_FRONT_END);
    }
    else if (request->header == TGDB_REQUEST_MODIFY_BREAKPOINT) {
        char *val = tgdb_client_modify_breakpoint_call(tgdb,
            request->choice.modify_breakpoint.file,
            request->choice.modify_breakpoint.line,
            request->choice.modify_breakpoint.addr,
            request->choice.modify_breakpoint.b);
        if (val)
        {
            ret = tgdb_send(tgdb, val, TGDB_COMMAND_FRONT_END);
            free(val);
        }
    } else {
        if (request->header == TGDB_REQUEST_INFO_SOURCES) {
            ret = commands_issue_command(tgdb->tcc,
                    ANNOTATE_INFO_SOURCES, NULL, 0);
        }
        else if (request->header == TGDB_REQUEST_CURRENT_LOCATION) {
            ret = commands_issue_command(tgdb->tcc,
                    ANNOTATE_INFO_FRAME, NULL, 0);
        }
        else if (request->header == TGDB_REQUEST_COMPLETE) {
            ret = commands_issue_command(tgdb->tcc,
                    ANNOTATE_COMPLETE, request->choice.complete.line, 1);
        }
        else if (request->header == TGDB_REQUEST_DISASSEMBLE_PC) {
            ret = tgdb_disassemble_pc(
                tgdb->tcc, request->choice.disassemble.lines);
        }
        else if (request->header == TGDB_REQUEST_DISASSEMBLE_FUNC) {
            ret = tgdb_disassemble_func(tgdb->tcc,
                                      request->choice.disassemble_func.raw,
                                      request->choice.disassemble_func.source);
        }

        if (!ret) {
            tgdb_process_client_commands(tgdb);
        }
    }


    return 0;
}

/* }}}*/

/* }}} */

/* TGDB Queue commands {{{*/

int tgdb_queue_append(struct tgdb *tgdb, tgdb_request_ptr request)
{
    if (!tgdb || !request)
        return -1;

    queue_append(tgdb->gdb_client_request_queue, request);

    return 0;
}

tgdb_request_ptr tgdb_queue_pop(struct tgdb * tgdb)
{
    tgdb_request_ptr item;

    if (!tgdb)
        return NULL;

    item = (tgdb_request_ptr)queue_pop(tgdb->gdb_client_request_queue);

    return item;
}

int tgdb_queue_size(struct tgdb *tgdb)
{
    return queue_size(tgdb->gdb_client_request_queue);
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

/* Config Options {{{*/

int tgdb_set_verbose_error_handling(struct tgdb *tgdb, int value)
{
    if (value == -1)
        return logger_is_recording(logger);

    if (value == 1 || value == 0)
        logger_set_record(logger, value);

    if (value == 1)
        logger_set_fd(logger, stderr);

    return logger_is_recording(logger);
}

/* }}}*/
