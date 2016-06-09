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

  /** breakpoint information */
    /*@{ */

  /** The current breakpoint being parsed.  */
    struct ibuf *breakpoint_string;

    /*@} */

  /** 'info source' information */

    /*@{ */

  /** The current info source line being parsed */
    struct ibuf *info_source_string;

    /*@} */

    /* info sources information {{{ */
    /*@{ */

  /** ??? Finished parsing the data being looked for.  */
    int sources_ready;

  /** All of the sources.  */
    struct ibuf *info_sources_string;

  /** All of the source, parsed in put in a list, 1 at a time.  */
    struct tgdb_list *inferior_source_files;

    /*@} */
    /* }}} */

    /* tab completion information {{{ */
    /*@{ */

  /** ??? Finished parsing the data being looked for.  */
    int tab_completion_ready;

  /** A tab completion item */
    struct ibuf *tab_completion_string;

  /** All of the tab completion items, parsed in put in a list, 1 at a time. */
    struct tgdb_list *tab_completions;

    /*@} */
    /* }}} */
};

struct commands *commands_initialize(void)
{
    struct commands *c =
            (struct commands *) cgdb_malloc(sizeof (struct commands));
    c->line_number = ibuf_init();

    c->cur_command_state = VOID_COMMAND;

    c->breakpoint_string = ibuf_init();

    c->info_source_string = ibuf_init();

    c->sources_ready = 0;
    c->info_sources_string = ibuf_init();
    c->inferior_source_files = tgdb_list_init();

    c->tab_completion_ready = 0;
    c->tab_completion_string = ibuf_init();
    c->tab_completions = tgdb_list_init();

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

    ibuf_free(c->breakpoint_string);
    c->breakpoint_string = NULL;

    ibuf_free(c->info_source_string);
    c->info_source_string = NULL;

    ibuf_free(c->info_sources_string);
    c->info_sources_string = NULL;

    ibuf_free(c->tab_completion_string);
    c->tab_completion_string = NULL;

    tgdb_list_destroy(c->tab_completions);

    tgdb_list_free(c->inferior_source_files, free_char_star);
    tgdb_list_destroy(c->inferior_source_files);

