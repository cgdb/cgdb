#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_STDLIB_H 
#include <stdlib.h>
#endif  /* HAVE_STDLIB_H */

/* This is for PATH_MAX */
#if HAVE_LIMITS_H
#include <limits.h>
#endif /* HAVE_LIMITS_H */

#if HAVE_SIGNAL_H
#include <signal.h> /* sig_atomic_t */
#endif /* HAVE_SIGNAL_H */

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#if HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#include "tgdb.h"
#include "tgdb_interface.h"
#include "a2-tgdb.h"
#include "fs_util.h"
#include "gdbmi_tgdb.h"
#include "error.h"
#include "rlctx.h"
#include "ibuf.h"
#include "io.h"

#include "state_machine.h"
#include "commands.h"
#include "pseudo.h" /* SLAVE_SIZE constant */
#include "fork_util.h"
#include "sys_util.h"

struct tgdb {
	struct annotate_two *a2;


	/* Reading from this will read from the debugger's output */
	int debugger_stdout;

	/* Writing to this will write to the debugger's stdin */
	int debugger_stdin;

	/* Reading from this will read the stdout from the program being debugged */
	int inferior_stdout;
	/* Writing to this will write to the stdin of the program being debugged */
	int inferior_stdin;

	/***************************************************************************
	 * All the queue's the clients can run commands through
	 **************************************************************************/

	/* 
	 * 	gdb_input_queue
	 *
	 *  The commands that need to be run through gdb. 
	 *  Examples are 'b main', 'run', ...
	 */
	struct queue *gdb_input_queue;

	/* 
	 * raw_input_queue: 
	 *
	 *  This is the data that the user typed to form a command.
	 *  This data must be sent through readline to get the actual command.
	 *  The data will be passed through readline and then readline will send to 
	 *  tgdb a command to run.
	 */
	struct queue *raw_input_queue;

	/* 
	 * oob_input_queue:
	 *
	 *  The out of band input queue. 
	 *  These commands should *always* be run first */
	struct queue *oob_input_queue;

	/* 
	 * rlc_input_queue:
	 *
	 *  This sends commands to the readline program.
	 */
	struct queue *rlc_input_queue;

	/*
	 * command_container
	 *
	 * This is used to recieve commands from the lower level subsystem.
	 */
	struct queue *command_container;

	/* Interface to readline capability */
	struct rlctx *rl;

	/* The current command the user is typing at readline */
	struct string *current_command;

	/* This variable needs to be removed from libannotate 
	 * I don't really know if its usefull anymore.
	 */
	unsigned short tgdb_partially_run_command;

	/* These are 2 very important state variables.
	 *
	 * IS_SUBSYSTEM_READY_FOR_NEXT_COMMAND
	 * -----------------------------------
	 *  If set to 1, libtgdb thinks the lower level subsystem is capable of 
	 *  recieving another command. It needs this so that it doesn't send 2
	 *  commands to the lower level before it can say it can't recieve a command.
	 *  At some point, maybe this can be removed?
	 *  When its set to 0, libtgdb thinks it can not send the lower level another
	 *  command.
	 *
	 * HAS_USER_SENT_COMMAND
	 * ---------------------
	 *  This is set to 1 if the user types a full command, followed by the newline.
	 *  This is used so that all commands, until that command is finished by the 
	 *  lower level subsystem, are queued. That way, only 1 command is sent at a 
	 *  time. If it is 0, then the data typed by the user goes directly to the 
	 *  readline context, it is not queued.
	 */
	int IS_SUBSYSTEM_READY_FOR_NEXT_COMMAND;
	int HAS_USER_SENT_COMMAND;

	sig_atomic_t control_c; /* If ^c was hit by user */

	/*
	 * This is the last GUI command that has been run.
	 * It is used to display to the client the GUI commands.
	 *
	 * It will either be NULL when it is not set or it should be
	 * the last GUI command run. If it is non-NULL it should be from the heap.
	 * As anyone is allowed to call xfree on it.
	 */
	char *last_gui_command;

	/*
	 * This is a TGDB option.
	 * It determines if the user wants to see the commands the GUI is running.
	 * 
	 * If it is 0, the user does not want to see the commands the GUI is 
	 * running. Otherwise, if it is 1, it does.
	 */
	int show_gui_commands;
};

/* Temporary prototypes */
static int tgdb_deliver_command ( struct tgdb *tgdb, int fd, struct command *command );
static int tgdb_run_command( struct tgdb *tgdb );
static int tgdb_init_readline ( struct tgdb *tgdb, char *config_dir, int *fd );
static int tgdb_dispatch_command ( struct tgdb *tgdb, struct command *com );

