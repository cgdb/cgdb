#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif /* HAVE_STDLIB_H */

#include "annotate.h"
#include "sys_util.h"
#include "logger.h"
#include "data.h"
#include "commands.h"
#include "globals.h"
#include "io.h"

static int
handle_source(struct annotate_two *a2, const char *buf, size_t n,
        struct tgdb_list *list)
{
    /* set up the info_source command to get file info */
    return commands_issue_command(a2->c, a2->client_command_list,
                    ANNOTATE_INFO_SOURCE, NULL, 1);
}

static int handle_misc_pre_prompt(struct annotate_two *a2, const char *buf,
        size_t n, struct tgdb_list *list)
{
    /* If tgdb is sending a command, then continue past it */
    if (data_get_state(a2->data) == INTERNAL_COMMAND) {
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
    globals_set_misc_prompt_command(a2->g, 1);
    data_set_state(a2, USER_AT_PROMPT);
    a2->command_finished = 1;
    return 0;
}

static int handle_misc_post_prompt(struct annotate_two *a2, const char *buf,
        size_t n, struct tgdb_list *list)
{
    globals_set_misc_prompt_command(a2->g, 0);
    data_set_state(a2, POST_PROMPT);

    return 0;
}

static int handle_pre_prompt(struct annotate_two *a2, const char *buf, size_t n,
        struct tgdb_list *list)
{
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
    char *tmp = (char *) malloc(sizeof (char) * (n + 1));
    int *i = (int *) malloc(sizeof (int));
    struct tgdb_response *response;

    sprintf(tmp, "%s", buf + 7);    /* Skip the 'exited ' part */
    *i = atoi(tmp);
    free(tmp);
    tmp = NULL;
    response = (struct tgdb_response *)
            cgdb_malloc(sizeof (struct tgdb_response));
    response->header = TGDB_INFERIOR_EXITED;
    response->choice.inferior_exited.exit_status = i;
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

int tgdb_parse_annotation(struct annotate_two *a2, char *data, size_t size,
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
