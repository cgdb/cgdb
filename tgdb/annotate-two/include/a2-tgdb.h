#ifndef __A2_TGDB_H__
#define __A2_TGDB_H__

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */

#include "types.h"
#include "tgdb_interface.h"

/*! \file
 * 		a2-tgdb.h
 * 	\brief
 * 		This interface documents the annotate two context.
 */

/** 
 * annotate-two
 *
 * This struct is a reference to a libannotate-two instance.
 */
struct annotate_two;

/**  
 * Commands
 *
 *   info breakpoints    -> Get a list of breakpoints
 *   tty                 -> Tell gdb where to send inferior's output
 *   complete            -> Complete the current console line
 *   info sources        -> Show all the sources inferior is made of
 *   info source         -> Shows information on the current source file
 *   list                -> displays the contents of current source file
 *   complete arg        -> completes the argument arg
 */
enum annotate_commands {
    ANNOTATE_VOID = 0,
    ANNOTATE_INFO_BREAKPOINTS,
    ANNOTATE_TTY,
    ANNOTATE_COMPLETE,
    ANNOTATE_INFO_SOURCES,
    ANNOTATE_INFO_SOURCE_RELATIVE,
    ANNOTATE_INFO_SOURCE_ABSOLUTE,
    ANNOTATE_INFO_SOURCE,
    ANNOTATE_LIST,
    ANNOTATE_SET_PROMPT
};

/******************************************************************************/
/**
 * @name Starting and Stopping Commands.
 * These functions are for starting and stopping the annotate_two context.
 */
/******************************************************************************/

//@{

/** 
 * a2_create_instance
 *
 * This invokes a libannotate_two library instance.
 *
 * The client must call this function before any other function in the 
 * tgdb library.
 *
 * \param debugger
 *	The path to the desired debugger to use. If this is NULL, then just
 *  "gdb" is used.
 *
 * \param argc
 *  The number of arguments to pass to the debugger
 *
 * \param argv
 *  The arguments to pass to the debugger    
 *
 * \param config_dir
 *  The current config directory. Files can be stored here.
 *
 * @return
 *  NULL on error, A valid descriptor upon success
 */
struct annotate_two* a2_create_instance ( 
	const char *debugger, 
	int argc, char **argv,
	const char *config_dir );

/** 
 * a2_initialize
 *
 * This initializes the libannotate_two libarary.
 *  
 * \param a2
 *  The annotate two context.
 *
 * \param command_container
 *  A list of commands that was generated from this call.
 *
 * \param debugger_stdin
 *  Writing to this descriptor, writes to the stdin of the debugger.
 *
 * \param debugger_stdout
 *  Reading from this descriptor, reads from the debugger's stdout.
 *
 * \param inferior_stdin
 *  Writing to this descriptor, writes to the stdin of the inferior.
 *
 * \param inferior_stdout
 *  Reading from this descriptor, reads from the inferior's stdout.
 *
 * @retrun Retruns
 *  0 on success, otherwise -1 on error.
 */
int a2_initialize ( 
	struct annotate_two *a2, 
	struct queue *command_container,
	int *debugger_stdin, int *debugger_stdout,
	int *inferior_stdin, int *inferior_stdout );

/** a2_shutdown
 *
 * Shuts down the annotate two context. No more calls can be made on the
 * current context. It will clean up after itself. All descriptors it 
 * opened, it will close.
 *
 * \param a2
 *  The annotate two context.
 *
 * @return
 *  0 on success, otherwise -1 on error.
 */
int a2_shutdown ( struct annotate_two *a2 );

//@}

/******************************************************************************/
/**
 * @name Status Commands
 * These functions are for querying the annotate_two context.
 */
/******************************************************************************/

//@{

/** 
 * a2_err_msg
 *
 * Returns the last error message ?
 * Not implemented yet.
 * What should it return? How should errors be handled?
 *
 * \param a2
 *  The annotate two context.
 *
 * @return
 *  0 on success, otherwise -1 on error.
 */