static int tgdb_process_command_container ( struct tgdb *tgdb ) {
	int i;
	int length = queue_size ( tgdb->command_container );
	
	for ( i = 0; i < length; i++ ) {
		if ( tgdb_dispatch_command ( tgdb, queue_pop ( tgdb->command_container )) == -1 ) {
			err_msg("%s:%d tgdb_dispatch_command error", __FILE__, __LINE__);
			return -1;
		}
	}

	return 0;
}

struct tgdb *initialize_tgdb_context ( void ) {
	struct tgdb *tgdb = 
		(struct tgdb *)xmalloc ( sizeof ( struct tgdb ) );
	
	tgdb->a2 			  = NULL;
	tgdb->control_c 	  = 0;

	tgdb->debugger_stdout = -1;
	tgdb->debugger_stdin  = -1;

	tgdb->inferior_stdout = -1;
	tgdb->inferior_stdin  = -1;

	tgdb->gdb_input_queue = NULL;
	tgdb->raw_input_queue = NULL;
	tgdb->oob_input_queue = NULL;
	tgdb->rlc_input_queue = NULL;

	tgdb->command_container = NULL;

	tgdb->rl 			  = NULL;

	tgdb->current_command = NULL;

	tgdb->tgdb_partially_run_command = 0;

	tgdb->IS_SUBSYSTEM_READY_FOR_NEXT_COMMAND = 1;
	tgdb->HAS_USER_SENT_COMMAND = 0;

	tgdb->last_gui_command = NULL;
	tgdb->show_gui_commands = 0;
	
	return tgdb;
}

/*******************************************************************************
 * This is the basic initialization
 ******************************************************************************/

/* 
 * tgdb_initialize_config_dir
 *
 *  Gets the users home dir and creates the config directory.
 *
 *  tgdb       - The tgdb context.
 *
 *  config_dir - Should be PATH_MAX in size on way in.
 *               On way out, it will be the path to the config dir
 *
 *  Returns -1 on error, or 0 on success
 */
static int tgdb_initialize_config_dir ( struct tgdb *tgdb, char *config_dir ) {
    /* Get the home directory */
    char *home_dir = getenv("HOME");
    const char *tgdb_dir = ".tgdb";

    /* Create the config directory */
    if ( ! fs_util_create_dir_in_base ( home_dir, tgdb_dir ) ) {
        err_msg("%s:%d fs_util_create_dir_in_base error", __FILE__, __LINE__);
        return -1; 
    }

    fs_util_get_path ( home_dir, tgdb_dir, config_dir );

    return 0;
}

struct tgdb* tgdb_initialize ( 
	const char *debugger, 
	int argc, char **argv,
	int *debugger_fd, int *inferior_fd, int *readline_fd ) {
	/* Initialize the libtgdb context */
	struct tgdb *tgdb = initialize_tgdb_context ();
    char config_dir[PATH_MAX];

    /* Create config directory */
    if ( tgdb_initialize_config_dir ( tgdb, config_dir ) == -1 ) {
        err_msg("%s:%d tgdb_initialize error", __FILE__, __LINE__);
		return NULL;
    }

    if ( tgdb_init_readline ( tgdb, config_dir , readline_fd ) == -1 ) {
        err_msg("%s:%d tgdb_init_readline error", __FILE__, __LINE__);
		return NULL;
    }

    /* Initialize the command queue's */

    /* initialize users buffer */
    tgdb->gdb_input_queue = queue_init();

    /* initialize raw data typed by user */
    tgdb->raw_input_queue = queue_init();

    /* initialize the out of band queue */
    tgdb->oob_input_queue = queue_init();

    /* initialize the readline command queue */
    tgdb->rlc_input_queue = queue_init();

    /* initialize the queue that the ss can append to */
    tgdb->command_container = queue_init();

	/* create an instance and initialize an annotate-two instance */
	if ( (tgdb->a2 = a2_create_instance ( debugger, argc, argv, config_dir ) ) == NULL ) {
        err_msg("%s:%d a2_create_instance error", __FILE__, __LINE__);
        return NULL; 
	}

	if ( a2_initialize ( tgdb->a2, 
					tgdb->command_container,
					&(tgdb->debugger_stdin), &(tgdb->debugger_stdout), 
					&(tgdb->inferior_stdin), &(tgdb->inferior_stdout)) == -1 ) {
        err_msg("%s:%d a2_initialize error", __FILE__, __LINE__);
        return NULL; 
	}

	tgdb_process_command_container ( tgdb );

	*debugger_fd = tgdb->debugger_stdout;
	*inferior_fd = tgdb->inferior_stdout;

    if ( 1 ) {

        /*a2_setup_io( );*/
    } else {
        /*init_gdbmi ();*/
        printf ( "Error: gdb does not support annotations\n" );
        return NULL;
    }

	return tgdb;
}

