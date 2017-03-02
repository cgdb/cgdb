#if HAVE_CONFIG_H
#include "config.h"
#endif

#if HAVE_STDIO_H
#include <stdio.h>
#endif

#if HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#if HAVE_CTYPE_H
#include <ctype.h> /* isspace, isdigit */
#endif

/* Local includes */
#include "sys_util.h"
#include "commands.h"
#include "io.h"
#include "tgdb_types.h"
#include "ibuf.h"
#include "gdbwire.h"

/**
 * This structure aids in parsing the gdb command output, in annotations or mi.
 */
struct commands {
    /**
     * The annotate two data structure this command structure corresponds
     * with. Perhaps combine these two at one point.
     */
    struct tgdb *tgdb;

    /**
     * The current command kind that is executing
     */
    enum command_kind cur_command_kind;

    /**
     * The current tab completion items.
     */
    char **completions;

    /**
     * The disassemble command output.
     */
    char **disasm;
    uint64_t address_start, address_end;

    /**
     * The gdbwire context to talk to GDB with.
     */
    struct gdbwire *wire;

    /**
     * True if the disassemble command supports /s, otherwise false.
     */
    int disassemble_supports_s_mode;

    /**
     * True if this is a console command, False otherwise.
     *
     * This member is used to determine if the output of the current
     * command running should be sent back to the console.
     */
    bool is_console_command;
};

void tgdb_run_or_queue_command(struct tgdb *tgdb, struct tgdb_command *command);

static void
commands_send_breakpoints(struct commands *c,
    struct tgdb_breakpoint *breakpoints)
{
    struct tgdb_response *response = (struct tgdb_response *)
        tgdb_create_response(TGDB_UPDATE_BREAKPOINTS);

    response->choice.update_breakpoints.breakpoints = breakpoints;

    tgdb_send_response(c->tgdb, response);
}

static void commands_process_breakpoint(
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

static void commands_process_breakpoints(struct commands *c,
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
            commands_process_breakpoint(breakpoints, breakpoint);

            if (breakpoint->multi) {
                struct gdbwire_mi_breakpoint *multi_bkpt =
                    breakpoint->multi_breakpoints;
                while (multi_bkpt) {
                    commands_process_breakpoint(breakpoints, multi_bkpt);
                    multi_bkpt = multi_bkpt->next;
                }
            }

            breakpoint = breakpoint->next;
        }

        commands_send_breakpoints(c, breakpoints);

        gdbwire_mi_command_free(mi_command);
    }
}

static void commands_send_source_files(struct commands *c,
        char **source_files)
{
    struct tgdb_response *response =
        tgdb_create_response(TGDB_UPDATE_SOURCE_FILES);
    response->choice.update_source_files.source_files = source_files;
    tgdb_send_response(c->tgdb, response);
}

/* This function is capable of parsing the output of 'info source'.
 * It can get both the absolute and relative path to the source file.
 */
static void
commands_process_info_sources(struct commands *c,
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

        commands_send_source_files(c, source_files);

        gdbwire_mi_command_free(mi_command);
    }
}

static void send_disassemble_func_complete_response(struct commands *c,
        struct gdbwire_mi_result_record *result_record)
{
    tgdb_response_type type = (c->cur_command_kind == COMMAND_DISASSEMBLE_PC) ?
        TGDB_DISASSEMBLE_PC : TGDB_DISASSEMBLE_FUNC;
    struct tgdb_response *response =
        tgdb_create_response(type);

    response->choice.disassemble_function.error = 
        (result_record->result_class == GDBWIRE_MI_ERROR);
            
    response->choice.disassemble_function.disasm = c->disasm;
    response->choice.disassemble_function.addr_start = c->address_start;
    response->choice.disassemble_function.addr_end = c->address_end;
    
    c->disasm = NULL;
    c->address_start = 0;
    c->address_end = 0;

    tgdb_send_response(c->tgdb, response);
}

static void send_command_complete_response(struct commands *c)
{
    struct tgdb_response *response =
        tgdb_create_response(TGDB_UPDATE_COMPLETIONS);

    response->choice.update_completions.completions = c->completions;
    /* Clear commands completions since we've just stolen that pointer. */
    c->completions = NULL;

    tgdb_send_response(c->tgdb, response);
}

static void
commands_send_source_file(struct commands *c, char *fullname, char *file,
        uint64_t address, char *from, char *func, int line)
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

    tgdb_send_response(c->tgdb, response);
}

