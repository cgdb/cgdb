#ifndef __TGDB_CLIENT_INTERFACE_H__
#define __TGDB_CLIENT_INTERFACE_H__

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */

/* TODO 
 * Remove this include. Don't make this unit depend on the queue type.
 * I am thinking that only tgdb-base should know about its queue's and
 * all the other clients should store data there own way. 
 */
#include "queue.h"
#include "tgdb_interface.h"

/*! \file
 * 		tgdb_client_interface.h
 * 	\brief
 * 		This file documents the interface that a client will have to 
 * 		implement in order to get TGDB to work with a particular 
 * 		debugger/protocol. TGDB only interfaces with this file. If a user
 * 		wishes to add a new debugger/protocol, they must look at the structure
 * 		'struct tgdb_client_debugger_interface' in tgdb_client_interface.c.
 */

/** 
 * This type represents a client context.
 */
struct tgdb_client_context;

/**  
 *
 * Commands that the client should be able to make or ignore.
 * 
 * \field TGDB_CLIENT_GET_BREAKPOINTS
 *     Get a list of breakpoints
 *
 * \field TGDB_CLIENT_COMPLETE_COMMAND_LINE
 *     Complete the current console line
 *
 * \field TGDB_CLIENT_GET_INFERIOR_SOURCE_FILES
 *     Get a list of all the source files the inferior is made up of
 *
 * \field TGDB_CLIENT_SET_DEBUGGER_PROMPT
 *     Set the debugger's prompt
 */
enum tgdb_client_commands {
    TGDB_CLIENT_VOID = 0,
    TGDB_CLIENT_GET_BREAKPOINTS,
    TGDB_CLIENT_COMPLETE_COMMAND_LINE,
    TGDB_CLIENT_GET_INFERIOR_SOURCE_FILES,
    TGDB_CLIENT_SET_DEBUGGER_PROMPT
};

/**
 *
 * All of the debuggers that TGDB supports.
 *
 * \field TGDB_CLIENT_DEBUGGER_GNU_GDB
 *     This is of course the gnu debugger (GDB).
 */
enum tgdb_client_supported_debuggers {
	TGDB_CLIENT_DEBUGGER_UNSUPPORTED = 0,
	TGDB_CLIENT_DEBUGGER_GNU_GDB
};

/**
 *
 * All of the protocols that are supported.
 * One debugger can work with many protocols. Basically, GDB supports 
 * the 'annotate two' interface, the 'gdbmi' interface, and several others.
 *
 * \field TGDB_CLIENT_PROTOCOL_AUTO
 *     TGDB will try to negotiate with the debugger in order to choose the
 *     most appropriate protocol to communicate with GDB.
 *
 * \field TGDB_CLIENT_PROTOCOL_GNU_GDB_ANNOTATE_TWO
 *     This is a functional protocol to communicate with GDB. This protocol is
 *     depricated. The annotate two protocol does not allow a lot of GDB's 
 *     functionality to be used. However, it works with really old GDB's.
 *
 * \field TGDB_CLIENT_PROTOCOL_GNU_GDB_GDBMI
 *     This is currently the best protocol to use when communicating with GDB.
 *     It allows for the most functionality to be retrieved from GDB and 
 *     sent back to the front end.
 */
enum tgdb_client_supported_protocols {
	TGDB_CLIENT_PROTOCOL_UNSUPPORTED = 0,
	TGDB_CLIENT_PROTOCOL_AUTO,
	TGDB_CLIENT_PROTOCOL_GNU_GDB_ANNOTATE_TWO,
	TGDB_CLIENT_PROTOCOL_GNU_GDB_GDBMI
};

/******************************************************************************/
/**
 * @name creating and destroying contexts.
 * These functions are for createing and destroying client contexts.
 */
/******************************************************************************/

//@{

/** 
 * This creates a client context. 
 * TGDB must call this function before any other function in the client library.
 * This function is responsible for determining which protocol will be used to
 * communicate with a particular debugger.
 *
 * \param debugger_path
 *  The path to the desired debugger to use. 
 *	If this is NULL, then just "gdb" should be used.
 *
 * \param argc
 *  The number of arguments to pass to the debugger
 *
 * \param argv
 *  The arguments to pass to the debugger    
 *
 * \param config_dir
 *  The current config directory. 
 *  Files can be stored here by the client library.
 *
 * \param debugger_type
 *  This is the type of debug section the user wishes to start.
 *
 * @return
 *  NULL on error or a valid client context upon success.
 */
