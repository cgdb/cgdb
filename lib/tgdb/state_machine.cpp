#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_STDIO_H
#include <stdio.h>
#endif /* HAVE_STDIO_H */

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif /* HAVE_STDLIB_H */

#include "state_machine.h"
#include "commands.h"
#include "logger.h"
#include "annotate_two.h"
#include "sys_util.h"
#include "ibuf.h"
#include "io.h"

static int tgdb_parse_annotation(struct annotate_two *a2, char *data,
    size_t size, struct tgdb_list *list);

/* This package looks for annotations coming from gdb's output.
 * The program that is being debugged does not have its output pass
 * through here. So only gdb's output is filtered here.
 *
 * This is a simple state machine that is looking for annotations
 * in gdb's output. Annotations are of the form
 * '\n\032\032annotation\n'
 * However, on windows \n gets mapped to \r\n So we take account
 * for that by matching the form
 * '\r+\n\032\032annotation\r+\n'
 * 
 * When an annotation is found, this unit passes the annotation to the 
 * annotate unit and this unit is free of all responsibility :)
 */

enum state {
    DATA,                       /* data from debugger */
    NEW_LINE,                   /* got '\n' */
    CONTROL_Z,                  /* got first ^Z '\032' */
    ANNOTATION,                 /* got second ^Z '\032' */
    NL_DATA                     /* got a nl at the end of annotation */
};

/** The data needed to parse the output of GDB. */
struct state_machine {

    /** The maximum size of the prompt. */
#define GDB_PROMPT_SIZE 1024

    /** the state of the data context */
    enum internal_state data_state;

    /** The debugger's prompt. */
    char gdb_prompt[GDB_PROMPT_SIZE];

    /** The size of gdb_prompt. */
    int gdb_prompt_size;

    /** What the debugger's prompt was before. */
    char gdb_prompt_last[GDB_PROMPT_SIZE];

    /** Annotations will be stored here. */
    struct ibuf *tgdb_buffer;

    /** The state of the annotation parser ( current context ). */
    enum state tgdb_state;

    /** Is a misc prompt command be run. */
    unsigned short misc_prompt_command;
};

struct state_machine *state_machine_initialize(void)
{
    struct state_machine *sm =
            (struct state_machine *) cgdb_malloc(sizeof (struct state_machine));

    sm->data_state = VOID;
    sm->gdb_prompt_size = 0;
    sm->gdb_prompt[0] = 0;
    sm->gdb_prompt_last[0] = 0;
    sm->tgdb_buffer = ibuf_init();
    sm->tgdb_state = DATA;
    sm->misc_prompt_command = 0;

    return sm;
}

void state_machine_shutdown(struct state_machine *sm)
{
    ibuf_free(sm->tgdb_buffer);
    free(sm);
    sm = NULL;
}

enum internal_state data_get_state(struct state_machine *sm)
{
    return sm->data_state;
}

void data_set_state(struct annotate_two *a2, enum internal_state state)
{
    /* if tgdb is at an internal command, than nothing changes that
     * state unless tgdb gets to the prompt annotation. This means that
     * the internal command is done */
    if (a2->sm->data_state == INTERNAL_COMMAND && state != USER_AT_PROMPT)
        return;

    a2->sm->data_state = state;

    switch (a2->sm->data_state) {
        case VOID:
            break;
        case AT_PROMPT:
            a2->sm->gdb_prompt_size = 0;
            break;
        case USER_AT_PROMPT:
            /* Null-Terminate the prompt */
            a2->sm->gdb_prompt[a2->sm->gdb_prompt_size] = '\0';

            if (strcmp(a2->sm->gdb_prompt, a2->sm->gdb_prompt_last) != 0) {
                strcpy(a2->sm->gdb_prompt_last, a2->sm->gdb_prompt);
                /* Update the prompt */
                if (a2->cur_response_list) {
                    struct tgdb_response *response = (struct tgdb_response *)
                            cgdb_malloc(sizeof (struct tgdb_response));

                    response->header = TGDB_UPDATE_CONSOLE_PROMPT_VALUE;
                    response->choice.update_console_prompt_value.prompt_value =
                            cgdb_strdup(a2->sm->gdb_prompt_last);
                    tgdb_list_append(a2->cur_response_list, response);
                }
            }

            a2->command_finished = 1;

            /* This is important, because it resets the commands state.
             * With this line not here, if the user hits 'o' from cgdb,
             * then the commands state gets set to INFO_SOURCES, then the
             * user hits ^c from the gdb window, the error occurs because 
             * commands state is INFO_SOURCES instead of VOID.
             */
            commands_set_state(a2->c, VOID_COMMAND);

            break;
        case POST_PROMPT:
            a2->sm->data_state = VOID;
            break;
        case GUI_COMMAND:
            break;
        case INTERNAL_COMMAND:
            break;
        case USER_COMMAND:
            break;
    }                           /* end switch */
}

