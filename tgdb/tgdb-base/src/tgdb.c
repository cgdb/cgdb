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
#include "globals.h"
#include "pseudo.h" /* SLAVE_SIZE constant */
#include "fork_util.h"
#include "sys_util.h"

/* Reading from this will read from the debugger's output */
static int debugger_stdout = -1;

/* Writing to this will write to the debugger's stdin */
static int debugger_stdin = -1;

/* Reading from this will read the stdout from the program being debugged */
/* Writing to this will write to the stdin of the program being debugged */
static int inferior_fd = -1;
static int inferior_slave_fd = -1;

/* The tty name of the inferior */
static char inferior_tty_name[SLAVE_SIZE];

/******************************************************************************* 
 * All the queue's the clients can run commands through
 *******************************************************************************/

/* gdb_input_queue: 
 * ----------------
 *  The commands that need to be run through gdb. 
 *  Examples are 'b main', 'run', ...
 */
static struct queue *gdb_input_queue;

/* raw_input_queue: 
 * ----------------
 *
 *  This is the data that the user typed to form a command.
 *  This data must be sent through readline to get the actual command.
 *  The data will be passed through readline and then readline will send to 
 *  tgdb a command to run.
 */
static struct queue *raw_input_queue;

/* oob_input_queue:
 * ----------------
 *  The out of band input queue. 
 *  These commands should *always* be run first */
static struct queue *oob_input_queue;

/* rlc_input_queue:
 * ----------------
 *
 *  This sends commands to the readline program.
 */
static struct queue *rlc_input_queue;

/* Interface to readline capability */
static struct rlctx *rl; /* Readline context */

/* The current command the user is typing at readline */
static struct string *current_command = NULL;
static struct queue *raw_input_queue;

/* This variable needs to be removed from libannotate */
extern unsigned short tgdb_partially_run_command;
    
/* Temporary prototypes */
static int tgdb_deliver_command ( int fd, struct command *command );

/* The client's shutdown routine. */ 
static int (*tgdb_client_shutdown)(void);

static char* (*tgdb_get_client_command)(enum tgdb_command c);
static char* (*tgdb_client_modify_breakpoint_call)(const char *file, int line, enum tgdb_breakpoint_action b);

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
static int IS_SUBSYSTEM_READY_FOR_NEXT_COMMAND = 1;
static int HAS_USER_SENT_COMMAND = 0;

void command_completion_callback ( void ) {
	IS_SUBSYSTEM_READY_FOR_NEXT_COMMAND = 1;
	HAS_USER_SENT_COMMAND = 0;
}

static void init_annotate_two ( void ) {
    tgdb_get_sources                    = a2_tgdb_get_sources;
    tgdb_get_source_absolute_filename   = a2_tgdb_get_source_absolute_filename;
    tgdb_err_msg                        = a2_tgdb_err_msg;
    tgdb_client_shutdown                = a2_tgdb_shutdown;
	tgdb_get_client_command             = a2_tgdb_return_client_command;
	tgdb_client_modify_breakpoint_call  = a2_tgdb_client_modify_breakpoint;
}

