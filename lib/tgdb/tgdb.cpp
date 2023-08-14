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

#include <list>
#include <sstream>

#include "tgdb.h"
#include "fork_util.h"
#include "io.h"
#include "terminal.h"
#include "pseudo.h" /* SLAVE_SIZE constant */
#include "sys_util.h"
#include "stretchy.h"
#include "cgdb_clog.h"
#include "gdbwire.h"

/* }}} */

/* struct tgdb {{{ */

typedef struct tgdb_request *tgdb_request_ptr;
typedef std::list<tgdb_request_ptr> tgdb_request_ptr_list;

/**
 * The TGDB context data structure.
 */
struct tgdb {
    /** Reading from this will read from the debugger's output */
    int debugger_stdout;

    /** Writing to this will write to the debugger's stdin */
    int debugger_stdin;

    /**
     * The gdb new-ui mi console file descriptor.
     *
     * Writing to this will write to the gdb/mi interpreter.
     * Reading from it will read the output of the gdb/mi interpreter.
     */
    int gdb_mi_ui_fd;

    /** The master, slave and slavename of the new-ui pty device */
    pty_pair_ptr new_ui_pty_pair;

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

    /** If ^c was hit by user */
    sig_atomic_t control_c;

    tgdb_callbacks callbacks;

    // commands structure
    // The current command request type that is executing. NULL to start.
    enum tgdb_request_type current_request_type;

    // The disassemble command output.
    char **disasm;
    uint64_t address_start, address_end;

    // The gdbwire context to talk to GDB with.
    struct gdbwire *wire;

    // True if the disassemble command supports /s, otherwise false.
    int disassemble_supports_s_mode;

    // True if GDB supports the 'new-ui' command, otherwise false.
    // If gdb prints out,
    //   Undefined command: "new-ui".  Try "help".
    // on the console output, then it does not support the new-ui command.
    // Otherwise it is assumed that it does support the command.
    bool gdb_supports_new_ui_command;

    // Temporary buffer used to store the line by line console output
    // in order to search for the unsupported new ui string above.
    std::string *undefined_new_ui_command;
};

// This is the type of request
struct tgdb_request {
    enum tgdb_request_type header;

    union {
        struct {
            // The null terminated console command to pass to GDB
            const char *command;
        } console_command;

        struct {
            const char *slavename;
        } tty_command;

        struct {
            // This is the command that libtgdb should run through the debugger
            enum tgdb_command_type c;
        } debugger_command;

        struct {
            // The filename to set the breakpoint in
            const char *file;
            // The corresponding line number
            int line;
            // The address to set breakpoint in (if file is null)
            uint64_t addr;
            // The action to take
            enum tgdb_breakpoint_action b;
        } modify_breakpoint;

        struct {
            int lines;
        } disassemble;

        struct {
            int source;
            int raw;
        } disassemble_func;

        struct {
            // The filename to set the breakpoint in
            const char *file;
            // The corresponding line number
            int line;
            // The address to set breakpoint in (if file is null)
            uint64_t addr;
        } until_line;
    } choice;
};

/* }}} */

/* Temporary prototypes {{{ */
struct tgdb_response *tgdb_create_response(enum tgdb_response_type header);
void tgdb_send_response(struct tgdb *tgdb, struct tgdb_response *response);
static void tgdb_run_request(struct tgdb *tgdb, struct tgdb_request *request);
static void tgdb_unqueue_and_deliver_command(struct tgdb *tgdb);
void tgdb_run_or_queue_request(struct tgdb *tgdb,
        struct tgdb_request *request, bool priority);

/* }}} */

// Command Functions {{{
static void
tgdb_commands_send_breakpoints(struct tgdb *tgdb,
    struct tgdb_breakpoint *breakpoints)
{
    struct tgdb_response *response = (struct tgdb_response *)
        tgdb_create_response(TGDB_UPDATE_BREAKPOINTS);

    response->choice.update_breakpoints.breakpoints = breakpoints;

    tgdb_send_response(tgdb, response);
}

static void tgdb_commands_process_breakpoint(
        struct tgdb_breakpoint *&breakpoints,
        struct gdbwire_mi_breakpoint *breakpoint)
{
    bool file_location_avialable =
        (breakpoint->fullname || breakpoint->file) && breakpoint->line != 0;
    bool assembly_location_available = breakpoint->address && 
        !breakpoint->pending && !breakpoint->multi;