static void data_process(struct annotate_two *a2, char a, char *buf, int *n)
{
    switch (a2->sm->data_state) {
        case VOID:
            buf[(*n)++] = a;
            break;
        case AT_PROMPT:
            a2->sm->gdb_prompt[a2->sm->gdb_prompt_size++] = a;
/*            buf[(*n)++] = a;*/
            break;
        case USER_AT_PROMPT:
            break;
        case GUI_COMMAND:
        case INTERNAL_COMMAND:
            if (a2->sm->data_state == INTERNAL_COMMAND)
                commands_process(a2->c, a);
            else if (a2->sm->data_state == GUI_COMMAND)
                buf[(*n)++] = a;

            break;              /* do nothing */
        case USER_COMMAND:
            break;
        case POST_PROMPT:
            break;
    }                           /* end switch */
}

int a2_handle_data(struct annotate_two *a2, struct state_machine *sm,
        const char *data, const size_t size,
        char *gui_data, size_t * gui_size, struct tgdb_list *command_list)
{
    int i, counter = 0;

    /* track state to find next file and line number */
    for (i = 0; i < size; ++i) {
        switch (data[i]) {
                /* Ignore all car returns outputted by gdb */
            case '\r':
                break;
            case '\n':
                switch (sm->tgdb_state) {
                    case DATA:
                        sm->tgdb_state = NEW_LINE;
                        break;
                    case NEW_LINE:
                        sm->tgdb_state = NEW_LINE;
                        data_process(a2, '\n', gui_data, &counter);
                        break;
                    case CONTROL_Z:
                        sm->tgdb_state = DATA;
                        data_process(a2, '\n', gui_data, &counter);
                        data_process(a2, '\032', gui_data, &counter);
                        break;
                    case ANNOTATION:   /* Found an annotation */
                        sm->tgdb_state = NL_DATA;
                        tgdb_parse_annotation(a2, ibuf_get(sm->tgdb_buffer),
                                ibuf_length(sm->tgdb_buffer), command_list);
                        ibuf_clear(sm->tgdb_buffer);
                        break;
                    case NL_DATA:
                        sm->tgdb_state = NEW_LINE;
                        break;
                    default:
                        logger_write_pos(logger, __FILE__, __LINE__,
                                "Bad state transition");
                        break;
                }               /* end switch */
                break;
            case '\032':
                switch (sm->tgdb_state) {
                    case DATA:
                        sm->tgdb_state = DATA;
                        data_process(a2, '\032', gui_data, &counter);
                        break;
                    case NEW_LINE:
                        sm->tgdb_state = CONTROL_Z;
                        break;
                    case NL_DATA:
                        sm->tgdb_state = CONTROL_Z;
                        break;
                    case CONTROL_Z:
                        sm->tgdb_state = ANNOTATION;
                        break;
                    case ANNOTATION:
                        ibuf_addchar(sm->tgdb_buffer, data[i]);
                        break;
                    default:
                        logger_write_pos(logger, __FILE__, __LINE__,
                                "Bad state transition");
                        break;
                }               /* end switch */
                break;
            default:
                switch (sm->tgdb_state) {
                    case DATA:
                        data_process(a2, data[i], gui_data, &counter);
                        break;
                    case NL_DATA:
                        sm->tgdb_state = DATA;
                        data_process(a2, data[i], gui_data, &counter);
                        break;
                    case NEW_LINE:
                        sm->tgdb_state = DATA;
                        data_process(a2, '\n', gui_data, &counter);
                        data_process(a2, data[i], gui_data, &counter);
                        break;
                    case CONTROL_Z:
                        sm->tgdb_state = DATA;
                        data_process(a2, '\n', gui_data, &counter);
                        data_process(a2, '\032', gui_data, &counter);
                        data_process(a2, data[i], gui_data, &counter);
                        break;
                    case ANNOTATION:
                        ibuf_addchar(sm->tgdb_buffer, data[i]);
                        break;
                    default:
                        logger_write_pos(logger, __FILE__, __LINE__,
                                "Bad state transition");
                        break;
                }               /* end switch */
                break;
        }                       /* end switch */
    }                           /* end for */

    gui_data[counter] = '\0';
    *gui_size = counter;
    return 0;
}