int tgdb_shutdown ( struct tgdb *tgdb ) {
    /* free readline */
    rlctx_close(tgdb->rl);
	return a2_shutdown ( tgdb->a2 );
}

char* tgdb_err_msg(struct tgdb *tgdb) {
	return NULL;
}

void command_completion_callback ( struct tgdb *tgdb ) {
	tgdb->IS_SUBSYSTEM_READY_FOR_NEXT_COMMAND = 1;
	tgdb->HAS_USER_SENT_COMMAND = 0;
}

static char* tgdb_get_client_command(struct tgdb *tgdb, enum tgdb_command c) {
	return a2_return_client_command ( tgdb->a2, c );
}

static char* tgdb_client_modify_breakpoint_call(struct tgdb *tgdb, const char *file, int line, enum tgdb_breakpoint_action b) {
	return a2_client_modify_breakpoint ( tgdb->a2, file, line , b );
}


/*static void init_gdbmi ( void ) {
    tgdb_get_sources                    = gdbmi_tgdb_get_sources;
    tgdb_get_source_absolute_filename   = gdbmi_tgdb_get_source_absolute_filename;
    tgdb_err_msg                        = gdbmi_tgdb_err_msg;
    tgdb_client_shutdown                = gdbmi_tgdb_shutdown;
	tgdb_get_client_command             = gdbmi_tgdb_return_client_command;
	tgdb_client_modify_breakpoint_call  = gdbmi_tgdb_client_modify_breakpoint;
}*/

/* These functions are used to determine the state of libtgdb */

/* tgdb_can_issue_command: 
 * -----------------------
 *
 *  Determines if tgdb should send data to gdb or put it in a buffer. This 
 *  is when the debugger is ready and there are no commands to run.
 *
 * Returns: TRUE if can issue directly to gdb. Otherwise FALSE.
 */
static int tgdb_can_issue_command(struct tgdb *tgdb) {
    if ( tgdb->IS_SUBSYSTEM_READY_FOR_NEXT_COMMAND &&
	     a2_is_client_ready( tgdb->a2 ) && 
		 (queue_size(tgdb->gdb_input_queue) == 0) )
        return TRUE;

    return FALSE;
}

/* tgdb_can_run_command: 
 * ---------------------
 *
 *  Determines if tgdb has commands it needs to run.
 *
 * Returns: TRUE if can issue directly to gdb. Otherwise FALSE.
 */
static int tgdb_has_command_to_run(struct tgdb *tgdb ) {
    if ( a2_is_client_ready(tgdb->a2) && (
           (queue_size(tgdb->gdb_input_queue) > 0) || 
           (queue_size(tgdb->raw_input_queue) > 0) || 
           (queue_size(tgdb->oob_input_queue) > 0) || 
           (queue_size(tgdb->rlc_input_queue) > 0) || 
           tgdb->current_command != NULL )
       )
        return TRUE;

    return FALSE;
}

/* a2_is_ready:
 * ------------
 *
 *  As of now, I don't know what this is used for.
 *
 * Returns 1 if ready, or 0 if not ready
 */
int is_ready ( struct tgdb *tgdb ) {
	if ( tgdb_can_issue_command(tgdb) &&
		(!tgdb->HAS_USER_SENT_COMMAND)  &&
		tgdb->current_command == NULL )
		return 1;

    return 0;
}

/* tgdb_handle_signals
 */
static int tgdb_handle_signals ( struct tgdb *tgdb ) {
	if ( tgdb->control_c ) {
    /* TODO: Put signal blocking code here so that ^c is not pressed while 
     * checking for it */

        queue_free_list(tgdb->gdb_input_queue, tgdb_interface_free_command);
        tgdb->control_c = 0;

		/* Tell readline that the signal occured */
		if ( rlctx_send_char(tgdb->rl, (char)3) == -1 ) {
			err_msg("(%s:%d) rlctx_send_char failed", __FILE__, __LINE__);
			return -1;
		}
    } 

	return 0;
}

/*******************************************************************************
 * This is the main_loop stuff for tgdb-base
 ******************************************************************************/

