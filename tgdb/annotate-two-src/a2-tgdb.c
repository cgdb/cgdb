#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_STDLIB_H 
#include <stdlib.h>
#endif  /* HAVE_STDLIB_H */

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#if HAVE_STDIO_H
#include <stdio.h> 
#endif /* HAVE_STDIO_H */

#if HAVE_UNISTD_H
#include <unistd.h>
#endif  /* HAVE_UNISTD_H */

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#if HAVE_LIMITS_H
#include <limits.h>
#endif /* HAVE_LIMITS_H */

#if HAVE_SIGNAL_H
#include <signal.h>
#endif /* HAVE_SIGNAL_H */

#if HAVE_ERRNO_H
#include <errno.h>
#endif /* HAVE_ERRNO_H */

#include "a2-tgdb.h"
#include "tgdb_util.h"
#include "pseudo.h"
#include "error.h"
#include "io.h"
#include "state_machine.h"
#include "data.h"
#include "commands.h"
#include "globals.h"
#include "tgdb_init.h"
#include "types.h"
#include "buffer.h"
#include "queue.h"
#include "util.h"
#include "rlctx.h"

/* This is set when tgdb has initialized itself */
static int tgdb_initialized = 0;


int masterfd = -1;                     /* master fd of the pseudo-terminal */
int gdb_stdout = -1; 
static char slavename[SLAVE_SIZE];     /* the name of the slave psuedo-termainl */

int master_tty_fd = -1, slave_tty_fd = -1;
static char child_tty_name[SLAVE_SIZE];  /* the name of the slave psuedo-termainl */

static pid_t debugger_pid;             /* pid of child process */

/* gdb_input_queue: The commands that need to be run through gdb. 
 * Examples are 'b main', 'run', ...
 * All commands in this queue will be run in order.
 */
static struct queue *gdb_input_queue;

/* raw_input_queue: This is the data that the user typed to form a command.
 * This data must be sent through readline to get the actual command.
 * The data will be passed through readline and then readline will send to 
 * tgdb a command to run.
 */
static struct queue *raw_input_queue;

static sig_atomic_t control_c = 0;              /* If control_c was hit by user */

/* Interface to readline capability */
static struct rlctx *rl;

/* This is the current command */
static struct string *current_command = NULL;

/* a2_tgdb_command_callback: This is called when readline determines a command
 *                           has been typed. 
 *
 *      line - The command the user typed without the '\n'.
 */
static int a2_tgdb_command_callback(const char *line) {
    a2_tgdb_send((char *)line, 0);
}

/* tgdb_is_debugger_ready: Determines if a command can be sent directly to gdb.
 * 
 * Returns: FALSE if gdb is busy running a command or a command can not be run.
 *          TRUE  if gdb can currently recieve a command.
 */
static int tgdb_is_debugger_ready(void) {
    if ( !tgdb_initialized )
        return FALSE;

    /* If the user is at the prompt and the raw queue is empty */
    if ( data_get_state() == USER_AT_PROMPT )
        return TRUE;

    return FALSE;
}

/* tgdb_can_issue_command: Determines if tgdb should send data to gdb or put
 *                         it in a buffer. This is when the debugger is ready
 *                         and there are no commands to run.
 *
 * Returns: TRUE if can issue directly to gdb. Otherwise FALSE.
 */
static int tgdb_can_issue_command(void) {
    if ( tgdb_is_debugger_ready() && queue_size(gdb_input_queue) == 0 )
        return TRUE;

    return FALSE;
}

/* tgdb_can_run_command: Determines if tgdb has commands it needs to run.
 *
 * Returns: TRUE if can issue directly to gdb. Otherwise FALSE.
 */
static int tgdb_has_command_to_run(void) {
    if ( tgdb_is_debugger_ready() && (
           (queue_size(gdb_input_queue) > 0) || 
           (queue_size(raw_input_queue) > 0) || 
           current_command != NULL )
       )
        return TRUE;

    return FALSE;
}

