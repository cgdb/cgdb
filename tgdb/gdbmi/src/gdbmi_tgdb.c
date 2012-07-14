#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif /* HAVE_STDLIB_H */

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#if HAVE_STDIO_H
#include <stdio.h>
#endif /* HAVE_STDIO_H */

#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#if HAVE_SIGNAL_H
#include <signal.h>
#endif /* HAVE_SIGNAL_H */

#if HAVE_ERRNO_H
#include <errno.h>
#endif /* HAVE_ERRNO_H */

#include "gdbmi_tgdb.h"
#include "fork_util.h"
#include "fs_util.h"
#include "pseudo.h"
#include "io.h"
#include "tgdb_types.h"
#include "queue.h"
#include "sys_util.h"
#include "ibuf.h"

#define PATH_MAX 4096
#define TTY_NAME_SIZE 64

/**
 * This is the main context for the gdbmi subsytem.
 */
struct tgdb_gdbmi {

    /** 
	 * This is set when this context has initialized itself
	 */
    int tgdb_initialized;

    /** 
	 * writing to this will write to the stdin of the debugger
	 */
    int debugger_stdin;

    /**
     * Reading from reads the stdout/stderr of the debugger
	 */
    int debugger_out;

    /**
     * writing to this will write to the stdin of the inferior
	 */
    int inferior_stdin;

    /**
	 * Reading from reads the stdout/stderr of the inferior
	 */
    int inferior_out;

    /**
	 * Only kept around so it can be closed properly
	 */
    int inferior_slave_fd;

    /** 
	 * pid of child process.
	 */
    pid_t debugger_pid;

    /** 
	 * The config directory that this context can write too.
	 */
    char config_dir[PATH_MAX];

    /**
	 * The init file for the debugger.
	 */
    char gdbmi_gdb_init_file[PATH_MAX];

    /**
	 * The name of the inferior tty.
	 */
    char inferior_tty_name[TTY_NAME_SIZE];

    /** 
	 * This is a list of all the commands generated since in the last call. 
	 */
    struct tgdb_list *client_command_list;

    /**
	 * This is the current output command from GDB.
	 */
    struct ibuf *tgdb_cur_output_command;
};

#if 0
//static int gdbmi_set_inferior_tty ( void *ctx ) {
//  struct annotate_two *a2 = (struct annotate_two *)ctx;
//
//    if ( commands_issue_command ( 
//              a2->c, 
//              a2->client_command_list,
//              ANNOTATE_TTY, 
//              a2->inferior_tty_name, 
//              0 ) == -1 ) {
//        err_msg("%s:%d commands_issue_command error", __FILE__, __LINE__);
//        return -1;
//    }
//
//    return 0;
//}
//
///* This is ok as static, all references will use the same data. */
//static char *gdbmi_tgdb_commands[] = {
//  "continue",
//  "finish",
//  "next",
//  "run",
//  "step",
//  "up",
//  "down"
//};
//
//
//static int close_inferior_connection ( void *ctx ) {
//  struct annotate_two *a2 = (struct annotate_two *)ctx;
//
//  if ( a2->inferior_stdin != -1 )
//      cgdb_close ( a2->inferior_stdin );
//
//  a2->inferior_stdin = -1;
//  a2->inferior_out   = -1;
//  
//  /* close tty connection */
//  if ( a2->inferior_slave_fd != -1 )
//      cgdb_close ( a2->inferior_slave_fd );
//
//  a2->inferior_slave_fd = -1;
//
//  if ( a2->inferior_tty_name[0] != '\0' )
//      pty_release ( a2->inferior_tty_name );
//
//  return 0;
//}
//
///* Here are the two functions that deal with getting tty information out
// * of the annotate_two subsystem.
// */
//
//int gdbmi_open_new_tty ( 
//      void *ctx,
//      int *inferior_stdin, 
//      int *inferior_stdout ) {
//  struct annotate_two *a2 = (struct annotate_two *)ctx;
//
//    close_inferior_connection(a2);
//
//  /* Open up the tty communication */
//  if ( util_new_tty(&(a2->inferior_stdin), &(a2->inferior_slave_fd), a2->inferior_tty_name) == -1){
//      err_msg("%s:%d -> Could not open child tty", __FILE__, __LINE__);
//      return -1;
//  }
//
//  *inferior_stdin     = a2->inferior_stdin;
//  *inferior_stdout    = a2->inferior_stdin;
//
//    a2_set_inferior_tty ( a2 );
//    
//    return 0;
//}
//
//char *a2_get_tty_name ( void *ctx ) {
//  struct annotate_two *a2 = (struct annotate_two *)ctx;
//  return a2->inferior_tty_name;
//}
#endif