/* 
 * Sends a command to the debugger.
 *
 * \param tgdb
 *  An instance of the tgdb library to operate on.
 *
 * \param command
 *  This is the command that should be sent to the debugger.
 *
 * \param bct
 *  This tells tgdb_send who is sending the command. Currently, if readline 
 *  gets a command from the user, it calls this function. Also, this function
 *  gets called usually when the GUI tries to run a command.
 *
 * @return
 *  0 on success, or -1 on error.
 */
static int tgdb_send(struct tgdb *tgdb, char *command, enum buffer_command_type bct) {
    struct command *ncom;
    static char temp_command[MAXLINE];
    int temp_length = strlen ( command );

    strncpy ( temp_command, command, temp_length + 1 );

    if ( temp_command[temp_length-1] != '\n' )
        strcat ( temp_command, "\n" );

    ncom = tgdb_interface_new_command ( 
            temp_command, 
            bct, 
            COMMANDS_SHOW_USER_OUTPUT, 
            COMMANDS_VOID, 
            NULL );

    if ( tgdb_dispatch_command ( tgdb, ncom ) == -1 ) {
        err_msg("%s:%d tgdb_dispatch_command error", __FILE__, __LINE__);
        return -1;
    }
    
    if ( a2_user_ran_command ( tgdb->a2, tgdb->command_container ) == -1 ) {
        err_msg("%s:%d commands_user_ran_command error", __FILE__, __LINE__);
        return -1;
    }

	tgdb_process_command_container ( tgdb );

	return 0;
}

/* tgdb_dispatch_command:
 * ----------------------
 *
 * This will dispatch a command to be run now if ready, otherwise,
 * the command will be put in a queue to run later.
 *
 * com - The command to dispatch
 *
 * return       - 0 on success or -1 on error
 */
static int tgdb_dispatch_command ( struct tgdb *tgdb, struct command *com ) {
    int ret = 0;

    switch ( com->com_type ) {
        case BUFFER_GUI_COMMAND:
        case BUFFER_TGDB_COMMAND:
            if ( tgdb_can_issue_command(tgdb) )
                ret = tgdb_deliver_command ( tgdb, tgdb->debugger_stdin, com );
            else
                queue_append ( tgdb->gdb_input_queue, com );
            break;

        case BUFFER_USER_COMMAND:
            if ( tgdb_can_issue_command(tgdb) )
                ret = tgdb_deliver_command ( tgdb, tgdb->debugger_stdin, com );
            else
                queue_append ( tgdb->raw_input_queue, com );
            break;
        
        case BUFFER_READLINE_COMMAND:
            if ( tgdb_can_issue_command(tgdb) ) {
                ret = tgdb_deliver_command ( tgdb, tgdb->debugger_stdin, com );
            } else
                queue_append ( tgdb->rlc_input_queue, com );
            break;

        case BUFFER_OOB_COMMAND:
            if ( tgdb_can_issue_command(tgdb) )
                ret = tgdb_deliver_command ( tgdb, tgdb->debugger_stdin, com );
            else
                queue_append ( tgdb->oob_input_queue, com ); break; 
        case BUFFER_VOID:
        default:
            err_msg("%s:%d invaled com_type", __FILE__, __LINE__);
            return -1;
    }

    if ( ret == -2 )
        return -2;

    return 0;
}

/* tgdb_deliver_command:
 * ---------------------
 *  
 *  Sends a command to the debugger  
 *
 *  fd      - the file descriptor to write to ( debugger's input )
 *  command - the command to run
 *
 *  NOTE: This function assummes valid commands are being sent to it. 
 *        Error checking should be done before inserting into queue.
 */
static int tgdb_deliver_command ( struct tgdb *tgdb, int fd, struct command *command ) {

    if ( command->com_type == BUFFER_READLINE_COMMAND ) {
        /* A readline command handled by tgdb-base */
        switch ( command->com_to_run ) {
            case COMMANDS_SET_PROMPT:
                if ( rlctx_change_prompt ( tgdb->rl, command->data ) == -1 ) {
                    err_msg("%s:%d rlctx_change_prompt  error", __FILE__, __LINE__);
                    return -1;
                }
                break;
            case COMMANDS_REDISPLAY:
                if ( rlctx_redisplay ( tgdb->rl ) == -1 ) {
                    err_msg("%s:%d rlctx_change_prompt  error", __FILE__, __LINE__);
                    return -1;
                }
                break;
            default:
                err_msg("(%s:%d) rlctx_send_char failed", __FILE__, __LINE__);
                return -1;
        }
    } else {
		tgdb->IS_SUBSYSTEM_READY_FOR_NEXT_COMMAND = 0;
		
		/* Here is where the command is actually given to the debugger.
		 * Before this is done, if the command is a GUI command, we save it,
		 * so that later, it can be printed to the client. Its for debugging
		 * purposes only, or for people who want to know the commands there
		 * debugger is being given.
		 */
		if ( command->com_type == BUFFER_GUI_COMMAND ) 
		   tgdb->last_gui_command = xstrdup ( command->data );

        /* A command for the debugger */
        if ( a2_prepare_for_command ( tgdb->a2, command ) == -1 ) {
            return -2;
        }
    
        /* A regular command from the client */
        io_debug_write_fmt ( "<%s>", command -> data );

        io_writen ( fd, command -> data, strlen ( command -> data ) );
    }

    return 0;
}

