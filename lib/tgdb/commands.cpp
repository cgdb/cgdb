#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif /* HAVE_STDLIB_H */

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */

#if HAVE_CTYPE_H
#include <ctype.h>
#endif

/* Local includes */
#include "commands.h"
#include "io.h"
#include "tgdb_types.h"
#include "logger.h"
#include "sys_util.h"
#include "queue.h"
#include "ibuf.h"
#include "a2-tgdb.h"
#include "queue.h"
#include "annotate_two.h"
#include "gdbwire.h"
#include "state_machine.h"

/**
 * This structure represents most of the I/O parsing state of the 
 * annotate_two subsytem.
 */
struct commands {
    /**
     * The annotate two data structure this command structure corresponds
     * with. Perhaps combine these two at one point.
     */
    struct annotate_two *a2;

    /**
     * The state of the command context.
     */
    enum COMMAND_STATE cur_command_state;

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
};

static void
commands_send_breakpoints(struct annotate_two *a2,
    struct tgdb_breakpoint *breakpoints)
{
    struct tgdb_response *response = (struct tgdb_response *)
        tgdb_create_response(TGDB_UPDATE_BREAKPOINTS);

    response->choice.update_breakpoints.breakpoints = breakpoints;

    sbpush(a2->responses, response);
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

static void commands_process_breakpoints(struct annotate_two *a2,
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

        commands_send_breakpoints(a2, breakpoints);

        gdbwire_mi_command_free(mi_command);
    }
}

static void commands_send_source_files(struct annotate_two *a2,
        char **source_files)
{
    struct tgdb_response *response =
        tgdb_create_response(TGDB_UPDATE_SOURCE_FILES);
    response->choice.update_source_files.source_files = source_files;
    sbpush(a2->responses, response);
}

/* This function is capable of parsing the output of 'info source'.
 * It can get both the absolute and relative path to the source file.
 */
static void
commands_process_info_sources(struct annotate_two *a2,
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

        commands_send_source_files(a2, source_files);

        gdbwire_mi_command_free(mi_command);
    }
}

static void send_disassemble_func_complete_response(struct annotate_two *a2,
        struct gdbwire_mi_result_record *result_record)
{
    struct tgdb_response *response =
        tgdb_create_response(TGDB_DISASSEMBLE_FUNC);

    response->choice.disassemble_function.error = 
        (result_record->result_class == GDBWIRE_MI_ERROR);
            
    response->choice.disassemble_function.disasm = a2->c->disasm;
    response->choice.disassemble_function.addr_start = a2->c->address_start;
    response->choice.disassemble_function.addr_end = a2->c->address_end;
    sbpush(a2->responses, response);
}

static void send_command_complete_response(struct annotate_two *a2)
{
    struct tgdb_response *response =
        tgdb_create_response(TGDB_UPDATE_COMPLETIONS);
    response->choice.update_completions.completions = a2->c->completions;
    sbpush(a2->responses, response);
}

static void
commands_send_source_file(struct annotate_two *a2, char *fullname, char *file,
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

    sbpush(a2->responses, response);
}

static void commands_process_info_source(struct annotate_two *a2,
        struct gdbwire_mi_result_record *result_record)
{
    enum gdbwire_result result;
    struct gdbwire_mi_command *mi_command = 0;
    result = gdbwire_get_mi_command(GDBWIRE_MI_FILE_LIST_EXEC_SOURCE_FILE,
        result_record, &mi_command);
    if (result == GDBWIRE_OK) {
        commands_send_source_file(a2,
                mi_command->variant.file_list_exec_source_file.fullname,
                mi_command->variant.file_list_exec_source_file.file,
                0, 0, 0,
                mi_command->variant.file_list_exec_source_file.line);

        gdbwire_mi_command_free(mi_command);
    }
}

static void commands_process_info_frame(struct annotate_two *a2,
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
            commands_send_source_file(a2, frame->fullname, frame->file,
                    address, frame->from, frame->func, frame->line);
        } else {
            require_source = true;
        }

        gdbwire_mi_command_free(mi_command);
    } else {
        require_source = true;
    }

    if (require_source) {
        commands_issue_command(a2, ANNOTATE_INFO_SOURCE, NULL, 1);
    }
}

static void gdbwire_stream_record_callback(void *context,
    struct gdbwire_mi_stream_record *stream_record)
{
    struct annotate_two *a2 = (struct annotate_two*)context;
    struct commands *c = a2->c;

