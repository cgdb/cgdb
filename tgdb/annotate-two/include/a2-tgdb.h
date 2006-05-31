#ifndef __A2_TGDB_H__
#define __A2_TGDB_H__

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
 * a2-tgdb.h
 * \brief
 * This interface documents the annotate two context.
 */

/** 
 * This struct is a reference to a libannotate-two instance.
 */
struct annotate_two;

/**  
 * This should probably be moved out of a2-tgdb.h
 */
enum annotate_commands {

	/**
	 * Currently not used.
	 */
    ANNOTATE_VOID = 0,

	/**
	 * Get a list of breakpoints.
	 */
    ANNOTATE_INFO_BREAKPOINTS,

	/**
     * Tell gdb where to send inferior's output
	 */
    ANNOTATE_TTY,

	/**
     * Complete the current console line
	 */
    ANNOTATE_COMPLETE,

	/**
 	 * Show all the sources inferior is made of
	 */
    ANNOTATE_INFO_SOURCES,

	/**
	 * relative source path.
	 */
    ANNOTATE_INFO_SOURCE_RELATIVE,

	/**
	 * absolute source path.
	 */
    ANNOTATE_INFO_SOURCE_FILENAME_PAIR,

	/**
 	 * Shows information on the current source file
	 */
    ANNOTATE_INFO_SOURCE,

	/**
 	 * displays the contents of current source file
	 */
    ANNOTATE_LIST,

	/**
 	 * Get's the current fullname, filename and line number.
	 * This is because the 'info line' command to GDB generates the source
	 * annotation, which in turn causes the a2 subsystem to get the 
	 * relative path.
	 */
    ANNOTATE_INFO_LINE,

	/**
	 * Sets the prompt.
	 */
    ANNOTATE_SET_PROMPT
};

/******************************************************************************/
/**
 * @name Starting and Stopping Commands.
 * These functions are for starting and stopping the annotate_two context.
 */
/******************************************************************************/

/*@{*/

/** 
 * This invokes a libannotate_two library instance.
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
void *a2_create_context ( 
	const char *debugger_path, 
	int argc, char **argv,
	const char *config_dir,
    struct logger *logger );

/** 
 * This initializes the libannotate_two libarary.
 *  
 * \param a2
 * The annotate two context.
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
int a2_initialize ( 
	void *a2, 
	int *debugger_stdin, int *debugger_stdout,
	int *inferior_stdin, int *inferior_stdout );

/**
 * Shuts down the annotate two context. No more calls can be made on the
 * current context. It will clean up after itself. All descriptors it 
 * opened, it will close.
 *
 * \param ctx
 * The annotate two context.
 *
 * @return
 * 0 on success, otherwise -1 on error.
 */
int a2_shutdown ( void *ctx );

/*@}*/

/******************************************************************************/
/**
 * @name Status Commands
 * These functions are for querying the annotate_two context.
 */
/******************************************************************************/

/*@{*/

/** 
 * Returns the last error message ?
 * Not implemented yet.
 * What should it return? How should errors be handled?
 *
 * \param ctx
 * The annotate two context.
 *
 * @return
 * 0 on success, otherwise -1 on error.
 */
int a2_err_msg ( void *ctx );

/** 
 * This determines if the annotate two context is ready to recieve
 * another command.
 *
 * \param ctx
 * The annotate two context.
 *
 * @return
 * 1 if it is ready, 0 if it is not.
 */
int a2_is_client_ready(void *ctx);

/** 
 * This lets the annotate_two know that the user ran a command.
 * The client can update itself here if it need to.
 *
 * \param ctx
 * The annotate two context.
 *
 * @return
 * -1 on error, 0 on success
 */
int a2_user_ran_command ( void *ctx );

/** 
 *  Prepare's the client for the command COM to be run.
 *
 * \param ctx
 * The annotate two context.
 *
 * \param com
 * The command to be run.
 *
 * @return
 * -1 on error, 0 on success
 */
int a2_prepare_for_command ( void *ctx, struct tgdb_command *com );

/** 
 * This is a hack. It should be removed eventually.
 * It tells tgdb-base not to send its internal commands when this is true.
 *
 * \param ctx
 * The annotate two context.
 *
 * @return
 * 1 if it is at a misc prompt, 0 if it is not.
 */