/* tgdb_run_buffered_command: Sends to gdb the next command.
 *
 * return:  0 on normal termination ( command was run )
 *          2 if the queue was cleared because of ^c
 */
static int tgdb_run_command( struct tgdb *tgdb ){
tgdb_run_command_tag:

    /* This will redisplay the prompt when a command is run
     * through the gui with data on the console.
     */
    if ( queue_size(tgdb->rlc_input_queue) > 0 ) {
        /* This runs commands through readline */
        struct command *item = NULL;
        item = queue_pop(tgdb->rlc_input_queue);
        tgdb_deliver_command(tgdb, tgdb->debugger_stdin, item);
        tgdb_interface_free_command ( item );
        
    /* The out of band commands should always be run first */
    } else if ( queue_size(tgdb->oob_input_queue) > 0 ) {
        /* These commands are always run. 
         * However, if an assumption is made that a misc
         * prompt can never be set while in this spot.
         */
        struct command *item = NULL;
        item = queue_pop(tgdb->oob_input_queue);
        tgdb_deliver_command(tgdb, tgdb->debugger_stdin, item);
        tgdb_interface_free_command ( item );
    /* If the queue is not empty, run a command */
    } else if ( queue_size(tgdb->gdb_input_queue) > 0 ) {
        struct command *item = NULL;
        item = queue_pop(tgdb->gdb_input_queue);

        /* TODO: The comment and code below is in only one of 2 spots.
         * It also belongs at tgdb_setup_buffer_command_to_run.
         */

        /* If at the misc prompt, don't run the internal tgdb commands,
         * In fact throw them out for now, since they are only 
         * 'info breakpoints' */
        if ( a2_is_misc_prompt(tgdb->a2) == TRUE ) {
            if ( item->com_type != BUFFER_USER_COMMAND ) {
                tgdb_interface_free_command ( item );
                goto tgdb_run_command_tag;
            }
        }

        /* This happens when a command was skipped because the client no longer
         * needs the command to be run */
        if ( tgdb_deliver_command(tgdb, tgdb->debugger_stdin, item) == -2 )
            goto tgdb_run_command_tag;

        if ( tgdb->tgdb_partially_run_command && 
             /* Don't redisplay the prompt for the redisplay prompt command :) */
             item->com_to_run != COMMANDS_REDISPLAY) {
            struct command *ncom;

            ncom = tgdb_interface_new_command ( 
                    NULL,
                    BUFFER_READLINE_COMMAND, 
                    COMMANDS_SHOW_USER_OUTPUT, 
                    COMMANDS_REDISPLAY, 
                    NULL );

            if ( tgdb_dispatch_command ( tgdb, ncom ) == -1 ) {
                err_msg("%s:%d tgdb_dispatch_command error", __FILE__, __LINE__);
                return -1;
            }
        }
        
        tgdb_interface_free_command ( item );

    /* If the user has typed a command, send it through readline */
    } else if ( queue_size(tgdb->raw_input_queue) > 0 ) { 
        struct command *item = queue_pop(tgdb->raw_input_queue);
        char *data = item->data;
        int i, j = strlen ( data );

        for ( i = 0; i < j; i++ ) {
            if ( rlctx_send_char(tgdb->rl, data[i]) == -1 ) {
                err_msg("(%s:%d) rlctx_send_char failed", __FILE__, __LINE__);
                return -1;
            }
        }

        free ( item->data ); 
    /* Send the partially typed command through readline */
    } else if ( tgdb->current_command != NULL ) {
        int i,j = string_length(tgdb->current_command);
        char *data = string_get(tgdb->current_command);
        
        for ( i = 0; i < j; i++ ) {
            if ( rlctx_send_char(tgdb->rl, data[i]) == -1 ) {
                err_msg("(%s:%d) rlctx_send_char failed", __FILE__, __LINE__);
                return -1;
            }
        }

        tgdb->tgdb_partially_run_command = 1;
        tgdb->current_command = NULL;
    }

    return 0;
}