int globals_is_misc_prompt(struct state_machine *sm)
{
        return sm->misc_prompt_command;
}

void globals_set_misc_prompt_command(struct state_machine *sm, unsigned short set)
{
    sm->misc_prompt_command = set;
}

static int
handle_source(struct annotate_two *a2, const char *buf, size_t n,
        struct tgdb_list *list)
{
    a2->request_source_location = 1;
    return 0;
}

static int
handle_frame_end(struct annotate_two *a2, const char *buf, size_t n,
        struct tgdb_list *list)
{
    a2->request_source_location = 1;
    return 0;
}

static int
handle_frames_invalid(struct annotate_two *a2, const char *buf, size_t n,
        struct tgdb_list *list)
{
    a2->request_source_location = 1;
    return 0;
}

static int handle_misc_pre_prompt(struct annotate_two *a2, const char *buf,
        size_t n, struct tgdb_list *list)
{
    /* If tgdb is sending a command, then continue past it */
    if (data_get_state(a2->sm) == INTERNAL_COMMAND) {
        if (io_write_byte(a2->debugger_stdin, '\n') == -1)
            logger_write_pos(logger, __FILE__, __LINE__,
                    "Could not send command");
    } else {
        data_set_state(a2, AT_PROMPT);
    }

    return 0;
}

static int handle_misc_prompt(struct annotate_two *a2, const char *buf,
        size_t n, struct tgdb_list *list)
{
    globals_set_misc_prompt_command(a2->sm, 1);
    data_set_state(a2, USER_AT_PROMPT);
    a2->command_finished = 1;
    return 0;
}

static int handle_misc_post_prompt(struct annotate_two *a2, const char *buf,
        size_t n, struct tgdb_list *list)
{
    globals_set_misc_prompt_command(a2->sm, 0);
    data_set_state(a2, POST_PROMPT);

    return 0;
}

static int handle_pre_prompt(struct annotate_two *a2, const char *buf, size_t n,
        struct tgdb_list *list)
{
    if (a2->request_source_location) {
        a2_get_current_location(a2);
        a2->request_source_location = 0;
    }

    data_set_state(a2, AT_PROMPT);

    return 0;
}

static int handle_prompt(struct annotate_two *a2, const char *buf, size_t n,
        struct tgdb_list *list)
{
    /* All done. */
    data_set_state(a2, USER_AT_PROMPT);
    return 0;
}

static int handle_post_prompt(struct annotate_two *a2, const char *buf,
        size_t n, struct tgdb_list *list)
{
    data_set_state(a2, POST_PROMPT);
    return 0;
}

