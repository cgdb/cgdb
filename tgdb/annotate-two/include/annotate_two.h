#ifndef __ANNOTATE_TWO_H__
#define __ANNOTATE_TWO_H__

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_LIMITS_H
#include <limits.h>
#endif /* HAVE_LIMITS_H */

#include "tgdb_interface.h"

#define TTY_NAME_SIZE 64

struct annotate_two {
	/* This is set when tgdb has initialized itself */
	int tgdb_initialized;

	int debugger_stdin;  /* writing to this will write to the stdin of the debugger */
	int debugger_out;    /* Reading from reads the stdout/stderr of the debugger */

	int inferior_stdin;  /* writing to this will write to the stdin of the inferior */
	int inferior_out;    /* Reading from reads the stdout/stderr of the inferior */
	int inferior_slave_fd; /* Only kept around so it can be closed properly */

	pid_t debugger_pid;             /* pid of child process */
	command_completed command_completed_callback;
	
	/* The config directory and the init file for gdb */
	char config_dir[PATH_MAX];
	char a2_gdb_init_file[PATH_MAX];

	/* This represents the data subsystem */
	struct data *data;

	/* This module is used for parsing the output of gdb for annotate 2 
	 * It is used to determine what is a gdb output and what is annotations
	 */
	struct state_machine *sm;

	/* This module is used for parsing the commands output of gdb */
	struct commands *c;

	/* This module is used for keeping shared global data */
	struct globals *g;

	char inferior_tty_name[TTY_NAME_SIZE];
};

#endif /* __ANNOTATE_TWO_H__ */