/* initialize_gdbmi
 *
 * initializes a gdbmi subsystem and sets up all initial values.
 */
static struct tgdb_gdbmi *initialize_tgdb_gdbmi(void)
{
    struct tgdb_gdbmi *gdbmi = (struct tgdb_gdbmi *)
            cgdb_malloc(sizeof (struct tgdb_gdbmi));

    gdbmi->tgdb_initialized = 0;
    gdbmi->debugger_stdin = -1;
    gdbmi->debugger_out = -1;

    gdbmi->inferior_stdin = -1;
    gdbmi->inferior_out = -1;
    gdbmi->inferior_slave_fd = -1;
    gdbmi->inferior_tty_name[0] = '\0';

    /* null terminate */
    gdbmi->config_dir[0] = '\0';
    gdbmi->gdbmi_gdb_init_file[0] = '\0';

    return gdbmi;
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
static int tgdb_setup_config_file(struct tgdb_gdbmi *gdbmi, const char *dir)
{
    FILE *fp;

    strncpy(gdbmi->config_dir, dir, strlen(dir) + 1);

    fs_util_get_path(dir, "gdbmi_gdb_init", gdbmi->gdbmi_gdb_init_file);

    if ((fp = fopen(gdbmi->gdbmi_gdb_init_file, "w"))) {
        fprintf(fp, "set height 0\n" "set prompt (tgdbmi) \n");
        fclose(fp);
    } else {
        logger_write_pos(logger, __FILE__, __LINE__, "fopen error '%s'",
                gdbmi->gdbmi_gdb_init_file);
        return 0;
    }

    return 1;
}

void *gdbmi_create_context(const char *debugger,
        int argc, char **argv, const char *config_dir, struct logger *logger)
{

    struct tgdb_gdbmi *gdbmi = initialize_tgdb_gdbmi();
    char gdbmi_debug_file[PATH_MAX];

    if (!tgdb_setup_config_file(gdbmi, config_dir)) {
        logger_write_pos(logger, __FILE__, __LINE__,
                "tgdb_init_config_file error");
        return NULL;
    }

    /* Initialize the debug file that gdbmi_tgdb writes to */
    fs_util_get_path(config_dir, "gdbmi_tgdb_debug.txt", gdbmi_debug_file);

    io_debug_init(gdbmi_debug_file);

    gdbmi->debugger_pid =
            invoke_debugger(debugger, argc, argv,
            &gdbmi->debugger_stdin, &gdbmi->debugger_out,
            1, gdbmi->gdbmi_gdb_init_file);

    /* Couldn't invoke process */
    if (gdbmi->debugger_pid == -1)
        return NULL;

    gdbmi->tgdb_cur_output_command = ibuf_init();

    return gdbmi;
}

int gdbmi_initialize(void *ctx,
        int *debugger_stdin, int *debugger_stdout,
        int *inferior_stdin, int *inferior_stdout)
{
    struct tgdb_gdbmi *gdbmi = (struct tgdb_gdbmi *) ctx;

    gdbmi->client_command_list = tgdb_list_init();

    *debugger_stdin = gdbmi->debugger_stdin;
    *debugger_stdout = gdbmi->debugger_out;

    gdbmi->tgdb_initialized = 1;

#if 0
    /* Need to set the prompt via a tgdb_response */
    struct tgdb_client_command *client_command = NULL;

    client_command = tgdb_client_command_create("(tgdbmi) ",
            TGDB_CLIENT_COMMAND_TGDB_BASE,
            TGDB_CLIENT_COMMAND_DISPLAY_NOTHING,
            TGDB_CLIENT_COMMAND_ACTION_CONSOLE_SET_PROMPT, NULL);

    tgdb_list_append(gdbmi->client_command_list, client_command);
#endif

    return 0;
}

int gdbmi_shutdown(void *ctx)
{
    struct tgdb_gdbmi *gdbmi = (struct tgdb_gdbmi *) ctx;

    cgdb_close(gdbmi->debugger_stdin);

    ibuf_free(gdbmi->tgdb_cur_output_command);
    gdbmi->tgdb_cur_output_command = NULL;

    return 0;
}

#if 0
/* TODO: Implement error messages. */
//int a2_err_msg ( void *ctx ) {
//  return -1;
//}
#endif

int gdbmi_is_client_ready(void *ctx)
{
    return 1;
}

enum newlinestyle {
    GDBMI_NL,
    GDBMI_CR_NL,
    GDBMI_CR
};

/**
 * Checks to see if ibuf ends with the string ending.
 *
 * \param ibuf
 * The string to check the ending of
 *
 * \param style
 * represents what kind of newline is associated with this command.
 *
 * \param success
 * 1 if ibuf ends with ending, otherwise 0
 *
 * \return
 * 0 on success, or -1 on error
 */
static int ends_with_gdbmi_prompt(struct ibuf *ibuf, enum newlinestyle style,
        int *success)
{
    char *ibuf_string;
    int length;

    if (!ibuf)
        return -1;

    if (!success)
        return -1;

    *success = 0;

    length = ibuf_length(ibuf);

    /* If the string is not long enough, don't try */
    if (style == GDBMI_CR_NL && length < 8)
        return 0;
    else if (length < 7)
        return 0;

    ibuf_string = ibuf_get(ibuf);

    /* Match the char's backwards, go to the end */
    ibuf_string = ibuf_string + length - 1;

    if (style == GDBMI_CR_NL)
        ibuf_string -= 2;
    else
        ibuf_string--;

    /* Check for the prompt */
    if (*(ibuf_string) == ' ' &&
            *(ibuf_string - 1) == ')' &&
            *(ibuf_string - 2) == 'b' &&
            *(ibuf_string - 3) == 'd' &&
            *(ibuf_string - 4) == 'g' && *(ibuf_string - 5) == '(')
        *success = 1;

    /* Look at newline */
    return 0;

}

/**
 * Determine's what kind of newline the string ends in.
 *
 */
static int gdbmi_get_newline_style(struct ibuf *ibuf, enum newlinestyle *style)
{
    char *buf;
    int length;

    if (!ibuf)
        return -1;

    if (!style)
        return -1;

    buf = ibuf_get(ibuf);
    length = ibuf_length(ibuf);

    /* Check to see if an entire command has been reached. */
    if (buf[length - 1] == '\r') {
        *style = GDBMI_CR;
    } else if (buf[length - 1] == '\n') {
        if (length > 1 && buf[length - 2] == '\r')
            *style = GDBMI_CR_NL;
        else
            *style = GDBMI_NL;
    }

    return 0;
}

/* 
 * 1. Align the 'input' data into null-terminated strings that
 *    end in a newline.
 * 2. Check to see if a command has been fully recieved.
 *    2a. If it has, go to step 3
 *    2b. If it hasn't, return from the function
 * 3. parse the command.
 * 4. traverse the parse tree to populate the tgdb_list with
 *    commands the user/front end is looking for.
 */
int gdbmi_parse_io(void *ctx,
        const char *input_data, const size_t input_data_size,
        char *debugger_output, size_t * debugger_output_size,
        char *inferior_output, size_t * inferior_output_size,
        struct tgdb_list *list)
{
    struct tgdb_gdbmi *gdbmi = (struct tgdb_gdbmi *) ctx;
    int found_command = 0;
    enum newlinestyle style = GDBMI_NL;

