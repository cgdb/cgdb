#ifndef __GDBMI_TGDB_H__
#define __GDBMI_TGDB_H__

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */

#include "tgdb_types.h"
#include "tgdb_command.h"
#include "logger.h"

/*! \file
 * gdbmi_tgdb.h
 * \brief
 * This interface documents the gdmi context.
 */

/** 
 * This struct is a reference to a libgdbmi instance.
 */
struct tgdb_gdbmi;

/**  
 * This should probably be moved out of gdbmi_tgdb.h
 */
enum gdbmi_commands {

    /**
	 * Currently not used.
	 */
    GDBMI_VOID = 0,

    /**
	 * Get a list of breakpoints.
	 */
    GDBMI_INFO_BREAKPOINTS,

    /**
     * Tell gdb where to send inferior's output
	 */
    GDBMI_TTY,

    /**
     * Complete the current console line
	 */
    GDBMI_COMPLETE,

    /**
 	 * Show all the sources inferior is made of
	 */
    GDBMI_INFO_SOURCES,

    /**
	 * relative source path.
	 */
    GDBMI_INFO_SOURCE_RELATIVE,

    /**
	 * absolute source path.
	 */
    GDBMI_INFO_SOURCE_ABSOLUTE,

    /**
 	 * Shows information on the current source file
	 */
    GDBMI_INFO_SOURCE,

    /**
 	 * displays the contents of current source file
	 */
    GDBMI_LIST,

    /**
	 * Sets the prompt.
	 */
    GDBMI_SET_PROMPT
};

/******************************************************************************/
/**
 * @name Starting and Stopping Commands.
 * These functions are for starting and stopping the gdbmi context.
 */
/******************************************************************************/

/*@{*/

/** 
 * This invokes a libgdbmi library instance.
 *
 * The client must call this function before any other function in the 
 * tgdb library.
 *
 * \param debugger_path
 * The path to the desired debugger to use. If this is NULL, then just
 * "gdb" is used.
 *
 * \param argc
 * The number of arguments to pass to the debugger
 *
 * \param argv
 * The arguments to pass to the debugger    
 *
 * \param config_dir
 * The current config directory. Files can be stored here.
 *
 * @return
 * NULL on error, A valid descriptor upon success
 */
void *gdbmi_create_context(const char *debugger_path,
        int argc, char **argv, const char *config_dir, struct logger *logger);

/** 
 * This initializes the libgdbmi libarary.
 *  
 * \param ctx 
 * The gdbmi context.
 *
 * \param debugger_stdin
 * Writing to this descriptor, writes to the stdin of the debugger.
 *
 * \param debugger_stdout
 * Reading from this descriptor, reads from the debugger's stdout.
 *
 * \param inferior_stdin
 * Writing to this descriptor, writes to the stdin of the inferior.
 *
 * \param inferior_stdout
 * Reading from this descriptor, reads from the inferior's stdout.
 *
 * @return Retruns
 * 0 on success, otherwise -1 on error.
 */
int gdbmi_initialize(void *ctx,
        int *debugger_stdin, int *debugger_stdout,
        int *inferior_stdin, int *inferior_stdout);

/**
 * Shuts down the gdbmi context. No more calls can be made on the
 * current context. It will clean up after itself. All descriptors it 
 * opened, it will close.
 *
 * \param ctx
 * The gdbmi context.
 *
 * @return
 * 0 on success, otherwise -1 on error.
 */
int gdbmi_shutdown(void *ctx);

/*@}*/

/******************************************************************************/
/**
 * @name Status Commands
 * These functions are for querying the gdbmi context.
 */
/******************************************************************************/

/*@{*/

/** 
 * Returns the last error message ?
 * Not implemented yet.
 * What should it return? How should errors be handled?
 *
 * \param ctx
 * The gdbmi context.
 *
 * @return
 * 0 on success, otherwise -1 on error.
 */
int gdbmi_err_msg(void *ctx);

/** 
 * This determines if the gdbmi context is ready to recieve
 * another command.
 *
 * \param ctx
 * The gdbmi context.
 *
 * @return
 * 1 if it is ready, 0 if it is not.
 */
int gdbmi_is_client_ready(void *ctx);

/** 
 * This lets the gdbmi know that the user ran a command.
 * The client can update itself here if it need to.
 *
 * \param ctx
 * The gdbmi context.
 *
 * @return
 * -1 on error, 0 on success
 */
int gdbmi_user_ran_command(void *ctx);

/** 
 *  Prepare's the client for the command COM to be run.
 *
 * \param ctx
 * The gdbmi context.
 *
 * \param com
 * The command to be run.
 *
 * @return
 * -1 on error, 0 on success
 */
int gdbmi_prepare_for_command(void *ctx, struct tgdb_command *com);

/** 
 * This is a hack. It should be removed eventually.
 * It tells tgdb-base not to send its internal commands when this is true.
 *
 * \param ctx
 * The gdbmi context.
 *
 * @return
 * 1 if it is at a misc prompt, 0 if it is not.
 */
int gdbmi_is_misc_prompt(void *ctx);

/*@}*/

/******************************************************************************/
/**
 * @name Input/Output commands
 * These functions are for communicating I/O with an gdbmi context.
 */
/******************************************************************************/