/* tgdb_setup_buffer_command_to_run: This sets up a command to run.
 *    It runs it right away if no other commands are being run. 
 *    Otherwise, it puts it in the queue and will run it in turn.
 *    
 *    com         - command to run
 *    Returns: -1 on error, 0 on success.
 */
static int tgdb_setup_buffer_command_to_run(
      char *com, 
      enum buffer_command_type com_type,        /* Who is running this command */
      enum buffer_output_type out_type,         /* Where should the output go */
      enum buffer_command_to_run com_to_run){   /* What command to run */

    struct command *item = buffer_new_item(com, com_type, out_type, com_to_run);
   
    /*fprintf(stderr, "SIZE_OF_BUFFER(%d)\n", buffer_size(head));*/
    if(tgdb_can_issue_command()) {
        commands_run_command(masterfd, item);
        buffer_free_item(item);
    } else /* writing the command for later execution */
        queue_append( gdb_input_queue, item );

    return 0;
}

/* signal_catcher: Is called when a signal is sent to this process. 
 *    It passes the signal along to gdb. Thats what the user intended.
 */ 
static void signal_catcher(int SIGNAL){
    /* signal recieved */
    global_set_signal_recieved(TRUE);

    if ( SIGNAL == SIGINT ) {               /* ^c */
        control_c = 1;
        kill(debugger_pid, SIGINT);
    } else if ( SIGNAL == SIGTERM ) { 
        kill(debugger_pid, SIGTERM);
    } else if ( SIGNAL == SIGQUIT ) {       /* ^\ */
        kill(debugger_pid, SIGQUIT);
    } else 
        err_msg("caught unknown signal: %d", debugger_pid);
}

/* tgdb_setup_signals: Sets up signal handling for the tgdb library.
 *    As of know, only SIGINT is caught and given a signal handler function.
 *    Return: returns 0 on success, or -1 on error.
 */
static int tgdb_setup_signals(void){
   struct sigaction action;

   action.sa_handler = signal_catcher;      
   sigemptyset(&action.sa_mask);   
   action.sa_flags = 0;

   if(sigaction(SIGINT, &action, NULL) < 0)
      err_ret("%s:%d -> sigaction failed ", __FILE__, __LINE__);

   if(sigaction(SIGTERM, &action, NULL) < 0)
      err_ret("%s:%d -> sigaction failed ", __FILE__, __LINE__);

   if(sigaction(SIGQUIT, &action, NULL) < 0)
      err_ret("%s:%d -> sigaction failed ", __FILE__, __LINE__);

   return 0;
}

int a2_tgdb_init(char *debugger, int argc, char **argv, int *gdb, int *child, int *readline){
   char *config_file;

   tgdb_init_setup_config_file();
   config_file = tgdb_util_get_config_gdb_debug_file();
   io_debug_init(config_file);
   commands_init();
   
   /* initialize users buffer */
   gdb_input_queue = queue_init();

   /* initialize raw data typed by user */
   raw_input_queue = queue_init();

   /* Initialize readline */
   if ( (rl = rlctx_init()) == NULL ) {
      err_msg("(%s:%d) rlctx_init failed", __FILE__, __LINE__);
      return -1;
   }

   /* Register callback for each command recieved at readline */
   if ( rlctx_register_callback(rl, &a2_tgdb_command_callback) == -1 ) {
      err_msg("(%s:%d) rlctx_register_callback failed", __FILE__, __LINE__);
      return -1;
   }

   if(( debugger_pid = invoke_debugger(debugger, argc, argv, &masterfd, &gdb_stdout, 0)) == -1 ) {
      err_msg("(%s:%d) invoke_debugger failed", __FILE__, __LINE__);
      return -1;
   }

   if ( tgdb_util_new_tty(&master_tty_fd, &slave_tty_fd, child_tty_name) == -1){
      err_msg("%s:%d -> Could not open child tty", __FILE__, __LINE__);
      return -1;
   }

   tgdb_setup_signals();
   
   tgdb_setup_buffer_command_to_run(child_tty_name , BUFFER_TGDB_COMMAND, COMMANDS_HIDE_OUTPUT, COMMANDS_TTY);
   a2_tgdb_get_source_absolute_filename(NULL);

   *gdb     = gdb_stdout;
   *child   = master_tty_fd;

   /* Let the GUI check this for reading, 
    * if it finds data, it should call tgdb_recv_input */
   if ( (*readline = rlctx_get_fd(rl)) == -1 ) {
      err_msg("%s:%d rlctx_get_fd error", __FILE__, __LINE__);
      return -1;
   }

   tgdb_initialized = 1;

   return 0;
}

