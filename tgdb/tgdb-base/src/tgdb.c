#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_STDLIB_H 
#include <stdlib.h>
#endif  /* HAVE_STDLIB_H */

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
#include "tgdb_command.h"
#include "tgdb_client_command.h"
#include "tgdb_client_interface.h"
#include "fs_util.h"
#include "rlctx.h"
#include "ibuf.h"
#include "io.h"
#include "queue.h"

#include "pseudo.h" /* SLAVE_SIZE constant */
#include "fork_util.h"
#include "sys_util.h"
#include "tgdb_list.h"
#include "logger.h"

static int num_loggers = 0;

/**
 * The TGDB context data structure.
 */
struct tgdb {

	/**
	 * A client context to abstract the debugger.
	 */
	struct tgdb_client_context *tcc;

	/** 
	 * Reading from this will read from the debugger's output
	 */
	int debugger_stdout;

	/** 
	 * Writing to this will write to the debugger's stdin
	 */
	int debugger_stdin;

	/** 
	 * Reading from this will read the stdout from the program being debugged
	 */
	int inferior_stdout;

	/** 
	 * Writing to this will write to the stdin of the program being debugged 
	 */
	int inferior_stdin;

	/***************************************************************************
	 * All the queue's the clients can run commands through
	 **************************************************************************/

	/**
	 * The commands that need to be run through gdb. 
	 * Examples are 'b main', 'run', ...
	 */
	struct queue *gdb_input_queue;

	/**
	 * This is the data that the user typed to form a command.
	 * This data must be sent through readline to get the actual command.
	 * The data will be passed through readline and then readline will send to 
	 * tgdb a command to run.
	 */
	struct queue *raw_input_queue;

	/** 
	 * The out of band input queue. 
	 * These commands should *always* be run first */
	struct queue *oob_input_queue;

	/**
	 * This sends commands to the readline program.
	 */
	struct queue *rlc_input_queue;

	/** 
	 * Interface to readline context
	 */
	struct rlctx *rl;

	/** 
	 * The current command the user is typing at readline 
	 */
	struct ibuf *current_command;

	/** 
	 * This variable needs to be removed from libannotate 
	 * I don't really know if its usefull anymore.
	 */
	unsigned short tgdb_partially_run_command;

	/** 
	 * These are 2 very important state variables.
	 */
	 
	/**
	 * If set to 1, libtgdb thinks the lower level subsystem is capable of 
	 * recieving another command. It needs this so that it doesn't send 2
	 * commands to the lower level before it can say it can't recieve a command.
	 * At some point, maybe this can be removed?
	 * When its set to 0, libtgdb thinks it can not send the lower level another
	 * command.
	 */
	int IS_SUBSYSTEM_READY_FOR_NEXT_COMMAND;

	/** 
	 * This is set to 1 if the user types a full command, followed by the newline.
	 * This is used so that all commands, until that command is finished by the 
	 * lower level subsystem, are queued. That way, only 1 command is sent at a 
	 * time. If it is 0, then the data typed by the user goes directly to the 
	 * readline context, it is not queued.
	 */
	int HAS_USER_SENT_COMMAND;

	/**
	 * If ^c was hit by user
	 */
	sig_atomic_t control_c; 

	/**
	 * This is the last GUI command that has been run.
	 * It is used to display to the client the GUI commands.
	 *
	 * It will either be NULL when it is not set or it should be
	 * the last GUI command run. If it is non-NULL it should be from the heap.
	 * As anyone is allowed to call xfree on it.
	 */
	char *last_gui_command;

	/**
	 * This is a TGDB option.
	 * It determines if the user wants to see the commands the GUI is running.
	 * 
	 * If it is 0, the user does not want to see the commands the GUI is 
	 * running. Otherwise, if it is 1, it does.
	 */
	int show_gui_commands;

	/**
	 * This is the queue of commands TGDB has currently made to give to the 
	 * front end.
	 */
	struct tgdb_list *command_list;

	/**
	 * An iterator into command_list.
	 */
	tgdb_list_iterator *command_list_iterator;
};

/* Temporary prototypes */
static int tgdb_deliver_command ( struct tgdb *tgdb, int fd, struct tgdb_queue_command *command );
static int tgdb_run_command( struct tgdb *tgdb );
static int tgdb_init_readline ( struct tgdb *tgdb, char *config_dir, int *fd );
static int tgdb_dispatch_command ( struct tgdb *tgdb, struct tgdb_queue_command *com );

