#ifndef __TGDB_INTERFACE_H__
#define __TGDB_INTERFACE_H__

/*! 
 * \file
 * tgdb_interface.h
 *
 * \brief
 * This interface is intended to be the abstraction layer between a client
 * and TGDB. The client should be able to create/add/print new commands and
 * then pass them onto TGDB.
 */

#include "tgdb_types.h"

/**
 * This type determines what type of command is next to be given to gdb.
 */
enum buffer_command_type {

	/**
	 * This command will be ignored by TGDB.
	 */
    BUFFER_VOID = 0,

	/**
	 * A command issued by the front end ( GUI )
	 */
    BUFFER_GUI_COMMAND,

	/**
	 * A command issued by TGDB itself.
	 */
    BUFFER_TGDB_COMMAND,

	/**
	 * A command issued by the user themselves.
	 */
    BUFFER_USER_COMMAND,

	/**
	 * A command directed toward readline.
	 */
    BUFFER_READLINE_COMMAND,

	/**
	 * If this command is given, it will go into a queue that will be
	 * run before any of the other commands. All commands of this type
	 * will be run in order.
	 */
    BUFFER_OOB_COMMAND
};

/** 
 * This type determines what the user will see when a particular gdb command
 * is run.
 */
enum buffer_output_type {

	/**
	 * This will show the debugger's output to the user.
	 */
    COMMANDS_SHOW_USER_OUTPUT = 1,

	/**
	 * This will not show the debugger's output to the user.
	 */
    COMMANDS_HIDE_OUTPUT
};

/** 
 * There are several commands that tgdb knows how to give to gdb. They are:
 */
enum buffer_command_to_run {

	/**
	 * This tells readline what the new prompt is.
	 */
    COMMANDS_SET_PROMPT,

	/**
	 * This forces readline to redisplay the current line.
	 */
    COMMANDS_REDISPLAY,

	/**
	 * This tab completes the current line.
	 */
    COMMANDS_TAB_COMPLETION,

	/**
	 * This does nothing.
	 */
    COMMANDS_VOID
};

/**
 * This is the type that is used to make up 1 complete request from a client 
 * to TGDB. With this, tgdb can run a command through the debugger, 
 * capture output and return values.
 */
struct tgdb_client_command {

	/**
	 * The actual command that should be given to the debugger.
	 */
    char *data;

	/**
	 * Who is sending the command?
	 */
    enum buffer_command_type com_type;

	/**
	 * Where should the output go?
	 */
    enum buffer_output_type out_type;

	/**
	 * Should readline do anything?
	 */
    enum buffer_command_to_run com_to_run;

	/**
	 * An extra buffer, to let the client store some data. Before the command is
	 * run, TGDB passes the command to the client. It can then do something with
	 * the data to prepare for the command to be run.
	 */
    void *client_data;
};

/* The functions below are not meant for the clients to use */

/******************************************************************************/
/**
 * @name Createing Using and Destroying a tgdb_client_command.
 * These functions are for createing, using and destroying a tgdb_client_command
 */
/******************************************************************************/

//@{

/**
 * Creates a new command and initializes it 
 *
 * \param data
 * The command to run
 *
 * \param com_type
 * Who is running the command
 *
 * \param out_type
 * Where the output should go
 *
 * \param com_to_run
 * What readline command to run
 *
 * \param client_data
 * Data that the client can use when prepare_client_for_command is called
 *
 * @return
 * Always is successfull, will call exit on failed malloc
 */
struct tgdb_client_command *tgdb_interface_new_command(    
        const char *data, 
        enum buffer_command_type    com_type, 
        enum buffer_output_type     out_type,
        enum buffer_command_to_run  com_to_run,
        void *client_data);

/**
 * This will print a TGDB queue command to stderr.
 * These are the commands given by TGDB to the debugger.
 * This is a function for debugging. 
 *
 * \param item
 * The command to print
 */
void tgdb_interface_print_command ( void *item );

/** 
 * This will free a TGDB queue command.
 * These are the commands given by TGDB to the debugger.
 * This is a function for debugging. 
 *
 * \param item
 * The command to free
 */
void tgdb_interface_free_command ( void *item);

//@}
#endif