static int handle_error(struct annotate_two *a2, const char *buf, size_t n,
        struct tgdb_list *list)
{
    data_set_state(a2, POST_PROMPT);    /* TEMPORARY */
    return 0;
}

static int handle_error_begin(struct annotate_two *a2, const char *buf,
        size_t n, struct tgdb_list *list)
{
    /* After a signal is sent (^c), the debugger will then output 
     * something like "Quit\n", so that should be displayed to the user.
     * Unfortunately, the debugger ( gdb ) isn't nice enough to return a 
     * post-prompt when a signal is received.
     */
    data_set_state(a2, VOID);

    return 0;
}

static int handle_quit(struct annotate_two *a2, const char *buf, size_t n,
        struct tgdb_list *list)
{
    data_set_state(a2, POST_PROMPT);    /* TEMPORARY */
    return 0;
}

static int handle_exited(struct annotate_two *a2, const char *buf, size_t n,
        struct tgdb_list *list)
{
    int exit_status;
    struct tgdb_response *response;

    // Buf should be something like:
    //    "exited 0"
    exit_status = (n >= 7) ? atoi(buf + 7) : -1;

    response = (struct tgdb_response *)
            cgdb_malloc(sizeof (struct tgdb_response));
    response->header = TGDB_INFERIOR_EXITED;
    response->choice.inferior_exited.exit_status = exit_status;
    tgdb_types_append_command(list, response);
    return 0;
}

/**
 * The main annotation data structure.
 * It represents all of the supported annotataions that can be parsed.
 */
static struct annotation {

    /**
	 * The name of the annotation.
	 */
    const char *data;

    /**
	 * The size of the annotation.
	 */
    size_t size;

    /**
	 * The function to call when the annotation is found.
	 */
    int (*f) (struct annotate_two * a2, const char *buf, size_t n,
            struct tgdb_list * list);
} annotations[] = {
    {
    "source", 6, handle_source}, {
    "frame-end", 9, handle_frame_end }, {
    "frames-invalid", 14, handle_frames_invalid }, {
    "pre-commands", 12, handle_misc_pre_prompt}, {
    "commands", 8, handle_misc_prompt}, {
    "post-commands", 13, handle_misc_post_prompt}, {
    "pre-overload-choice", 19, handle_misc_pre_prompt}, {
    "overload-choice", 15, handle_misc_prompt}, {
    "post-overload-choice", 20, handle_misc_post_prompt}, {
    "pre-instance-choice", 19, handle_misc_pre_prompt}, {
    "instance-choice", 15, handle_misc_prompt}, {
    "post-instance-choice", 20, handle_misc_post_prompt}, {
    "pre-query", 9, handle_misc_pre_prompt}, {
    "query", 5, handle_misc_prompt}, {
    "post-query", 10, handle_misc_post_prompt}, {
    "pre-prompt-for-continue", 23, handle_misc_pre_prompt}, {
    "prompt-for-continue", 19, handle_misc_prompt}, {
    "post-prompt-for-continue", 24, handle_misc_post_prompt}, {
    "pre-prompt", 10, handle_pre_prompt}, {
    "prompt", 6, handle_prompt}, {
    "post-prompt", 11, handle_post_prompt}, {
    "error-begin", 11, handle_error_begin}, {
    "error", 5, handle_error}, {
    "quit", 4, handle_quit}, {
    "exited", 6, handle_exited}, {
    NULL, 0, NULL}
};

static int tgdb_parse_annotation(struct annotate_two *a2, char *data, size_t size,
        struct tgdb_list *list)
{
    int i;

    for (i = 0; annotations[i].data != NULL; ++i) {
        if (strncmp(data, annotations[i].data, annotations[i].size) == 0) {
            if (annotations[i].f(a2, data, size, list) == -1) {
                logger_write_pos(logger, __FILE__, __LINE__,
                        "parsing annotation failed");
            } else {
                break; /* only match one annotation */
            }
        }
    }

    /*err_msg("ANNOTION(%s)", data); */
    return 0;
}