    if (file_location_avialable || assembly_location_available) {
        struct tgdb_breakpoint tb;

        if (file_location_avialable) {
            tb.path = (breakpoint->fullname)?
                cgdb_strdup(breakpoint->fullname):
                cgdb_strdup(breakpoint->file);
            tb.line = breakpoint->line;
        } else {
            tb.path = 0;
            tb.line = 0;
        }

        if (assembly_location_available) {
            uint64_t address = 0;
            cgdb_hexstr_to_u64(breakpoint->address, &address);
            tb.addr = address;
        } else {
            tb.addr = 0;
        }

        tb.enabled = breakpoint->enabled;
        sbpush(breakpoints, tb);
    }
}

static void tgdb_commands_process_breakpoints(struct tgdb *tgdb,
        struct gdbwire_mi_result_record *result_record)
{
    enum gdbwire_result result;
    struct gdbwire_mi_command *mi_command = 0;
    result = gdbwire_get_mi_command(GDBWIRE_MI_BREAK_INFO,
        result_record, &mi_command);
    if (result == GDBWIRE_OK) {
        struct tgdb_breakpoint *breakpoints = NULL;
        struct gdbwire_mi_breakpoint *breakpoint =
            mi_command->variant.break_info.breakpoints;
        while (breakpoint) {
            tgdb_commands_process_breakpoint(breakpoints, breakpoint);

            if (breakpoint->multi) {
                struct gdbwire_mi_breakpoint *multi_bkpt =
                    breakpoint->multi_breakpoints;
                while (multi_bkpt) {
                    tgdb_commands_process_breakpoint(breakpoints, multi_bkpt);
                    multi_bkpt = multi_bkpt->next;
                }
            }

            breakpoint = breakpoint->next;
        }

        tgdb_commands_send_breakpoints(tgdb, breakpoints);

        gdbwire_mi_command_free(mi_command);
    }
}

static void tgdb_commands_send_source_files(struct tgdb *tgdb,
        char **source_files)
{
    struct tgdb_response *response =
        tgdb_create_response(TGDB_UPDATE_SOURCE_FILES);
    response->choice.update_source_files.source_files = source_files;
    tgdb_send_response(tgdb, response);
}

/* This function is capable of parsing the output of 'info source'.
 * It can get both the absolute and relative path to the source file.
 */
static void
tgdb_commands_process_info_sources(struct tgdb *tgdb,
        struct gdbwire_mi_result_record *result_record)
{
    enum gdbwire_result result;
    struct gdbwire_mi_command *mi_command = 0;
    result = gdbwire_get_mi_command(GDBWIRE_MI_FILE_LIST_EXEC_SOURCE_FILES,
        result_record, &mi_command);
    if (result == GDBWIRE_OK) {
        char **source_files = NULL;
        struct gdbwire_mi_source_file *files =
            mi_command->variant.file_list_exec_source_files.files;
        while (files) {
            char *file = (files->fullname)?files->fullname:files->file;
            sbpush(source_files, strdup(file));
            files = files->next;
        }

        tgdb_commands_send_source_files(tgdb, source_files);

        gdbwire_mi_command_free(mi_command);
    }
}

static void send_disassemble_func_complete_response(struct tgdb *tgdb,
        struct gdbwire_mi_result_record *result_record)
{
    tgdb_response_type type =
            (tgdb->current_request_type == TGDB_REQUEST_DISASSEMBLE_PC) ?
                TGDB_DISASSEMBLE_PC : TGDB_DISASSEMBLE_FUNC;
    struct tgdb_response *response =
        tgdb_create_response(type);

    response->choice.disassemble_function.error = 
        (result_record->result_class == GDBWIRE_MI_ERROR);
            
    response->choice.disassemble_function.disasm = tgdb->disasm;
    response->choice.disassemble_function.addr_start = tgdb->address_start;
    response->choice.disassemble_function.addr_end = tgdb->address_end;
    
    tgdb->disasm = NULL;
    tgdb->address_start = 0;
    tgdb->address_end = 0;

    tgdb_send_response(tgdb, response);
}