/*@{*/

 /** 
  * This recieves all of the output from the debugger. It is all routed 
  * through this function. 
  *
  * \param ctx
  * The gdbmi context.
  *
  * \param input_data
  * This is the stdout from the debugger. This is the data that parse_io 
  * will parse.
  *
  * \param input_data_size
  * This is the size of input_data.
  *
  * \param debugger_output
  * This is an out variable. It contains data that has been determined to
  * be the output of the debugger that the user should see.
  *
  * \param debugger_output_size
  * This is the size of debugger_output
  *
  * \param inferior_output
  * This is an out variable. It contains data that has been determined to
  * be the output of the inferior that the user should see.
  *
  * \param inferior_output_size
  * This is the size of inferior_output
  *
  * \param list
  * Any commands that the gdbmi context has discovered will
  * be added to the queue Q. This will eventually update the client
  * of the libtgdb library.
  *
  * @return
  * 1 when it has finished a command, 
  * 0 on success but hasn't recieved enough I/O to finish the command, 
  * otherwise -1 on error.
  */
int gdbmi_parse_io(void *ctx,
        const char *input_data, const size_t input_data_size,
        char *debugger_output, size_t * debugger_output_size,
        char *inferior_output, size_t * inferior_output_size,
        struct tgdb_list *list);

/**
 * Returns all of the commands the gdbmi subsystem generated during
 * the last call.
 *
 * \param ctx
 * The gdbmi context.
 *
 * @return
 * NULL if no commands were generated.
 * Otherwise, a list of tgdb_client_commands.
 */
struct tgdb_list *gdbmi_get_client_commands(void *ctx);

/*@}*/

/******************************************************************************/
/**
 * @name Functional commands
 * These functinos are used to ask an gdbmi context to perform a task.
 */
/******************************************************************************/

/*@h*/

/** 
 * Gets the Absolute path of FILE.
 *  
 * \param ctx
 * The gdbmi context.
 *
 * \param file
 * The relative path that gdb outputted.
 *
 * @return
 * 0 on success, otherwise -1 on error.
 */
int gdbmi_get_source_absolute_filename(void *ctx, const char *file);

/** 
 * Gets all the source files that the inferior makes up.
 *
 * \param ctx
 * The gdbmi context.
 *
 * @return
 * 0 on success, otherwise -1 on error.
 */
int gdbmi_get_inferior_sources(void *ctx);

/** 
 * This will change the prompt the user sees to PROMPT.
 *
 * \param ctx
 * The gdbmi context.
 *
 * \param prompt
 * The new prompt to change too.
 *
 * @return
 * 0 on success, otherwise -1 on error.
 */
int gdbmi_change_prompt(void *ctx, const char *prompt);

/** 
 * This is called when readline determines a command has been typed. 
 *
 * \param ctx
 * The gdbmi context.
 *
 * \param command
 * The command the user typed without the '\n'.
 *
 * @return
 * 0 on success, otherwise -1 on error.
 */
int gdbmi_command_callback(void *ctx, const char *command);

/** 
 * This is called when readline determines a command needs to be completed.
 *
 * \param ctx
 * The gdbmi context.
 *
 * \param command
 * The command to be completed
 *
 * @return
 * 0 on success, otherwise -1 on error.
 */
int gdbmi_completion_callback(void *ctx, const char *command);

/** 
 * This returns the command to send to gdb for the enum C.
 * It will return NULL on error, otherwise correct string on output.
 *
 * \param ctx
 * The gdbmi context.
 *
 * \param c
 * The command to run.
 *
 * @return
 * Command on success, otherwise NULL on error.
 */
char *gdbmi_return_client_command(void *ctx, enum tgdb_command_type c);

/** 
 * \param ctx
 * The gdbmi context.
 *
 * \param file
 * The file to set the breakpoint in.
 *
 * \param line
 * The line in FILE to set the breakpoint in.
 *
 * \param b
 * Determines what the user wants to do with the breakpoint.
 *
 * @return
 * NULL on error or message to print to terminal
 */
char *gdbmi_client_modify_breakpoint(void *ctx,
        const char *file, int line, enum tgdb_breakpoint_action b);

/** 
 * \param ctx
 * The gdbmi context.
 *
 * @return 
 * -1 on error. Or pid on Success.
 */
pid_t gdbmi_get_debugger_pid(void *ctx);

/*@}*/

/******************************************************************************/
/**
 * @name Inferior tty commands
 * These functinos are used to alter an gdbmi contexts tty state.
 */
/******************************************************************************/

/*@{*/

/** 
 * \param ctx
 * The gdbmi context.
 *
 * \param inferior_stdin
 * Writing to this descriptor, writes to the stdin of the inferior.
 *
 * \param inferior_stdout
 * Reading from this descriptor, reads from the inferior's stdout.
 *
 * @return
 * 0 on success, otherwise -1 on error.
 */
int gdbmi_open_new_tty(void *ctx, int *inferior_stdin, int *inferior_stdout);

/** 
 * \param ctx
 * The gdbmi context.
 *
 * @return
 * tty name on success, otherwise NULL on error.
 */
char *gdbmi_get_tty_name(void *ctx);

/*@}*/

#endif /* __GDBMI_TGDB_H__ */
