#ifndef __TGDB_CLIENT_COMMAND_H__
#define __TGDB_CLIENT_COMMAND_H__

/* NOTES
 *
 * The clients should be able to ask TGDB to run a command. This implies that 
 * TGDB has a list of commands that it allows the clients to ask to be run.
 */

/*! 
 * \file
 * tgdb_client_command.h
 *
 * \brief
 * This interface is intended to be the abstraction layer between a client
 * and TGDB. The client should be able to create/add/print new commands and
 * then pass them onto TGDB.
 */

#include "tgdb_types.h"

/**
 * Determine's the exact source of the command.
 * TGDB can determine who sent the current command by looking at this type.
 */
enum tgdb_client_command_choice {

	/**
	 * A command issued by a client of TGDB.
	 */
	TGDB_CLIENT_COMMAND_NORMAL,

	/**
	 * If this command is given, it will go into a queue that will be
	 * run before any of the other commands (TGDB_CLIENT_COMMAND_NORMAL). 
	 * All commands of this type will be run in order.
	 *
	 * TODO: This should probably go away. Commands should either have
	 * priority's or not need to use this feature. It's here because it
	 * was usefull early on to get the annotate_two client to work.
	 */
	TGDB_CLIENT_COMMAND_PRIORITY,
};

/**
 * TGDB can do several actions for the client. The client is allowed to ask
 * TGDB to perform such action's through this interface.
 */
enum tgdb_client_command_action_choice {

	/**
	 * This is relevant when there is no action for TGDB to take.
	 */
	TGDB_CLIENT_COMMAND_ACTION_NONE,

	/**
	 * This tells TGDB to change the current prompt.
	 */
	TGDB_CLIENT_COMMAND_ACTION_CONSOLE_SET_PROMPT
};

/** 
 * This is a client command. It can be used to send to TGDB-BASE.
 */
struct tgdb_client_command {
	/**
	 * The actual command to give to the debugger.
	 */
	char *tgdb_client_command_data;

	/**
	 * The type of command this one is.
	 */
	enum tgdb_client_command_choice command_choice;
	
	/**
	 * What action should TGDB_BASE take?
	 */
	enum tgdb_client_command_action_choice action_choice;

	/**
	 * Private data, used by the client to keep track of there own state.
	 */
	void *tgdb_client_private_data;
};

/**
 * Creates a new command and initializes it 
 *
 * \param data
 * The command to run
 *
 * \param command_choice
 * The type of command to run.
 *
 * \param display_choice
 * What part of the command should be sent to the user.
 *
 * \param action_choice
 * The type of action the client would like TGDB  to perform for it.
 *
 * \param client_data
 * Data that the client can use when prepare_client_for_command is called
 *
 * @return
 * Always is successfull, will call exit on failed malloc
 */
struct tgdb_client_command *tgdb_client_command_create(    
        const char *data, 
        enum tgdb_client_command_choice command_choice, 
        enum tgdb_client_command_action_choice action_choice,
        void *client_data);

/** 
 * This will free a TGDB queue command.
 * These are the commands given by TGDB to the debugger.
 * This is a function for debugging. 
 *
 * \param item
 * The command to free
 *
 * @return
 * 0 on success, or -1 on error
 */
int tgdb_client_command_destroy ( void *item);

/**
 * This will print a TGDB queue command to stderr.
 * These are the commands given by TGDB to the debugger.
 * This is a function for debugging. 
 *
 * \param item
 * The command to print
 */
void tgdb_client_command_print ( void *item );

#endif
