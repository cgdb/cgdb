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

/* Here is a prototype of the interface planned for .4.0 
 *
 */

/* annotate-two
 * ------------
 *
 * This struct is a reference to a libannotate-two instance.
 */
struct annotate_two;

/*  The only commands used
    ----------------------------
    info breakpoints    -> Get a list of breakpoints
    tty                 -> Tell gdb where to send inferior's output
    complete            -> Complete the current console line
    info sources        -> Show all the sources inferior is made of
    info source         -> Shows information on the current source file
    list                -> displays the contents of current source file
    complete arg        -> completes the argument arg
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

/***************************************************************************
 * Starting and Stopping Commands.
 **************************************************************************/

/* a2_create_instance
 * ------------------
 *
 * This invokes a libannotate_two library instance.
 *
 * The client must call this function before any other function in the 
 * tgdb library.
 *
 * debugger
 * --------
 *  The path to the desired debugger to use. If this is NULL, then just
 *  "gdb" is used.
 *
 * argc
 * ----
 *  The number of arguments to pass to the debugger
 *
 * argv
 * ----
 *  The arguments to pass to the debugger    
 *
 * config_dir
 * ----------
 *  The current config directory. Files can be stored here.
 *
 * Returns
 * -------
 *  NULL on error, A valid descriptor upon success
 *
 */
struct annotate_two* a2_create_instance ( 
	const char *debugger, 
	int argc, char **argv,
	const char *config_dir );

/* a2_initialize
 * -------------
 *
 * This initializes the libannotate_two libarary.
 *  
 * a2
 * --
 *  The annotate two context.
 *
 * debugger_stdin
 * --------------
 *  Writing to this descriptor, writes to the stdin of the debugger.
 *
 * debugger_stdout
 * ---------------
 *  Reading from this descriptor, reads from the debugger's stdout.
 *
 * inferior_stdin
 * --------------
 *  Writing to this descriptor, writes to the stdin of the inferior.
 *
 * inferior_stdout
 * ---------------
 *  Reading from this descriptor, reads from the inferior's stdout.
 *
 * Retruns
 * -------
 *  0 on success, otherwise -1 on error.
 */
int a2_initialize ( 
	struct annotate_two *a2, 
	int *debugger_stdin, int *debugger_stdout,
	int *inferior_stdin, int *inferior_stdout,
	command_completed command_finished);

/* a2_shutdown
 * -----------
 *
 * Shuts down the annotate two context. No more calls can be made on the
 * current context. It will clean up after itself. All descriptors it 
 * opened, it will close.
 *
 * a2
 * --
 *  The annotate two context.
 *
 * Retruns
 * -------
 *  0 on success, otherwise -1 on error.
 */
int a2_shutdown ( struct annotate_two *a2 );

/***************************************************************************
 * Status Commands
 **************************************************************************/

/* a2_err_msg
 * ----------
 *
 * Returns the last error message ?
 * Not implemented yet.
 * What should it return? How should errors be handled?
 *
 * a2
 * --
 *  The annotate two context.
 *
 * Returns
 * -------
 *  0 on success, otherwise -1 on error.
 */
int a2_err_msg ( struct annotate_two *a2 );

/* a2_is_client_ready
 * ------------------
 *
 * This determines if the annotate two subsystem is ready to recieve
 * another command.
 *
 * a2
 * --
 *  The annotate two context.
 *
 * Returns
 * -------
 *  1 if it is ready, 0 if it is not.
 */
int a2_is_client_ready(struct annotate_two *a2);

/* a2_user_ran_command
 * -------------------
 *
 * This lets the annotate_two know that the user ran a command.
 * The client can update itself here if it need to.
 *
 * a2
 * --
 *  The annotate two context.
 *
 *
 * Returns: -1 on error, 0 on success
 */
int a2_user_ran_command ( struct annotate_two *a2 );

/* a2_prepare_for_command
 * ----------------------
 *
 *  Prepare's the client for the command COM to be run.
 *
 * a2
 * --
 *  The annotate two context.
 *
 *  com:    The command to be run.
 */
int a2_prepare_for_command ( struct annotate_two *a2, struct command *com );

/* a2_is_misc_prompt
 * -----------------
 *
 *  This is a hack. It should be removed eventually.
 *  It tells tgdb-base not to send its internal commands when this is true.
 *
 * a2
 * --
 *  The annotate two context.
 *
 * Returns
 * -------
 *  1 if it is at a misc prompt, 0 if it is not.
 */
int a2_is_misc_prompt ( struct annotate_two *a2 );

