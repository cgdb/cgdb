#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include <signal.h>

#include "tgdb.h"
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
#include "util.h"

static int tgdb_initialized = 0;

/* Does gdb need the nl mapping */
static int tgdb_need_mapping = -1;

static int tgdb_setup_buffer_command_to_run(char *com, 
               enum buffer_command_type com_type,      /* Who is running this command */
               enum buffer_output_type out_type,       /* Where should the output go */
               enum buffer_command_to_run com_to_run);   /* What command to run */

int masterfd = -1;                     /* master fd of the pseudo-terminal */
static char slavename[SLAVE_SIZE];     /* the name of the slave psuedo-termainl */

int master_tty_fd = -1, slave_tty_fd = -1;
static char child_tty_name[SLAVE_SIZE];  /* the name of the slave psuedo-termainl */

static pid_t debugger_pid;             /* pid of child process */

/*static struct buffer users_commands;   users commands buffered */
static struct node *head;
static sig_atomic_t control_c = 0;              /* If control_c was hit by user */


/* signal_catcher: Is called when a signal is sent to this process. 
 *    It passes the signal along to gdb. Thats what the user intended.
 */ 
static void signal_catcher(int SIGNAL){
   /* signal recieved */
   global_set_signal_recieved(TRUE);

   control_c = 1;

   if ( SIGNAL == SIGINT ) {
      if ( io_write_byte( masterfd, 3) == -1)
         err_msg("could not write byte: %c", (char) 3);

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

   return 0;
}

int tgdb_init(char *debugger, int argc, char **argv, int *gdb, int *child){
   tgdb_init_setup_config_file();
   io_debug_init(NULL);
   
   if ( (tgdb_need_mapping = tgdb_init_does_gdb_need_mapping(debugger)) == -1) {
      err_msg("(%s:%d) Couldn't detect mapping", __FILE__, __LINE__);
      return -1;
   }

   /* initialize users circular buffer */
   buffer_clear_string();
   head = NULL;

   if( (debugger_pid = tgdb_init_forkAndExecPty(debugger, argc, argv, slavename, &masterfd, tgdb_need_mapping)) == -1){
      err_msg("(%s:%d) Couldn't exec gdb through pty", __FILE__, __LINE__);
      return -1;
   }
   if ( tgdb_init_new_tty(&master_tty_fd, &slave_tty_fd, child_tty_name) == -1){
      err_msg("%s:%d -> Could not open child tty", __FILE__, __LINE__);
      return -1;
   }


   tgdb_setup_signals();
   
   tgdb_setup_buffer_command_to_run(child_tty_name , BUFFER_TGDB_COMMAND, COMMANDS_HIDE_OUTPUT, COMMANDS_TTY);
   tgdb_get_source_absolute_filename(NULL);

   *gdb     = masterfd;
   *child   = master_tty_fd;
   tgdb_initialized = 1;

   return 0;
}

int tgdb_shutdown(void){
   /* free up the psuedo-terminal members */
   pty_release(slavename);
   close(masterfd);

   /* tty for gdb child */
   close(master_tty_fd);
   close(slave_tty_fd);
   pty_release(child_tty_name);

   return 0;
}

static int tgdb_run_users_buffered_commands(void){
   static char buffered_cmd[2000+1];
   extern int DATA_AT_PROMPT;
   int buf_ret_val = 0, recv_sig = 0;
   enum buffer_command_type com_type = BUFFER_VOID;
   enum buffer_output_type out_type  = COMMANDS_SHOW_USER_OUTPUT;
   enum buffer_command_to_run com_to_run = COMMANDS_VOID;

   /* Check to see if a ^C has been entered since users last command was run */
   sigset_t newmask, oldmask, pendmask;
   sigemptyset(&newmask);
   sigaddset(&newmask, SIGINT);
   
   /* Block SIGINT and save current signal mask */
   if(sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0)
      err_msg("%s:%d -> SIG_BLOCK error", __FILE__, __LINE__);

   if(!control_c) { /* only check for data if signal has not been recieved */
      if ( head != NULL ) {
         if ( ( (struct command *) head->data)->data != NULL ) 
            strcpy(buffered_cmd, ((struct command *)head->data)->data);
         else
            buffered_cmd[0] = 0;

         com_type = ((struct command *) head->data)->com_type;
         out_type = ((struct command *) head->data)->out_type;
         com_to_run = ((struct command *) head->data)->com_to_run;

         head = buffer_remove_and_increment ( head, buffer_free_command );
         buf_ret_val = 1;
      } else {
         /* Maybe there was a part of a command left */
         if ( buffer_get_incomplete_command(buffered_cmd) != NULL ) {
            com_type = BUFFER_USER_COMMAND;
            out_type = COMMANDS_SHOW_USER_OUTPUT;
            com_to_run = COMMANDS_VOID;
            buf_ret_val = 1;
         } else
            buffered_cmd[0] = 0;
      }
   }

   if(sigpending(&pendmask) < 0)
      err_msg("%s:%d -> sigpending error", __FILE__, __LINE__);

   /* The SIGINT signal has been called since the last command was run,
    * Reset buffer and signal flag.
    */
   if(sigismember(&pendmask, SIGINT) || control_c){
      buffer_clear_string();
      control_c = 0;
      recv_sig = 1;
      head = NULL;
   }

   /* Reset signal mask which unblocks SIGINT */
   if(sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0)
      err_msg("%s:%d -> SIG_BLOCK error", __FILE__, __LINE__);

   /* Recieved a signal, data is not needed */
   if(recv_sig)
      return 0;

   /* A SIGINT has not been recieved yet, continue on as normal 
    * If a SIGINT is recieved between now and when the command is passed to gdb
    *    then the user will still have that command run.
    */
   if(buf_ret_val >= 0) {
      int length = strlen(buffered_cmd);
      if(length > 0 || buf_ret_val == 1) {
         DATA_AT_PROMPT = 0;
         
         switch(com_type){
            case BUFFER_GUI_COMMAND:
               /* Gui commands only care about where the output goes. */
               return commands_run_command(masterfd, buffered_cmd, out_type);
            case BUFFER_TGDB_COMMAND:
               /* tgdb is running a command */
               if(com_to_run == COMMANDS_INFO_SOURCES)
                  return commands_run_info_sources(masterfd);
               else if(com_to_run == COMMANDS_INFO_LIST){
                  buffered_cmd[--length] = '\0'; /* remove new line */
                  if ( buffered_cmd[0] == 0 )
                     return commands_run_list(NULL, masterfd);

                  return commands_run_list(buffered_cmd, masterfd);
               } else if(com_to_run == COMMANDS_INFO_BREAKPOINTS){
                  commands_run_info_breakpoints(masterfd);
               } else if(com_to_run == COMMANDS_TTY){
                  return commands_run_tty(buffered_cmd, masterfd);
               } else
                  err_msg("%s:%d -> could not run tgdb command(%s)", __FILE__, __LINE__, buffered_cmd);
               break;
            case BUFFER_USER_COMMAND:
               /* writing user command to file */
            
               /* running user command */
               if(io_writen(masterfd, buffered_cmd, length) != length)
                  err_msg("%s:%d -> could not write messge(%s)", __FILE__, __LINE__, buffered_cmd);
               break;
            default:
               err_msg("%s:%d -> could not run command(%s)", __FILE__, __LINE__, buffered_cmd);
               err_msg("%s:%d -> GOT(%d)", __FILE__, __LINE__, ((buffered_cmd[0])));
               break;
         } /* end switch */

         memset(buffered_cmd, '\0', 2000);
      }
   }

   return 0;
}

/* tgdb_can_issue_command:
 * This is used to see if gdb can currently issue a comand directly to gdb or
 * if it should put it in the buffer.
 *
 * Returns: TRUE if can issue directly to gdb. Otherwise FALSE.
 */
static int tgdb_can_issue_command(void) {
   if ( 
      tgdb_initialized && 
      /* The user is at the prompt */
      data_get_state() == USER_AT_PROMPT && 
      /* The user has not typed enter */
      global_did_user_press_enter() == 0 &&
      /* This line boiles down to:
       * If the buffered list is empty or the user is at the misc prompt 
       */
      ( (head == NULL && buffer_is_empty()== TRUE) || 
         global_can_issue_command() == FALSE))
      return TRUE;
   else 
      return FALSE;
}

/* tgdb_setup_buffer_command_to_run: This runs a command for the gui or for tgdb.
 *    It runs it right away if no other commands are being run or are in the
 *    queue to run. Otherwise, it puts it into the queue and it will run when 
 *    gdb is ready.
 *    
 *    com   -> command to run
 *    type  -> what to do with the output. (hide or show to user).
 *
 *    Returns: -1 on error, 0 on success.
 */
static int tgdb_setup_buffer_command_to_run(char *com, 
               enum buffer_command_type com_type,      /* Who is running this command */
               enum buffer_output_type out_type,       /* Where should the output go */
               enum buffer_command_to_run com_to_run){   /* What command to run */
   struct command *command;
    
   if(global_can_issue_command()  == TRUE && tgdb_can_issue_command()){
      /* set up commands to run from tgdb */
      if(com_type == BUFFER_TGDB_COMMAND && com_to_run == COMMANDS_INFO_SOURCES)
         return commands_run_info_sources(masterfd);
      else if(com_type == BUFFER_TGDB_COMMAND && com_to_run == COMMANDS_INFO_LIST)
         return commands_run_list(com, masterfd);
      else if(com_type == BUFFER_TGDB_COMMAND && com_to_run == COMMANDS_INFO_BREAKPOINTS)
         return commands_run_info_breakpoints(masterfd);
      else if(com_type == BUFFER_TGDB_COMMAND && com_to_run == COMMANDS_TTY)
         return commands_run_tty(com, masterfd);
      else /* running another command (from gui) */
         return commands_run_command(masterfd, com, out_type);
   }else{ /* writing the command for later execution */
      
      command = ( struct command * ) xmalloc ( sizeof (struct command) );
      if ( com != NULL ) {
         command->data = ( char * ) xmalloc ( sizeof (char *) * ( strlen ( com ) + 1 ));
         strcpy( command->data, com );
      } else 
         command->data = NULL;

      command->com_type   = com_type;
      command->out_type   = out_type;
      command->com_to_run = com_to_run;
      
      head = buffer_write_command_and_append( head, command );
      return 0;
   }

   return 0;
}

int tgdb_run_command(char *com){
   int val = 0;
   val = tgdb_setup_buffer_command_to_run(com, BUFFER_GUI_COMMAND, COMMANDS_SHOW_USER_OUTPUT, COMMANDS_VOID);

  /* tgdb always requests that breakpoints be checked because of
   * buggy gdb annotations */
  tgdb_setup_buffer_command_to_run ( NULL, BUFFER_TGDB_COMMAND, COMMANDS_HIDE_OUTPUT, COMMANDS_INFO_BREAKPOINTS );
 
  return val;
}

int tgdb_get_source_absolute_filename(char *file){
   return tgdb_setup_buffer_command_to_run(file, BUFFER_TGDB_COMMAND, COMMANDS_HIDE_OUTPUT, COMMANDS_INFO_LIST);
}

int tgdb_get_sources(void){
   return tgdb_setup_buffer_command_to_run(NULL , BUFFER_TGDB_COMMAND, COMMANDS_HIDE_OUTPUT, COMMANDS_INFO_SOURCES);
}

/* tgdb_recv: returns to the caller data and commands
 *
 */
size_t tgdb_recv(char *buf, size_t n, struct Command ***com){
   char local_buf[n + 1];
   ssize_t size, buf_size;
   extern int DATA_AT_PROMPT;

   /* init com to NULL */
   *com = NULL;

   /* set buf to null for debug reasons */
   memset(buf,'\0', n);

   /* 1. read all the data possible from gdb that is ready. */
   if( (size = io_read(masterfd, local_buf, n)) < 0){
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

   /* At this point local_buf has everything new from this read.
    * Basically this function is responsible for seperating the annotations
    * that gdb writes from the data. 
    */
   buf_size = tgdb_handle_data(local_buf, size, buf, n, com);

   /* 3. runs the users buffered command if any exists */
   if( global_can_issue_command() == TRUE && 
       data_get_state() == USER_AT_PROMPT && 
        DATA_AT_PROMPT &&  /* Only one command at a time */
        global_did_user_press_enter() == 0)
      tgdb_run_users_buffered_commands();

   tgdb_finish:

   if(tgdb_end_command(com) == -1)
      err_msg("%s:%d -> could not terminate commands", __FILE__, __LINE__);

   return buf_size;
}

/* Sends a character to gdb */
char *tgdb_send(char c){
   static char buf[4];
   memset(buf, '\0', 4); 
   
   if ( tgdb_can_issue_command()) {     
      global_user_typed_char(c);
      
      /* tgdb always requests that breakpoints be checked because of
       * buggy gdb annotations */
      if(c == '\n')
         tgdb_setup_buffer_command_to_run ( NULL, BUFFER_TGDB_COMMAND, COMMANDS_HIDE_OUTPUT, COMMANDS_INFO_BREAKPOINTS );
      
      if(io_write_byte(masterfd, c) == -1){
         err_ret("%s:%d -> could not write byte", __FILE__, __LINE__);
         return NULL;
      }
   } else {
      head = buffer_write_char ( head, c );

      if(c == '\n')
         tgdb_setup_buffer_command_to_run ( NULL, BUFFER_TGDB_COMMAND, COMMANDS_HIDE_OUTPUT, COMMANDS_INFO_BREAKPOINTS );
   }
   
   return buf;   
}

/* Sends a character to program being debugged by gdb */
char *tgdb_tty_send(char c){
   static char buf[4];
   memset(buf, '\0', 4); 
   
   if(io_write_byte(master_tty_fd, c) == -1){
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
   if( (size = io_read(master_tty_fd, local_buf, n)) < 0){
      err_ret("%s:%d -> could not read from master_tty_fd", __FILE__, __LINE__);
      return -1;
   } 
   strncpy(buf, local_buf, size); 
   buf[size] = '\0';

   return size; 
}

int tgdb_new_tty(void) {
   /* Free old child information */
   close(master_tty_fd);
   close(slave_tty_fd);
   pty_release(child_tty_name);

   /* Ask for a new tty */
   if ( tgdb_init_new_tty(&master_tty_fd, &slave_tty_fd, child_tty_name) == -1){
      err_msg("%s:%d -> Could not open child tty", __FILE__, __LINE__);
      return -1;
   }

   /* Send request to gdb */
   tgdb_setup_buffer_command_to_run(child_tty_name , BUFFER_TGDB_COMMAND, COMMANDS_HIDE_OUTPUT, COMMANDS_TTY);
   return 0;
}

char *tgdb_tty_name(void) {
    return child_tty_name;
}

char *tgdb_err_msg(void) {
   return err_get();
}
