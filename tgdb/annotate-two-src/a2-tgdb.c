#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include <signal.h>

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

static int tgdb_initialized = 0;

int masterfd = -1;                     /* master fd of the pseudo-terminal */
int gdb_stdout = -1;
static int readline_fd[2] = { -1, -1 };
static char slavename[SLAVE_SIZE];     /* the name of the slave psuedo-termainl */

int master_tty_fd = -1, slave_tty_fd = -1;
static char child_tty_name[SLAVE_SIZE];  /* the name of the slave psuedo-termainl */

static pid_t debugger_pid;             /* pid of child process */

/*static struct buffer users_commands;   users commands buffered */
static struct queue *gdb_input_queue;
static sig_atomic_t control_c = 0;              /* If control_c was hit by user */

/* The current user command */
static char user_cur_command[MAXLINE];
static int user_cur_command_pos = 0;

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
   if(global_can_issue_command() == TRUE && tgdb_can_issue_command()) {
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
   
   /* initialize users circular buffer */
   gdb_input_queue = queue_init();

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

   /* Set up the readline pipe */
   if ( pipe(readline_fd) == -1 ) {
      err_msg("(%s:%d) pipe failed", __FILE__, __LINE__);
      return -1;
   }

   /* Let the GUI check this for reading, 
    * if it finds data, it should call tgdb_recv_input */
   *readline    = readline_fd[0];
   
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
    extern int DATA_AT_PROMPT;
    struct command *item = NULL;

    /* TODO: Put signal blocking code here so that ^c is not pressed while 
     * checking for it */

    /* If a signal has been recieved, clear the queue and return */
    if(control_c) { 
        queue_free_list(gdb_input_queue, buffer_free_item);
        return 2;
    } 
    
    /* If the queue is empty, return */
    if ( queue_size(gdb_input_queue) == 0 )
       return 2;
   
    DATA_AT_PROMPT = 0;
    item = queue_pop(gdb_input_queue);
    commands_run_command(masterfd, item);
    buffer_free_item(item);
    return 0;
}

/* tgdb_can_issue_command:
 * This is used to see if gdb can currently issue a comand directly to gdb or
 * if it should put it in the buffer.
 *
 * Returns: TRUE if can issue directly to gdb. Otherwise FALSE.
 */