/**
 * Process the commands that were created by the client
 *
 * \param tgdb
 * The TGDB context
 *
 * @return
 * -1 on error, 0 on success
 */
static int tgdb_process_client_commands ( struct tgdb *tgdb ) {
	struct tgdb_list *client_command_list;
	tgdb_list_iterator *iterator;
	struct tgdb_client_command *client_command;
	struct tgdb_queue_command *command;
	char *command_data = NULL;

	client_command_list = tgdb_client_get_client_commands ( tgdb->tcc );
	iterator = tgdb_list_get_first ( client_command_list );

	while ( iterator ) {
		enum tgdb_command_choice command_choice;
		enum tgdb_command_action_choice action_choice = TGDB_COMMAND_ACTION_NONE;

		client_command = ( struct tgdb_client_command *) tgdb_list_get_item ( iterator );

		switch ( client_command->command_choice ) {
			case TGDB_CLIENT_COMMAND_NORMAL:
				command_choice = TGDB_COMMAND_TGDB_CLIENT;
				break;

			case TGDB_CLIENT_COMMAND_PRIORITY:
				command_choice = TGDB_COMMAND_TGDB_CLIENT_PRIORITY;
				break;
				
			case TGDB_CLIENT_COMMAND_TGDB_BASE:
				command_choice = TGDB_COMMAND_READLINE;
				switch ( client_command->action_choice ) {
					case TGDB_CLIENT_COMMAND_ACTION_NONE:
						break;
					case TGDB_CLIENT_COMMAND_ACTION_CONSOLE_SET_PROMPT:
						action_choice = TGDB_COMMAND_ACTION_CONSOLE_SET_PROMPT;
						command_data = strdup ( client_command->tgdb_client_command_data );
						break;

					default:
						logger_write_pos ( logger, __FILE__, __LINE__, "unknown switch case" );
						return -1;
				}
				break;

			default:
				logger_write_pos ( logger, __FILE__, __LINE__, "unknown switch case" );
				return -1;
		}

		command = tgdb_command_create ( 
				command_data,
				command_choice,
				action_choice,
				client_command );

		if ( tgdb_dispatch_command ( tgdb, command ) == -1 ) {
			logger_write_pos ( logger, __FILE__, __LINE__, "tgdb_dispatch_command failed" );
			return -1;
		}

		iterator = tgdb_list_next ( iterator );
	}

	/* free the list of client commands */
	tgdb_list_clear ( client_command_list );

	return 0;
}

static struct tgdb *initialize_tgdb_context ( void ) {
	struct tgdb *tgdb = 
		(struct tgdb *)xmalloc ( sizeof ( struct tgdb ) );
	
	tgdb->tcc 			  = NULL;
	tgdb->control_c 	  = 0;

	tgdb->debugger_stdout = -1;
	tgdb->debugger_stdin  = -1;

	tgdb->inferior_stdout = -1;
	tgdb->inferior_stdin  = -1;

	tgdb->gdb_input_queue = NULL;
	tgdb->raw_input_queue = NULL;
	tgdb->oob_input_queue = NULL;
	tgdb->rlc_input_queue = NULL;

	tgdb->rl 			  = NULL;

	tgdb->current_command = NULL;

	tgdb->tgdb_partially_run_command = 0;

	tgdb->IS_SUBSYSTEM_READY_FOR_NEXT_COMMAND = 1;
	tgdb->HAS_USER_SENT_COMMAND = 0;

	tgdb->last_gui_command = NULL;
	tgdb->show_gui_commands = 0;

	tgdb->command_list = tgdb_list_init();

	logger = NULL;
	
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
 *  config_dir - Should be FSUTIL_PATH_MAX in size on way in.
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
        logger_write_pos ( logger, __FILE__, __LINE__, "fs_util_create_dir_in_base error");
        return -1; 
    }

    fs_util_get_path ( home_dir, tgdb_dir, config_dir );

    return 0;
}

/**
 * Knowing the user's home directory, TGDB can initialize the logger interface
 *
 *  \param tgdb
 *  The tgdb context.
 *
 *  \param config_dir 
 *  The path to the user's config directory
 *
 *  \return
 *  -1 on error, or 0 on success
 */
