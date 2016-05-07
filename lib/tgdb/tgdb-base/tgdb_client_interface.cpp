#include "tgdb_client_interface.h"

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif /* HAVE_STDLIB_H */

#include "gdbmi_tgdb.h"
#include "a2-tgdb.h"
#include "sys_util.h"

/* Define to use the annotate 2 interface. */
#define ANNOTATE_TWO

/**
 * This is a context that abstracts the lower level interface from TGDB.
 * By doing this, all of the lower levels can communicate here, and TGDB
 * just makes sure that it calls the functions in this header.
 */
struct tgdb_client_context {

    /**
     * The current client debugger being used.
	 */
#ifdef ANNOTATE_TWO
    struct annotate_two *tgdb_debugger_context;
#else
    struct tgdb_gdbmi *tgdb_debugger_context;
#endif

    struct logger *logger;
};

struct tgdb_client_context *tgdb_client_create_context(const char
        *debugger_path, int argc, char **argv, const char *config_dir,
        struct logger *logger_in)
{
    struct tgdb_client_context *tcc;

    tcc = (struct tgdb_client_context *)
            cgdb_malloc(sizeof (struct tgdb_client_context));
    tcc->logger = logger_in;

#ifdef ANNOTATE_TWO
    tcc->tgdb_debugger_context = a2_create_context(
        debugger_path, argc, argv, config_dir, logger_in);
#else
    tcc->tgdb_debugger_context = gdbmi_create_context(
        debugger_path, argc, argv, config_dir, logger_in);
#endif

    if (tcc->tgdb_debugger_context == NULL) {
        logger_write_pos(tcc->logger, __FILE__, __LINE__,
                "a2_create_instance failed");
        free(tcc);
        return NULL;
    }

    return tcc;
}

int tgdb_client_initialize_context(struct tgdb_client_context *tcc,
        int *debugger_stdin, int *debugger_stdout,
        int *inferior_stdin, int *inferior_stdout)
{
#ifdef ANNOTATE_TWO
    return a2_initialize(tcc->tgdb_debugger_context, debugger_stdin, debugger_stdout,
            inferior_stdin, inferior_stdout);
#else
    return gdbmi_initialize(tcc->
            tgdb_debugger_context, debugger_stdin, debugger_stdout,
            inferior_stdin, inferior_stdout);
#endif
}

int tgdb_client_destroy_context(struct tgdb_client_context *tcc)
{
#ifdef ANNOTATE_TWO
    return a2_shutdown(tcc->tgdb_debugger_context);
#else
    return gdbmi_shutdown(tcc->tgdb_debugger_context);
#endif
}

int tgdb_client_err_msg(struct tgdb_client_context *tcc)
{
    return -1;
}

int tgdb_client_is_client_ready(struct tgdb_client_context *tcc)
{
#ifdef ANNOTATE_TWO
    return a2_is_client_ready(tcc->tgdb_debugger_context);
#else
    return gdbmi_is_client_ready(tcc->tgdb_debugger_context);
#endif
}

int tgdb_client_tgdb_ran_command(struct tgdb_client_context *tcc)
{
#ifdef ANNOTATE_TWO
    return a2_user_ran_command(tcc->tgdb_debugger_context);
#else
    return gdbmi_user_ran_command(tcc->tgdb_debugger_context);
#endif
}

int tgdb_client_prepare_for_command(struct tgdb_client_context *tcc,
        struct tgdb_command *com)
{
#ifdef ANNOTATE_TWO
    return a2_prepare_for_command(tcc->tgdb_debugger_context, com);
#else
    return gdbmi_prepare_for_command(tcc->tgdb_debugger_context, com);
#endif
}

int tgdb_client_can_tgdb_run_commands(struct tgdb_client_context *tcc)
{
#ifdef ANNOTATE_TWO
    return a2_is_misc_prompt(tcc->tgdb_debugger_context);
#else
    return -1;
#endif
}

int tgdb_client_parse_io(struct tgdb_client_context *tcc,
        const char *input_data, const size_t input_data_size,
        char *debugger_output, size_t * debugger_output_size,
        char *inferior_output, size_t * inferior_output_size,
        struct tgdb_list *command_list)
{
#ifdef ANNOTATE_TWO
    return a2_parse_io(tcc-> tgdb_debugger_context, input_data,
            input_data_size, debugger_output,
            debugger_output_size, inferior_output, inferior_output_size,
            command_list);
#else
    return gdbmi_parse_io(tcc-> tgdb_debugger_context, input_data,
            input_data_size, debugger_output,
            debugger_output_size, inferior_output, inferior_output_size,
            command_list);
#endif
}

struct tgdb_list *tgdb_client_get_client_commands(struct tgdb_client_context
        *tcc)
{
#ifdef ANNOTATE_TWO
    return a2_get_client_commands(tcc->tgdb_debugger_context);
#else
    return gdbmi_get_client_commands(tcc->tgdb_debugger_context);
#endif
}

int tgdb_client_get_filename_pair(struct tgdb_client_context *tcc,
        const char *path)
{
#ifdef ANNOTATE_TWO
    return a2_get_source_filename_pair(tcc->tgdb_debugger_context, path);
#else
    return -1;
#endif
}

int tgdb_client_get_current_location(struct tgdb_client_context *tcc,
        int on_startup)
{
#ifdef ANNOTATE_TWO
    return a2_get_current_location(tcc->tgdb_debugger_context, on_startup);
#else
    return -1;
#endif
}

int tgdb_client_get_inferior_source_files(struct tgdb_client_context *tcc)
{
#ifdef ANNOTATE_TWO
    return a2_get_inferior_sources(tcc->tgdb_debugger_context);
#else
    return -1;
#endif
}

int tgdb_client_completion_callback(struct tgdb_client_context *tcc,
        const char *completion_command)
{
#ifdef ANNOTATE_TWO
    return a2_completion_callback(tcc->
            tgdb_debugger_context, completion_command);
#else
    return -1;
#endif
}

const char *tgdb_client_return_command(struct tgdb_client_context *tcc,
        enum tgdb_command_type c)
{
#ifdef ANNOTATE_TWO
    return a2_return_client_command(tcc->tgdb_debugger_context, c);
#else
    return NULL;
#endif
}

char *tgdb_client_modify_breakpoint(struct tgdb_client_context *tcc,
        const char *file, int line, enum tgdb_breakpoint_action b)
{
#ifdef ANNOTATE_TWO
   return a2_client_modify_breakpoint(tcc->tgdb_debugger_context,
        file, line, b);
#else
    return NULL;
#endif
}

pid_t tgdb_client_get_debugger_pid(struct tgdb_client_context * tcc)
{
#ifdef ANNOTATE_TWO
    return a2_get_debugger_pid(tcc->tgdb_debugger_context);
#else
    return -1;
#endif
}

int tgdb_client_open_new_tty(struct tgdb_client_context *tcc,
        int *inferior_stdin, int *inferior_stdout)
{
#ifdef ANNOTATE_TWO
    return a2_open_new_tty(tcc->tgdb_debugger_context,
        inferior_stdin, inferior_stdout);
#else
    return -1;
#endif
}

const char *tgdb_client_get_tty_name(struct tgdb_client_context *tcc)
{
#ifdef ANNOTATE_TWO
    return a2_get_tty_name(tcc->tgdb_debugger_context);
#else
    return NULL;
#endif
}