static int tgdb_can_issue_command(void) {
   if ( tgdb_initialized && 
      /* The user is at the prompt */
      data_get_state() == USER_AT_PROMPT && 
      /* This line boiles down to:
       * If the buffered list is empty or the user is at the misc prompt 
       */
      ( queue_size(gdb_input_queue) == 0 || global_can_issue_command() == FALSE))
      return TRUE;
   else 
      return FALSE;
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
size_t a2_tgdb_recv(char *buf, size_t n, struct Command ***com){
   char local_buf[10*n];
   ssize_t size, buf_size;
   extern int DATA_AT_PROMPT;

   /* init com to NULL */
   *com = NULL;

   /* set buf to null for debug reasons */
   memset(buf,'\0', n);

   /* 1. read all the data possible from gdb that is ready. */
   if( (size = io_read(gdb_stdout, local_buf, n)) < 0){
      err_ret("%s:%d -> could not read from masterfd", __FILE__, __LINE__);
      tgdb_append_command(com, QUIT, NULL, NULL, NULL);
      return -1;
   } else if ( size == 0 ) {/* EOF */ 
      buf_size = 0;
      
      if(tgdb_append_command(com, QUIT, NULL, NULL, NULL) == -1)
         err_msg("%s:%d -> Could not send command", __FILE__, __LINE__);
      
      goto tgdb_finish;
   }

   local_buf[size] = '\0';

   /* 2. This is a hack, copies the buffer back into local buffer, 
    * then it translates all '\n' into '\r\n'
    */ 
   {
        char b[n + n], *c;
        int i = 0;
        strcpy(b, local_buf); /* Copy local_buf into the buffer */
        c = b;
        while(*c != '\0') {
            if ( *c == '\n' ) {
                local_buf[i++] = '\r'; 
                local_buf[i++] = '\n'; 
                size++;
            } else
                local_buf[i++] = *c;

            c++;
        }
        local_buf[i++] = '\0';
   }

   /* 3. At this point local_buf has everything new from this read.
    * Basically this function is responsible for seperating the annotations
    * that gdb writes from the data. 
    *
    * buf and buf_size are the data to be returned from the user.
    */
   buf_size = a2_handle_data(local_buf, size, buf, n, com);

   /* 4. runs the users buffered command if any exists */
   if( global_can_issue_command() == TRUE && 
       data_get_state() == USER_AT_PROMPT && 
       DATA_AT_PROMPT) { /* Only one command at a time */
       int result = tgdb_run_command();

        /* There are no more command to run,
         * Display the data the user's unfinished command if
         * the current command is done outputting */
       if ( result == 2 && user_cur_command_pos > 0 ) {
           char temp[MAXLINE * 5];
           user_cur_command[user_cur_command_pos] = '\0';
           temp[0] = '\0';

           if ( buf_size > 0 )
               strcat(temp, buf);

              strcat(temp, user_cur_command);

              /* copy back */
              strcpy(buf, temp);
              buf_size += (user_cur_command_pos + 1);
              user_cur_command_pos = 0;
              user_cur_command[0] = '\0';
       }
   }

   tgdb_finish:

   if(tgdb_end_command(com) == -1)
      err_msg("%s:%d -> could not terminate commands", __FILE__, __LINE__);

   return buf_size;
}

/* Sends the user typed line to gdb */
char *a2_tgdb_send(char *command, int out_type) {
   static char buf[MAXLINE];
   
   /* tgdb always requests breakpoints because of buggy gdb annotations */
   tgdb_setup_buffer_command_to_run ( command, BUFFER_USER_COMMAND, COMMANDS_SHOW_USER_OUTPUT, COMMANDS_VOID );
   tgdb_setup_buffer_command_to_run ( NULL, BUFFER_TGDB_COMMAND, COMMANDS_HIDE_OUTPUT, COMMANDS_INFO_BREAKPOINTS );
   return buf;   
}

int a2_tgdb_send_input(char c) {
    /* If tgdb is in a state to issue a command, then this data can be
     * returned right away */
    if( tgdb_can_issue_command() == TRUE ) {
       if(io_write_byte(readline_fd[1], c) == -1){
          err_ret("%s:%d -> io_write_byte error", __FILE__, __LINE__);
          return -1;
       }
    } else {
        /* At this point, tgdb is busy, and has more output to display before
         * this data, lets save the command. to be displayed.
         *
         * Start buffering the data to show it later, either
         * 1. Will show a unfinished chunk of data
         *      This happens when the user types data, but not a full command, 
         *      and then gdb finishes what it is doing. This unfinished
         *      command must be returned.
         *
         * 2. Will display the completed command before data is processed.
         *      This happens when the user typed a whole command ( and '\n' )
         *      while gdb was busy.
         */
         user_cur_command[user_cur_command_pos++] = c;
         
         if ( c == '\n' || c == '\r') {
             user_cur_command_pos = 0;
             user_cur_command[0] = '\0';

         }
    }

    return 0;
}

int a2_tgdb_recv_input(char *buf) {
    /* This should return everything in it, the first try */
    ssize_t size;
    
    if ( ( size = io_read(readline_fd[0], buf, MAXLINE)) < 0 ) {
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
   tgdb_setup_buffer_command_to_run(child_tty_name , BUFFER_TGDB_COMMAND, COMMANDS_HIDE_OUTPUT, COMMANDS_TTY);
   return 0;
}

char *a2_tgdb_tty_name(void) {
    return child_tty_name;
}

char *a2_tgdb_err_msg(void) {
   return err_get();
}

char *a2_tgdb_get_prompt(void) {
    data_get_prompt();
}