static int tgdb_initialize_logger_interface ( 
		struct tgdb *tgdb, 
		char *config_dir ) {

    /* Get the home directory */
    const char *tgdb_log_file = "tgdb_log.txt";
	char tgdb_log_path[FSUTIL_PATH_MAX];

    fs_util_get_path ( config_dir, tgdb_log_file, tgdb_log_path );

	/* Initialize the logger */
	if ( num_loggers == 0 ) {
		logger = logger_create ();

		if ( !logger ) {
			printf ( "Error: Could not create log file\n" );
			return -1;
		}
	}

	++num_loggers;

	if ( logger_set_file ( logger, tgdb_log_path ) == -1 ) {
        printf ( "Error: Could not open log file\n" );
		return -1;	
	}

    return 0;
}

struct tgdb* tgdb_initialize ( 
	const char *debugger, 
	int argc, char **argv,
	int *debugger_fd, int *inferior_fd, int *readline_fd ) {
	/* Initialize the libtgdb context */
	struct tgdb *tgdb = initialize_tgdb_context ();
    char config_dir[FSUTIL_PATH_MAX];

    /* Create config directory */
    if ( tgdb_initialize_config_dir ( tgdb, config_dir ) == -1 ) {
        logger_write_pos ( logger, __FILE__, __LINE__, "tgdb_initialize error");
		return NULL;
    }

	if ( tgdb_initialize_logger_interface ( tgdb, config_dir ) == -1 ) {
        printf ( "Could not initialize logger interface\n" );
		return NULL;
	}

    if ( tgdb_init_readline ( tgdb, config_dir , readline_fd ) == -1 ) {
		logger_write_pos ( logger, __FILE__, __LINE__, "tgdb_init_readline failed" );
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

	tgdb->tcc = tgdb_client_create_context ( 
			debugger, argc, argv, config_dir,
			TGDB_CLIENT_DEBUGGER_GNU_GDB,
			TGDB_CLIENT_PROTOCOL_GNU_GDB_ANNOTATE_TWO,
		    logger	);

	/* create an instance and initialize a tgdb_client_context */
	if ( tgdb->tcc == NULL ) {
		logger_write_pos ( logger, __FILE__, __LINE__, "tgdb_client_create_context failed" );
        return NULL; 
	}

	if ( tgdb_client_initialize_context ( 
				tgdb->tcc,
				&(tgdb->debugger_stdin), &(tgdb->debugger_stdout), 
				&(tgdb->inferior_stdin), &(tgdb->inferior_stdout)) == -1 ) {
		logger_write_pos ( logger, __FILE__, __LINE__, "tgdb_client_initialize failed" );
        return NULL; 
	}

	tgdb_process_client_commands ( tgdb );

	*debugger_fd = tgdb->debugger_stdout;
	*inferior_fd = tgdb->inferior_stdout;

    if ( 1 ) {

        /*a2_setup_io( );*/
    } else {
        /*init_gdbmi ();*/
		logger_write_pos ( logger, __FILE__, __LINE__, "GDB does not support annotations" );
        return NULL;
    }

	return tgdb;
}

int tgdb_shutdown ( struct tgdb *tgdb ) {
    /* free readline */
    rlctx_close(tgdb->rl);

	/* Free the logger */	
	if ( num_loggers == 1 ) {
		if ( logger_destroy ( logger ) == -1 ) {
			printf ( "Could not destroy logger interface\n" );
			return -1;
		}
	}

	--num_loggers;
	
	return tgdb_client_destroy_context ( tgdb->tcc );
}

char* tgdb_err_msg(struct tgdb *tgdb) {
	return NULL;
}

void command_completion_callback ( struct tgdb *tgdb ) {
	tgdb->IS_SUBSYSTEM_READY_FOR_NEXT_COMMAND = 1;
	tgdb->HAS_USER_SENT_COMMAND = 0;
}

static char* tgdb_get_client_command(struct tgdb *tgdb, enum tgdb_command_type c) {
	return tgdb_client_return_command ( tgdb->tcc, c );
}

static char* tgdb_client_modify_breakpoint_call(struct tgdb *tgdb, const char *file, int line, enum tgdb_breakpoint_action b) {
	return tgdb_client_modify_breakpoint ( tgdb->tcc, file, line , b );
}


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
	     tgdb_client_is_client_ready( tgdb->tcc ) && 
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
    if ( tgdb_client_is_client_ready(tgdb->tcc) && (
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

        queue_free_list(tgdb->gdb_input_queue, tgdb_command_destroy);
        tgdb->control_c = 0;

		/* Tell readline that the signal occured */
		if ( rlctx_send_char(tgdb->rl, (char)3) == -1 ) {
			logger_write_pos ( logger, __FILE__, __LINE__, "rlctx_send_char failed" );
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
 * An instance of the tgdb library to operate on.
 *
 * \param command
 * This is the command that should be sent to the debugger.
 *
 * \param command_choice
 * This tells tgdb_send who is sending the command. Currently, if readline 
 * gets a command from the user, it calls this function. Also, this function
 * gets called usually when the GUI tries to run a command.
 *
 * @return
 * 0 on success, or -1 on error.
 */
static int tgdb_send (
		struct tgdb *tgdb, 
		char *command, 
		enum tgdb_command_choice command_choice) {

   struct tgdb_client_command *tcc;
	struct tgdb_queue_command *tc;
	struct ibuf *temp_command = ibuf_init ();
   int length = strlen ( command );

	/* Add a newline to the end of the command if it doesn't exist */
	ibuf_add ( temp_command, command );

    if ( command[length-1] != '\n' )
		ibuf_addchar ( temp_command, '\n' );

	/* Create the client command */
    tcc = tgdb_client_command_create ( 
            ibuf_get ( temp_command ), 
			   TGDB_CLIENT_COMMAND_NORMAL,
            TGDB_CLIENT_COMMAND_DISPLAY_COMMAND_AND_RESULTS, 
            TGDB_CLIENT_COMMAND_ACTION_NONE, 
            NULL );

	ibuf_free ( temp_command );
	temp_command = NULL;

	/* Create the TGDB command */
	tc = tgdb_command_create ( 
			NULL,
			command_choice, 
			TGDB_COMMAND_ACTION_NONE,
			tcc );

    if ( tgdb_dispatch_command ( tgdb, tc ) == -1 ) {
		logger_write_pos ( logger, __FILE__, __LINE__, "tgdb_dispatch_command failed" );
        return -1;
    }
    
    if ( tgdb_client_tgdb_ran_command ( tgdb->tcc ) == -1 ) {
		logger_write_pos ( logger, __FILE__, __LINE__, "tgdb_client_tgdb_ran_command failed" );
        return -1;
    }

	tgdb_process_client_commands ( tgdb );

	return 0;
}

/**
 * This will dispatch a command to be run now if ready, otherwise,
 * the command will be put in a queue to run later.
 *
 * \param tgdb
 * The TGDB context to use.
 *
 * \param command
 * The command to dispatch
 *
 * @return
 * 0 on success or -1 on error
 */
static int tgdb_dispatch_command ( 
		struct tgdb *tgdb, 
		struct tgdb_queue_command *command ) {

    int ret = 0;

    switch ( command->command_choice ) {
        case TGDB_COMMAND_FRONT_END:
		case TGDB_COMMAND_TGDB_CLIENT:
            if ( tgdb_can_issue_command(tgdb) )
                ret = tgdb_deliver_command ( tgdb, tgdb->debugger_stdin, command );
            else
                queue_append ( tgdb->gdb_input_queue, command );
            break;

        case TGDB_COMMAND_CONSOLE:
            if ( tgdb_can_issue_command(tgdb) )
                ret = tgdb_deliver_command ( tgdb, tgdb->debugger_stdin, command );
            else
                queue_append ( tgdb->raw_input_queue, command);
            break;
        
        case TGDB_COMMAND_READLINE:
            if ( tgdb_can_issue_command(tgdb) ) {
                ret = tgdb_deliver_command ( tgdb, tgdb->debugger_stdin, command );
            } else
                queue_append ( tgdb->rlc_input_queue, command );
            break;

        case TGDB_COMMAND_TGDB_CLIENT_PRIORITY:
            if ( tgdb_can_issue_command(tgdb) )
                ret = tgdb_deliver_command ( tgdb, tgdb->debugger_stdin, command );
            else
                queue_append ( tgdb->oob_input_queue, command ); break; 
        default:
			logger_write_pos ( logger, __FILE__, __LINE__, "unimplemented command" );
            return -1;
    }

    if ( ret == -2 )
        return -2;

    return 0;
}

/** 
 *  Sends a command to the debugger  
 *
 * \param tgdb
 * The TGDB context to use.
 *
 * \param fd 
 * the file descriptor to write to ( debugger's input )
 *
 * \param command 
 * the command to run
 *
 *  NOTE: This function assummes valid commands are being sent to it. 
 *        Error checking should be done before inserting into queue.
 */
static int tgdb_deliver_command ( 
		struct tgdb *tgdb, 
		int fd, 
		struct tgdb_queue_command *command ) {

	struct tgdb_client_command *client_command = command->client_command;

    if ( command->command_choice == TGDB_COMMAND_READLINE ) {
        /* A readline command handled by tgdb-base */
        switch ( command->action_choice ) {
            case TGDB_COMMAND_ACTION_CONSOLE_SET_PROMPT:
                if ( rlctx_change_prompt ( tgdb->rl, client_command->tgdb_client_command_data ) == -1 ) {
					logger_write_pos ( logger, __FILE__, __LINE__, "rlctx_change_prompt failed" );
                    return -1;
                }
                break;
            case TGDB_COMMAND_ACTION_CONSOLE_REDISPLAY_PROMPT:
                if ( rlctx_redisplay ( tgdb->rl ) == -1 ) {
					logger_write_pos ( logger, __FILE__, __LINE__, "rlctx_redisplay failed" );
                    return -1;
                }
                break;
            default:
				logger_write_pos ( logger, __FILE__, __LINE__, "unknown switch case" );
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
		if ( command->command_choice == TGDB_COMMAND_FRONT_END ) 
		   tgdb->last_gui_command = xstrdup ( client_command->tgdb_client_command_data );

        /* A command for the debugger */
		if ( tgdb_client_prepare_for_command ( tgdb->tcc, client_command ) == -1 ) {
			return -2;
		}
    
        /* A regular command from the client */
        io_debug_write_fmt ( "<%s>", client_command->tgdb_client_command_data );

        io_writen ( 
				fd, 
				client_command->tgdb_client_command_data, 
				strlen ( client_command->tgdb_client_command_data ) );

        /* Uncomment this if you wish to see all of the commands, that are 
         * passed to GDB. */
#if 0
        {
            char *s = strdup ( client_command->tgdb_client_command_data );
            int length = strlen ( s );
            s[length-1] = '\0';
            fprintf ( stderr, "[%s]\n", s );  
            s[length-1] = ' ';
            free ( s );
            s = NULL;
        }
#endif
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
        struct tgdb_queue_command *item = NULL;
        item = queue_pop(tgdb->rlc_input_queue);
        tgdb_deliver_command(tgdb, tgdb->debugger_stdin, item);
        tgdb_command_destroy ( item );
        
    /* The out of band commands should always be run first */
    } else if ( queue_size(tgdb->oob_input_queue) > 0 ) {
        /* These commands are always run. 
         * However, if an assumption is made that a misc
         * prompt can never be set while in this spot.
         */
        struct tgdb_queue_command *item = NULL;
        item = queue_pop(tgdb->oob_input_queue);
        tgdb_deliver_command(tgdb, tgdb->debugger_stdin, item);
        tgdb_command_destroy ( item );
    /* If the queue is not empty, run a command */
    } else if ( queue_size(tgdb->gdb_input_queue) > 0 ) {
        struct tgdb_queue_command *item = NULL;
        item = queue_pop(tgdb->gdb_input_queue);

        /* TODO: The comment and code below is in only one of 2 spots.
         * It also belongs at tgdb_setup_buffer_command_to_run.
         */

        /* If at the misc prompt, don't run the internal tgdb commands,
         * In fact throw them out for now, since they are only 
         * 'info breakpoints' */
        if ( tgdb_client_can_tgdb_run_commands(tgdb->tcc) == TRUE ) {
            if ( item->command_choice != TGDB_COMMAND_CONSOLE ) {
                tgdb_command_destroy ( item );
                goto tgdb_run_command_tag;
            }
        }

        /* This happens when a command was skipped because the client no longer
         * needs the command to be run */
        if ( tgdb_deliver_command(tgdb, tgdb->debugger_stdin, item) == -2 )
            goto tgdb_run_command_tag;

        if ( tgdb->tgdb_partially_run_command && 
             /* Don't redisplay the prompt for the redisplay prompt command :) */
             item->action_choice != TGDB_COMMAND_ACTION_CONSOLE_REDISPLAY_PROMPT) {
			struct tgdb_queue_command *command;

			command = tgdb_command_create ( 
					NULL,
					TGDB_COMMAND_READLINE,
					TGDB_COMMAND_ACTION_CONSOLE_REDISPLAY_PROMPT,
					NULL);

            if ( tgdb_dispatch_command ( tgdb, command ) == -1 ) {
				logger_write_pos ( logger, __FILE__, __LINE__, "tgdb_dispatch_command failed" );
                return -1;
            }
        }
        
        tgdb_command_destroy ( item );

    /* If the user has typed a command, send it through readline */
    } else if ( queue_size(tgdb->raw_input_queue) > 0 ) { 
        struct tgdb_queue_command *item = queue_pop(tgdb->raw_input_queue);
        char *data = item->client_command->tgdb_client_command_data;
        int i, j = strlen ( data );

        for ( i = 0; i < j; i++ ) {
            if ( rlctx_send_char(tgdb->rl, data[i]) == -1 ) {
				logger_write_pos ( logger, __FILE__, __LINE__, "rlctx_send_char failed" );
                return -1;
            }
        }

        tgdb_command_destroy ( item );
        item = NULL;
    /* Send the partially typed command through readline */
    } else if ( tgdb->current_command != NULL ) {
        int i,j = ibuf_length(tgdb->current_command);
        char *data = ibuf_get(tgdb->current_command);
        
        for ( i = 0; i < j; i++ ) {
            if ( rlctx_send_char(tgdb->rl, data[i]) == -1 ) {
				logger_write_pos ( logger, __FILE__, __LINE__, "rlctx_send_char failed" );
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
   struct ibuf *command = ibuf_init ();
   struct tgdb *tgdb = (struct tgdb *)p;
   ibuf_add ( command, line );
   ibuf_addchar ( command, '\n' );
   tgdb_send(tgdb, ibuf_get ( command ), TGDB_COMMAND_CONSOLE);
   ibuf_free ( command );
   command = NULL;
   return 0;
}

int tgdb_change_prompt ( struct tgdb *tgdb, const char *prompt ) {

	struct tgdb_queue_command *command = tgdb_command_create ( 
			prompt,
			TGDB_COMMAND_READLINE,
			TGDB_COMMAND_ACTION_CONSOLE_SET_PROMPT,
			NULL );

    if ( tgdb_dispatch_command ( tgdb, command ) == -1 ) {
		logger_write_pos ( logger, __FILE__, __LINE__, "tgdb_dispatch_command failed" );
        return -1;
    }

    return 0;
}

/**
 * This is called called when the user desires tab completion.
 */
static int tgdb_completion_callback ( void *p, const char *line ) {
	struct tgdb_queue_command *command;
	struct tgdb *tgdb = (struct tgdb *)p;

    /* Allow the client to generate a command for tab completion */
    tgdb_client_completion_callback ( tgdb->tcc, line ); 

	tgdb_process_client_commands ( tgdb );

	command = tgdb_command_create ( 
			NULL,
			TGDB_COMMAND_READLINE,
			TGDB_COMMAND_ACTION_CONSOLE_REDISPLAY_PROMPT,
			NULL );

    if ( tgdb_dispatch_command ( tgdb, command ) == -1 ) {
		logger_write_pos ( logger, __FILE__, __LINE__, "tgdb_dispatch_command failed" );
        return -1;
    }
    
    return 0;
}

static int tgdb_init_readline ( struct tgdb *tgdb, char *config_dir, int *fd ) {
    /* Initialize readline */
    if ( (tgdb->rl = rlctx_init((const char *)config_dir, "tgdb", (void*)tgdb)) == NULL ) {
		logger_write_pos ( logger, __FILE__, __LINE__, "rlctx_init failed" );
        return -1;
    }

    /* Register callback for each command recieved at readline */
    if ( rlctx_register_command_callback(tgdb->rl, &tgdb_command_callback) == -1 ) {
		logger_write_pos ( logger, __FILE__, __LINE__, "rlctx_register_command_callback failed" );
        return -1;
    }

    /* Register callback for tab completion */
    if ( rlctx_register_completion_callback(tgdb->rl, &tgdb_completion_callback) == -1 ) {
		logger_write_pos ( logger, __FILE__, __LINE__, "rlctx_register_completion_callback failed" );
        return -1;
    }

    /* Let the GUI check this for reading, 
     * if it finds data, it should call tgdb_recv_input */
    if ( (*fd = rlctx_get_fd(tgdb->rl)) == -1 ) {
		logger_write_pos ( logger, __FILE__, __LINE__, "rlctx_get_fd failed" );
        return -1;
    }

    return 0;
}

int tgdb_rl_send ( struct tgdb *tgdb, char c ) {
    if ( rlctx_send_char ( tgdb->rl, c ) == -1 ) {
		logger_write_pos ( logger, __FILE__, __LINE__, "rlctx_send_char failed" );
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
			tgdb->current_command = ibuf_init ( ) ;

        ibuf_addchar ( tgdb->current_command, c );

        if ( c == '\n' ) {
          struct tgdb_client_command *tcc;
          struct tgdb_queue_command *tc;
          tcc = tgdb_client_command_create ( 
                  ibuf_get ( tgdb->current_command ), 
                  TGDB_CLIENT_COMMAND_NORMAL,
                  TGDB_CLIENT_COMMAND_DISPLAY_COMMAND_AND_RESULTS, 
                  TGDB_CLIENT_COMMAND_ACTION_NONE, 
                  NULL );
           tc = tgdb_command_create ( 
              NULL,
              TGDB_COMMAND_CONSOLE,
              TGDB_COMMAND_ACTION_NONE,
              tcc );

           queue_append ( tgdb->raw_input_queue, tc );
           ibuf_free ( tgdb->current_command );
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
			logger_write_pos ( logger, __FILE__, __LINE__, "tgdb_send_debugger_char failed" );
			return -1;
		}
	}

	return 0;
}

/* These functions are used to communicate with the inferior */
int tgdb_send_inferior_char ( struct tgdb *tgdb, char c ) {
   if(io_write_byte(tgdb->inferior_stdout, c) == -1){
	  logger_write_pos ( logger, __FILE__, __LINE__, "io_write_byte failed" );
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
			logger_write_pos ( logger, __FILE__, __LINE__, "tgdb_send_inferior_char failed" );
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
      logger_write_pos ( logger, __FILE__, __LINE__, "inferior_fd read failed");
      return -1;
   } 
   
   strncpy(buf, local_buf, size); 
   buf[size] = '\0';

   return size; 
}

size_t tgdb_recv_readline_data ( struct tgdb *tgdb, char *buf, size_t n ) {
    int length, i;

    if ( rlctx_recv ( tgdb->rl, buf, n ) == -1 ) {
		logger_write_pos ( logger, __FILE__, __LINE__, "rlctx_recv failed" );
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
static int tgdb_get_quit_command ( struct tgdb *tgdb ) {
	pid_t pid 	= tgdb_client_get_debugger_pid ( tgdb->tcc );
	int status 	= 0;
	pid_t ret;
	struct tgdb_debugger_exit_status *tstatus = (struct tgdb_debugger_exit_status *)
		xmalloc ( sizeof ( struct tgdb_debugger_exit_status ) );

	ret = waitpid ( pid, &status, WNOHANG );

	if ( ret == -1 ) {
		logger_write_pos ( logger, __FILE__, __LINE__, "waitpid error" );
		return -1;
	} else if ( ret == 0 ) {
		/* The child didn't die, whats wrong */
		logger_write_pos ( logger, __FILE__, __LINE__, "waitpid error" );
		return -1;
	} else if ( ret != pid ) {
		/* What process just died ?!? */ 
		logger_write_pos ( logger, __FILE__, __LINE__, "waitpid error" );
		return -1;
	}

	if ( (WIFEXITED(status)) == 0 ) {
		/* Child did not exit normally */
		tstatus->exit_status  = -1;
		tstatus->return_value = 0;
	} else {
		tstatus->exit_status  = 0;
		tstatus->return_value = WEXITSTATUS(status);
	}

	tgdb_types_append_command ( tgdb->command_list, TGDB_QUIT, tstatus );

	return 0;
}

size_t tgdb_recv_debugger_data ( struct tgdb *tgdb, char *buf, size_t n ) {
    char local_buf[10*n];
    ssize_t size, buf_size;

    /* make the queue empty */
    tgdb_delete_commands(tgdb);

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
        logger_write_pos ( logger, __FILE__, __LINE__, "could not read from masterfd");
		tgdb_get_quit_command ( tgdb );
        return -1;
    } else if ( size == 0 ) {/* EOF */ 
        buf_size = 0;
      
		tgdb_get_quit_command ( tgdb );
      
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
		result = tgdb_client_parse_io ( 
				tgdb->tcc, 
				local_buf, size, 
				buf, &buf_size, 
				infbuf, &infbuf_size, 
				tgdb->command_list );

		tgdb_process_client_commands ( tgdb );

		if ( result == 0 ) {
			/* success, and more to parse, ss isn't done */
		} else if ( result == 1 ) {
			/* success, and finished command */
			command_completion_callback(tgdb);
		} else if ( result == -1 ) { 
			logger_write_pos ( logger, __FILE__, __LINE__, "tgdb_client_parse_io failed" );
		}
	}

	/* 3. if ^c has been sent, clear the buffers.
     * 	  If a signal has been recieved, clear the queue and return
     */
	if ( tgdb_handle_signals (tgdb) == -1 ) {
		logger_write_pos ( logger, __FILE__, __LINE__, "tgdb_handle_signals failed" );
		return -1;
	}

    /* 4. runs the users buffered command if any exists */
    if ( tgdb_has_command_to_run(tgdb))
        tgdb_run_command(tgdb);

    tgdb_finish:

	/* Set the iterator to the beggining. So when the user
	 * calls tgdb_get_command it, it will be in the right spot.
	 */
	tgdb->command_list_iterator = tgdb_list_get_first ( tgdb->command_list );

    return buf_size;
}

struct tgdb_command *tgdb_get_command ( struct tgdb *tgdb ) {
	struct tgdb_command *command;

	if ( tgdb->command_list_iterator == NULL )
		return NULL;

	command = (struct tgdb_command *) tgdb_list_get_item ( 
			tgdb->command_list_iterator );

	tgdb->command_list_iterator = tgdb_list_next ( 
			tgdb->command_list_iterator );

	return command;
}

int tgdb_tty_new ( struct tgdb *tgdb ) {
	int ret =
    tgdb_client_open_new_tty ( 
			tgdb->tcc, 
			&tgdb->inferior_stdin, &tgdb->inferior_stdout );

	tgdb_process_client_commands ( tgdb );

	return ret;
}

const char *tgdb_tty_name ( struct tgdb *tgdb ) {
	return tgdb_client_get_tty_name ( tgdb->tcc );
}

int tgdb_get_inferiors_source_files ( struct tgdb *tgdb ) {
	int ret = tgdb_client_get_inferior_source_files ( tgdb->tcc );
	tgdb_process_client_commands ( tgdb );
	return ret;
}

int tgdb_get_absolute_path ( struct tgdb *tgdb, const char *file ) {
	int ret = tgdb_client_get_absolute_path ( tgdb->tcc, file );
	tgdb_process_client_commands ( tgdb );
	return ret;
}

int tgdb_run_debugger_command ( struct tgdb *tgdb, enum tgdb_command_type c ) {
 	return tgdb_send ( tgdb, tgdb_get_client_command (tgdb, c), TGDB_COMMAND_FRONT_END );
}

int tgdb_modify_breakpoint ( struct tgdb *tgdb, const char *file, int line, enum tgdb_breakpoint_action b ) {
	char *val = tgdb_client_modify_breakpoint_call ( tgdb, file, line, b );

	if ( val == NULL )
		return -1;

	if ( tgdb_send ( tgdb, val, TGDB_COMMAND_FRONT_END ) == -1 ) {
		logger_write_pos ( logger, __FILE__, __LINE__, "tgdb_send failed" );
		return -1;
	}

	free ( val );

	return 0;
}

int tgdb_signal_notification ( struct tgdb *tgdb, int signum ) {
    pid_t pid; 

    pid = tgdb_client_get_debugger_pid( tgdb->tcc );

    if ( pid == -1 )
       return -1;

    if ( signum == SIGINT ) {               /* ^c */
        tgdb->control_c = 1;
        kill(pid, SIGINT);
    } else if ( signum == SIGTERM ) { 
        kill(pid, SIGTERM);
    } else if ( signum == SIGQUIT ) {       /* ^\ */
        kill(pid, SIGQUIT);
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

int tgdb_set_verbose_error_handling ( struct tgdb *tgdb, int value ) {
	if ( value == -1 )
		return logger_is_recording ( logger );

	if ( value == 1 || value == 0)
		logger_set_record ( logger, value );

	if ( value == 1 )
		logger_set_fd ( logger, stderr );

	return logger_is_recording ( logger );
}

void tgdb_traverse_commands ( struct tgdb *tgdb ) {
    tgdb_list_foreach ( tgdb->command_list, tgdb_types_print_command);
}

void tgdb_delete_commands ( struct tgdb *tgdb ) {
    tgdb_list_free (tgdb->command_list, tgdb_types_free_command);
}