static void
tgdb_commands_send_source_file(struct tgdb *tgdb, const char *fullname,
        const char *file, uint64_t address, const char *from,
        const char *func, int line)
{
    /* This section allocates a new structure to add into the queue 
     * All of its members will need to be freed later.
     */
    struct tgdb_file_position *tfp = (struct tgdb_file_position *)
            cgdb_malloc(sizeof (struct tgdb_file_position));
    struct tgdb_response *response =
            tgdb_create_response(TGDB_UPDATE_FILE_POSITION);

    if (fullname || file) {
        tfp->path = (fullname)?cgdb_strdup(fullname):cgdb_strdup(file);
    } else {
        tfp->path = 0;
    }
    tfp->addr = address;
    tfp->from = (from)?cgdb_strdup(from):0;
    tfp->func = (func)?cgdb_strdup(func):0;
    tfp->line_number = line;

    response->choice.update_file_position.file_position = tfp;

    tgdb_send_response(tgdb, response);
}

static void tgdb_commands_process_info_source(struct tgdb *tgdb,
        struct gdbwire_mi_result_record *result_record)
{
    enum gdbwire_result result;
    struct gdbwire_mi_command *mi_command = 0;
    result = gdbwire_get_mi_command(GDBWIRE_MI_FILE_LIST_EXEC_SOURCE_FILE,
        result_record, &mi_command);
    if (result == GDBWIRE_OK) {
        tgdb_commands_send_source_file(tgdb,
                mi_command->variant.file_list_exec_source_file.fullname,
                mi_command->variant.file_list_exec_source_file.file,
                0, 0, 0,
                mi_command->variant.file_list_exec_source_file.line);

        gdbwire_mi_command_free(mi_command);
    }
}

static void tgdb_commands_process_info_frame(struct tgdb *tgdb,
        struct gdbwire_mi_result_record *result_record)
{
    bool require_source = false;
    enum gdbwire_result result;
    struct gdbwire_mi_command *mi_command = 0;
    result = gdbwire_get_mi_command(GDBWIRE_MI_STACK_INFO_FRAME,
        result_record, &mi_command);
    if (result == GDBWIRE_OK) {
        struct gdbwire_mi_stack_frame *frame =
            mi_command->variant.stack_info_frame.frame;
        uint64_t address = 0;
        cgdb_hexstr_to_u64(frame->address, &address);

        if (frame->address || frame->file || frame->fullname) {
            tgdb_commands_send_source_file(tgdb, frame->fullname, frame->file,
                    address, frame->from, frame->func, frame->line);
        } else {
            require_source = true;
        }

        gdbwire_mi_command_free(mi_command);
    } else {
        require_source = true;
    }

    if (require_source) {
        tgdb_request_ptr request;
        request = (tgdb_request_ptr) cgdb_malloc(sizeof (struct tgdb_request));
        request->header = TGDB_REQUEST_INFO_SOURCE_FILE;
        tgdb_run_or_queue_request(tgdb, request, true);
    }
}

static void gdbwire_stream_record_callback(void *context,
    struct gdbwire_mi_stream_record *stream_record)
{
    struct tgdb *tgdb = (struct tgdb*)context;

    switch (tgdb->current_request_type) {
        case TGDB_REQUEST_BREAKPOINTS:
        case TGDB_REQUEST_INFO_FRAME:
            /**
             * When using GDB with annotate=2 and also using interpreter-exec,
             * GDB spits out the annotations in the MI output. All of these
             * annotations can be ignored.
             */
            break;
        case TGDB_REQUEST_DISASSEMBLE_PC:
        case TGDB_REQUEST_DISASSEMBLE_FUNC:
            if (stream_record->kind == GDBWIRE_MI_CONSOLE) {
                uint64_t address;
                int result;
                char *str = stream_record->cstring;
                size_t length = strlen(str);
                char *colon = 0, colon_char = 0;

                if (str[length-1] == '\n') {
                    str[length-1] = 0;
                }

                /* Trim the gdb current location pointer off */
                if (length > 2 && str[0] == '=' && str[1] == '>') {
                    str[0] = ' ';
                    str[1] = ' ';
                }

                sbpush(tgdb->disasm, cgdb_strdup(str));

                colon = strchr((char*)str, ':');
                if (colon) {
                    colon_char = *colon;
                    *colon = 0;
                }

                result = cgdb_hexstr_to_u64(str, &address);

                if (colon) {
                    *colon = colon_char;
                }

                if (result == 0 && address) {
                    tgdb->address_start = tgdb->address_start ?
                         MIN(address, tgdb->address_start) : address;
                    tgdb->address_end = MAX(address, tgdb->address_end);
                }
            }
            break;
        case TGDB_REQUEST_DATA_DISASSEMBLE_MODE_QUERY:
        case TGDB_REQUEST_INFO_SOURCES:
        case TGDB_REQUEST_INFO_SOURCE_FILE:
        case TGDB_REQUEST_TTY:
        case TGDB_REQUEST_DEBUGGER_COMMAND:
        case TGDB_REQUEST_MODIFY_BREAKPOINT:
        case TGDB_REQUEST_UNTIL_LINE:
            break;
    }
}

