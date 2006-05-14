#ifndef __TGDB_H__
#define __TGDB_H__

/*! 
 * \file
 * tgdb.h
 *
 * \brief
 * This interface is intended to be the abstraction layer between a front end 
 * and the low level debugger the front end is trying to communicate with.
 */

#ifdef __cplusplus
extern "C"
{
#endif

/* includes {{{*/
#if HAVE_CONFIG_H
#include "config.h"
#endif				/* HAVE_CONFIG_H */

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif				/* HAVE_SYS_TYPES_H */

#include "tgdb_types.h"
/* }}}*/

/* Createing and Destroying a libtgdb context. {{{*/
/******************************************************************************/
/**
 * @name Createing and Destroying a libtgdb context.
 * These functions are for createing and destroying a tgdb context.
 */
/******************************************************************************/

/*@{*/

  /**
   *  This struct is a reference to a libtgdb instance.
   */
  struct tgdb;

  /**
   * This initializes a tgdb library instance. It starts up the debugger and 
   * returns all file descriptors the client must select on.
   *
   * The client must call this function before any other function in the 
   * tgdb library.
   *
   * \param debugger
   * The path to the desired debugger to use. If this is NULL, then just
   * "gdb" is used.
   *
   * \param argc
   * The number of arguments to pass to the debugger
   *
   * \param argv
   * The arguments to pass to the debugger    
   *
   * \param debugger_fd
   * The descriptor to the debugger's I/O
   *
   * \param inferior_fd
   * The descriptor to the I/O of the program being debugged.
   *
   * @return
   * NULL on error, a valid context on success.
   */
  struct tgdb *tgdb_initialize (const char *debugger,
				int argc, char **argv,
				int *debugger_fd, int *inferior_fd);

  /**
   * This will terminate a libtgdb session. No functions should be called on
   * the tgdb context passed into this function after this call.
   *
   * \param tgdb
   * An instance of the tgdb library to operate on.
   *
   * @return
   * 0 on success or -1 on error
   */
  int tgdb_shutdown (struct tgdb *tgdb);

/*@}*/
/* }}}*/

/* Status Commands {{{*/
/******************************************************************************/
/**
 * @name Status Commands
 * These functions are for querying the tgdb context.
 */
/******************************************************************************/

/*@{*/

  /**
   * If a function returns an error, this can be called to report more
   * about the type of error, or the error message itself.
   *
   * \param tgdb
   * An instance of the tgdb library to operate on.
   *
   * @return
   * Error Message or NULL on error
   */
  char *tgdb_err_msg (struct tgdb *tgdb);

  /**
   * This will check to see if TGDB is currently capable of receiving another command.
   * 
   * \param tgdb
   * An instance of the tgdb library to operate on.
   *
   * \param is_busy
   * Will return as 1 if tgdb is busy, otherwise 0.
   *
   * \return
   * 0 on success, or -1 on error
   */
  int tgdb_is_busy (struct tgdb *tgdb, int *is_busy);

/*@}*/
/* }}}*/

/* Input/Output commands {{{*/
/******************************************************************************/
/**
 * @name Input/Output commands
 * These functions are for communicating I/O with the tgdb context.
 */
/******************************************************************************/

/*@{*/

  /**
   * Have TGDB process a command.
   * 
   * \param tgdb
   * An instance of the tgdb library to operate on.
   * 
   * \param request
   * The requested command to have TGDB process.
   *
   * \return
   * 0 on success or -1 on error
   */
  int tgdb_process_command (struct tgdb *tgdb, tgdb_request_ptr request);
  
  /**
   * This function does most of the dirty work in TGDB. It is capable of 
   * processing the output of the debugger, to either satisfy a previously 
   * made request, or to simply get console output for the caller to have.
   *
   * The data returned from this function is the console output of the 
   * debugger.
   * 
   * \param tgdb
   * An instance of the tgdb library to operate on.
   *
   * \param buf
   * The output of the debugger will be returned in this buffer.
   * The buffer passed back will not exceed N in size.
   *
   * \param n
   * Tells libtgdb how large the buffer BUF is that the client passed in.
   *
   * \param is_finished
   * If this is passed in as NULL, it is not set.
   *
   * If it is non-null, it will be set to 1 if TGDB finished processing the
   * current request. Otherwise, it will be set to 0 if TGDB needs more input
   * in order to finish processing the current requested command.
   *
   * @return
   * The number of valid bytes in BUF on success, or -1 on error.
   */
  size_t tgdb_process (struct tgdb *tgdb, char *buf, size_t n, int *is_finished);

  /**
   * This sends a byte of data to the program being debugged.
   *
   * \param tgdb
   * An instance of the tgdb library to operate on.
   *
   * \param c
   * The character to pass to the program being debugged.
   *
   * @return
   * 0 on success or -1 on error
   */
  int tgdb_send_inferior_char (struct tgdb *tgdb, char c);

  /**
   * Gets the ouput from the program being debugged.
   * 
   * \param tgdb
   * An instance of the tgdb library to operate on.
   *
   * \param buf
   * The output of the program being debugged will be returned here.
   *
   * \param n
   * Tells libtgdb how large the buffer BUF is.
   *
   * @return
   * The number of valid bytes in BUF on success, or 0 on error.
   */
  size_t tgdb_recv_inferior_data (struct tgdb *tgdb, char *buf, size_t n);

/*@}*/
/* }}}*/

/* Getting Data out of TGDB {{{*/
/******************************************************************************/
/**
 * @name Getting Data out of TGDB
 * These functions are for dealing with getting back data from TGDB
 */
/******************************************************************************/

/*@{*/

  /**
   * Gets a response from TGDB.
   * This should be called after tgdb_recv_debugger_data
   *
   * \param tgdb
   * An instance of the tgdb library to operate on.
   *
   * @return
   * A valid response if responses still exist.
   * Null if no more responses exist.
   */
  struct tgdb_response *tgdb_get_response (struct tgdb *tgdb);

  /**
   * This will traverse all of the responses that the context tgdb currently
   * has and will print them. It is currently used for debugging purposes.
   *
   * \param tgdb
   * An instance of the tgdb library to operate on.
   */
  void tgdb_traverse_responses (struct tgdb *tgdb);

  /**
   * This will free all of the memory used by the responses that tgdb returns.
   *
   * \param tgdb
   * An instance of the tgdb library to operate on.
   */
  void tgdb_delete_responses (struct tgdb *tgdb);

/*@}*/
/* }}}*/

/* Inferior tty commands {{{*/
/******************************************************************************/
/**
 * @name Inferior tty commands
 * These functinos are used to alter the tty state of the inferior program.
 * These functions currently are not always supported. If the annotate_two
 * subsytem is being used, the tty commands are supported.
 */
/******************************************************************************/

/*@{*/

  /**
   * This allocates a new tty and tells the debugger to use it for I/O
   * with the program being debugged.
   *
   * Whatever was left in the old tty is lost, the debugged program starts
   * with a new tty.
   *
   * \param tgdb
   * An instance of the tgdb library to operate on.
   *
   * NOTE
   *  The return value only indicates whether the tty was allocated properly,
   *  not whether the debugger accepted the tty, since this can only be determined 
   *  when the debugger responds, not when the command is given.
   *
   * @return
   * 0 on success or -1 on error
   */
  int tgdb_tty_new (struct tgdb *tgdb);

  /**
   * Gets the name of file that debugger is using for I/O with the program
   * being debugged.
   * 
   * \param tgdb
   * An instance of the tgdb library to operate on.
   *
   * @return
   * Name of tty or NULL on error.
   */
  const char *tgdb_tty_name (struct tgdb *tgdb);

/*@}*/
/* }}}*/

/* Functional commands {{{*/
/******************************************************************************/
/**
 * @name Functional commands
 * These functinos are used to ask the TGDB context to perform a task.
 */
/******************************************************************************/

/*@{*/

  /**
   * This sends a console command to the debugger (GDB).
   *
   * \param tgdb
   * An instance of the tgdb library to operate on.
   *
   * \param command
   * The null terminated command to pass to GDB as a console command.
   * 
   * \return
   * Will return as a tgdb request command on success, otherwise NULL.
   */
  tgdb_request_ptr tgdb_request_run_console_command (struct tgdb *tgdb, 
						     const char *command);

  /**
   * Gets a list of source files that make up the program being debugged.
   *
   * This function does not actually do anything but put a command in the 
   * queue to be run when libtgdb is ready. When the libtgdb runs the
   * command to get the inferior's source files, it will return 1 of 2
   * things next time tgdb_recv is called.
   *
   * If the function succeeds the gui will get back TGDB_UPDATE_SOURCE_FILES
   * containing a list of all the source files. Otherwise the gui will get
   * back TGDB_SOURCES_DENIED.
   *
   * \param tgdb
   * An instance of the tgdb library to operate on.
   *
   * \return
   * Will return as a tgdb request command on success, otherwise NULL.
   */
  tgdb_request_ptr tgdb_request_inferiors_source_files (struct tgdb *tgdb);

  /**
   * This gets the absolute path and the relative path from the given file, 
   * FILE. The file parameter should have been returned by 
   * tgdb_request_inferiors_source_files. It can return both relative and 
   * absolute filenames.
   *
   * Basically, if the parameter file is a relative path OR an absolute path, 
   * both the relative path and the absolute path will be returned in the 
   * response.
   *
   * If this functions succeeds TGDB_ABSOLUTE_SOURCE_ACCEPTED will be returned.
   * Otherwise, TGDB_ABSOLUTE_SOURCE_DENIED gets returned.
   *
   * \param tgdb
   * An instance of the tgdb library to operate on.
   *
   * \param file
   * The relative filename to get the absolute path of. If this is passed in as
   * NULL, then this function will return NULL.
   *
   * \return
   * Will return as a tgdb request command on success, otherwise NULL.
   */
  tgdb_request_ptr tgdb_request_filename_pair (struct tgdb *tgdb, 
					       const char *file);

  /**
   * This will ask the debugger for it's current file and line number.
   * It will return to the caller a tgdb_response with the 
   * response->update_source_files set. This is the same response you 
   * will get when TGDB asynchronously sends the update_file_postition.
   *
   * \param tgdb
   * An instance of the tgdb library to operate on.
   * 
   * \param on_starup
   * This variable can be set to 1 if the front end wants to probe GDB
   * for the initial file and location of the program being debugged.
   * However, each initial time after that, this variable should be 
   * set to 0.
   *
   * \return
   * Will return as a tgdb request command on success, otherwise NULL.
   */
  tgdb_request_ptr tgdb_request_current_location (struct tgdb *tgdb, 
						  int on_startup);

  /**
   * This tells libtgdb to run a command through the debugger.
   *
   * \param tgdb
   * An instance of the tgdb library to operate on.
   *
   * \param c
   * This is the command that libtgdb should run through the debugger.
   *
   * @return
   * Will return as a tgdb request command on success, otherwise NULL.
   */
  tgdb_request_ptr tgdb_request_run_debugger_command (
		     struct tgdb *tgdb, enum tgdb_command_type c);

  /**
   * Modify's a breakpoint.
   *
   * \param tgdb
   * An instance of the tgdb library to operate on.
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
   * Will return as a tgdb request command on success, otherwise NULL.
   */
  tgdb_request_ptr tgdb_request_modify_breakpoint (struct tgdb *tgdb,
			      const char *file,
			      int line, 
			      enum tgdb_breakpoint_action b);

  /**
   * Used to get all of the possible tab completion options for LINE.
   *
   * \param tgdb
   * An instance of the tgdb library to operate on.
   *
   * \param line
   * The line to tab complete.
   *
   * \return
   * Will return as a tgdb request command on success, otherwise NULL.
   */
  tgdb_request_ptr tgdb_request_complete (struct tgdb *tgdb, const char *line);

/*@}*/
/* }}}*/

/* TGDB Queue commands {{{*/
/******************************************************************************/
/**
 * @name Queuing of TGDB commands
 * These functinos are used to allow clients to queue multiple commands for 
 * TGDB to be able to run.
 */
/******************************************************************************/

  /**
   * Have TGDB append a command to it's list of commands it needs to run.
   * 
   * \param tgdb
   * An instance of the tgdb library to operate on.
   * 
   * \param request
   * The requested command to have TGDB process.
   *
   * \return
   * 0 on success or -1 on error
   */
  int tgdb_queue_append (struct tgdb *tgdb, tgdb_request_ptr request);

  /**
   * Get a tgdb_request command back from TGDB.
   *
   * \param tgdb
   * An instance of the tgdb library to operate on.
   *
   * \return
   * The requested command at the top of the stack, or NULL on error.
   * Popping a command when the queue is empty is also considered an error,
   * and NULL will be returned.
   */
  tgdb_request_ptr tgdb_queue_pop (struct tgdb *tgdb);

  /**
   * Get's the number of items that are in the queue for TGDB to run.
   *
   * \param tgdb
   * An instance of the tgdb library to operate on.
   *
   * \size
   * The number of items in the list.
   *
   * \return
   * 0 on success or -1 on error
   */
  int tgdb_queue_size (struct tgdb *tgdb, int *size);

/* }}}*/

/* Signal Handling Support {{{*/
/******************************************************************************/
/**
 * @name Signal Handling Support
 * These functinos are used to notify TGDB of signal recieved.
 */
/******************************************************************************/

/*@{*/

  /**
   * The front end can use this function to notify libtgdb that an
   * asynchronous event has occurred. If signal SIGNUM is relavant
   * to libtgdb, the appropriate processing will be done.
   * Currently, TGDB only handles SIGINT,SIGTERM and SIGQUIT.
   *
   * libtgdb will remove all elements from it's queue when a SIGINT
   * is received.
   *
   * \param tgdb
   * An instance of the tgdb library to operate on.
   *
   * \param signum
   * The signal number SIGNUM that has occured.
   *
   * @return
   * 0 on success or -1 on error
   */
  int tgdb_signal_notification (struct tgdb *tgdb, int signum);

/*@}*/
/* }}}*/

/* Config Options {{{*/
/******************************************************************************/
/**
 * @name TGDB Config Options
 * These functinos are used to change the state TGDB works. TGDB can currently
 * only be configured through this interface.
 */
/******************************************************************************/

/*@{*/

  /**
   * This sets the verbosity of the GUI's commands.
   * If the value is set to 0, the GUI's commands will not be shown.
   * If the value is set to 1, the GUI's commands will be shown.
   * The default value for TGDB is to be off.
   *
   * \param tgdb
   * An instance of the tgdb library to operate on.
   *
   * \param value
   * 0 to not show GUI commands, 1 to show them, otherwise nothing is done.
   * You would use a value other than 0 or 1 just to query if the option is set.
   *
   * @return
   * 1 if option is set, otherwise 0
   */
  int tgdb_set_verbose_gui_command_output (struct tgdb *tgdb, int value);

  /**
   * This will make TGDB handle error's in a verbose mode.
   * The basically mean's that when TGDB find's an error, the message is
   * printed to stdout/stderr. Normally this is not acceptable because TGDB can
   * simple not print when any front end is using it. Imparticular, TGDB can
   * not print when a curses based front end is using it.
   *
   * \param tgdb
   * An instance of the tgdb library to operate on.
   *
   * \param value
   * if -1, query to see if the option is set.
   * If 0, the option will be turned off.
   * If 1, the option will be turned on.
   *
   * @return
   * 1 if option is set, otherwise 0
   */
  int tgdb_set_verbose_error_handling (struct tgdb *tgdb, int value);

/*@}*/
/* }}}*/

#ifdef __cplusplus
}
#endif

#endif				/* __TGDB_H__ */