static void init_gdbmi ( void ) {
    tgdb_get_sources                    = gdbmi_tgdb_get_sources;
    tgdb_get_source_absolute_filename   = gdbmi_tgdb_get_source_absolute_filename;
    tgdb_err_msg                        = gdbmi_tgdb_err_msg;
    tgdb_client_shutdown                = gdbmi_tgdb_shutdown;
	tgdb_get_client_command             = gdbmi_tgdb_return_client_command;
	tgdb_client_modify_breakpoint_call  = gdbmi_tgdb_client_modify_breakpoint;
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
static int tgdb_can_issue_command(void) {
    if ( IS_SUBSYSTEM_READY_FOR_NEXT_COMMAND &&
	     a2_tgdb_is_debugger_ready() && 
		 (queue_size(gdb_input_queue) == 0) )
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
static int tgdb_has_command_to_run(void) {
    if ( a2_tgdb_is_debugger_ready() && (
           (queue_size(gdb_input_queue) > 0) || 
           (queue_size(raw_input_queue) > 0) || 
           (queue_size(oob_input_queue) > 0) || 
           (queue_size(rlc_input_queue) > 0) || 
           current_command != NULL )
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
int is_ready ( void ) {
    /* tgdb is not busy, send the data to readline write away */
//    if( tgdb_can_issue_command() == TRUE )
//        return 1;

	if ( tgdb_can_issue_command() &&
		(!HAS_USER_SENT_COMMAND)  &&
		current_command == NULL )
		return 1;

    return 0;
}

/*******************************************************************************
 * This is the main_loop stuff for tgdb-base
 ******************************************************************************/

/* tgdb_recv: 
 * ----------
 *
 *  Reads data from the debugger.
 *
 */
size_t tgdb_recv(char *buf, size_t n, struct queue *q){
    char local_buf[10*n];
    ssize_t size, buf_size;

    /* make the queue empty */
    tgdb_delete_commands(q);

    /* set buf to null for debug reasons */
    memset(buf,'\0', n);

    /* 1. read all the data possible from gdb that is ready. */
    if( (size = io_read(debugger_stdout, local_buf, n)) < 0){
        err_ret("%s:%d -> could not read from masterfd", __FILE__, __LINE__);
        tgdb_append_command(q, TGDB_QUIT, NULL );
        return -1;
    } else if ( size == 0 ) {/* EOF */ 
        buf_size = 0;
      
        tgdb_append_command(q, TGDB_QUIT, NULL );
      
        goto tgdb_finish;
    }

    local_buf[size] = '\0';

    /* 2. At this point local_buf has everything new from this read.
     * Basically this function is responsible for seperating the annotations
     * that gdb writes from the data. 
     *
     * buf and buf_size are the data to be returned from the user.
     */
    buf_size = a2_handle_data(local_buf, size, buf, n, q);

    /* 3. runs the users buffered command if any exists */
    if ( tgdb_has_command_to_run())
        tgdb_run_command();

    tgdb_finish:

    return buf_size;
}

/* tgdb_send:
 * ----------
 *
 * tgdb_send - sends a commands from the gui to the debugger.
 */
static char *tgdb_send(char *command, int out_type) {
    static char buf[MAXLINE];
    struct command *ncom;
    static char temp_command[MAXLINE];
    int temp_length = strlen ( command );

    strncpy ( temp_command, command, temp_length + 1 );

    if ( temp_command[temp_length-1] != '\n' )
        strcat ( temp_command, "\n" );

    /* The gui's commands do not get returned. So a newline needs to be 
     * returned so that its output is correct.
     */
    if ( out_type == 2 ) {
        buf[0] = '\n';
        buf[1] = '\0';
    } else
        buf[0] = '\0';

    ncom = tgdb_interface_new_command ( 
            temp_command, 
            BUFFER_GUI_COMMAND, 
            COMMANDS_SHOW_USER_OUTPUT, 
            COMMANDS_VOID, 
            NULL );

    if ( tgdb_dispatch_command ( ncom ) == -1 ) {
        err_msg("%s:%d tgdb_dispatch_command error", __FILE__, __LINE__);
        return NULL;
    }
    
    if ( commands_user_ran_command ( ) == -1 ) {
        err_msg("%s:%d commands_user_ran_command error", __FILE__, __LINE__);
        return NULL;
    }

    return buf;   
}

char* tgdb_run_client_command ( enum tgdb_command c ) {
 	return tgdb_send ( tgdb_get_client_command (c), 2 );
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
int tgdb_dispatch_command ( struct command *com ) {
    int ret = 0;

    switch ( com->com_type ) {
        case BUFFER_GUI_COMMAND:
        case BUFFER_TGDB_COMMAND:
            if ( tgdb_can_issue_command() )
                ret = tgdb_deliver_command ( debugger_stdin, com );
            else
                queue_append ( gdb_input_queue, com );
            break;

        case BUFFER_USER_COMMAND:
            if ( tgdb_can_issue_command() )
                ret = tgdb_deliver_command ( debugger_stdin, com );
            else
                queue_append ( raw_input_queue, com );
            break;
        
        case BUFFER_READLINE_COMMAND:
            if ( tgdb_can_issue_command() ) {
                ret = tgdb_deliver_command ( debugger_stdin, com );
            } else
                queue_append ( rlc_input_queue, com );
            break;

        case BUFFER_OOB_COMMAND:
            if ( tgdb_can_issue_command() )
                ret = tgdb_deliver_command ( debugger_stdin, com );
            else
                queue_append ( oob_input_queue, com );
            break;

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
static int tgdb_deliver_command ( int fd, struct command *command ) {

    if ( command->com_type == BUFFER_READLINE_COMMAND ) {
        /* A readline command handled by tgdb-base */
        switch ( command->com_to_run ) {
            case COMMANDS_SET_PROMPT:
                if ( rlctx_change_prompt ( rl, command->data ) == -1 ) {
                    err_msg("%s:%d rlctx_change_prompt  error", __FILE__, __LINE__);
                    return -1;
                }
                break;
            case COMMANDS_REDISPLAY:
                if ( rlctx_redisplay ( rl ) == -1 ) {
                    err_msg("%s:%d rlctx_change_prompt  error", __FILE__, __LINE__);
                    return -1;
                }
                break;
            default:
                err_msg("(%s:%d) rlctx_send_char failed", __FILE__, __LINE__);
                return -1;
        }
    } else {
		IS_SUBSYSTEM_READY_FOR_NEXT_COMMAND = 0;

        /* A command for the debugger */
        if ( commands_prepare_for_command ( command ) == -1 ) {
//            fprintf ( stderr, "SKIPPING COMMAND(%s)", command->data );
            return -2;
        }
    
        /* A regular command from the client */
        io_debug_write_fmt ( "<%s>", command -> data );

        io_writen ( fd, command -> data, strlen ( command -> data ) );
    }

    return 0;
}

sig_atomic_t control_c = 0; /* If ^c was hit by user */

/* tgdb_run_buffered_command: Sends to gdb the next command.
 *
 * return:  0 on normal termination ( command was run )
 *          2 if the queue was cleared because of ^c
 */
int tgdb_run_command(void){
    /* TODO: Put signal blocking code here so that ^c is not pressed while 
     * checking for it */

    /* If a signal has been recieved, clear the queue and return */
    if(control_c) { 
        queue_free_list(gdb_input_queue, tgdb_interface_free_command);
        /* TODO: Setting control_c here stops crashing, but it doesn't solve
         * the problem. readline needs to know to reset the prompt. */
        control_c = 0;
        return 2;
    } 

tgdb_run_command_tag:

    /* This will redisplay the prompt when a command is run
     * through the gui with data on the console.
     */
    if ( queue_size(rlc_input_queue) > 0 ) {
        /* This runs commands through readline */
        struct command *item = NULL;
        item = queue_pop(rlc_input_queue);
        tgdb_deliver_command(debugger_stdin, item);
        tgdb_interface_free_command ( item );
        
    /* The out of band commands should always be run first */
    } else if ( queue_size(oob_input_queue) > 0 ) {
        /* These commands are always run. 
         * However, if an assumption is made that a misc
         * prompt can never be set while in this spot.
         */
        struct command *item = NULL;
        item = queue_pop(oob_input_queue);
        tgdb_deliver_command(debugger_stdin, item);
        tgdb_interface_free_command ( item );
    /* If the queue is not empty, run a command */
    } else if ( queue_size(gdb_input_queue) > 0 ) {
        struct command *item = NULL;
        item = queue_pop(gdb_input_queue);

        /* TODO: The comment and code below is in only one of 2 spots.
         * It also belongs at tgdb_setup_buffer_command_to_run.
         */

        /* If at the misc prompt, don't run the internal tgdb commands,
         * In fact throw them out for now, since they are only 
         * 'info breakpoints' */
        if ( globals_is_misc_prompt() == TRUE ) {
            if ( item->com_type != BUFFER_USER_COMMAND ) {
                tgdb_interface_free_command ( item );
                goto tgdb_run_command_tag;
            }
        }

        /* This happens when a command was skipped because the client no longer
         * needs the command to be run */
        if ( tgdb_deliver_command(debugger_stdin, item) == -2 )
            goto tgdb_run_command_tag;

        if ( tgdb_partially_run_command && 
             /* Don't redisplay the prompt for the redisplay prompt command :) */
             item->com_to_run != COMMANDS_REDISPLAY) {
            struct command *ncom;

            ncom = tgdb_interface_new_command ( 
                    NULL,
                    BUFFER_READLINE_COMMAND, 
                    COMMANDS_SHOW_USER_OUTPUT, 
                    COMMANDS_REDISPLAY, 
                    NULL );

            if ( tgdb_dispatch_command ( ncom ) == -1 ) {
                err_msg("%s:%d tgdb_dispatch_command error", __FILE__, __LINE__);
                return -1;
            }
        }
        
        tgdb_interface_free_command ( item );

    /* If the user has typed a command, send it through readline */
    } else if ( queue_size(raw_input_queue) > 0 ) { 
        struct command *item = queue_pop(raw_input_queue);
        char *data = item->data;
        int i, j = strlen ( data );

        for ( i = 0; i < j; i++ ) {
            if ( rlctx_send_char(rl, data[i]) == -1 ) {
                err_msg("(%s:%d) rlctx_send_char failed", __FILE__, __LINE__);
                return -1;
            }
        }

        free ( item->data ); 
    /* Send the partially typed command through readline */
    } else if ( current_command != NULL ) {
        int i,j = string_length(current_command);
        char *data = string_get(current_command);
        
        for ( i = 0; i < j; i++ ) {
            if ( rlctx_send_char(rl, data[i]) == -1 ) {
                err_msg("(%s:%d) rlctx_send_char failed", __FILE__, __LINE__);
                return -1;
            }
        }

        tgdb_partially_run_command = 1;
        current_command = NULL;
    }

    return 0;
}

/*******************************************************************************
 * This is the basic initialization
 ******************************************************************************/

/* tgdb_initialize:
 * ----------------
 *
 *  Gets the users home dir and creates the config directory.
 *
 *  config_dir - Should be PATH_MAX in size on way in.
 *               On way out, it will be the path to the config dir
 *
 *  Returns -1 on error, or 0 on success
 */
static int tgdb_initialize ( char *config_dir ) {
    
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

/* tgdb_command_callback:
 * ----------------------
 *
 *  This is called when readline says it has recieved a full line.
 */
static int tgdb_command_callback(const char *line) {
    static char command[MAXLINE];
    sprintf ( command, "%s\n", line );
    tgdb_send(command, 0);
    return 0;
}

int tgdb_change_prompt ( const char *prompt ) {
    struct command *ncom= tgdb_interface_new_command ( 
            prompt, 
            BUFFER_READLINE_COMMAND, 
            COMMANDS_HIDE_OUTPUT, 
            COMMANDS_SET_PROMPT, 
            NULL );

    if ( tgdb_dispatch_command ( ncom ) == -1 ) {
        err_msg("%s:%d tgdb_dispatch_command error", __FILE__, __LINE__);
        return -1;
    }

    return 0;
}

/* tgdb_complete_command:
 * ----------------------
 */
int tgdb_complete_command ( struct queue *list, const char *target ) {
    struct string *s;
//    struct queue *tlist = queue_init ();
//    int length;
//    int tlength = strlen ( target );
//    char *cur;


    if ( queue_size ( list ) == 0 )
        return 0;

    while ( queue_size ( list ) != 0 ) {
        s = queue_pop ( list );
        fprintf ( stderr, "%s\n", string_get ( s ) );
        string_free ( s );
    }

//    while ( queue_size ( list ) != 0 ) {
//        s = queue_pop ( list );
//        length = string_length ( s ); 
//        cur = string_get ( s );
//
//        fprintf ( stderr, "ALL(%s)\n", string_get ( s ) );
//
//        if ( length >= tlength && ( strncmp ( cur, target, tlength ) == 0 ))
//            queue_append ( tlist, s );
//        else
//            string_free ( s );
//    }
//
//    if ( queue_size ( tlist )
//
//    while ( queue_size ( tlist ) != 0 ) {
//        s = queue_pop ( tlist );
//        fprintf ( stderr, "LEFT(%s)\n", string_get ( s ) );
//    }

    return 0;
}

static int tgdb_completion_callback ( const char *line ) {
    struct command *ncom;

    /* Allow the client to generate a command for tab completion */
    a2_tgdb_completion_callback ( line ); 

    /* Redisplay the prompt */
    ncom = tgdb_interface_new_command ( 
            NULL,
            BUFFER_READLINE_COMMAND, 
            COMMANDS_SHOW_USER_OUTPUT, 
            COMMANDS_REDISPLAY, 
            NULL );

    if ( tgdb_dispatch_command ( ncom ) == -1 ) {
        err_msg("%s:%d tgdb_dispatch_command error", __FILE__, __LINE__);
        return -1;
    }
    
    return 0;
}

static int tgdb_init_readline ( char *config_dir, int *fd ) {
    /* Initialize readline */
    if ( (rl = rlctx_init((const char *)config_dir, "tgdb")) == NULL ) {
        err_msg("(%s:%d) rlctx_init failed", __FILE__, __LINE__);
        return -1;
    }

    /* Register callback for each command recieved at readline */
    if ( rlctx_register_command_callback(rl, &tgdb_command_callback) == -1 ) {
        err_msg("(%s:%d) rlctx_register_callback failed", __FILE__, __LINE__);
        return -1;
    }

    /* Register callback for tab completion */
    if ( rlctx_register_completion_callback(rl, &tgdb_completion_callback) == -1 ) {
        err_msg("(%s:%d) rlctx_register_callback failed", __FILE__, __LINE__);
        return -1;
    }

    /* Let the GUI check this for reading, 
     * if it finds data, it should call tgdb_recv_input */
    if ( (*fd = rlctx_get_fd(rl)) == -1 ) {
        err_msg("%s:%d rlctx_get_fd error", __FILE__, __LINE__);
        return -1;
    }

    return 0;
}

int tgdb_recv_input ( char *buf ) {
    int length, i;

    if ( rlctx_recv ( rl, buf, MAXLINE ) == -1 ) {
        err_msg("%s:%d rlctx_recv error", __FILE__, __LINE__);
        return -1;
    }

    length = strlen ( buf );

    for ( i = 0; i < length; i++) {
        if ( buf [i] == '\031' ) {
            if ( tgdb_has_command_to_run())
                tgdb_run_command();
        }
    }
    
    return 0;
}

int tgdb_rl_send ( char c ) {
    if ( rlctx_send_char ( rl, c ) == -1 ) {
        err_msg("(%s:%d) rlctx_send_char failed", __FILE__, __LINE__);
        return -1;
    }

    return 0;
}

int tgdb_send_input ( char c ) {
    /* The debugger is ready for input. Send it */
    if ( is_ready() ) {

		if ( c == '\n' )
			HAS_USER_SENT_COMMAND = 1;

        return tgdb_rl_send ( c );

    /* The debugger is not ready, save the input for later */
    } else {
		if ( current_command == NULL )
			current_command = string_init ( ) ;

        string_addchar ( current_command, c );

        if ( c == '\n' ) {
            queue_append ( raw_input_queue, current_command );
            current_command = NULL;
        }
    }

    return 0;
}

int tgdb_init (
            char *debugger, 
            int argc, char **argv, 
            int *gdb, int *child, int *readline) {

    int result;
    char config_dir[PATH_MAX];

    /* Create config directory */
    if ( tgdb_initialize ( config_dir ) == -1 ) {
        err_msg("%s:%d tgdb_initialize error", __FILE__, __LINE__);
        return -1; 
    }

    if ( tgdb_init_readline ( config_dir , readline ) == -1 ) {
        err_msg("%s:%d tgdb_init_readline error", __FILE__, __LINE__);
        return -1; 
    }

    /* Determine what debugger is valid:
     *
     * As of now, only the annotations protocol is valid */
    result = a2_find_valid_debugger ( 
                debugger, 
                argc, argv, 
                &debugger_stdin, gdb,
                config_dir );

    debugger_stdout = *gdb;

    /* Initialize the command queue's */

    /* initialize users buffer */
    gdb_input_queue = queue_init();

    /* initialize raw data typed by user */
    raw_input_queue = queue_init();

    /* initialize the out of band queue */
    oob_input_queue = queue_init();

    /* initialize the readline command queue */
    rlc_input_queue = queue_init();

    if ( util_new_tty(child, &inferior_slave_fd, inferior_tty_name) == -1){
        err_msg("%s:%d -> Could not open child tty", __FILE__, __LINE__);
        return -1;
    }
    
    inferior_fd     = *child;

    if ( result == 1 ) {
        a2_tgdb_init ( inferior_tty_name, &command_completion_callback );
        init_annotate_two();

        /*a2_setup_io( );*/
    } else {
        init_gdbmi ();
        printf ( "Error: gdb does not support annotations\n" );
        return -1;
    }

    return 0;
}

static int close_inferior_connection ( void ) {
    /* close inferior connection */
    xclose(inferior_fd);
    xclose(inferior_slave_fd);
    pty_release(inferior_tty_name);

    return 0;
}

int tgdb_shutdown ( void ) {
    /* free readline */
    rlctx_close(rl);

    close_inferior_connection();

    return tgdb_client_shutdown(); 
}


/* These functions are used to communicate with the inferior */
char *tgdb_tty_send(char c){
   static char buf[4];
   memset(buf, '\0', 4); 
   
   if(io_write_byte(inferior_fd, c) == -1){
      err_ret("%s:%d -> could not write byte", __FILE__, __LINE__);
      return NULL;
   }
   
   return buf;   
}

/* tgdb_tty_recv: returns to the caller data from the child */
size_t tgdb_tty_recv(char *buf, size_t n){
   char local_buf[n + 1];
   ssize_t size;

   /* read all the data possible from the child that is ready. */
   if( (size = io_read(inferior_fd, local_buf, n)) < 0){
      err_ret("%s:%d inferior_fd read failed", __FILE__, __LINE__);
      return -1;
   } 
   
   strncpy(buf, local_buf, size); 
   buf[size] = '\0';

   return size; 
}

int tgdb_new_tty(void) {
    close_inferior_connection();

    /* Ask for a new tty */
    if ( util_new_tty(&inferior_fd, &inferior_slave_fd, inferior_tty_name) == -1){
        err_msg("%s:%d -> Could not open child tty", __FILE__, __LINE__);
        return -1;
    }

    a2_set_inferior_tty ( inferior_tty_name );
    
    return 0;
}

char *tgdb_tty_name(void) {
    return inferior_tty_name;
}

char* tgdb_modify_breakpoint ( const char *file, int line, enum tgdb_breakpoint_action b ) {
	char *val = tgdb_client_modify_breakpoint_call ( file, line, b );

	if ( val == NULL )
		return NULL;

	if ( tgdb_send ( val, 2 ) == NULL )
		return NULL;

	free ( val );

	return "\n";
}