static void
source_position_changed(struct tgdb *tgdb, gdbwire_mi_result *result)
{
    std::string frame("frame");
    std::string addr("addr");
    std::string fullname("fullname");
    std::string line("line");

    while (result) {
        if (frame == result->variable &&
            result->kind == GDBWIRE_MI_TUPLE) {

            uint64_t addr_value = 0;
            bool addr_value_set = false;

            std::string fullname_value;
            bool fullname_value_set = false;

            int line_value = 0;
            bool line_value_set = false;

            struct gdbwire_mi_result *fresult = result->variant.result;

            while (fresult) {
                if (addr == fresult->variable) {
                    addr_value = std::stoull(
                            fresult->variant.cstring, 0, 16);
                    addr_value_set = true;
                } else if (fullname == fresult->variable) {
                    fullname_value = fresult->variant.cstring;
                    fullname_value_set = true;
                } else if (line == fresult->variable) {
                    line_value = std::stoi(fresult->variant.cstring);
                    line_value_set = true;
                }
                fresult = fresult->next;
            }

            if(addr_value_set || (fullname_value_set && line_value_set)) {
                tgdb_commands_send_source_file(tgdb, fullname_value.c_str(),
                        NULL, addr_value, NULL, NULL, line_value);
            }
        }

        result = result->next;
    }
}

void tgdb_breakpoints_changed(void *context);
static void gdbwire_async_record_callback(void *context,
        struct gdbwire_mi_async_record *async_record)
{
    struct tgdb *tgdb = (struct tgdb*)context;

    switch (async_record->async_class) {
        case GDBWIRE_MI_ASYNC_STOPPED:
        case GDBWIRE_MI_ASYNC_THREAD_SELECTED:
            source_position_changed(tgdb, async_record->result);
            break;
        case GDBWIRE_MI_ASYNC_BREAKPOINT_CREATED:
        case GDBWIRE_MI_ASYNC_BREAKPOINT_MODIFIED:
        case GDBWIRE_MI_ASYNC_BREAKPOINT_DELETED:
            tgdb_breakpoints_changed(tgdb);
            break;
        default:
            break;
    }
}

static void gdbwire_result_record_callback(void *context,
        struct gdbwire_mi_result_record *result_record)
{
    struct tgdb *tgdb = (struct tgdb*)context;

    switch (tgdb->current_request_type) {
        case TGDB_REQUEST_BREAKPOINTS:
            tgdb_commands_process_breakpoints(tgdb, result_record);
            break;
        case TGDB_REQUEST_INFO_SOURCES:
            tgdb_commands_process_info_sources(tgdb, result_record);
            break;
        case TGDB_REQUEST_DISASSEMBLE_PC:
        case TGDB_REQUEST_DISASSEMBLE_FUNC:
            send_disassemble_func_complete_response(tgdb, result_record);
            break;
        case TGDB_REQUEST_DATA_DISASSEMBLE_MODE_QUERY:
            /**
             * If the mode was to high, than the result record would be
             * an error, meaning the mode is not supported. Otherwise,
             * the mode is supported.
             */
            if (result_record->result_class == GDBWIRE_MI_DONE) {
                tgdb->disassemble_supports_s_mode = 1;
                clog_info(CLOG_CGDB, "disassemble supports s mode");
            }
            break;
        case TGDB_REQUEST_INFO_SOURCE_FILE:
            tgdb_commands_process_info_source(tgdb, result_record);
            break;
        case TGDB_REQUEST_INFO_FRAME:
            tgdb_commands_process_info_frame(tgdb, result_record);
            break;
        case TGDB_REQUEST_TTY:
        case TGDB_REQUEST_DEBUGGER_COMMAND:
        case TGDB_REQUEST_MODIFY_BREAKPOINT:
        case TGDB_REQUEST_UNTIL_LINE:
            break;
    }
}