int a2_err_msg ( struct annotate_two *a2 );

/** 
 * a2_is_client_ready
 *
 * This determines if the annotate two context is ready to recieve
 * another command.
 *
 * \param a2
 *  The annotate two context.
 *
 * @return
 *  1 if it is ready, 0 if it is not.
 */
int a2_is_client_ready(struct annotate_two *a2);

/** 
 * a2_user_ran_command
 *
 * This lets the annotate_two know that the user ran a command.
 * The client can update itself here if it need to.
 *
 * \param a2
 *  The annotate two context.
 *
 * \param command_container
 *  A list of commands that was generated from this call.
 *
 * @return
 * 	-1 on error, 0 on success
 */
int a2_user_ran_command ( struct annotate_two *a2, struct queue *command_container );

/** 
 * a2_prepare_for_command
 *
 *  Prepare's the client for the command COM to be run.
 *
 * \param a2
 *  The annotate two context.
 *
 *  \param com
 *  	The command to be run.
 *
 * @return
 * 	-1 on error, 0 on success
 */
int a2_prepare_for_command ( struct annotate_two *a2, struct command *com );

/** 
 * a2_is_misc_prompt
 *
 *  This is a hack. It should be removed eventually.
 *  It tells tgdb-base not to send its internal commands when this is true.
 *
 * \param a2
 *  The annotate two context.
 *
 * @return
 *  1 if it is at a misc prompt, 0 if it is not.
 */
int a2_is_misc_prompt ( struct annotate_two *a2 );

//@}

/******************************************************************************/
/**
 * @name Input/Output commands
 * These functions are for communicating I/O with an annotate two context.
 */
/******************************************************************************/

//@{

 /** 
  * a2_parse_io
  *
  * This recieves all of the output from the debugger. It is all routed 
  * through this function. 
  *
  * \param a2
  *  The annotate two context.
  *
  * \param command_container
  *  A list of commands that was generated from this call.
  *
  * \param input_data
  *  This is the stdout from the debugger. This is the data that parse_io 
  *  will parse.
  *
  * \param input_data_size
  *  This is the size of input_data.
  *
  * \param debugger_output
  *  This is an out variable. It contains data that has been determined to
  *  be the output of the debugger that the user should see.
  *
  * \param debugger_output_size
  *  This is the size of debugger_output
  *
  * \param inferior_output
  *  This is an out variable. It contains data that has been determined to
  *  be the output of the inferior that the user should see.
  *
  * \param inferior_output_size
  *  This is the size of inferior_output
  *
  * \param q
  *  Any commands that the annotate_two context has discovered will
  *  be added to the queue Q. This will eventually update the client
  *  of the libtgdb library.
  *
  * @return
  *  1 when it has finished a command, 
  *  0 on success but hasn't recieved enough I/O to finish the command, 
  *  otherwise -1 on error.
  */
int a2_parse_io ( 
		struct annotate_two *a2,
		struct queue *command_container,
		const char *input_data, const size_t input_data_size,
		char *debugger_output, size_t *debugger_output_size,
		char *inferior_output, size_t *inferior_output_size,
		struct queue *q );
//@}


/******************************************************************************/
/**
 * @name Functional commands
 * These functinos are used to ask an annotate_two context to perform a task.
 */
/******************************************************************************/

//@{

/** 
 * a2_get_source_absolute_filename
 *
 *  Gets the Absolute path of FILE.
 *  
 *  \param a2
 *   The annotate two context.
 *
 *  \param command_container
 *   A list of commands that was generated from this call.
 *
 *  \param file
 *   The relative path that gdb outputted.
 *
 * @return
 *   0 on success, otherwise -1 on error.
 */
int a2_get_source_absolute_filename ( 
		struct annotate_two *a2, 
		struct queue *command_container,
		const char *file );