static void commands_process_info_source(struct commands *c,
        struct gdbwire_mi_result_record *result_record)
{
    enum gdbwire_result result;
    struct gdbwire_mi_command *mi_command = 0;
    result = gdbwire_get_mi_command(GDBWIRE_MI_FILE_LIST_EXEC_SOURCE_FILE,
        result_record, &mi_command);
    if (result == GDBWIRE_OK) {
        commands_send_source_file(c,
                mi_command->variant.file_list_exec_source_file.fullname,
                mi_command->variant.file_list_exec_source_file.file,
                0, 0, 0,
                mi_command->variant.file_list_exec_source_file.line);

        gdbwire_mi_command_free(mi_command);
    }
}

static void commands_process_info_frame(struct commands *c,
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
            commands_send_source_file(c, frame->fullname, frame->file,
                    address, frame->from, frame->func, frame->line);
        } else {
            require_source = true;
        }

        gdbwire_mi_command_free(mi_command);
    } else {
        require_source = true;
    }

    if (require_source) {
        commands_issue_command(c, COMMAND_INFO_SOURCE, NULL, 1);
    }
}

static void gdbwire_stream_record_callback(void *context,
    struct gdbwire_mi_stream_record *stream_record)
{
    struct commands *c = (struct commands*)context;

    switch (c->cur_command_kind) {
        case COMMAND_BREAKPOINTS:
        case COMMAND_INFO_FRAME:
            /**
             * When using GDB with annotate=2 and also using interpreter-exec,
             * GDB spits out the annotations in the MI output. All of these
             * annotations can be ignored.
             */
            break;
        case COMMAND_DISASSEMBLE_PC:
        case COMMAND_DISASSEMBLE_FUNC:
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

                sbpush(c->disasm, cgdb_strdup(str));

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
                    c->address_start = c->address_start ?
                         MIN(address, c->address_start) : address;
                    c->address_end = MAX(address, c->address_end);
                }
            }
            break;
        case COMMAND_COMPLETE:
            if (stream_record->kind == GDBWIRE_MI_CONSOLE) {
                char *str = stream_record->cstring;
                size_t length = strlen(str);
                if (str[length-1] == '\n') {
                    str[length-1] = 0;
                }
                sbpush(c->completions, cgdb_strdup(str));
            }
            break;
        case COMMAND_DATA_DISASSEMBLE_MODE_QUERY:
        case COMMAND_USER_COMMAND:
        case COMMAND_INFO_SOURCE:
        case COMMAND_INFO_SOURCES:
        case COMMAND_TTY:
            break;
    }
}

static void gdbwire_async_record_callback(void *context,
        struct gdbwire_mi_async_record *async_record)
{
}

static void gdbwire_result_record_callback(void *context,
        struct gdbwire_mi_result_record *result_record)
{
    struct commands *c = (struct commands*)context;

    switch (c->cur_command_kind) {
        case COMMAND_BREAKPOINTS:
            commands_process_breakpoints(c, result_record);
            break;
        case COMMAND_INFO_SOURCES:
            commands_process_info_sources(c, result_record);
            break;
        case COMMAND_DISASSEMBLE_PC:
        case COMMAND_DISASSEMBLE_FUNC:
            send_disassemble_func_complete_response(c, result_record);
            break;
        case COMMAND_COMPLETE:
            send_command_complete_response(c);
            break;
        case COMMAND_DATA_DISASSEMBLE_MODE_QUERY:
            /**
             * If the mode was to high, than the result record would be
             * an error, meaning the mode is not supported. Otherwise,
             * the mode is supported.
             */
            if (result_record->result_class == GDBWIRE_MI_DONE) {
                c->disassemble_supports_s_mode = 1;
                clog_info(CLOG_GDBIO, "disassemble supports s mode");
            }
            break;
        case COMMAND_INFO_SOURCE:
            commands_process_info_source(c, result_record);
            break;
        case COMMAND_INFO_FRAME:
            commands_process_info_frame(c, result_record);
            break;
        case COMMAND_TTY:
        case COMMAND_USER_COMMAND:
            break;
    }
}

static void gdbwire_prompt_callback(void *context, const char *prompt)
{
}