void tgdb_console_at_prompt(void *context);
static void gdbwire_prompt_callback(void *context, const char *prompt)
{
    struct tgdb *tgdb = (struct tgdb*)context;
    tgdb_console_at_prompt(tgdb);
}

static void gdbwire_parse_error_callback(void *context, const char *mi,
            const char *token, struct gdbwire_mi_position position)
{
    clog_error(CLOG_CGDB,
        "gdbwire parse error\n"
        "  mi text=[%s]\n"
        "  token=[%s]\n"
        "  start col=[%d]\n"
        "  end col=[%d]\n",
        mi, token, position.start_column, position.end_column);
}

static struct gdbwire_callbacks wire_callbacks =
    {
        0,
        gdbwire_stream_record_callback,
        gdbwire_async_record_callback,
        gdbwire_result_record_callback,
        gdbwire_prompt_callback,
        gdbwire_parse_error_callback
    };

int free_char_star(void *item)
{
    char *s = (char *) item;

    free(s);
    s = NULL;

    return 0;
}

void tgdb_commands_process(struct tgdb *tgdb, const std::string &str)
{
   gdbwire_push_data(tgdb->wire, str.data(), str.size());
}

void tgdb_commands_set_current_request_type(struct tgdb *tgdb,
        enum tgdb_request_type type)
{
    tgdb->current_request_type = type;
}

int tgdb_commands_disassemble_supports_s_mode(struct tgdb *tgdb)
{
    return tgdb->disassemble_supports_s_mode;
}

// }}}

static struct tgdb *initialize_tgdb_context(tgdb_callbacks callbacks)
{
    struct tgdb *tgdb = (struct tgdb *) cgdb_malloc(sizeof (struct tgdb));

    tgdb->control_c = 0;

    tgdb->debugger_stdout = -1;
    tgdb->debugger_stdin = -1;

    tgdb->gdb_mi_ui_fd = -1;

    tgdb->new_ui_pty_pair = NULL;

    tgdb->command_requests = new tgdb_request_ptr_list();

    tgdb->is_gdb_ready_for_next_command = 0;

    tgdb->callbacks = callbacks;

    tgdb->disasm = NULL;
    tgdb->address_start = 0;
    tgdb->address_end = 0;

    wire_callbacks.context = (void*)tgdb;
    tgdb->wire = gdbwire_create(wire_callbacks);

    tgdb->disassemble_supports_s_mode = 0;
    tgdb->gdb_supports_new_ui_command = true;
    tgdb->undefined_new_ui_command = new std::string();

    return tgdb;
}

/*******************************************************************************
 * This is the basic initialization
 ******************************************************************************/

static void tgdb_issue_request(struct tgdb *tgdb, enum tgdb_request_type type,
        bool priority)
{
    tgdb_request_ptr request_ptr;
    request_ptr = (tgdb_request_ptr)cgdb_malloc(sizeof (struct tgdb_request));
    request_ptr->header = type;
    tgdb_run_or_queue_request(tgdb, request_ptr, priority);
}
 
void tgdb_breakpoints_changed(void *context)
{
    struct tgdb *tgdb = (struct tgdb*)context;
    tgdb_issue_request(tgdb, TGDB_REQUEST_BREAKPOINTS, true);
}

static void tgdb_source_location_changed(void *context)
{
    struct tgdb *tgdb = (struct tgdb*)context;
    tgdb_request_current_location(tgdb);
}

static void tgdb_command_error(void *context, const std::string &msg)
{
    struct tgdb *tgdb = (struct tgdb*)context;

    /* Send cgdb the error message */
    tgdb->callbacks.console_output_callback(tgdb->callbacks.context, msg);
}

