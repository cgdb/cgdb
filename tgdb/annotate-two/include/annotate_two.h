#ifndef __ANNOTATE_TWO_H__
#define __ANNOTATE_TWO_H__

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */

#include "tgdb_command.h"
#include "fs_util.h"
#include "fork_util.h"          /* For pty_pair_ptr */

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

    /** The master, slave and slavename of the pty device */
    pty_pair_ptr pty_pair;

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
    char config_dir[FSUTIL_PATH_MAX];

    /**
	 * The init file for the debugger.
	 */
    char a2_gdb_init_file[FSUTIL_PATH_MAX];

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
	 * This is a list of all the commands generated since in the last call. 
	 */
    struct tgdb_list *client_command_list;

    /** 
	 * This is used to show if the source annotation has been recieved yet.
	 * It is set to 0 if it hasn't, otherwise 1.
	 */
    int source_already_received;

    /**
	 * The current response list.
	 */
    struct tgdb_list *cur_response_list;
};

#endif /* __ANNOTATE_TWO_H__ */
