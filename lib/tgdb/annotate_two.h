#ifndef __ANNOTATE_TWO_H__
#define __ANNOTATE_TWO_H__

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */

#include "fs_util.h"
#include "fork_util.h"          /* For pty_pair_ptr */

/** This is the main context for the annotate two subsytem. */
struct annotate_two {

    /**  This is set when this context has initialized itself */
    int tgdb_initialized;

    /** writing to this will write to the stdin of the debugger */
    int debugger_stdin;

    /** Reading from reads the stdout/stderr of the debugger */
    int debugger_out;

    /** The master, slave and slavename of the pty device */
    pty_pair_ptr pty_pair;

    /** pid of child process. */
    pid_t debugger_pid;

    /** Marks whether gdb command has finished. Ie, at gdb prompt or not. */
    int command_finished;

    /** The config directory that this context can write too. */
    char config_dir[FSUTIL_PATH_MAX];

    /** The init file for the debugger. */
    char a2_gdb_init_file[FSUTIL_PATH_MAX];

    /** 
	 * This module is used for parsing the output of gdb for annotate 2 
	 * It is used to determine what is a gdb output and what is annotations
	 */
    struct state_machine *sm;

    /**
	 * This module is used for parsing the commands output of gdb
	 */
    struct commands *c;

    /** This is a list of all the commands generated since in the last call.  */
    struct tgdb_command **client_commands;

    /** The current response list. */
    struct tgdb_list *cur_response_list;

    /**
     * Request the source location from gdb.
     */
    int request_source_location;
};

#endif /* __ANNOTATE_TWO_H__ */