void tgdb_console_at_prompt(void *context)
{
    struct tgdb *tgdb = (struct tgdb*)context;

    tgdb->is_gdb_ready_for_next_command = 1;

    if (tgdb->command_requests->size() > 0) {
        tgdb_unqueue_and_deliver_command(tgdb);
    }
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
        case TGDB_REQUEST_INFO_SOURCES:
            break;
        case TGDB_REQUEST_DEBUGGER_COMMAND:
            break;
        case TGDB_REQUEST_MODIFY_BREAKPOINT:
            free((char *) request_ptr->choice.modify_breakpoint.file);
            request_ptr->choice.modify_breakpoint.file = NULL;
            break;
        case TGDB_REQUEST_UNTIL_LINE:
            free((char *) request_ptr->choice.until_line.file);
            request_ptr->choice.until_line.file = NULL;
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

struct tgdb *tgdb_initialize(tgdb_callbacks callbacks)
{
    /* Initialize the libtgdb context */
    struct tgdb *tgdb = initialize_tgdb_context(callbacks);

    // create the new ui pty pair
    // initialize the size to the gdb window size
    tgdb->new_ui_pty_pair = pty_pair_create();
    if (!tgdb->new_ui_pty_pair) {
        clog_error(CLOG_CGDB, "pty_pair_create failed");
        return NULL;
    }

    tgdb->gdb_mi_ui_fd = pty_pair_get_masterfd(tgdb->new_ui_pty_pair);
    tty_set_echo(tgdb->gdb_mi_ui_fd, 0);

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

    return tgdb;
}

int tgdb_start_gdb(struct tgdb *tgdb,
        const char *debugger, int argc, char **argv,
        int gdb_win_rows, int gdb_win_cols, int *gdb_console_fd,
        int *gdb_mi_fd)
{
    tgdb->debugger_pid = invoke_debugger(debugger, argc, argv,
            gdb_win_rows, gdb_win_cols, 
            &tgdb->debugger_stdin, &tgdb->debugger_stdout,
            pty_pair_get_slavename(tgdb->new_ui_pty_pair));

    /* Couldn't invoke process */
    if (tgdb->debugger_pid == -1)
        return -1;

    *gdb_console_fd = tgdb->debugger_stdout;
    *gdb_mi_fd = tgdb->gdb_mi_ui_fd;

    return 0;
}

int tgdb_shutdown(struct tgdb *tgdb)
{
    delete tgdb->undefined_new_ui_command;

    tgdb_request_ptr_list::iterator iter = tgdb->command_requests->begin();
    for (; iter != tgdb->command_requests->end(); ++iter) {
        tgdb_request_destroy(*iter);
    }

    delete tgdb->command_requests;
    tgdb->command_requests = 0;

    if (tgdb->debugger_stdin != -1) {
        cgdb_close(tgdb->debugger_stdin);
        tgdb->debugger_stdin = -1;
    }

    gdbwire_destroy(tgdb->wire);

    return 0;
}

void tgdb_close_logfiles()
{
    clog_info(CLOG_CGDB, "Closing logfile.");
    clog_free(CLOG_CGDB_ID);

    clog_info(CLOG_GDBIO, "Closing logfile.");
    clog_free(CLOG_GDBIO_ID);

    clog_info(CLOG_GDBMIIO, "Closing logfile.");
    clog_free(CLOG_GDBMIIO_ID);
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
        case TGDB_NEXTI:
            return "nexti";
        case TGDB_START:
            return "start";
        case TGDB_RUN:
            return "run";
        case TGDB_KILL:
            return "kill";
        case TGDB_STEP:
            return "step";
        case TGDB_STEPI:
            return "stepi";
        case TGDB_UNTIL:
            return "until";
        case TGDB_UP:
            return "up";
        case TGDB_DOWN:
            return "down";
    }

    return NULL;
}