struct tgdb_client_context *tgdb_client_create_context ( 
	const char *debugger_path, 
	int argc, char **argv,
	const char *config_dir,
	enum tgdb_client_supported_debuggers debugger,
	enum tgdb_client_supported_protocols protocol);

/** 
 * This will initialize a client context.
 *
 * \param tcc
 *  The client context to initialize
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
 * @return
 *  0 on success, otherwise -1 on error.
 *
 * \post
 *  debugger_stdin, debugger_stdout, inferior_stdin, inferior_stdout 
 *  are all set.
 */
int tgdb_client_initialize_context ( 
	struct tgdb_client_context *tcc,
	struct queue *command_container,
	int *debugger_stdin, int *debugger_stdout,
	int *inferior_stdin, int *inferior_stdout );

/** 
 * Shuts down the client context. No more calls can be made on the
 * current context. It will clean up after itself. 
 * For instance, all descriptors it opened, it will close.
 *
 * \param tcc
 *  The client context.
 *
 * @return
 *  0 on success, otherwise -1 on error.
 */
int tgdb_client_destroy_context ( struct tgdb_client_context *tcc );

//@}

/******************************************************************************/
/**
 * @name Status Commands
 * These functions are for querying the client context.
 */
/******************************************************************************/

//@{

/** 
 * Returns the last error message ?
 * Not implemented yet.
 * What should it return? How should errors be handled?
 *
 * \param tcc
 *  The client context.
 *
 * @return
 *  0 on success, otherwise -1 on error.
 */
int tgdb_client_err_msg ( struct tgdb_client_context *tcc );

/** 
 * This determines if the client context is ready to allow TGDB to run
 * another command.
 *
 * \param tcc
 *  The client context.
 *
 * @return
 *  1 if it is ready to allow TGDB to run another command, 0 if not.
 */
int tgdb_client_is_client_ready ( struct tgdb_client_context *tcc );

/** 
 * This is currently called after TGDB has sent a command to the debugger.
 * Currently, the client can run commands that has to be run ( for TGDB ) 
 * everytime the user/GUI runs a commnad. For example, this is necessary 
 * for keeping the breakpoints current.
 *
 * In the future, it seems like this function should go away, and TGDB 
 * should be responsible for running the commands necessary ???.
 *
 * \param tcc
 *  The client context.
 *
 * \param command_container
 *  A list of commands that was generated from this call.
 *
 * @return
 * 	-1 on error, 0 on success
 *
 * \post
 *  command_container is set.
 */
int tgdb_client_tgdb_ran_command ( 
		struct tgdb_client_context *tcc, 
		struct queue *command_container );

/** 
 * This is currently called by TGDB before TGDB runs the command.
 * It can prepare the client for the command COM to be run.
 *
 * \param tcc
 *  The client context.
 *
 *  \param com
 *  	The command to be run.
 *
 * @return
 * 	-1 on error, 0 on success
 */
int tgdb_client_prepare_for_command ( 
		struct tgdb_client_context *tcc, 
		struct tgdb_client_command *com );

/** 
 * Determines if the client is capable of accepting TGDB commands. The client
 * may not be willing to allow TGDB to run its internal commands if the 
 * debugger is prompting a "Yes/No" question or needs to get an answer from
 * the user related to the last command.
 *
 * \param tcc
 *  The client context.
 *
 * @return
 *  1 if TGDB can run its internal commands, 0 if TGDB can't.
 */
int tgdb_client_can_tgdb_run_commands ( struct tgdb_client_context *tcc );

//@}

/******************************************************************************/
/**
 * @name Input/Output commands
 * These functions are for communicating I/O with a client context.
 */
/******************************************************************************/

//@{

 /** 
  * This recieves all of the output from the debugger. It is all routed 
  * through this function. 
  *
  * \param tcc
  *  The client context.
  *
  * \param command_container
  *  A list of commands that was generated from this call.
  *
  * \param input_data
  *  This is the stdout/stderr from the debugger. This is the data that 
  *  should parse be parsed.
  *
  * \param input_data_size
  *  This is the size of input_data.
  *
  * \param debugger_output
  *  Contains data that has been determined to be the output of the 
  *  debugger that the user should see.
  *
  * \param debugger_output_size
  *  This is the size of debugger_output
  *
  * \param inferior_output
  *  Contains data that has been determined to be the output of the 
  *  inferior that the user should see.
  *
  * \param inferior_output_size
  *  This is the size of inferior_output
  *
  * @return
  *  1 when it has finished a command, 
  *  0 on success but hasn't recieved enough I/O to finish the command, 
  *  otherwise -1 on error.
  *
  *  \post
  *  debugger_output, debugger_output_size, inferior_output, 
  *  inferior_output_size are all set.
  */