/** 
 * a2_get_inferior_sources
 *
 *  Gets all the source files that the inferior makes up.
 *
 *  \param a2
 *   The annotate two context.
 *
 *  \param command_container
 *   A list of commands that was generated from this call.
 *
 * @return
 *   0 on success, otherwise -1 on error.
 */
int a2_get_inferior_sources ( 
		struct annotate_two *a2, 
		struct queue *command_container );

/** 
 * a2_change_prompt
 *
 *  This will change the prompt the user sees to PROMPT.
 *
 * \param a2
 *  The annotate two context.
 *
 * \param prompt
 *  The new prompt to change too.
 *
 * \param command_container
 *  A list of commands that was generated from this call.
 *
 * @return
 *   0 on success, otherwise -1 on error.
 */
int a2_change_prompt(
		struct annotate_two *a2, 
		struct queue *command_container,
		const char *prompt);

/** 
 * a2_command_callback
 *
 * This is called when readline determines a command has been typed. 
 *
 * \param a2
 *  The annotate two context.
 *
 * \param command_container
 *  A list of commands that was generated from this call.
 *
 * \param command
 *  The command the user typed without the '\n'.
 *
 * @return
 *   0 on success, otherwise -1 on error.
 */
int a2_command_callback(
		struct annotate_two *a2, 
		struct queue *command_container,
		const char *command);

/** 
 * a2_completion_callback
 *
 * This is called when readline determines a command needs to be completed.
 *
 * \param a2
 *  The annotate two context.
 *
 * \param command_container
 *  A list of commands that was generated from this call.
 *
 * \param command
 *  The command to be completed
 *
 * @return
 *   0 on success, otherwise -1 on error.
 */
int a2_completion_callback(
		struct annotate_two *a2, 
		struct queue *command_container,
		const char *command);

/** 
 * a2_return_client_command
 *
 *  This returns the command to send to gdb for the enum C.
 *  It will return NULL on error, otherwise correct string on output.
 *
 * \param a2
 *  	The annotate two context.
 *
 * \param c
 *  	The command to run.
 *
 * @return
 *   	Command on success, otherwise NULL on error.
 */
char *a2_return_client_command ( struct annotate_two *a2, enum tgdb_command c );

/** 
 * a2_client_modify_breakpoint
 *
 * \param a2
 * 	The annotate two context.
 *
 * \param file
 * 	The file to set the breakpoint in.
 * \param line
 * 	The line in FILE to set the breakpoint in.
 * \param b
 * 	Determines what the user wants to do with the breakpoint.
 *
 * @return
 * 	NULL on error or message to print to terminal
 */
char *a2_client_modify_breakpoint ( struct annotate_two *a2, const char *file, int line, enum tgdb_breakpoint_action b );

/** 
 * a2_get_debuger_pid
 *
 * \param a2
 *  The annotate two context.
 *
 *  @return 
 *  -1 on error. Or pid on Success.
 */
pid_t a2_get_debugger_pid ( struct annotate_two *a2 );

//@}

/******************************************************************************/
/**
 * @name Inferior tty commands
 * These functinos are used to alter an annotate_two contexts tty state.
 */
/******************************************************************************/

//@{

/** 
 * a2_open_new_tty
 *
 * \param a2
 *  The annotate two context.
 *
 * \param command_container
 *  A list of commands that was generated from this call.
 *
 * \param inferior_stdin
 *  Writing to this descriptor, writes to the stdin of the inferior.
 *
 * \param inferior_stdout
 *  Reading from this descriptor, reads from the inferior's stdout.
 *
 * @return
 *  0 on success, otherwise -1 on error.
 */
int a2_open_new_tty ( 
		struct annotate_two *a2, 
		struct queue *command_container,
		int *inferior_stdin, 
		int *inferior_stdout );

/** 
 * a2_get_tty_name
 *
 * \param a2
 *  The annotate two context.
 *
 * @return
 * 	tty name on success, otherwise NULL on error.
 */
char *a2_get_tty_name ( struct annotate_two *a2 );

//@}

#endif /* __A2_TGDB_H__ */