/* tgdb_command_callback:
 * ----------------------
 *
 *  This is called when readline says it has recieved a full line.
 */
static int tgdb_command_callback(void *p, const char *line) {
    static char command[MAXLINE];
	struct tgdb *tgdb = (struct tgdb *)p;
    sprintf ( command, "%s\n", line );
    tgdb_send(tgdb, command, BUFFER_USER_COMMAND);
    return 0;
}

int tgdb_change_prompt ( struct tgdb *tgdb, const char *prompt ) {
    struct command *ncom= tgdb_interface_new_command ( 
            prompt, 
            BUFFER_READLINE_COMMAND, 
            COMMANDS_HIDE_OUTPUT, 
            COMMANDS_SET_PROMPT, 
            NULL );

    if ( tgdb_dispatch_command ( tgdb, ncom ) == -1 ) {
        err_msg("%s:%d tgdb_dispatch_command error", __FILE__, __LINE__);
        return -1;
    }

    return 0;
}

static int tgdb_completion_callback ( void *p, const char *line ) {
    struct command *ncom;
	struct tgdb *tgdb = (struct tgdb *)p;

    /* Allow the client to generate a command for tab completion */
    a2_completion_callback ( tgdb->a2, tgdb->command_container, line ); 

	tgdb_process_command_container ( tgdb );

    /* Redisplay the prompt */
    ncom = tgdb_interface_new_command ( 
            NULL,
            BUFFER_READLINE_COMMAND, 
            COMMANDS_SHOW_USER_OUTPUT, 
            COMMANDS_REDISPLAY, 
            NULL );

    if ( tgdb_dispatch_command ( tgdb, ncom ) == -1 ) {
        err_msg("%s:%d tgdb_dispatch_command error", __FILE__, __LINE__);
        return -1;
    }
    
    return 0;
}

static int tgdb_init_readline ( struct tgdb *tgdb, char *config_dir, int *fd ) {
    /* Initialize readline */
    if ( (tgdb->rl = rlctx_init((const char *)config_dir, "tgdb", (void*)tgdb)) == NULL ) {
        err_msg("(%s:%d) rlctx_init failed", __FILE__, __LINE__);
        return -1;
    }

    /* Register callback for each command recieved at readline */
    if ( rlctx_register_command_callback(tgdb->rl, &tgdb_command_callback) == -1 ) {
        err_msg("(%s:%d) rlctx_register_callback failed", __FILE__, __LINE__);
        return -1;
    }

    /* Register callback for tab completion */
    if ( rlctx_register_completion_callback(tgdb->rl, &tgdb_completion_callback) == -1 ) {
        err_msg("(%s:%d) rlctx_register_callback failed", __FILE__, __LINE__);
        return -1;
    }

    /* Let the GUI check this for reading, 
     * if it finds data, it should call tgdb_recv_input */
    if ( (*fd = rlctx_get_fd(tgdb->rl)) == -1 ) {
        err_msg("%s:%d rlctx_get_fd error", __FILE__, __LINE__);
        return -1;
    }

    return 0;
}

int tgdb_rl_send ( struct tgdb *tgdb, char c ) {
    if ( rlctx_send_char ( tgdb->rl, c ) == -1 ) {
        err_msg("(%s:%d) rlctx_send_char failed", __FILE__, __LINE__);
        return -1;
    }

    return 0;
}

int tgdb_send_debugger_char ( struct tgdb *tgdb, char c ) {
    /* The debugger is ready for input. Send it */
    if ( is_ready( tgdb ) ) {

		if ( c == '\n' ) {
			tgdb->HAS_USER_SENT_COMMAND = 1;
			tgdb->tgdb_partially_run_command = 0;
		} else
			tgdb->tgdb_partially_run_command = 1;

        return tgdb_rl_send ( tgdb, c );

    /* The debugger is not ready, save the input for later */
    } else {
		if ( tgdb->current_command == NULL )
			tgdb->current_command = string_init ( ) ;

        string_addchar ( tgdb->current_command, c );

        if ( c == '\n' ) {
            queue_append ( tgdb->raw_input_queue, tgdb->current_command );
            tgdb->current_command = NULL;
        }
    }

    return 0;
}

/* Maybe this could be optimized, but for now, it just calls 
 * tgdb_send_debugger_char */
