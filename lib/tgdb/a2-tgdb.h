#ifndef __A2_TGDB_H__
#define __A2_TGDB_H__

#include "fs_util.h"
#include "fork_util.h" /* For pty_pair_ptr */

/*! \file
 * a2-tgdb.h
 * \brief
 * This interface documents the annotate two context.
 */

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

    /** The config directory that this context can write too. */
    char config_dir[FSUTIL_PATH_MAX];

    /** The init file for the debugger. */
    char gdb_init_file[FSUTIL_PATH_MAX];

    /**
	 * This module is used for parsing the commands output of gdb
	 */
    struct commands *c;

    /** This is a list of all the commands generated since in the last call.  */
    struct tgdb_command **client_commands;

    /**
     * Request the source location from gdb.
     */
    int request_source_location;

    /** This is the queue of responses to tgdb commands for the front end. */
    struct tgdb_response **responses;
};

/******************************************************************************/
/**
 * @name Starting and Stopping Commands.
 * These functions are for starting and stopping the annotate_two context.
 */
/******************************************************************************/

/*@{*/

/** 
 * This invokes a libannotate_two library instance.
 *
 * The client must call this function before any other function in the 
 * tgdb library.
 *
 * \param debugger_path
 * The path to the desired debugger to use. If this is NULL, then just
 * "gdb" is used.
 *
 * \param argc
 * The number of arguments to pass to the debugger
 *
 * \param argv
 * The arguments to pass to the debugger    
 *
 * \param config_dir
 * The current config directory. Files can be stored here.
 *
 * @return
 * NULL on error, A valid descriptor upon success
 */
struct annotate_two *a2_create_context(const char *debugger_path,
        int argc, char **argv, const char *config_dir);

/** 
 * This initializes the libannotate_two libarary.
 *  
 * \param a2
 * The annotate two context.
 *
 * \param debugger_stdin
 * Writing to this descriptor, writes to the stdin of the debugger.
 *
 * \param debugger_stdout
 * Reading from this descriptor, reads from the debugger's stdout.
 *
 * \param inferior_stdin
 * Writing to this descriptor, writes to the stdin of the inferior.
 *
 * \param inferior_stdout
 * Reading from this descriptor, reads from the inferior's stdout.
 *
 * @return Retruns
 * 0 on success, otherwise -1 on error.
 */
int a2_initialize(struct annotate_two *a2,
        int *debugger_stdin, int *debugger_stdout,
        int *inferior_stdin, int *inferior_stdout);

/**
 * Shuts down the annotate two context. No more calls can be made on the
 * current context. It will clean up after itself. All descriptors it 
 * opened, it will close.
 *
 * \param ctx
 * The annotate two context.
 *
 * @return
 * 0 on success, otherwise -1 on error.
 */
int a2_shutdown(struct annotate_two *a2);

/**
   * This will free all of the memory used by the responses that tgdb returns.
   *
   * \param tgdb
   * An instance of the tgdb library to operate on.
   */
void a2_delete_responses(struct annotate_two *a2);

/*@}*/

/******************************************************************************/
/**
 * @name Status Commands
 * These functions are for querying the annotate_two context.
 */
/******************************************************************************/

/*@{*/

/** 
 * This lets the annotate_two know that the user ran a command.
 * The client can update itself here if it need to.
 *
 * \param ctx
 * The annotate two context.
 *
 * @return
 * -1 on error, 0 on success
 */
int a2_user_ran_command(struct annotate_two *a2);

/** 
 *  Prepare's the client for the command COM to be run.
 *
 * \param ctx
 * The annotate two context.
 *
 * \param com
 * The command to be run.
 *
 * @return
 * -1 on error, 0 on success
 */
void a2_prepare_for_command(struct annotate_two *a2, struct tgdb_command *com);

/*@}*/

/******************************************************************************/
/**
 * @name Functional commands
 * These functinos are used to ask an annotate_two context to perform a task.
 */
/******************************************************************************/

/*@{*/

int a2_get_current_location(struct annotate_two *a2);

/** 
 * \param ctx
 * The annotate two context.
 *
 * @return 
 * -1 on error. Or pid on Success.
 */
pid_t a2_get_debugger_pid(struct annotate_two *a2);

/*@}*/

/******************************************************************************/
/**
 * @name Inferior tty commands
 * These functinos are used to alter an annotate_two contexts tty state.
 */
/******************************************************************************/

/*@{*/

/** 
 * \param ctx
 * The annotate two context.
 *
 * \param inferior_stdin
 * Writing to this descriptor, writes to the stdin of the inferior.
 *
 * \param inferior_stdout
 * Reading from this descriptor, reads from the inferior's stdout.
 *
 * @return
 * 0 on success, otherwise -1 on error.
 */
int a2_open_new_tty(struct annotate_two *a2, int *inferior_stdin, int *inferior_stdout);

/*@}*/

#endif /* __A2_TGDB_H__ */