    switch (c->cur_command_state) {
        case INFO_BREAKPOINTS:
        case INFO_FRAME:
            /**
             * When using GDB with annotate=2 and also using interpreter-exec,
             * GDB spits out the annotations in the MI output. All of these
             * annotations can be ignored.
             */
            break;
        case INFO_DISASSEMBLE_PC:
        case INFO_DISASSEMBLE_FUNC:
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
        case DATA_DISASSEMBLE_MODE_QUERY:
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
    struct annotate_two *a2 = (struct annotate_two*)context;
    struct commands *c = a2->c;

    switch (c->cur_command_state) {
        case INFO_BREAKPOINTS:
            commands_process_breakpoints(a2, result_record);
            break;
        case INFO_SOURCES:
            commands_process_info_sources(a2, result_record);
            break;
        case INFO_DISASSEMBLE_PC:
        case INFO_DISASSEMBLE_FUNC:
            send_disassemble_func_complete_response(a2, result_record);
            break;
        case COMMAND_COMPLETE:
            send_command_complete_response(a2);
            break;
        case DATA_DISASSEMBLE_MODE_QUERY:
            /**
             * If the mode was to high, than the result record would be
             * an error, meaning the mode is not supported. Otherwise,
             * the mode is supported.
             */
            if (result_record->result_class == GDBWIRE_MI_DONE) {
                c->disassemble_supports_s_mode = 1;
            }
            
            break;
        case INFO_SOURCE:
            commands_process_info_source(a2, result_record);
            break;
        case INFO_FRAME:
            commands_process_info_frame(a2, result_record);
            break;
    }
    commands_set_state(c, VOID_COMMAND);
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

struct commands *commands_initialize(struct annotate_two *a2)
{
    struct commands *c =
            (struct commands *) cgdb_malloc(sizeof (struct commands));
    c->a2 = a2;
    c->cur_command_state = VOID_COMMAND;
    c->completions = NULL;

    c->disasm = NULL;

    struct gdbwire_callbacks callbacks = wire_callbacks;
    callbacks.context = (void*)c->a2;
    c->wire = gdbwire_create(callbacks);

    c->disassemble_supports_s_mode = 0;

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

void
commands_set_state(struct commands *c, enum COMMAND_STATE state)
{
    c->cur_command_state = state;
}

enum COMMAND_STATE commands_get_state(struct commands *c)
{
    return c->cur_command_state;
}

static void
commands_prepare_info_source(struct annotate_two *a2, struct commands *c)
{
    data_set_state(a2, INTERNAL_COMMAND);
    commands_set_state(c, INFO_SOURCE);
}

void commands_process(struct commands *c, char a)
{
    switch (commands_get_state(c)) {
        case VOID_COMMAND:
            break;
        default:
            gdbwire_push_data(c->wire, &a, 1);
            break;
    }
}

int
commands_prepare_for_command(struct annotate_two *a2,
        struct commands *c, struct tgdb_command *com)
{
    enum annotate_commands a_com = com->command;

    /* Set the commands state to nothing */
    commands_set_state(c, VOID_COMMAND);

    if (a_com == -1) {
        data_set_state(a2, USER_COMMAND);
        return 0;
    }

    switch (a_com) {
        case ANNOTATE_INFO_SOURCES:
            commands_set_state(c, INFO_SOURCES);
            break;
        case ANNOTATE_INFO_SOURCE:
            commands_prepare_info_source(a2, c);
            break;
        case ANNOTATE_INFO_FRAME:
            data_set_state(a2, INTERNAL_COMMAND);
            commands_set_state(c, INFO_FRAME);
            break;
        case ANNOTATE_INFO_BREAKPOINTS:
            commands_set_state(c, INFO_BREAKPOINTS);
            break;
        case ANNOTATE_TTY:
            break;              /* Nothing to do */
        case ANNOTATE_COMPLETE:
            c->completions = 0;
            commands_set_state(c, COMMAND_COMPLETE);
            io_debug_write_fmt("<%s\n>", com->gdb_command);
            break;
        case ANNOTATE_DATA_DISASSEMBLE_MODE_QUERY:
            c->disassemble_supports_s_mode = 0;

            commands_set_state(c, DATA_DISASSEMBLE_MODE_QUERY);
            break;
        case ANNOTATE_DISASSEMBLE_PC:
            c->disasm = 0;
            c->address_start = 0;
            c->address_end = 0;
            
            commands_set_state(c, INFO_DISASSEMBLE_PC);
            break;
        case ANNOTATE_DISASSEMBLE_FUNC:
            c->disasm = 0;
            c->address_start = 0;
            c->address_end = 0;
            
            commands_set_state(c, INFO_DISASSEMBLE_FUNC);
            break;
        default:
            logger_write_pos(logger, __FILE__, __LINE__,
                    "commands_prepare_for_command error");
            break;
    };

    data_set_state(a2, INTERNAL_COMMAND);
    io_debug_write_fmt("<%s\n>", com->gdb_command);

    return 0;
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
        enum annotate_commands com, const char *data)
{
    switch (com) {
        case ANNOTATE_INFO_SOURCES:
            return strdup("server interpreter-exec mi \"-file-list-exec-source-files\"\n");
        case ANNOTATE_INFO_SOURCE:
            /* server info source */
            return strdup("server interpreter-exec mi \"-file-list-exec-source-file\"\n");
        case ANNOTATE_INFO_FRAME:
            /* server info frame */
            return strdup("server interpreter-exec mi \"-stack-info-frame\"\n");
        case ANNOTATE_DISASSEMBLE_PC:
            return sys_aprintf("server interpreter-exec mi \"x/%si $pc\"\n", data);
        case ANNOTATE_DISASSEMBLE_FUNC:
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
        case ANNOTATE_INFO_BREAKPOINTS:
            return strdup("server interpreter-exec mi \"-break-info\"\n");
        case ANNOTATE_TTY:
            /* server tty %s */
            return sys_aprintf("server interpreter-exec mi \"-inferior-tty-set %s\"\n", data);
        case ANNOTATE_COMPLETE:
            /* server complete */
            return sys_aprintf("server interpreter-exec mi \"complete %s\"\n", data);
        case ANNOTATE_DATA_DISASSEMBLE_MODE_QUERY:
            return sys_aprintf("server interpreter-exec mi \"-data-disassemble -s 0 -e 0 -- 4\"\n");
        default:
            logger_write_pos(logger, __FILE__, __LINE__, "switch error");
            break;
    };

    return NULL;
}

struct tgdb_command *tgdb_command_create(const char *gdb_command,
        enum tgdb_command_choice command_choice,
        enum annotate_commands command)
{
    struct tgdb_command *tc;

    tc = (struct tgdb_command *) cgdb_malloc(sizeof (struct tgdb_command));

    tc->gdb_command = gdb_command? strdup(gdb_command) : NULL;
    tc->command_choice = command_choice;
    tc->command = command;

    return tc;
}

void tgdb_command_destroy(void *item)
{
    struct tgdb_command *tc = (struct tgdb_command *) item;

    free(tc->gdb_command);
    free(tc);
}

int
commands_user_ran_command(struct annotate_two *a2) {
    return commands_issue_command(a2, ANNOTATE_INFO_BREAKPOINTS, NULL, 0);

#if 0
    /* This was added to allow support for TGDB to tell the FE when the user
     * switched locations due to a 'list foo:1' command. The info line would
     * get issued and the FE would know exactly what GDB was currently looking
     * at. However, it was noticed that the FE couldn't distinguish between when
     * a new file location should be displayed, or when a new file location 
     * shouldn't be displayed. For instance, if the user moves around in the
     * source window, and then types 'p argc' it would then get the original
     * position it was just at and the FE would show that spot again, but this
     * isn't necessarily what the FE wants.
     */
    return commands_issue_command(c,
                    client_command_list, ANNOTATE_INFO_LINE, NULL, 0);
#endif
}

int commands_issue_command(struct annotate_two *a2,
    enum annotate_commands command, const char *data, int oob)
{
    char *ncom = create_gdb_command(a2->c, command, data);
    struct tgdb_command *client_command = NULL;

    enum tgdb_command_choice choice = (oob == 1) ?
           TGDB_COMMAND_TGDB_CLIENT_PRIORITY : TGDB_COMMAND_TGDB_CLIENT;

    /* This should send the command to tgdb-base to handle */
    client_command = tgdb_command_create(ncom, choice, command);

    /* Append to the command_container the commands */
    sbpush(a2->client_commands, client_command);

    free(ncom);
    return 0;
}

int commands_disassemble_supports_s_mode(struct commands *c)
{
    return c->disassemble_supports_s_mode;
}
