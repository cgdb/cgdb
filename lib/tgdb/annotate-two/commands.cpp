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

#if HAVE_REGEX_H
#include <regex.h>
#endif /* HAVE_REGEX_H */

/* Local includes */
#include "commands.h"
#include "data.h"
#include "io.h"
#include "tgdb_types.h"
#include "globals.h"
#include "logger.h"
#include "sys_util.h"
#include "queue.h"
#include "ibuf.h"
#include "a2-tgdb.h"
#include "queue.h"
#include "tgdb_list.h"
#include "annotate_two.h"
#include "gdbwire.h"

/**
 * This structure represents most of the I/O parsing state of the 
 * annotate_two subsytem.
 */
struct commands {

  /** The current line number the debugger is at in the inferior.  */
    struct ibuf *line_number;

  /** The state of the command context.  */
    enum COMMAND_STATE cur_command_state;

  /** 'info source' information */

    /*@{ */

  /** The current info source line being parsed */
    struct ibuf *info_source_string;

    /*@} */

    /* info sources information {{{ */
    /*@{ */

  /** All of the sources.  */
    struct ibuf *info_sources_string;

    /*@} */
    /* }}} */

    /* tab completion information {{{ */
    /*@{ */

  /** All of the tab completion items, parsed in put in a list, 1 at a time. */
    struct tgdb_list *tab_completions;

    /*@} */
    /* }}} */

    /**
     * A complete hack.
     *
     * The commands interface currently needs to append responses to
     * a data structure that comes from another module. I think after
     * the remaining tgdb refactors are done, this hack will be easy to
     * remove.
     */
    struct tgdb_list *response_list;

    /**
     * The gdbwire context to talk to GDB with.
     */
    struct gdbwire *wire;
};

static void gdbwire_stream_record_callback(void *context,
    struct gdbwire_mi_stream_record *stream_record)
{
    struct commands *c = (struct commands*)context;

    switch (c->cur_command_state) {
        case INFO_BREAKPOINTS:
            /**
             * When using GDB with annotate=2 and also using interpreter-exec,
             * GDB spits out the annotations in the MI output. All of these
             * annotations can be ignored.
             */
            break;
        case COMMAND_COMPLETE:
            if (stream_record->kind == GDBWIRE_MI_CONSOLE) {
                char *str = stream_record->cstring;
                size_t length = strlen(str);
                if (str[length-1] == '\n') {
                    str[length-1] = 0;
                }
                tgdb_list_append(c->tab_completions, cgdb_strdup(str));
            }
            break;
    }

}

static void gdbwire_async_record_callback(void *context,
        struct gdbwire_mi_async_record *async_record)
{
}

static void send_command_complete_response(struct commands *c)
{
    struct tgdb_response *response = (struct tgdb_response *)
        cgdb_malloc(sizeof (struct tgdb_response));
    response->header = TGDB_UPDATE_COMPLETIONS;
    response->choice.update_completions.completion_list =
        c->tab_completions;
    tgdb_types_append_command(c->response_list, response);
}

static void
commands_send_breakpoints(struct commands *c, struct tgdb_list *bkpt_list)
{
    struct tgdb_response *response = (struct tgdb_response *)
        cgdb_malloc(sizeof (struct tgdb_response));

    response->header = TGDB_UPDATE_BREAKPOINTS;
    response->choice.update_breakpoints.breakpoint_list = bkpt_list;

    tgdb_types_append_command(c->response_list, response);
}

static void commands_process_breakpoint(
        struct tgdb_list *breakpoint_list,
        struct gdbwire_mi_breakpoint *breakpoint)
{
    if ((breakpoint->fullname || breakpoint->file) && breakpoint->line != 0) {
        struct tgdb_breakpoint *tb = (struct tgdb_breakpoint *)
            cgdb_malloc(sizeof (struct tgdb_breakpoint));
        tb->file = cgdb_strdup(breakpoint->file);
        tb->fullname = cgdb_strdup(breakpoint->fullname);
        tb->line = breakpoint->line;
        tb->enabled = breakpoint->enabled;

        tgdb_list_append(breakpoint_list, tb);
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
        struct tgdb_list *bkpt_list = tgdb_list_init();
        struct gdbwire_mi_breakpoint *breakpoint =
            mi_command->variant.break_info.breakpoints;
        while (breakpoint) {
            commands_process_breakpoint(bkpt_list, breakpoint);

            if (breakpoint->multi) {
                struct gdbwire_mi_breakpoint *multi_bkpt =
                    breakpoint->multi_breakpoints;
                while (multi_bkpt) {
                    commands_process_breakpoint(bkpt_list, multi_bkpt);
                    multi_bkpt = multi_bkpt->next;
                }
            }

            breakpoint = breakpoint->next;
        }

        commands_send_breakpoints(c, bkpt_list);

        gdbwire_mi_command_free(mi_command);
    }
}