int tgdb_send_debugger_data ( struct tgdb *tgdb, const char *buf, const size_t n ) {
	int i;
	for ( i = 0; i < n; i++ ) {
		if ( tgdb_send_debugger_char ( tgdb, buf[i] ) == -1 ) {
        	err_msg("%s:%d tgdb_send_debugger_char error", __FILE__, __LINE__);
			return -1;
		}
	}

	return 0;
}

/* These functions are used to communicate with the inferior */
int tgdb_send_inferior_char ( struct tgdb *tgdb, char c ) {
   if(io_write_byte(tgdb->inferior_stdout, c) == -1){
      err_ret("%s:%d -> could not write byte", __FILE__, __LINE__);
      return -1;
   }
   
   return 0;
}

/* Maybe this could be optimized, but for now, it just calls 
 * tgdb_send_inferior_char */
int tgdb_send_inferior_data ( struct tgdb *tgdb, const char *buf, const size_t n ) {
	int i;
	for ( i = 0; i < n; i++ ) {
		if ( tgdb_send_inferior_char ( tgdb, buf[i] ) == -1 ) {
        	err_msg("%s:%d tgdb_send_debugger_char error", __FILE__, __LINE__);
			return -1;
		}
	}

	return 0;
}

/* tgdb_tty_recv: returns to the caller data from the child */
size_t tgdb_recv_inferior_data ( struct tgdb *tgdb, char *buf, size_t n ) {
   char local_buf[n + 1];
   ssize_t size;

   /* read all the data possible from the child that is ready. */
   if( (size = io_read(tgdb->inferior_stdin, local_buf, n)) < 0){
      err_ret("%s:%d inferior_fd read failed", __FILE__, __LINE__);
      return -1;
   } 
   
   strncpy(buf, local_buf, size); 
   buf[size] = '\0';

   return size; 
}

size_t tgdb_recv_readline_data ( struct tgdb *tgdb, char *buf, size_t n ) {
    int length, i;

    if ( rlctx_recv ( tgdb->rl, buf, MAXLINE ) == -1 ) {
        err_msg("%s:%d rlctx_recv error", __FILE__, __LINE__);
        return -1;
    }

    length = strlen ( buf );

    for ( i = 0; i < length; i++) {
        if ( buf [i] == '\031' ) {
            if ( tgdb_has_command_to_run(tgdb))
                tgdb_run_command(tgdb);
        }
    }
    
    return length;
}

/* 
 * This is called when GDB has finished.
 * Its job is to add the type of QUIT command that is appropriate.
 *
 *
 */
static int tgdb_get_quit_command ( struct tgdb *tgdb, struct queue *q ) {
	pid_t pid 	= a2_get_debugger_pid ( tgdb->a2 );
	int status 	= 0;
	pid_t ret;

	ret = waitpid ( pid, &status, WNOHANG );

	if ( ret == -1 ) {
        err_msg("%s:%d waitpid error", __FILE__, __LINE__);
		return -1;
	} else if ( ret == 0 ) {
		/* The child didn't die, whats wrong */
        err_msg("%s:%d waitpid error", __FILE__, __LINE__);
		return -1;
	} else if ( ret != pid ) {
		/* What process just died ?!? */ 
        err_msg("%s:%d waitpid error", __FILE__, __LINE__);
		return -1;
	}

	if ( (WIFEXITED(status)) == 0 ) /* Child did not exit normally */
		tgdb_append_command(q,TGDB_QUIT_ABNORMAL, NULL );
	else
		tgdb_append_command(q, TGDB_QUIT_NORMAL, NULL );

	return 0;
}