    /* TODO: free source_files queue */

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

/* This function is capable of parsing the output of 'info source'.
 * It can get both the absolute and relative path to the source file.
 */
static void
commands_process_info_sources(struct commands *c, char a)
{
    ibuf_addchar(c->info_sources_string, a);

    if (a == '\n') {
        enum gdbwire_result result;
        struct gdbwire_mi_command *mi_command = 0;
        result = gdbwire_interpreter_exec(ibuf_get(c->info_sources_string),
            GDBWIRE_MI_FILE_LIST_EXEC_SOURCE_FILES, &mi_command);
        if (result == GDBWIRE_OK) {
            struct gdbwire_mi_source_file *files =
                mi_command->variant.file_list_exec_source_files.files;
            while (files) {
                char *file = (files->fullname)?files->fullname:files->file;
                tgdb_list_append(c->inferior_source_files, strdup(file));
                files = files->next;
            }

            gdbwire_mi_command_free(mi_command);
        }

        c->sources_ready = 1;
        ibuf_clear(c->info_sources_string);
    }
}

static void
commands_send_breakpoints(struct tgdb_list *list, struct tgdb_list *bkpt_list)
{
    struct tgdb_response *response = (struct tgdb_response *)
        cgdb_malloc(sizeof (struct tgdb_response));

    response->header = TGDB_UPDATE_BREAKPOINTS;
    response->choice.update_breakpoints.breakpoint_list = bkpt_list;

    tgdb_types_append_command(list, response);
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

static void commands_process_breakpoints(struct commands *c, char a,
        struct tgdb_list *list)
{
    ibuf_addchar(c->breakpoint_string, a);

    if (a == '\n') {
        /**
         * When using GDB with annotate=2 and also using interpreter-exec,
         * GDB spits out the annotations in the MI output. All of these
         * annotations can be ignored. */
        if (strncmp(ibuf_get(c->breakpoint_string), "^done", 5) == 0) {
            enum gdbwire_result result;
            struct gdbwire_mi_command *mi_command = 0;
            result = gdbwire_interpreter_exec(ibuf_get(c->breakpoint_string),
                GDBWIRE_MI_BREAK_INFO, &mi_command);
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

                commands_send_breakpoints(list, bkpt_list);

                gdbwire_mi_command_free(mi_command);
            }
        }

        ibuf_clear(c->breakpoint_string);
    }
}

static void commands_process_completion(struct commands *c)
{
    const char *ptr = ibuf_get(c->tab_completion_string);
    const char *scomplete = "server complete ";

    /* Do not add the "server complete " matches, which is returned with 
     * GNAT 3.15p version of GDB. Most likely this could happen with other 
     * implementations that are derived from GDB.
     */
    if (strncmp(ptr, scomplete, strlen(scomplete)) != 0)
        tgdb_list_append(c->tab_completions, strdup(ptr));
}

/* process's source files */
static void commands_process_complete(struct commands *c, char a)
{
    ibuf_addchar(c->tab_completion_string, a);

    if (a == '\n') {
        ibuf_delchar(c->tab_completion_string); /* remove '\n' and null terminate */

        if (ibuf_length(c->tab_completion_string) > 0)
            commands_process_completion(c);

        ibuf_clear(c->tab_completion_string);
    }
}

void commands_free(struct commands *c, void *item)
{
    free((char *) item);
}

void commands_send_gui_sources(struct commands *c, struct tgdb_list *list)
{
    struct tgdb_response *response = (struct tgdb_response *)
            cgdb_malloc(sizeof (struct tgdb_response));

    response->header = TGDB_UPDATE_SOURCE_FILES;
    response->choice.update_source_files.source_files =
            c->inferior_source_files;
    tgdb_types_append_command(list, response);
}

void commands_send_gui_completions(struct commands *c, struct tgdb_list *list)
{
    /* If the inferior program was not compiled with debug, then no sources
     * will be available. If no sources are available, do not return the
     * TGDB_UPDATE_SOURCE_FILES command. */
/*  if (tgdb_list_size ( c->tab_completions ) > 0)*/
    struct tgdb_response *response = (struct tgdb_response *)
            cgdb_malloc(sizeof (struct tgdb_response));

    response->header = TGDB_UPDATE_COMPLETIONS;
    response->choice.update_completions.completion_list = c->tab_completions;
    tgdb_types_append_command(list, response);
}

void commands_process(struct commands *c, char a, struct tgdb_list *list)
{
    if (commands_get_state(c) == INFO_SOURCES) {
        commands_process_info_sources(c, a);
    } else if (commands_get_state(c) == INFO_BREAKPOINTS) {
        commands_process_breakpoints(c, a, list);
    } else if (commands_get_state(c) == COMPLETE) {
        commands_process_complete(c, a);
    } else if (commands_get_state(c) == INFO_SOURCE) {
        commands_process_info_source(c, a, list);
    }
}

/*******************************************************************************
 * This must be translated to just return the proper command.
 ******************************************************************************/

/* commands_prepare_info_breakpoints: 
 * ----------------------------------
 *  
 *  This prepares the command 'info breakpoints' 
 */
static void commands_prepare_info_breakpoints(struct commands *c)
{
    ibuf_clear(c->breakpoint_string);
    commands_set_state(c, INFO_BREAKPOINTS, NULL);
}

/* commands_prepare_tab_completion:
 * --------------------------------
 *
 * This prepares the tab completion command
 */
static void
commands_prepare_tab_completion(struct annotate_two *a2, struct commands *c)
{
    c->tab_completion_ready = 0;
    ibuf_clear(c->tab_completion_string);
    commands_set_state(c, COMPLETE, NULL);
    global_set_start_completion(a2->g);
}

/* commands_prepare_info_sources: 
 * ------------------------------
 *
 *  This prepares the command 'info sources' by setting certain variables.
 */
static void
commands_prepare_info_sources(struct annotate_two *a2, struct commands *c)
{
    c->sources_ready = 0;
    ibuf_clear(c->info_sources_string);
    commands_set_state(c, INFO_SOURCES, NULL);
    global_set_start_info_sources(a2->g);
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
            commands_prepare_info_sources(a2, c);
            break;
        case ANNOTATE_INFO_SOURCE:
            commands_prepare_info_source(a2, c);
            break;
        case ANNOTATE_INFO_BREAKPOINTS:
            commands_prepare_info_breakpoints(c);
            break;
        case ANNOTATE_TTY:
            break;              /* Nothing to do */
        case ANNOTATE_COMPLETE:
            commands_prepare_tab_completion(a2, c);
            io_debug_write_fmt("<%s\n>", com->tgdb_command_data);
            break;              /* Nothing to do */
        case ANNOTATE_SET_PROMPT:
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
            ncom = strdup("server interp mi \"-file-list-exec-source-file\"\n");
            break;
        case ANNOTATE_INFO_BREAKPOINTS:
            ncom = strdup("server interpreter-exec mi \"-break-info\"\n");
            break;
        case ANNOTATE_TTY:
        {
            //$ TODO mikesart: -inferior-tty-set
            struct ibuf *temp_tty_name = ibuf_init();

            ibuf_add(temp_tty_name, data);
            ncom = (char *) cgdb_malloc(sizeof (char) * (13 + strlen(data)));
            strcpy(ncom, "server tty ");
            strcat(ncom, ibuf_get(temp_tty_name));
            strcat(ncom, "\n");

            ibuf_free(temp_tty_name);
            temp_tty_name = NULL;
            break;
        }
        case ANNOTATE_COMPLETE:
            ncom = (char *) cgdb_malloc(sizeof (char) * (18 + strlen(data)));
            strcpy(ncom, "server complete ");
            strcat(ncom, data);
            strcat(ncom, "\n");
            break;
        case ANNOTATE_SET_PROMPT:
            ncom = strdup(data);
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
