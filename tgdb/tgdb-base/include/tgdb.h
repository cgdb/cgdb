#ifndef __TGDB_H__
#define __TGDB_H__

/* tgdb:
 * -----
 *
 *  This is the interface to the gui.
 *  The gui can call any functions in this interface.
 */

#ifdef __cplusplus
extern "C" {
#endif 

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */

#include "types.h"

/* tgdb
 * ----
 *
 *  This struct is a reference to a libtgdb instance.
 */
struct tgdb;

/* tgdb_initialize
 * ---------------
 *
 * This initializes a tgdb library instance. It starts up the debugger and 
 * returns all file descriptors the client must select on.
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
 * debugger_fd
 * -----------
 *  The descriptor to the debugger's I/O
 *
 * inferior_fd
 * -----------
 *  The descriptor to the I/O of the program being debugged.
 *
 * readline_fd
 * -----------
 *  The descriptor to the readline I/O library
 *
 *  Returns
 *  -------
 *      NULL on error, A valid descriptor upon success
 *
 */
struct tgdb* tgdb_initialize ( 
	const char *debugger, 
	int argc, char **argv,
	int *debugger_fd, int *inferior_fd, int *readline_fd );

/* tgdb_shutdown
 * -------------
 *
 *  This will terminate a libtgdb session. No functions should be called on
 *  the tgdb context passed into this function after this call.
 *
 * tgdb
 * ----
 *  An instance of the tgdb library to operate on.
 *
 *  Returns
 *  -------
 *   0 on success or -1 on error
 */
int tgdb_shutdown ( struct tgdb *tgdb );

/* tgdb_err_msg
 * ------------
 *
 *  If a function returns an error, this can be called to report more
 *  about the type of error, or the error message itself.
 *
 * tgdb
 * ----
 *  An instance of the tgdb library to operate on.
 *
 * Returns
 * -------
 *  Error Message or NULL on error
 */
char *tgdb_err_msg ( struct tgdb *tgdb );

/* tgdb_send_debugger_char
 * -----------------------
 *
 * This sends a byte of data to the debugger.
 *
 * tgdb
 * ----
 *  An instance of the tgdb library to operate on.
 *
 * c
 * -
 *  The character to pass to the debugger.
 *
 * Returns
 * -------
 *      0 on success or -1 on error
 */
int tgdb_send_debugger_char ( struct tgdb *tgdb, char c );

/* tgdb_send_debugger_data
 * -----------------------
 *
 * This sends a string of data to the debugger.
 *
 * tgdb
 * ----
 *     An instance of the tgdb library to operate on.
 *
 * buf
 * ---
 *     The string to pass to the debugger.
 *
 * n
 * -
 *  	The number of bytes in BUF to send to debugger.
 *
 * Returns
 * -------
 *      0 on success or -1 on error
 */
int tgdb_send_debugger_data ( struct tgdb *tgdb, const char *buf, const size_t n );

/* tgdb_send_inferior_char
 * -----------------------
 *
 * This sends a byte of data to the program being debugged.
 *
 * tgdb
 * ----
 *  An instance of the tgdb library to operate on.
 *
 * c
 * -
 *  The character to pass to the program being debugged.
 *
 * Returns
 * -------
 *      0 on success or -1 on error
 */
int tgdb_send_inferior_char ( struct tgdb *tgdb, char c );

/* tgdb_send_inferior_data
 * -----------------------
 *
 * This sends a string of data to the program being debugged.
 *
 * tgdb
 * ----
 *     An instance of the tgdb library to operate on.
 *
 * buf
 * ---
 *     The string to pass to the program being debugged.
 *
 * n
 * -
 *  	The number of bytes in BUF to send to the program being debugged.
 *
 * Returns
 * -------
 *      0 on success or -1 on error
 */
int tgdb_send_inferior_data ( struct tgdb *tgdb, const char *buf, const size_t n );

/* tgdb_recv_inferior_data
 * -----------------------
 *
 * Gets the ouput from the program being debugged.
 * 
 * tgdb
 * ----
 *  An instance of the tgdb library to operate on.
 *
 * buf
 * ---
 *  The output of the program being debugged will be returned here.
 *
 * n
 * -
 *  Tells libtgdb how large the buffer BUF is.
 *
 * Returns
 * ------- 
 *  The number of valid bytes in BUF on success, or 0 on error.
 */
size_t tgdb_recv_inferior_data ( struct tgdb *tgdb, char *buf, size_t n );

/* tgdb_recv_readline_data
 * -----------------------
 *
 * Data returned from the command line is returned here.
 * 
 * tgdb
 * ----
 *  An instance of the tgdb library to operate on.
 *
 * buf
 * ---
 *  The output of the debugger will be returned in this buffer.
 *
 * n
 * -
 *  Tells libtgdb how large the buffer BUF is.
 *
 * Returns
 * ------- 
 *  The number of valid bytes in BUF on success, or -1 on error.
 */
size_t tgdb_recv_readline_data ( struct tgdb *tgdb, char *buf, size_t n );

/* tgdb_recv_debugger_data
 * -----------------------
 *
 * Gets output from the debugger.
 * Also, at this point, libtgdb returns a list of commands the client 
 * can use.
 * 
 * tgdb
 * ----
 *  An instance of the tgdb library to operate on.
 *
 * buf
 * ---
 *  The output of the debugger will be returned in this buffer.
 *  The buffer passed back will not exceed N in size.
 *
 * n
 * -
 *  Tells libtgdb how large the buffer BUF is that the client passed in.
 *
 * Returns
 * ------- 
 *  The number of valid bytes in BUF on success, or -1 on error.
 */
size_t tgdb_recv_debugger_data ( struct tgdb *tgdb, char *buf, size_t n );

/* These functions are for dealing with getting back data from TGDB */

/* tgdb_get_command
 * ----------------
 *
 *  Gets a command from TGDB.
 *  This should be called after tgdb_recv_debugger_data
 *
 * tgdb
 * ----
 *  An instance of the tgdb library to operate on.
 *
 * Retruns
 * -------
 * 	A valid command if commands still exist.
 *  Null if no more commands exist.
 */
struct tgdb_command *tgdb_get_command ( struct tgdb *tgdb );

/* tgdb_traverse_commands
 * ----------------------
 *
 *  This will traverse all of the commands that the context tgdb currently
 *  has and will print them. It is currently used for debugging purposes.
 *
 * tgdb
 * ----
 *  An instance of the tgdb library to operate on.
 *
 */
void tgdb_traverse_commands ( struct tgdb *tgdb );

/* These functions are for dealing with setting up the inferior program.
 * 
 */

/* tgdb_tty_new
 * ------------
 *
 *  This allocates a new tty and tells the debugger to use it for I/O
 *  with the program being debugged.
 *
 *  Whatever was left in the old tty is lost, the debugged program starts
 *  with a new tty.
 *
 * tgdb
 * ----
 *  An instance of the tgdb library to operate on.
 *
 *  NOTE
 *  ----
 *   The return value only indicates whether the tty was allocated properly,
 *   not whether the debugger accepted the tty, since this can only be determined 
 *   when the debugger responds, not when the command is given.
 *
 * Returns 0 on success or -1 on error
 */
int tgdb_tty_new ( struct tgdb *tgdb );

/* tgdb_tty_name
 * -------------
 *
 * Gets the name of file that debugger is using for I/O with the program
 * being debugged.
 * 
 * tgdb
 * ----
 *  An instance of the tgdb library to operate on.
 *
 * Returns
 * -------
 *  Name of tty or NULL on error.
 *
 */
const char *tgdb_tty_name ( struct tgdb *tgdb );

/* tgdb_get_inferiors_source_files
 * -------------------------------
 *
 *  Gets a list of source files that make up the program being debugged.
 *
 *  This function does not actually do anything but put a command in the 
 *  queue to be run when libtgdb is ready. When the libtgdb runs the
 *  command to get the inferior's source files, it will return 1 of 2
 *  things next time tgdb_recv is called.
 *
 *  If the function succeeds the gui will get back TGDB_UPDATE_SOURCE_FILES
 *  containing a list of all the source files. Otherwise the gui will get
 *  back TGDB_SOURCES_DENIED.
 *
 * tgdb
 * ----
 *  An instance of the tgdb library to operate on.
 *
 *  Returns
 *  -------
 *   0 on success or -1 on error
 */
int tgdb_get_inferiors_source_files ( struct tgdb *tgdb );

/* tgdb_get_absolute_path
 * ----------------------
 *
 *  This gets the absolute path from FILE. FILE must have been returned
 *  from libtgdb as a relative path. libtgdb has a mechanism for getting the
 *  absolute path from the debugger given the relative path.
 *
 * If this functions succeeds TGDB_ABSOLUTE_SOURCE_ACCEPTED will be returned.
 * Otherwise, TGDB_ABSOLUTE_SOURCE_DENIED gets returned.
 *
 * tgdb
 * ----
 *  An instance of the tgdb library to operate on.
 *
 *  Returns
 *  -------
 *   0 on success or -1 on error
 */
int tgdb_get_absolute_path ( struct tgdb *tgdb, const char *file );

/* tgdb_run_debugger_command
 * -------------------------
 *
 * This tells libtgdb to run a command through the debugger.
 *
 * tgdb
 * ----
 *  An instance of the tgdb library to operate on.
 *
 * c
 * -
 *  This is the command that libtgdb should run through the debugger.
 *
 * Returns
 * -------
 *  0 on success or -1 on error
 */
int tgdb_run_debugger_command ( struct tgdb *tgdb, enum tgdb_command_type c );

/* tgdb_set_breakpoint_file_line
 * -----------------------------
 *  
 *  Modify's a breakpoint.
 *
 * tgdb
 * ----
 *  An instance of the tgdb library to operate on.
 *
 *  file
 *  ----
 *   	The file to set the breakpoint in.
 *
 *  line
 *  ----
 *  	The line in FILE to set the breakpoint in.
 *
 *  modify
 *  ------
 *  	Determines what the user wants to do with the breakpoint.
 *
 * Returns
 * -------
 *   0 on success or -1 on error
 */
int tgdb_modify_breakpoint ( struct tgdb *tgdb, const char *file, int line, enum tgdb_breakpoint_action b );

/***************************************************************************
 * tgdb's signal handling
 **************************************************************************/

/* Needless to say, libtgdb needs to be able to know when asynchonous events
 * occur. It depends on knowing when certain signals are sent by the user.
 * This happens when the user hits ^c or ^\.
 *
 * To deal with these issues. libtgdb provides two solutions.
 *  1. libtgdb can handle the signals it has interest in.
 *     SIGINT,SIGTERM,SIGQUIT
 *  2. libtgdb can recieve signal notification from the front end.
 *     it will only do special processing on the signals it is interested in.
 *
 *  The default value for libtgdb is too catch signals. So, if this is 
 *  unacceptable for the front end's environment call TGDB_CATCH_SIGNALS.
 */

/* tgdb_catch_signals
 * ------------------
 *
 *  This function tells libtgdb if it should be catching signals or not.
 *  This will return if tgdb was catching signals.
 *
 * tgdb
 * ----
 *  An instance of the tgdb library to operate on.
 *
 * catch_signals
 * -------------
 *  If non-zero, libtgdb will install signals handlers for asynchronous 
 *  events. If 0, libtgdb will depend on notification of signals.
 *
 * Returns
 * -------
 *   non-zero if libtgdb was previously installing custom signal handlers,
 *   otherwise returns 0 if libtgdb is depending on signal notification.
 */
//int tgdb_catch_signals ( struct tgdb *tgdb, int catch_signals );

/* tgdb_signal_notification
 * ------------------------
 *
 *  The front end can use this function to notify libtgdb that an
 *  asynchronous event has occurred. If signal SIGNUM is relavant
 *  to libtgdb, the appropriate processing will be done.
 *
 * tgdb
 * ----
 *  An instance of the tgdb library to operate on.
 *
 * signum
 * ------
 *  The signal number SIGNUM that has occured.
 *
 * Returns
 * -------
 *   0 on success or -1 on error
 *
 * Note
 * ----
 *  Currently, TGDB only handles SIGINT,SIGTERM and SIGQUIT
 */
int tgdb_signal_notification ( struct tgdb *tgdb, int signum );

/***************************************************************************
 * tgdb's options
 **************************************************************************/

/*
 * This sets the verbosity of the GUI's commands.
 * If the value is set to 0, the GUI's commands will not be shown.
 * If the value is set to 1, the GUI's commands will be shown.
 * The default value for TGDB is to be off.
 *
 * tgdb
 *  An instance of the tgdb library to operate on.
 *
 * value
 *  0 to not show GUI commands, 1 to show them, otherwise nothing is done.
 *  You would use a value other than 0 or 1 just to query if the option is set.
 *
 * Returns
 *  1 if option is set, otherwise 0
 */
int tgdb_set_verbose_gui_command_output ( struct tgdb *tgdb, int value );

#ifdef __cplusplus
}
#endif

#endif /* __TGDB_H__ */