static char *tgdb_break_call(const char *action,
    const char *file, int line, uint64_t addr)
{
    if (file)
        return sys_aprintf("%s \"%s\":%d", action, file, line);

    return sys_aprintf("%s *0x%" PRIx64, action, addr);
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

    return tgdb_break_call(action, file, line, addr);
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
    int can_issue;
    
    // Debugger commands currently get executed in the gdb console
    // rather than the gdb mi channel. The gdb console is no longer
    // queued by CGDB, rather CGDB passes everything along to it that the
    // user types. So always issue debugger commands for now.
    if (request->header == TGDB_REQUEST_DEBUGGER_COMMAND) {
        can_issue = 1;
    } else {
        can_issue = tgdb->is_gdb_ready_for_next_command;
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

    tgdb->is_gdb_ready_for_next_command = 0;

    tgdb_get_gdb_command(tgdb, request, command);

    /* Add a newline to the end of the command if it doesn't exist */
    if (*command.rbegin() != '\n') {
        command.push_back('\n');
    }

    /* Send what we're doing to log file */
    std::string str = sys_quote_nonprintables(command.c_str(), -1);
    clog_debug(CLOG_GDBMIIO, "%s", str.c_str());

    /* A command for the debugger */
    tgdb_commands_set_current_request_type(tgdb, request->header);

    if (request->header == TGDB_REQUEST_DEBUGGER_COMMAND) {
        // since debugger commands are sent to the debugger's stdin
        // and not to the new-ui mi window, then we don't have to wait
        // for gdb to respond with an mi prompt. CGDB can send as many
        // commands as it likes, just as if the user typed it at the console
        tgdb->is_gdb_ready_for_next_command = 1;
        io_writen(tgdb->debugger_stdin, command.c_str(), command.size());
    } else {
        io_writen(tgdb->gdb_mi_ui_fd, command.c_str(), command.size());
    }

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

int tgdb_send_char(struct tgdb *tgdb, char c)
{
    if (io_write_byte(tgdb->debugger_stdin, c) == -1) {
        clog_error(CLOG_CGDB, "io_write_byte failed");
        return -1;
    }

    return 0;
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
static int tgdb_add_quit_command(struct tgdb *tgdb, bool new_ui_unsupported)
{
    struct tgdb_response *response;
    response = tgdb_create_response(TGDB_QUIT);
    response->choice.quit.new_ui_unsupported = new_ui_unsupported;
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
    int waitpid_result;

    do {
        waitpid_result = waitpid(tgdb->debugger_pid, &status, WNOHANG);
        if (waitpid_result == -1) {
            result = -1;
            clog_error(CLOG_CGDB, "waitpid error %d %s",
                    errno, strerror(errno));
        }
    } while (waitpid_result == 0);

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
    }
}

// Search for the string
//   Undefined command: "new-ui"
// to determine if this gdb supports the new-ui command or not.
// If the string is found, set tgdb->gdb_supports_new_ui_command to false
static void tgdb_search_for_unsupported_new_ui_message(
        struct tgdb *tgdb, const std::string &console_output)
{
    static const char *new_ui_text = "Undefined command: \"new-ui\".";
    
    (*tgdb->undefined_new_ui_command) += console_output;

    std::istringstream ss(*tgdb->undefined_new_ui_command);
    std::string line;
    while (getline(ss, line)) {
        if (line.find(new_ui_text) == 0) {
            tgdb->gdb_supports_new_ui_command = false;
            break;
        }
    }

    // Remove everything up to the last newline character so that
    // only newly added new lines are searched
    size_t pos = tgdb->undefined_new_ui_command->find_last_of("\n");
    if (pos != std::string::npos) {
        *tgdb->undefined_new_ui_command = 
                tgdb->undefined_new_ui_command->substr(pos);
    }
}

int tgdb_process(struct tgdb * tgdb, int fd)
{
    const int n = 4096;
    static char buf[n];
    ssize_t size;
    int result = 0;

    // If ^c has been typed at the prompt, clear the queues
    tgdb_handle_control_c(tgdb);

    size = io_read(fd, buf, n);
    if (size < 0) {
        // Error reading from GDB
        clog_error(CLOG_CGDB, "Error reading from gdb's stdout, closing down");
        result = -1;
        tgdb_add_quit_command(tgdb, false);
    } else if (size == 0) {
        // Read EOF from GDB
        clog_info(CLOG_GDBIO, "read EOF from GDB, closing down");
        tgdb_add_quit_command(tgdb, false);
    } else {
        if (fd == tgdb->debugger_stdout) {
            // Read some GDB console output, process it
            std::string str = sys_quote_nonprintables(buf, size);
            clog_debug(CLOG_GDBIO, "%s", str.c_str());
            std::string msg(buf, size);

            // Determine if this gdb supports the new-ui command.
            // If it does not, send the quit command to alert the user
            // that they need a newer gdb.
            tgdb_search_for_unsupported_new_ui_message(tgdb, msg);
            if (!tgdb->gdb_supports_new_ui_command) {
                tgdb_add_quit_command(tgdb, true);
            }

            tgdb->callbacks.console_output_callback(
                    tgdb->callbacks.context, msg);
        } else if (fd == tgdb->gdb_mi_ui_fd){
            // Read some GDB console output, process it
            std::string msg(buf, size);
            clog_debug(CLOG_GDBMIIO, "%s", msg.c_str());
            tgdb_commands_process(tgdb, msg);
        } else {
        }
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

int tgdb_resize_console(struct tgdb *tgdb, int rows, int cols)
{
    struct winsize size;
    int result;

    if (tgdb->debugger_stdin == -1) {
        return 0;
    }
    
    size.ws_row = rows;
    size.ws_col = cols;
    size.ws_xpixel = 0;
    size.ws_ypixel = 0;

    result = ioctl(tgdb->debugger_stdin, TIOCSWINSZ, &size);
    if (result == -1) {
        clog_error(CLOG_CGDB, "ioctl failed");
    }

    return 0;
}

/* }}}*/

/* Functional commands {{{ */

/* Request {{{*/

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

void tgdb_request_breakpoints(struct tgdb * tgdb)
{
    tgdb_request_ptr request_ptr;

    request_ptr = (tgdb_request_ptr)cgdb_malloc(sizeof (struct tgdb_request));

    request_ptr->header = TGDB_REQUEST_BREAKPOINTS;

    tgdb_run_or_queue_request(tgdb, request_ptr, false);
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

void tgdb_request_until_line(struct tgdb *tgdb,
        const char *file, int line, uint64_t addr)
{
    tgdb_request_ptr request_ptr;

    request_ptr = (tgdb_request_ptr)cgdb_malloc(sizeof (struct tgdb_request));
    request_ptr->header = TGDB_REQUEST_UNTIL_LINE;

    request_ptr->choice.until_line.file = file ? cgdb_strdup(file) : NULL;
    request_ptr->choice.until_line.line = line;
    request_ptr->choice.until_line.addr = addr;

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
        case TGDB_REQUEST_INFO_SOURCES:
            command = "-file-list-exec-source-files\n";
            break;
        case TGDB_REQUEST_INFO_SOURCE_FILE:
            command = "-file-list-exec-source-file\n";
            break;
        case TGDB_REQUEST_BREAKPOINTS:
            command = "-break-info\n";
            break;
        case TGDB_REQUEST_TTY:
            str = sys_aprintf("-inferior-tty-set %s\n",
                    request->choice.tty_command.slavename);
            command = str;
            free(str);
            str = NULL;
            break;
        case TGDB_REQUEST_INFO_FRAME:
            command = "-stack-info-frame\n";
            break;
        case TGDB_REQUEST_DATA_DISASSEMBLE_MODE_QUERY:
            command = "-data-disassemble -s 0 -e 0 -- 4\n";
            break;
        case TGDB_REQUEST_DEBUGGER_COMMAND:
            command = tgdb_get_client_command(tgdb,
                    request->choice.debugger_command.c);

            break;
        case TGDB_REQUEST_MODIFY_BREAKPOINT:
            str = tgdb_client_modify_breakpoint_call(tgdb,
                    request->choice.modify_breakpoint.file,
                    request->choice.modify_breakpoint.line,
                    request->choice.modify_breakpoint.addr,
                    request->choice.modify_breakpoint.b);
            command = str;
            free(str);
            str = NULL;
            break;
        case TGDB_REQUEST_DISASSEMBLE_PC:
            str = sys_aprintf("x/%di $pc\n", request->choice.disassemble.lines);
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
                    tgdb_commands_disassemble_supports_s_mode(tgdb)) {
                data = "/s";
            }
            str = sys_aprintf("disassemble%s%s\n",
                    data ? " " : "", data ? data : "");
            command = str;
            free(str);
            str = NULL;
            break;
        }
        case TGDB_REQUEST_UNTIL_LINE:
            str = tgdb_break_call("until",
                request->choice.until_line.file,
                request->choice.until_line.line,
                request->choice.until_line.addr);
            command = str;
            free(str);
            str = NULL;
            break;
    }

    return 0;
}

/* }}}*/

/* }}} */

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
        // GDB has died, clean up the zombie and send the quit command
        tgdb_handle_sigchld(tgdb);
        tgdb_add_quit_command(tgdb, false);
    }

    return 0;
}

/* }}}*/