static void gdbwire_parse_error_callback(void *context, const char *mi,
            const char *token, struct gdbwire_mi_position position)
{
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

struct commands *commands_initialize(struct tgdb *tgdb)
{
    struct commands *c =
            (struct commands *) cgdb_malloc(sizeof (struct commands));
    c->tgdb = tgdb;
    c->cur_command_kind = COMMAND_USER_COMMAND;
    c->completions = NULL;

    c->disasm = NULL;
    c->address_start = 0;
    c->address_end = 0;

    struct gdbwire_callbacks callbacks = wire_callbacks;
    callbacks.context = (void*)c;
    c->wire = gdbwire_create(callbacks);

    c->disassemble_supports_s_mode = 0;
    c->is_console_command = true;

    return c;
}

int free_char_star(void *item)
{
    char *s = (char *) item;

    free(s);
    s = NULL;

    return 0;
}

void commands_shutdown(struct commands *c)
{
    if (c == NULL)
        return;

    /* TODO: free source_files queue */

    gdbwire_destroy(c->wire);

    free(c);
    c = NULL;
}

void commands_process(struct commands *c, const std::string &str)
{
   gdbwire_push_data(c->wire, str.data(), str.size());
}

void commands_process_error(struct commands *c)
{
    c->is_console_command = false;
}

void
commands_prepare_for_command(struct commands *c, struct tgdb_command *com)
{
    /* Set the commands state to nothing */
    c->is_console_command = (com->command_choice == TGDB_COMMAND_CONSOLE);
    c->cur_command_kind = com->kind;
}

/** 
 * This is responsible for creating a command to run through the debugger.
 *
 * \param com 
 * The annotate command to run
 *
 * \param data 
 * Information that may be needed to create the command
 *
 * \return
 * A command ready to be run through the debugger or NULL on error.
 * The memory is malloc'd, and must be freed.
 */
static char *create_gdb_command(struct commands *c,
        enum command_kind kind, const char *data)
{
    switch (kind) {
        case COMMAND_INFO_SOURCES:
            return strdup("server interpreter-exec mi \"-file-list-exec-source-files\"\n");
        case COMMAND_INFO_SOURCE:
            /* server info source */
            return strdup("server interpreter-exec mi \"-file-list-exec-source-file\"\n");
        case COMMAND_INFO_FRAME:
            /* server info frame */
            return strdup("server interpreter-exec mi \"-stack-info-frame\"\n");
        case COMMAND_DISASSEMBLE_PC:
            return sys_aprintf("server interpreter-exec mi \"x/%si $pc\"\n", data);
        case COMMAND_DISASSEMBLE_FUNC:
            /* disassemble 'driver.cpp'::main
                 /m: source lines included
                 /s: source lines included, output in pc order
                 /r: raw instructions included in hex
                 single argument: function surrounding is dumped
                 two arguments: start,end or start,+length
                 disassemble 'driver.cpp'::main
                 interp mi "disassemble /s 'driver.cpp'::main,+10"
                 interp mi "disassemble /r 'driver.cpp'::main,+10"
             */
            return sys_aprintf("server interp mi \"disassemble%s%s\"\n",
                               data ? " " : "", data ? data : "");
        case COMMAND_BREAKPOINTS:
            return strdup("server interpreter-exec mi \"-break-info\"\n");
        case COMMAND_TTY:
            /* server tty %s */
            return sys_aprintf("server interpreter-exec mi \"-inferior-tty-set %s\"\n", data);
        case COMMAND_COMPLETE:
            /* server complete */
            return sys_aprintf("server interpreter-exec mi \"complete %s\"\n", data);
        case COMMAND_DATA_DISASSEMBLE_MODE_QUERY:
            return sys_aprintf("server interpreter-exec mi \"-data-disassemble -s 0 -e 0 -- 4\"\n");
        default:
            clog_error(CLOG_CGDB, "switch error");
            break;
    };

    return NULL;
}

struct tgdb_command *tgdb_command_create(const char *gdb_command,
        enum tgdb_command_choice command_choice, enum command_kind kind)
{
    struct tgdb_command *tc;

    tc = (struct tgdb_command *) cgdb_malloc(sizeof (struct tgdb_command));

    tc->gdb_command = gdb_command ? strdup(gdb_command) : NULL;
    tc->command_choice = command_choice;
    tc->kind = kind;

    return tc;
}

void tgdb_command_destroy(struct tgdb_command *tc)
{
    free(tc->gdb_command);
    free(tc);
}

int commands_issue_command(struct commands *c,
    enum command_kind kind, const char *data, int oob)
{
    char *ncom = create_gdb_command(c, kind, data);
    struct tgdb_command *client_command = NULL;

    enum tgdb_command_choice choice = (oob == 1) ?
           TGDB_COMMAND_TGDB_CLIENT_PRIORITY : TGDB_COMMAND_TGDB_CLIENT;

    /* This should send the command to tgdb-base to handle */
    client_command = tgdb_command_create(ncom, choice, kind);

    free(ncom);

    tgdb_run_or_queue_command(c->tgdb, client_command);
    return 0;
}

int commands_disassemble_supports_s_mode(struct commands *c)
{
    return c->disassemble_supports_s_mode;
}

bool commands_is_console_command(struct commands *c)
{
    return c->is_console_command;
}