int a2_tgdb_shutdown(void){
   /* free up the psuedo-terminal members */
   pty_release(slavename);
   close(masterfd);

   /* tty for gdb child */
   close(master_tty_fd);
   close(slave_tty_fd);
   pty_release(child_tty_name);

   return 0;
}

/* tgdb_run_buffered_command: Sends to gdb the next command.
 *
 * return:  0 on normal termination ( command was run )
 *          2 if the queue was cleared because of ^c
 */
static int tgdb_run_command(void){
    /* TODO: Put signal blocking code here so that ^c is not pressed while 
     * checking for it */

    /* If a signal has been recieved, clear the queue and return */
    if(control_c) { 
        queue_free_list(gdb_input_queue, buffer_free_item);
        /* TODO: Setting control_c here stops crashing, but it doesn't solve
         * the problem. readline needs to know to reset the prompt. */
        control_c = 0;
        return 2;
    } 

tgdb_run_command_tag:
    
    /* If the queue is not empty, run a command */
    if ( queue_size(gdb_input_queue) > 0 ) {
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
                buffer_free_item(item);
                goto tgdb_run_command_tag;
            }
        }

        commands_run_command(masterfd, item);
        buffer_free_item(item);
    
    /* If the user has typed a command, send it through readline */
    } else if ( queue_size(raw_input_queue) > 0 ) { 
        struct string *item = queue_pop(raw_input_queue);
        char *data = string_get(item);
        int i, j = string_length(item);

        for ( i = 0; i < j; i++ ) {
            if ( rlctx_send_char(rl, data[i]) == -1 ) {
                err_msg("(%s:%d) rlctx_send_char failed", __FILE__, __LINE__);
                return -1;
            }
        }

        string_free(item);
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

        current_command = NULL;
    }

    return 0;
}

int a2_tgdb_get_source_absolute_filename(char *file){
   return tgdb_setup_buffer_command_to_run(file, BUFFER_TGDB_COMMAND, COMMANDS_HIDE_OUTPUT, COMMANDS_INFO_LIST);
}

int a2_tgdb_get_sources(void){
   return tgdb_setup_buffer_command_to_run(NULL , BUFFER_TGDB_COMMAND, COMMANDS_HIDE_OUTPUT, COMMANDS_INFO_SOURCES);
}

/* tgdb_recv: returns to the caller data and commands
 *
 */
