#ifndef __ANNOTATE_TWO_H__
#define __ANNOTATE_TWO_H__

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_LIMITS_H
#include <limits.h>
#endif /* HAVE_LIMITS_H */

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */

#include "tgdb_client_command.h"

#define TTY_NAME_SIZE 64

/**
 * This is the main context for the annotate two subsytem.
 */
struct annotate_two {

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
	 * ???
	 */
	int command_finished;
	
	/** 
	 * The config directory that this context can write too.
	 */
	char config_dir[PATH_MAX];

	/**
	 * The init file for the debugger.
	 */
	char a2_gdb_init_file[PATH_MAX];

	/** 
	 * This represents the data subsystem
	 */
	struct data *data;

	/** 
	 * This module is used for parsing the output of gdb for annotate 2 
	 * It is used to determine what is a gdb output and what is annotations
	 */
	struct state_machine *sm;

	/**
	 * This module is used for parsing the commands output of gdb
	 */
	struct commands *c;

	/** 
	 * This module is used for keeping shared global data
	 */
	struct globals *g;

	/**
	 * The name of the inferior tty.
	 */
	char inferior_tty_name[TTY_NAME_SIZE];

	/** 
	 * This is a list of all the commands generated since in the last call. 
	 */
	struct tgdb_list *client_command_list;

	/** 
	 * This is to determine if the first annotation prompt has been reached
	 * yet. If it hasn't been reached, this should be zero. Otherwise one.
	 * Its used with the variable below to determine if the annotate 
	 * subsystem needs to probe gdb for the initial file to display.
	 */
	int first_prompt_reached;
	
	/** 
	 * This is used to show if the source annotation has been recieved yet.
	 * It is set to 0 if it hasn't, otherwise 1.
	 */
	int source_already_received;
};

#endif /* __ANNOTATE_TWO_H__ */
