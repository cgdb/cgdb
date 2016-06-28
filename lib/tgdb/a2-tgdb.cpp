#if HAVE_CONFIG_H
#include "config.h"
#endif

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif

#if HAVE_STRING_H
#include <string.h>
#endif

#if HAVE_STDIO_H
#include <stdio.h>
#endif

#include "sys_util.h"
#include "a2-tgdb.h"
#include "io.h"
#include "state_machine.h"
#include "commands.h"
#include "ibuf.h"

/* Here are the two functions that deal with getting tty information out
 * of the annotate_two subsystem.
 */

int a2_open_new_tty(struct annotate_two *a2, int *inferior_stdin, int *inferior_stdout)
{
    if (a2->pty_pair)
        pty_pair_destroy(a2->pty_pair);

    a2->pty_pair = pty_pair_create();
    if (!a2->pty_pair) {
        clog_error(CLOG_CGDB, "pty_pair_create failed");
        return -1;
    }

    *inferior_stdin = pty_pair_get_masterfd(a2->pty_pair);
    *inferior_stdout = pty_pair_get_masterfd(a2->pty_pair);

    commands_issue_command(a2, ANNOTATE_TTY,
        pty_pair_get_slavename(a2->pty_pair), 1);

    return 0;
}

/* initialize_annotate_two
 *
 * initializes an annotate_two subsystem and sets up all initial values.
 */
static struct annotate_two *initialize_annotate_two(void)
{
    struct annotate_two *a2 = (struct annotate_two *)
            cgdb_malloc(sizeof (struct annotate_two));

    a2->tgdb_initialized = 0;
    a2->debugger_stdin = -1;
    a2->debugger_out = -1;

    a2->pty_pair = NULL;

    /* null terminate */
    a2->config_dir[0] = '\0';
    a2->a2_gdb_init_file[0] = '\0';

    a2->request_source_location = 0;

    return a2;
}

/* tgdb_setup_config_file: 
 * -----------------------
 *  Creates a config file for the user.
 *
 *  Pre: The directory already has read/write permissions. This should have
 *       been checked by tgdb-base.
 *
 *  Return: 1 on success or 0 on error
 */
static int tgdb_setup_config_file(struct annotate_two *a2, const char *dir)
{
    FILE *fp;

    strncpy(a2->config_dir, dir, strlen(dir) + 1);

    fs_util_get_path(dir, "a2_gdb_init", a2->a2_gdb_init_file);

    if ((fp = fopen(a2->a2_gdb_init_file, "w"))) {
        fprintf(fp, "set annotate 2\n" "set height 0\n");
        fclose(fp);
    } else {
        clog_error(CLOG_CGDB, "fopen error '%s'",
            a2->a2_gdb_init_file);
        return 0;
    }

    return 1;
}

struct annotate_two *a2_create_context(const char *debugger,
        int argc, char **argv, const char *config_dir)
{
    struct annotate_two *a2 = (struct annotate_two *)
        cgdb_calloc(1, sizeof(struct annotate_two));

    a2->debugger_stdin = -1;
    a2->debugger_out = -1;

    if (!tgdb_setup_config_file(a2, config_dir)) {
        clog_error(CLOG_CGDB,  "tgdb_init_config_file error");
        return NULL;
    }

    a2->debugger_pid = invoke_debugger(debugger, argc, argv,
            &a2->debugger_stdin, &a2->debugger_out, 0, a2->a2_gdb_init_file);

    /* Couldn't invoke process */
    if (a2->debugger_pid == -1)
        return NULL;

    return a2;
}

int a2_initialize(struct annotate_two *a2,
        int *debugger_stdin, int *debugger_stdout,
        int *inferior_stdin, int *inferior_stdout)
{
    *debugger_stdin = a2->debugger_stdin;
    *debugger_stdout = a2->debugger_out;

    a2->sm = state_machine_initialize();
    a2->c = commands_initialize(a2);
    a2->client_commands = NULL;

    a2_open_new_tty(a2, inferior_stdin, inferior_stdout);

    /* Need to get source information before breakpoint information otherwise
     * the TGDB_UPDATE_BREAKPOINTS event will be ignored in process_commands()
     * because there are no source files to add the breakpoints to.
     */
    a2_get_current_location(a2);

    /* gdb may already have some breakpoints when it starts. This could happen
     * if the user puts breakpoints in there .gdbinit.
     * This makes sure that TGDB asks for the breakpoints on start up.
     */
    if (commands_issue_command(a2,
                    ANNOTATE_INFO_BREAKPOINTS, NULL, 0) == -1) {
        return -1;
    }

    /**
     * Query if disassemble supports the /s flag
     */
    if (commands_issue_command(a2,
                    ANNOTATE_DATA_DISASSEMBLE_MODE_QUERY, NULL, 1) == -1) {
        return -1;
    }

    a2->tgdb_initialized = 1;

    return 0;
}

int a2_shutdown(struct annotate_two *a2)
{
    int i;

    cgdb_close(a2->debugger_stdin);
    a2->debugger_stdin = -1;

    state_machine_shutdown(a2->sm);
    commands_shutdown(a2->c);

    a2_delete_responses(a2);
    sbfree(a2->responses);
    a2->responses = NULL;

    for (i = 0; i < sbcount(a2->client_commands); i++)
    {
        struct tgdb_command *tc = a2->client_commands[i];

        free(tc->gdb_command);
        free(tc);
    }
    sbfree(a2->client_commands);
    a2->client_commands = NULL;
    return 0;
}

int tgdb_delete_response(struct tgdb_response *com);
void a2_delete_responses(struct annotate_two *a2)
{
    int i;

    for (i = 0; i < sbcount(a2->responses); i++)
        tgdb_delete_response(a2->responses[i]);

    sbsetcount(a2->responses, 0);
}


int a2_is_client_ready(struct annotate_two *a2)
{
    if (!a2->tgdb_initialized)
        return 0;

    /* Return 1 if the user is at the prompt */
    return (data_get_state(a2->sm) == USER_AT_PROMPT);
}

int a2_get_current_location(struct annotate_two *a2)
{
    return commands_issue_command(a2, ANNOTATE_INFO_FRAME, NULL, 1);
}

pid_t a2_get_debugger_pid(struct annotate_two *a2)
{
    return a2->debugger_pid;
}

int a2_user_ran_command(struct annotate_two *a2)
{
    return commands_user_ran_command(a2);
}

void a2_prepare_for_command(struct annotate_two *a2, struct tgdb_command *com)
{
    commands_prepare_for_command(a2, a2->c, com);
}

int a2_is_misc_prompt(struct annotate_two *a2)
{
    return sm_is_misc_prompt(a2->sm);
}