/***************************************************************************
 * Input/Output Commands
 **************************************************************************/

 /* a2_parse_io
  * -----------
  *
  * This recieves all of the output from the debugger. It is all routed 
  * through this function. 
  *
  * a2
  * --
  *  The annotate two context.
  *
  * input_data
  * ----------
  *  This is the stdout from the debugger. This is the data that parse_io 
  *  will parse.
  *
  * input_data_size
  * ---------------
  *  This is the size of debuggger_stdout
  *
  * debugger_output
  * ---------------
  *  This is an out variable. It contains data that has been determined to
  *  be the output of the debugger that the user should see.
  *
  * debugger_output_size
  * --------------------
  *  This is the size of debugger_output
  *
  * inferior_output
  * ---------------
  *  This is an out variable. It contains data that has been determined to
  *  be the output of the inferior that the user should see.
  *
  * inferior_output_size
  * --------------------
  *  This is the size of inferior_output
  *
  * q
  * -
  *  Any commands that the annotate_two subsystem has discovered will
  *  be added to the queue Q. This will eventually update the client
  *  of the libtgdb library.
  *
  * Returns
  * -------
  *  0 on success, otherwise -1 on error.
  */
int a2_parse_io ( 
		struct annotate_two *a2,
		const char *input_data, const size_t input_data_size,
		char *debugger_output, size_t *debugger_output_size,
		char *inferior_output, size_t *inferior_output_size,
		struct queue *q );

/***************************************************************************
 * Functional Commands
 **************************************************************************/

/* a2_get_source_absolute_filename
 * ------------------------------------
 *
 *  Gets the Absolute path of FILE.
 *  
 *  a2
 *  --
 *   The annotate two context.
 *
 *  file
 *  ----
 *   The relative path that gdb outputted.
 */
int a2_get_source_absolute_filename ( struct annotate_two *a2, const char *file );

/* a2_get_inferior_sources
 * -----------------------
 *
 *  Gets all the source files that the inferior makes up.
 *
 *  a2
 *  --
 *   The annotate two context.
 */
int a2_get_inferior_sources ( struct annotate_two *a2 );

/* a2_change_prompt
 * ----------------
 *
 *  This will change the prompt the user sees to PROMPT.
 *
 * a2
 * --
 *  The annotate two context.
 *
 * prompt
 * ------
 *  The new prompt to change too.
 */
int a2_change_prompt(struct annotate_two *a2, const char *prompt);

/* a2_command_callback
 * ------------------------
 *
 * This is called when readline determines a command has been typed. 
 *
 * a2
 * --
 *  The annotate two context.
 *
 * command
 * ---- 
 *  The command the user typed without the '\n'.
 */
int a2_command_callback(struct annotate_two *a2, const char *command);

/* a2_completion_callback
 * ---------------------------
 *
 * This is called when readline determines a command needs to be completed.
 *
 * a2
 * --
 *  The annotate two context.
 *
 * command
 * ----
 *  The command to be completed
 */
int a2_completion_callback(struct annotate_two *a2, const char *command);

/* a2_return_client_command
 * -----------------------------
 *
 *  This returns the command to send to gdb for the enum C.
 *  It will return NULL on error, otherwise correct string on output.
 *
 * a2
 * --
 *  The annotate two context.
 *
 * c
 * -
 *  The command to run.
 */
char *a2_return_client_command ( struct annotate_two *a2, enum tgdb_command c );

/* a2_client_modify_breakpoint
 * --------------------------------
 *
 * a2
 * --
 *  The annotate two context.
 *
 * Look at tgdb.h
 *
 */
char *a2_client_modify_breakpoint ( struct annotate_two *a2, const char *file, int line, enum tgdb_breakpoint_action b );

/* a2_get_debuger_pid
 * ------------------
 *
 * a2
 * --
 *  The annotate two context.
 *
 *  Returns -1 on error. Or pid on Success.
 */
pid_t a2_get_debugger_pid ( struct annotate_two *a2 );

/***************************************************************************
 * Inferior tty commands
 **************************************************************************/

/* a2_open_new_tty
 * ---------------
 *
 * a2
 * --
 *  The annotate two context.
 *
 * inferior_stdin
 * --------------
 *  Writing to this descriptor, writes to the stdin of the inferior.
 *
 * inferior_stdout
 * ---------------
 *  Reading from this descriptor, reads from the inferior's stdout.
 *
 * Retruns
 * -------
 *  0 on success, otherwise -1 on error.
 */
int a2_open_new_tty ( struct annotate_two *a2, int *inferior_stdin, int *inferior_stdout );

/* a2_get_tty_name
 * ---------------
 *
 * a2
 * --
 *  The annotate two context.
 *
 * Retruns
 * -------
 *  tty name on success, otherwise NULL on error.
 */
char *a2_get_tty_name ( struct annotate_two *a2 );

#endif /* __A2_TGDB_H__ */