int tgdb_client_parse_io ( 
		struct tgdb_client_context *tcc,
		struct queue *command_container,
		const char *input_data, const size_t input_data_size,
		char *debugger_output, size_t *debugger_output_size,
		char *inferior_output, size_t *inferior_output_size,
	    struct queue *q	);
//@}


/******************************************************************************/
/**
 * @name Functional commands
 * These functinos are used to ask the client context to perform a task.
 */
/******************************************************************************/

//@{

/** 
 *  Gets the absolute path of a relative path outputted by a debugger.
 *  
 *  \param tcc
 *   The client context.
 *
 *  \param command_container
 *   A list of commands that was generated from this call.
 *
 *  \param relative_path
 *   The path that the debugger outputted. ( Usually relative ).
 *
 * @return
 *   0 on success, otherwise -1 on error.
 *
 *   \pre
 *    If relative_path is absolute ( not relative ), then this function should not
 *    be called.
 */
int tgdb_client_get_absolute_path ( 
		struct tgdb_client_context *tcc, 
		struct queue *command_container,
		const char *relative_path );

/** 
 *  Gets all of the source files that the inferior is made of.
 *
 *  \param tcc
 *   The client context.
 *
 *  \param command_container
 *   A list of commands that was generated from this call.
 *
 * @return
 *   0 on success, otherwise -1 on error.
 */
int tgdb_client_get_inferior_source_files ( 
		struct tgdb_client_context *tcc, 
		struct queue *command_container );

/** 
 *  This will change the prompt the user sees to PROMPT.
 *
 * \param tcc
 *  The client context.
 *
 * \param command_container
 *  A list of commands that was generated from this call.
 *
 * \param prompt
 *  The new prompt to change to.
 *
 * @return
 *  0 on success, otherwise -1 on error.
 */
int tgdb_client_change_debugger_prompt(
		struct tgdb_client_context *tcc,
		struct queue *command_container,
		const char *prompt);

/** 
 * TGDB calls this function when it determines a command needs to be completed.
 *
 * \param tcc
 *  The client context.
 *
 * \param command_container
 *  A list of commands that was generated from this call.
 *
 * \param completion_command
 *  The command to be completed
 *
 * @return
 *   0 on success, otherwise -1 on error.
 */
int tgdb_client_completion_callback(
		struct tgdb_client_context *tcc,
		struct queue *command_container,
		const char *completion_command);

/** 
 *  This returns the command to send to gdb for the enum C.
 *  It will return NULL on error, otherwise correct string on output.
 *
 * \param tcc
 *  The client context.
 *
 * \param c
 *  The command to run.
 *
 * @return
 *  Command on success, otherwise NULL on error.
 */
char *tgdb_client_return_command ( 
		struct tgdb_client_context *tcc, 
		enum tgdb_command_type c );

/** 
 * \param tcc
 * 	The client context.
 *
 * \param file
 * 	The file to set the breakpoint in.
 *
 * \param line
 * 	The line in FILE to set the breakpoint in.
 *
 * \param b
 * 	Determines what the user wants to do with the breakpoint.
 *
 * @return
 *  -1 on error, or 0 on success
 */
char *tgdb_client_modify_breakpoint ( 
		struct tgdb_client_context *tcc, 
		const char *file, 
		int line, 
		enum tgdb_breakpoint_action b );

/** 
 * \param tcc
 *  The client context.
 *
 *  @return 
 *   -1 on error or pid on success.
 */
pid_t tgdb_client_get_debugger_pid ( struct tgdb_client_context *tcc );

//@}

/******************************************************************************/
/**
 * @name Inferior tty commands
 * These functinos are used to alter an annotate_two contexts tty state.
 */
/******************************************************************************/

//@{

/** 
 * \param tcc
 *  The client context.
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
 *
 *  \post
 *   inferior_stdin and inferior_stdout are set if the return value 
 *   of tgdb_client_open_new_tty is acceptable.
 */
int tgdb_client_open_new_tty ( 
		struct tgdb_client_context *tcc,
		struct queue *command_container,
		int *inferior_stdin, 
		int *inferior_stdout );

/** 
 * \param tcc
 *  The client context.
 *
 * @return
 * 	tty name on success, otherwise NULL on error.
 */
char *tgdb_client_get_tty_name ( struct tgdb_client_context *tcc );

//@}

#endif