static void gdbwire_result_record_callback(void *context,
        struct gdbwire_mi_result_record *result_record)
{
    if (result_record->result_class == GDBWIRE_MI_DONE) {
        struct commands *c = (struct commands*)context;

        switch (c->cur_command_state) {
            case INFO_BREAKPOINTS:
                commands_process_breakpoints(c, result_record);
                break;
            case COMMAND_COMPLETE:
                send_command_complete_response(c);
                break;
        }
        commands_set_state(c, VOID_COMMAND, NULL);
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

struct commands *commands_initialize(void)
{

    struct commands *c =
            (struct commands *) cgdb_malloc(sizeof (struct commands));
    c->line_number = ibuf_init();

    c->cur_command_state = VOID_COMMAND;

    c->info_source_string = ibuf_init();

    c->info_sources_string = ibuf_init();

    c->tab_completions = tgdb_list_init();

    struct gdbwire_callbacks callbacks = wire_callbacks;
    callbacks.context = (void*)c;
    c->wire = gdbwire_create(callbacks);

    return c;
}

int free_breakpoint(void *item)
{
    struct tgdb_breakpoint *bp = (struct tgdb_breakpoint *) item;

    if (bp->file) {
        free(bp->file);
        bp->file = NULL;
    }

    if (bp->fullname) {
        free(bp->fullname);
        bp->fullname = NULL;
    }

    free(bp);
    bp = NULL;

    return 0;
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

    ibuf_free(c->line_number);
    c->line_number = NULL;

    ibuf_free(c->info_source_string);
    c->info_source_string = NULL;

    ibuf_free(c->info_sources_string);
    c->info_sources_string = NULL;

    tgdb_list_destroy(c->tab_completions);

    /* TODO: free source_files queue */

    gdbwire_destroy(c->wire);

    free(c);
    c = NULL;
}

void
commands_set_state(struct commands *c,
        enum COMMAND_STATE state, struct tgdb_list *list)
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
    ibuf_clear(c->info_source_string);

    commands_set_state(c, INFO_SOURCE, NULL);
}

static void
commands_send_source_file(struct commands *c, char *fullname, char *file,
        int line, struct tgdb_list *list)
{
    /* This section allocates a new structure to add into the queue 
     * All of its members will need to be freed later.
     */
    struct tgdb_file_position *tfp = (struct tgdb_file_position *)
            cgdb_malloc(sizeof (struct tgdb_file_position));
    struct tgdb_response *response = (struct tgdb_response *)
            cgdb_malloc(sizeof (struct tgdb_response));

    tfp->absolute_path = (fullname)?strdup(fullname):0;
    tfp->relative_path = strdup(file);
    tfp->line_number = line;

    response->header = TGDB_UPDATE_FILE_POSITION;
    response->choice.update_file_position.file_position = tfp;

    tgdb_types_append_command(list, response);
}

static void commands_process_info_source(struct commands *c, char a,
        struct tgdb_list *list)
{
    ibuf_addchar(c->info_source_string, a);

    if (a == '\n') {
        enum gdbwire_result result;
        struct gdbwire_mi_command *mi_command = 0;
        result = gdbwire_interpreter_exec(ibuf_get(c->info_source_string),
            GDBWIRE_MI_FILE_LIST_EXEC_SOURCE_FILE, &mi_command);
        if (result == GDBWIRE_OK) {
            commands_send_source_file(c,
                    mi_command->variant.file_list_exec_source_file.fullname,
                    mi_command->variant.file_list_exec_source_file.file,
                    mi_command->variant.file_list_exec_source_file.line, list);

            gdbwire_mi_command_free(mi_command);
        }

        ibuf_clear(c->info_source_string);
    }
}

static void commands_send_source_files(struct commands *c,
        struct tgdb_list *list, struct tgdb_list *source_files)
{
    struct tgdb_response *response = (struct tgdb_response *)
            cgdb_malloc(sizeof (struct tgdb_response));

    response->header = TGDB_UPDATE_SOURCE_FILES;
    response->choice.update_source_files.source_files = source_files;
    tgdb_types_append_command(list, response);
}

/* This function is capable of parsing the output of 'info source'.
 * It can get both the absolute and relative path to the source file.
 */
static void
commands_process_info_sources(struct commands *c, char a,
        struct tgdb_list *list)
{
    ibuf_addchar(c->info_sources_string, a);

    if (a == '\n') {
        enum gdbwire_result result;
        struct gdbwire_mi_command *mi_command = 0;
        result = gdbwire_interpreter_exec(ibuf_get(c->info_sources_string),
            GDBWIRE_MI_FILE_LIST_EXEC_SOURCE_FILES, &mi_command);
        if (result == GDBWIRE_OK) {
            struct tgdb_list *source_files = tgdb_list_init();
            struct gdbwire_mi_source_file *files =
                mi_command->variant.file_list_exec_source_files.files;
            while (files) {
                char *file = (files->fullname)?files->fullname:files->file;
                tgdb_list_append(source_files, strdup(file));
                files = files->next;
            }

            commands_send_source_files(c, list, source_files);

            gdbwire_mi_command_free(mi_command);

        }

        ibuf_clear(c->info_sources_string);
    }
}

void commands_process(struct commands *c, char a, struct tgdb_list *list)
{
    // Wow, this is ugly, but I think by the time I'm done with Michael's
    // patches this whole mess will be unraveled.
    c->response_list = list;

    if (commands_get_state(c) == INFO_SOURCES) {
        commands_process_info_sources(c, a, list);
    } else if (commands_get_state(c) == INFO_BREAKPOINTS) {
        gdbwire_push_data(c->wire, &a, 1);
    } else if (commands_get_state(c) == COMMAND_COMPLETE) {
        gdbwire_push_data(c->wire, &a, 1);
    } else if (commands_get_state(c) == INFO_SOURCE) {
        commands_process_info_source(c, a, list);
    }
}

int
commands_prepare_for_command(struct annotate_two *a2,
        struct commands *c, struct tgdb_command *com)
{