    ibuf_add(gdbmi->tgdb_cur_output_command, input_data);

    if (input_data[input_data_size - 1] == '\r' ||
            input_data[input_data_size - 1] == '\n') {
        if (gdbmi_get_newline_style(gdbmi->tgdb_cur_output_command,
                        &style) == -1) {
            logger_write_pos(logger, __FILE__, __LINE__,
                    "gdbmi_get_newline_stlye error");
            return -1;
        }

        if (ends_with_gdbmi_prompt(gdbmi->tgdb_cur_output_command, style,
                        &found_command) == -1) {
            logger_write_pos(logger, __FILE__, __LINE__,
                    "ends_with_gdbmi_prompt error");
            return -1;
        }
    }

    if (found_command) {
/*		gdbmi_walk_command ( gdbmi->tgdb_cur_output_command );*/
        ibuf_clear(gdbmi->tgdb_cur_output_command);
    }

    /* Return nothing for now */
    *debugger_output_size = 0;
    *inferior_output_size = 0;

    if (found_command)
        return 1;

    return 0;
}

struct tgdb_list *gdbmi_get_client_commands(void *ctx)
{
    struct tgdb_gdbmi *gdbmi = (struct tgdb_gdbmi *) ctx;

    return gdbmi->client_command_list;
}

#if 0
//
//int a2_get_source_absolute_filename ( 
//      void *ctx,
//      const char *file ) {
//  struct annotate_two *a2 = (struct annotate_two *)ctx;
//
//    if ( commands_issue_command ( 
//              a2->c, 
//              a2->client_command_list,
//              ANNOTATE_LIST, 
//              file, 
//              0 ) == -1 ) {
//        err_msg("%s:%d commands_issue_command error", __FILE__, __LINE__);
//        return -1;
//    }
//
//    if ( commands_issue_command ( 
//              a2->c, 
//              a2->client_command_list,
//              ANNOTATE_INFO_SOURCE_ABSOLUTE, 
//              file, 
//              0 ) == -1 ) {
//        err_msg("%s:%d commands_issue_command error", __FILE__, __LINE__);
//        return -1;
//    }
//
//  return 0;
//}
//
//int a2_get_inferior_sources ( void *ctx) {
//  struct annotate_two *a2 = (struct annotate_two *)ctx;
//    if ( commands_issue_command ( 
//              a2->c, 
//              a2->client_command_list,
//              ANNOTATE_INFO_SOURCES, 
//              NULL, 
//              0 ) == -1 ) {
//        err_msg("%s:%d commands_issue_command error", __FILE__, __LINE__);
//        return -1;
//    }
//
//  return 0;
//}
//
//int a2_change_prompt(
//      void *ctx,
//      const char *prompt) {
//  struct annotate_two *a2 = (struct annotate_two *)ctx;
//
//    /* Must call a callback to change the prompt */
//    if ( commands_issue_command ( 
//              a2->c, 
//              a2->client_command_list,
//              ANNOTATE_SET_PROMPT, 
//              prompt, 
//              2 ) == -1 ) {
//        err_msg("%s:%d commands_issue_command error", __FILE__, __LINE__);
//        return -1;
//    }
//            
//    return 0;
//}
//
//int a2_command_callback(
//      void *ctx,
//      const char *command) {
//  /* Unimplemented */
//  return -1;
//}
//
//char *a2_return_client_command ( void *ctx, enum tgdb_command_type c ) {
//  if ( c < TGDB_CONTINUE || c >= TGDB_ERROR )
//      return NULL;
//
//  return a2_tgdb_commands[c];
//}
//
//char *a2_client_modify_breakpoint ( 
//      void *ctx, 
//      const char *file, 
//      int line, 
//      enum tgdb_breakpoint_action b ) {
//  char *val = (char*)cgdb_malloc ( sizeof(char)* ( strlen(file) + 128 ) );
//
//  if ( b == TGDB_BREAKPOINT_ADD ) {
//      sprintf ( val, "break %s:%d", file, line ); 
//      return val;
//  } else if ( b == TGDB_BREAKPOINT_DELETE ) {
//      sprintf ( val, "clear %s:%d", file, line ); 
//      return val;
//  } else 
//      return NULL;
//}
#endif

pid_t gdbmi_get_debugger_pid(void *ctx)
{
    struct tgdb_gdbmi *gdbmi = (struct tgdb_gdbmi *) ctx;

    return gdbmi->debugger_pid;
}

#if 0
//int a2_completion_callback(
//      void *ctx,
//      const char *command) {
//  struct annotate_two *a2 = (struct annotate_two *)ctx;
//    if ( commands_issue_command ( 
//              a2->c, 
//              a2->client_command_list,
//              ANNOTATE_COMPLETE, command, 4 ) == -1 ) {
//        err_msg("%s:%d commands_issue_command error", __FILE__, __LINE__);
//        return -1;
//    }
//
//    return 0;
//}
#endif

int gdbmi_user_ran_command(void *ctx)
{
    return 0;
}

int gdbmi_prepare_for_command(void *ctx, struct tgdb_command *com)
{
    return 0;
}

#if 0
//int a2_is_misc_prompt ( void *ctx ) {
//  struct annotate_two *a2 = (struct annotate_two *)ctx;
//  return globals_is_misc_prompt ( a2->g );
//}
#endif