size_t tgdb_recv_debugger_data ( struct tgdb *tgdb, char *buf, size_t n, struct queue *q ) {
    char local_buf[10*n];
    ssize_t size, buf_size;

    /* make the queue empty */
    tgdb_delete_commands(q);

	/* TODO: This is kind of a hack.
	 * Since I know that I didn't do a read yet, the next select loop will
	 * get me back here. This probably shouldn't return, however, I have to
	 * re-write a lot of this function. Also, I think this function should
	 * return a malloc'd string, not a static buffer.
	 *
	 * Currently, I see it as a bigger hack to try to just append this to the
	 * beggining of buf.
	 */
	if ( tgdb->last_gui_command != NULL ) {
		int ret;
		if ( tgdb->show_gui_commands ) {
			strcpy ( buf, tgdb->last_gui_command );
			ret = strlen ( tgdb->last_gui_command );
		} else {
			strcpy ( buf, "\n" );
			ret = 1;
		}

		free ( tgdb->last_gui_command );
		tgdb->last_gui_command = NULL;
		return ret;
	}

    /* set buf to null for debug reasons */
    memset(buf,'\0', n);

    /* 1. read all the data possible from gdb that is ready. */
    if( (size = io_read(tgdb->debugger_stdout, local_buf, n)) < 0){
        err_ret("%s:%d -> could not read from masterfd", __FILE__, __LINE__);
		tgdb_get_quit_command ( tgdb, q );
        return -1;
    } else if ( size == 0 ) {/* EOF */ 
        buf_size = 0;
      
		tgdb_get_quit_command ( tgdb, q );
      
        goto tgdb_finish;
    }

    local_buf[size] = '\0';

    /* 2. At this point local_buf has everything new from this read.
     * Basically this function is responsible for seperating the annotations
     * that gdb writes from the data. 
     *
     * buf and buf_size are the data to be returned from the user.
     */
	{
		/* unused for now */
		char *infbuf = NULL;
		size_t infbuf_size;
		int result;
		result = a2_parse_io ( tgdb->a2, tgdb->command_container, local_buf, size, buf, &buf_size, infbuf, &infbuf_size, q );

		tgdb_process_command_container ( tgdb );

		if ( result == 0 ) {
			/* success, and more to parse, ss isn't done */
		} else if ( result == 1 ) {
			/* success, and finished command */
			command_completion_callback(tgdb);
		} else if ( result == -1 ) { 
			/* error */
        	err_msg("%s:%d a2_parse_io error", __FILE__, __LINE__);
		}
	}

	/* 3. if ^c has been sent, clear the buffers.
     * 	  If a signal has been recieved, clear the queue and return
     */
	if ( tgdb_handle_signals (tgdb) == -1 ) {
        err_msg("%s:%d tgdb_handle_signals error", __FILE__, __LINE__);
		return -1;
	}

    /* 4. runs the users buffered command if any exists */
    if ( tgdb_has_command_to_run(tgdb))
        tgdb_run_command(tgdb);

    tgdb_finish:

    return buf_size;
}


int tgdb_tty_new ( struct tgdb *tgdb ) {
	int ret =
    a2_open_new_tty ( tgdb->a2, tgdb->command_container, &tgdb->inferior_stdin, &tgdb->inferior_stdout );

	tgdb_process_command_container ( tgdb );

	return ret;
}

const char *tgdb_tty_name ( struct tgdb *tgdb ) {
	return a2_get_tty_name ( tgdb->a2 );
}

int tgdb_get_inferiors_source_files ( struct tgdb *tgdb ) {
	int ret = a2_get_inferior_sources ( tgdb->a2, tgdb->command_container );
	tgdb_process_command_container ( tgdb );
	return ret;
}

int tgdb_get_absolute_path ( struct tgdb *tgdb, const char *file ) {
	int ret = a2_get_source_absolute_filename ( tgdb->a2, tgdb->command_container, file );
	tgdb_process_command_container ( tgdb );
	return ret;
}

int tgdb_run_debugger_command ( struct tgdb *tgdb, enum tgdb_command c ) {
 	return tgdb_send ( tgdb, tgdb_get_client_command (tgdb, c), BUFFER_GUI_COMMAND );
}

int tgdb_modify_breakpoint ( struct tgdb *tgdb, const char *file, int line, enum tgdb_breakpoint_action b ) {
	char *val = tgdb_client_modify_breakpoint_call ( tgdb, file, line, b );

	if ( val == NULL )
		return -1;

	if ( tgdb_send ( tgdb, val, BUFFER_GUI_COMMAND ) == -1 ) {
        err_msg("%s:%d tgdb_send error", __FILE__, __LINE__);
		return -1;
	}

	free ( val );

	return 0;
}

int tgdb_signal_notification ( struct tgdb *tgdb, int signum ) {
    if ( signum == SIGINT ) {               /* ^c */
        tgdb->control_c = 1;
        kill(a2_get_debugger_pid( tgdb->a2 ), SIGINT);
    } else if ( signum == SIGTERM ) { 
        kill(a2_get_debugger_pid( tgdb->a2 ), SIGTERM);
    } else if ( signum == SIGQUIT ) {       /* ^\ */
        kill(a2_get_debugger_pid( tgdb->a2 ), SIGQUIT);
    }

	return 0;
}

int tgdb_set_verbose_gui_command_output ( struct tgdb *tgdb, int value ) {
	if ( (value == 0) || (value == 1) )
		tgdb->show_gui_commands = value;

	if ( tgdb->show_gui_commands == 1 )
		return 1;
	
	return 0;
}