    enum annotate_commands *a_com =
            (enum annotate_commands *) com->tgdb_client_private_data;

    /* Set the commands state to nothing */
    commands_set_state(c, VOID_COMMAND, NULL);

    if (a_com == NULL) {
        data_set_state(a2, USER_COMMAND);
        return 0;
    }

    switch (*a_com) {
        case ANNOTATE_INFO_SOURCES:
            ibuf_clear(c->info_sources_string);
            commands_set_state(c, INFO_SOURCES, NULL);
            break;
        case ANNOTATE_INFO_SOURCE:
            commands_prepare_info_source(a2, c);
            break;
        case ANNOTATE_INFO_BREAKPOINTS:
            commands_set_state(c, INFO_BREAKPOINTS, NULL);
            break;
        case ANNOTATE_TTY:
            break;              /* Nothing to do */
        case ANNOTATE_COMPLETE:
            commands_set_state(c, COMMAND_COMPLETE, NULL);
            io_debug_write_fmt("<%s\n>", com->tgdb_command_data);
            break;
        case ANNOTATE_VOID:
            break;
        default:
            logger_write_pos(logger, __FILE__, __LINE__,
                    "commands_prepare_for_command error");
            break;
    };

    data_set_state(a2, INTERNAL_COMMAND);
    io_debug_write_fmt("<%s\n>", com->tgdb_command_data);

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
static char *commands_create_command(struct commands *c,
        enum annotate_commands com, const char *data)
{
    char *ncom = NULL;

    switch (com) {
        case ANNOTATE_INFO_SOURCES:
            ncom = strdup("server interpreter-exec mi \"-file-list-exec-source-files\"\n");
            break;
        case ANNOTATE_INFO_SOURCE:
            /* server info source */
            ncom = strdup("server interpreter-exec mi \"-file-list-exec-source-file\"\n");
            break;
        case ANNOTATE_INFO_BREAKPOINTS:
            ncom = strdup("server interpreter-exec mi \"-break-info\"\n");
            break;
        case ANNOTATE_TTY:
            /* server tty %s */
            ncom = sys_aprintf("server interpreter-exec mi \"-inferior-tty-set %s\"\n", data);
            break;
        case ANNOTATE_COMPLETE:
            /* server complete */
            ncom = sys_aprintf("server interpreter-exec mi \"complete %s\"\n", data);
            break;
        case ANNOTATE_VOID:
        default:
            logger_write_pos(logger, __FILE__, __LINE__, "switch error");
            break;
    };

    return ncom;
}

int
commands_user_ran_command(struct commands *c,
        struct tgdb_list *client_command_list)
{
    if (commands_issue_command(c,
                    client_command_list,
                    ANNOTATE_INFO_BREAKPOINTS, NULL, 0) == -1) {
        logger_write_pos(logger, __FILE__, __LINE__,
                "commands_issue_command error");
        return -1;
    }
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
    if (commands_issue_command(c,
                    client_command_list, ANNOTATE_INFO_LINE, NULL, 0) == -1) {
        logger_write_pos(logger, __FILE__, __LINE__,
                "commands_issue_command error");
        return -1;
    }
#endif

    return 0;
}

int
commands_issue_command(struct commands *c,
        struct tgdb_list *client_command_list,
        enum annotate_commands com, const char *data, int oob)
{
    char *ncom = commands_create_command(c, com, data);
    struct tgdb_command *client_command = NULL;
    enum annotate_commands *nacom =
            (enum annotate_commands *) cgdb_malloc(sizeof (enum
                    annotate_commands));

    *nacom = com;

    if (ncom == NULL) {
        logger_write_pos(logger, __FILE__, __LINE__,
                "commands_issue_command error");
        return -1;
    }

    /* This should send the command to tgdb-base to handle */
    if (oob == 0) {
        client_command = tgdb_command_create(ncom, TGDB_COMMAND_TGDB_CLIENT,
                (void *) nacom);
    } else if (oob == 1) {
        client_command = tgdb_command_create(ncom,
                TGDB_COMMAND_TGDB_CLIENT_PRIORITY, (void *) nacom);
    } else if (oob == 4) {
        client_command = tgdb_command_create(ncom, TGDB_COMMAND_TGDB_CLIENT,
                (void *) nacom);
    }

    if (ncom) {
        free(ncom);
        ncom = NULL;
    }

    /* Append to the command_container the commands */
    tgdb_list_append(client_command_list, client_command);

    return 0;
}