int a2_is_misc_prompt ( void *ctx );

/*@}*/

/******************************************************************************/
/**
 * @name Input/Output commands
 * These functions are for communicating I/O with an annotate two context.
 */
/******************************************************************************/

/*@{*/

 /** 
  * This recieves all of the output from the debugger. It is all routed 
  * through this function. 
  *
  * \param ctx
  * The annotate two context.
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
  * Any commands that the annotate_two context has discovered will
  * be added to the queue Q. This will eventually update the client
  * of the libtgdb library.
  *
  * @return
  * 1 when it has finished a command, 
  * 0 on success but hasn't recieved enough I/O to finish the command, 
  * otherwise -1 on error.
  */
int a2_parse_io ( 
		void *ctx,
		const char *input_data, const size_t input_data_size,
		char *debugger_output, size_t *debugger_output_size,
		char *inferior_output, size_t *inferior_output_size,
		struct tgdb_list *list );

/**
 * Returns all of the commands the annotate two subsystem generated during
 * the last call.
 *
 * \param ctx
 * The annotate two context.
 *
 * @return
 * NULL if no commands were generated.
 * Otherwise, a list of tgdb_client_commands.
 */
struct tgdb_list *a2_get_client_commands ( void *ctx );

/*@}*/

/******************************************************************************/
/**
 * @name Functional commands
 * These functinos are used to ask an annotate_two context to perform a task.
 */
/******************************************************************************/

/*@{*/

/** 
 * Gets the Absolute path of FILE.
 *  
 * \param ctx
 * The annotate two context.
 *
 * \param file
 * The relative path that gdb outputted.
 *
 * @return
 * 0 on success, otherwise -1 on error.
 */
int a2_get_source_filename_pair ( 
		void *ctx, 
		const char *file );

/**
 * Get's the fullname, filename and line number GDB is currently at.
 *
 * \param ctx
 * The annotate two context.
 *
 * \param on_startup
 * This variable can be set to 1 if the front end wants to probe GDB
 * for the initial file and location of the program being debugged.
 * However, each initial time after that, this variable should be 
 * set to 0.
 *
 * @return
 * 0 on success, otherwise -1 on error.
 */
int a2_get_current_location (void *ctx, int on_startup);

/** 
 * Gets all the source files that the inferior makes up.
 *
 * \param ctx
 * The annotate two context.
 *
 * @return
 * 0 on success, otherwise -1 on error.
 */
int a2_get_inferior_sources ( void *ctx );

/** 
 * This is called when readline determines a command needs to be completed.
 *
 * \param ctx
 * The annotate two context.
 *
 * \param command
 * The command to be completed
 *
 * @return
 * 0 on success, otherwise -1 on error.
 */
int a2_completion_callback( void *ctx, const char *command);

/** 
 * This returns the command to send to gdb for the enum C.
 * It will return NULL on error, otherwise correct string on output.
 *
 * \param ctx
 * The annotate two context.
 *
 * \param c
 * The command to run.
 *
 * @return
 * Command on success, otherwise NULL on error.
 */
char *a2_return_client_command ( void *ctx, enum tgdb_command_type c );

/** 
 * \param ctx
 * The annotate two context.
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
char *a2_client_modify_breakpoint ( 
		void *ctx, 
		const char *file, 
		int line, 
		enum tgdb_breakpoint_action b );

/** 
 * \param ctx
 * The annotate two context.
 *
 * @return 
 * -1 on error. Or pid on Success.
 */
pid_t a2_get_debugger_pid ( void *ctx );

/*@}*/

/******************************************************************************/
/**
 * @name Inferior tty commands
 * These functinos are used to alter an annotate_two contexts tty state.
 */
/******************************************************************************/

/*@{*/

/** 
 * \param ctx
 * The annotate two context.
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
int a2_open_new_tty ( void *ctx, int *inferior_stdin, int *inferior_stdout );

/** 
 * \param ctx
 * The annotate two context.
 *
 * @return
 * tty name on success, otherwise NULL on error.
 */
const char *a2_get_tty_name ( void *ctx );

/*@}*/

#endif /* __A2_TGDB_H__ */