size_t a2_tgdb_recv(char *buf, size_t n, struct queue *q){
    char local_buf[10*n];
    ssize_t size, buf_size;

    /* make the queue empty */
    tgdb_delete_commands(q);

    /* set buf to null for debug reasons */
    memset(buf,'\0', n);

    /* 1. read all the data possible from gdb that is ready. */
    if( (size = io_read(gdb_stdout, local_buf, n)) < 0){
        err_ret("%s:%d -> could not read from masterfd", __FILE__, __LINE__);
        tgdb_append_command(q, QUIT, NULL, NULL, NULL);
        return -1;
    } else if ( size == 0 ) {/* EOF */ 
        buf_size = 0;
      
        if(tgdb_append_command(q, QUIT, NULL, NULL, NULL) == -1)
            err_msg("%s:%d -> Could not send command", __FILE__, __LINE__);
      
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

/* Sends the user typed line to gdb */
char *a2_tgdb_send(char *command, int out_type) {
   static char buf[MAXLINE];

   /* The gui's commands do not get returned. So a newline needs to be 
    * returned so that its output is correct.
    */
   if ( out_type == 2 ) {
        buf[0] = '\n';
        buf[1] = '\0';
   } else
       buf[0] = '\0';

   /* tgdb always requests breakpoints because of buggy gdb annotations */
   tgdb_setup_buffer_command_to_run ( command, BUFFER_USER_COMMAND, COMMANDS_SHOW_USER_OUTPUT, COMMANDS_VOID );
   tgdb_setup_buffer_command_to_run ( NULL, BUFFER_TGDB_COMMAND, COMMANDS_HIDE_OUTPUT, COMMANDS_INFO_BREAKPOINTS );
   return buf;   
}

int a2_tgdb_send_input(char c) {
    extern int COMMAND_TYPED_AT_PROMPT;
    if ( current_command == NULL )
        current_command = string_init();

    /* tgdb is not busy, send the data to readline write away */
    if( tgdb_can_issue_command() == TRUE && !COMMAND_TYPED_AT_PROMPT) {
        /* A command has been recieved for this prompt, don't allow another */
        if ( c == '\n' )
            COMMAND_TYPED_AT_PROMPT = 1;

        if ( rlctx_send_char(rl, c) == -1 ) {
            err_ret("%s:%d rlctx_send_char error", __FILE__, __LINE__);
            return -1;
        }
    } else {
    /* tgdb is busy, save the data in a queue to run through readline later */

        /* Add to the current command */
        string_addchar(current_command, c);

        /* Another command is completed, save it in the queue */
        if ( c == '\n' ) {
            queue_append(raw_input_queue, current_command);
            current_command = NULL;
        } 
    }

    return 0;
}

int a2_tgdb_recv_input(char *buf) {
    /* This should return everything in it, the first try */
    ssize_t size;

    if ( rlctx_recv(rl, buf, MAXLINE) == -1 ) {
        err_ret("%s:%d -> io_read error", __FILE__, __LINE__);
        return -1;
    } 

    return 0;
}

/* Sends a character to program being debugged by gdb */
char *a2_tgdb_tty_send(char c){
   static char buf[4];
   memset(buf, '\0', 4); 
   
   if(io_write_byte(master_tty_fd, c) == -1){
      err_ret("%s:%d -> could not write byte", __FILE__, __LINE__);
      return NULL;
   }
   
   return buf;   
}

/* tgdb_tty_recv: returns to the caller data from the child */
size_t a2_tgdb_tty_recv(char *buf, size_t n){
   char local_buf[n + 1];
   ssize_t size;

   /* read all the data possible from the child that is ready. */
   if( (size = io_read(master_tty_fd, local_buf, n)) < 0){
      err_ret("%s:%d -> could not read from master_tty_fd", __FILE__, __LINE__);
      return -1;
   } 
   strncpy(buf, local_buf, size); 
   buf[size] = '\0';

   return size; 
}

int a2_tgdb_new_tty(void) {
    /* Free old child information */
    close(master_tty_fd);
    close(slave_tty_fd);
    pty_release(child_tty_name);

    /* Ask for a new tty */
    if ( tgdb_util_new_tty(&master_tty_fd, &slave_tty_fd, child_tty_name) == -1){
        err_msg("%s:%d -> Could not open child tty", __FILE__, __LINE__);
        return -1;
    }

    /* Send request to gdb */
    tgdb_setup_buffer_command_to_run(
        child_tty_name, 
        BUFFER_TGDB_COMMAND, 
        COMMANDS_HIDE_OUTPUT, 
        COMMANDS_TTY);

   return 0;
}

char *a2_tgdb_tty_name(void) {
    return child_tty_name;
}

char *a2_tgdb_err_msg(void) {
   return err_get();
}
