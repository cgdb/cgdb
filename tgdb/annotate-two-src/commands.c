/* From configure */
#include "config.h"

/* Library includes */
#include <stdio.h>
#include <stdlib.h>

/* Local includes */
#include "commands.h"
#include "data.h"
#include "io.h"
#include "string.h"
#include "types.h"
#include "globals.h"
#include "error.h"

extern int masterfd;
static int commands = 0;

static enum COMMAND_STATE cur_command_state = VOID_COMMAND;
static int cur_field_num = 0;

/* breakpoint information */
static char *breakpoint_buffer = NULL;
static int breakpoint_buffer_block_size = 0;
static int breakpoint_buffer_pos = 0;
static int breakpoint_table = 0;
static int breakpoint_enabled = FALSE;
static int breakpoint_started = FALSE;

/* 'info source' information */
static int info_source_buf_pos = 0;
static char info_source_buf[MAXLINE];
static char last_info_source_requested[MAXLINE];

/* 'info sources' information */
static int sources_ready = 0;
static char *sources_buf;
static int sources_buf_block_size = 0;
static int sources_buf_pos = 0;

static char *sources[MAXLINE];
static int sources_pos = 0;

/* String that is output by gdb to get the absolute path to a file */
static char *source_prefix = "Located in ";
static int source_prefix_length = 11;

static int buf_add(char **buf, char c, int *pos, int *blocksize) {
    if(*pos == MAXLINE*(*blocksize)){
       *blocksize = *blocksize + 1;
        *buf = (char *)realloc(*buf, (*blocksize)*MAXLINE);
    }

    (*buf)[(*pos)++] = c;   
    return 0;
}

int commands_parse_field(const char *buf, size_t n, int *field){
   if(sscanf(buf, "field %d", field) != 1)
      err_msg("%s:%d -> parsing field annotation failed (%s)\n", __FILE__, __LINE__, buf);

   return 0;
}

/* source filename:line:character:middle:addr */
int commands_parse_source(const char *buf, size_t n, struct Command ***com){
   int i = 0;
   char copy[n];
   char *cur = copy + n;
   char file[MAXLINE], line[MAXLINE];
   strncpy(copy, buf, n); /* modify local copy */
   
   while(cur != copy && i <= 3){
      if(*cur == ':'){
         if(i == 3)
            if(sscanf(cur + 1, "%s", line) != 1)
               err_msg("%s:%d -> Could not get line number", __FILE__, __LINE__);

         *cur = '\0';
         ++i; 
     }
      --cur;
   } /* end while */
     
   if(sscanf(copy, "source %s", file) != 1)
      err_msg("%s:%d -> Could not get file name", __FILE__, __LINE__);
   
   if(tgdb_append_command(com, SOURCE_FILE_UPDATE, file, NULL, NULL) == -1)
      err_msg("%s:%d -> Could not send command", __FILE__, __LINE__);

   if(tgdb_append_command(com, LINE_NUMBER_UPDATE, line, NULL, NULL) == -1)
      err_msg("%s:%d -> Could not send command", __FILE__, __LINE__);

   return 0;
}

static void free_breakpoint(void){
   if(breakpoint_buffer != NULL){
      free(breakpoint_buffer);
      breakpoint_buffer_block_size += 0;
      breakpoint_buffer_pos = 0;
      breakpoint_buffer = NULL;
   }
}

static void parse_breakpoint(struct Command ***com){
   char copy[breakpoint_buffer_pos];
   char *cur = copy +breakpoint_buffer_pos, *fcur;
   char fname[MAXLINE + 2], file[MAXLINE], line[MAXLINE];
   memset(fname, '\0', MAXLINE + 2);
   memset(file, '\0', MAXLINE);
   memset(line, '\0', MAXLINE);

   strncpy(copy, breakpoint_buffer, breakpoint_buffer_pos); /* modify local copy */

   while(cur != copy){
      if((*cur) == ':'){
         if(sscanf(cur + 1, "%s", line) != 1)
            err_msg("%s:%d -> Could not get line number", __FILE__, __LINE__);

         *cur = '\0';
         break; /* in case of multiple ':' in the line */
      } 

      --cur;
   } /* end while */

   if(sscanf(copy, "in %s at ", fname) != 1) { /* regular breakpoint */
       if(sscanf(copy, "on %s at ", fname) != 1) /* Break on ada exception */
          err_msg("%s:%d -> Could not scan function and file name\n"
               "\tWas the program compiled with debug info?\n", __FILE__, __LINE__);
   }
   
   fcur = copy + strlen(fname) + 7;

   strncpy(file, fcur, strlen(fcur));

   if(breakpoint_enabled == TRUE)
      strcat(fname, " y");
   else
      strcat(fname, " n");

   if(tgdb_append_command(com, BREAKPOINT, fname, line, file) == -1)
      err_msg("%s:%d -> Could not send command", __FILE__, __LINE__);
}


void commands_set_state(enum COMMAND_STATE state, struct Command ***com){
   cur_command_state = state;

   switch(cur_command_state){
      case RECORD:  
         if(breakpoint_buffer_pos > 0){
            parse_breakpoint(com);
            free_breakpoint();
            breakpoint_enabled = FALSE;
         }
       break;
      case BREAKPOINT_TABLE_END:  
         if(breakpoint_buffer_pos > 0){
            parse_breakpoint(com);

            if(tgdb_append_command(com, BREAKPOINTS_END, NULL, NULL, NULL) == -1)
               err_msg("%s:%d -> Could not send command", __FILE__, __LINE__);

            free_breakpoint();
            breakpoint_enabled = FALSE;
         }

         if(breakpoint_started == FALSE){
            if(tgdb_append_command(com, BREAKPOINTS_BEGIN, NULL, NULL, NULL) == -1)
               err_msg("%s:%d -> Could not send command", __FILE__, __LINE__);
            
            if(tgdb_append_command(com, BREAKPOINTS_END, NULL, NULL, NULL) == -1)
               err_msg("%s:%d -> Could not send command", __FILE__, __LINE__);
         }

         breakpoint_started = FALSE;
       break;
      case BREAKPOINT_HEADERS: 
         breakpoint_table = 0; 
         break;
      case BREAKPOINT_TABLE_BEGIN: 
         if(tgdb_append_command(com, BREAKPOINTS_BEGIN, NULL, NULL, NULL) == -1)
            err_msg("%s:%d -> Could not send command", __FILE__, __LINE__);
         breakpoint_table = 1; 
         breakpoint_started = TRUE;
         break;
      case INFO_SOURCES:

         break;
      default: break;
   }
}

void commands_set_field_num(int field_num){
   cur_field_num = field_num;

   /* clear buffer and start over */
   if(breakpoint_table && cur_command_state == FIELD && cur_field_num == 5){
      free_breakpoint();
      breakpoint_buffer = (char *)calloc(MAXLINE, MAXLINE);
      breakpoint_buffer_block_size = 1;
   }
}

enum COMMAND_STATE commands_get_state(void){
   return cur_command_state;
}

/* If type is non-zero, server is prepended to com */
static int commands_run_com(int fd, char *com, int type){
   int length;

   if ( com == NULL )
      length = 0;
   else
      length = strlen(com);

   data_prepare_run_command();

   if ( type )
      io_writen(fd, "server ", 7);

   if ( length > 0 )
      io_writen(fd, com, length);
   io_writen(fd, "\n", 1);
   return 0;
}

/* commands_run_info_breakpoints: This runs the command 'info breakpoints' and prepares tgdb
 *                            by setting certain variables.
 * 
 *    fd -> The file descriptor to write the command to.
 */
static int commands_run_info_breakpoints( int fd ) {
   return commands_run_com(fd, "info breakpoints", 1);
}

static int commands_run_tty(char *tty, int fd){
   char line[MAXLINE];
   memset(line, '\0', MAXLINE);
   strcat(line, "tty ");
   strcat(line, tty);
   return commands_run_com(fd, line, 1);
}

/* commands_run_info_sources: This runs the command 'info sources' and prepares tgdb
 *                            by setting certain variables.
 * 
 *    fd -> The file descriptor to write the command to.
 *    Returns: -1 on error, 0 on sucess.
 */
static int commands_run_info_sources(int fd){
   sources_ready = 0;
   sources_buf_pos = 0; 

   if(sources_buf != NULL){
      free(sources_buf);
      sources_buf = NULL;
      sources_buf_block_size = 0;
      sources_buf_pos = 0;
   }

   commands_set_state(INFO_SOURCES, NULL);
   global_set_start_info_sources();
   return commands_run_com(fd, "info sources", 1);
}

/* commands_run_info_source:  This runs the command 'list filename:1' and then runs
 *                            'info source' to find out what the absolute path to 
 *                            filename is.
 * 
 *    filename -> The name of the file to check the absolute path of.
 *    fd -> The file descriptor to write the command to.
 */
static int commands_run_list(char *filename, int fd){
   char local_com[MAXLINE];

   memset(local_com, '\0', MAXLINE);
   strcat(local_com, "list ");

   if(filename != NULL){
      strcat(local_com, filename);
      strcat(local_com, ":1");
      memset(last_info_source_requested, '\0', MAXLINE);
      strcpy(last_info_source_requested, filename);
   } else {
      last_info_source_requested[0] = '\0';
   }
   
   commands_set_state(INFO_LIST, NULL);
   global_set_start_list();
   return commands_run_com(fd, local_com, 1);
}

static int commands_run_info_source(int fd){
   data_set_state(INTERNAL_COMMAND);
   info_source_buf_pos = 0;
   memset(info_source_buf, '\0', MAXLINE);
   
   commands_set_state(INFO_SOURCE, NULL);
   global_set_start_info_source();
   return commands_run_com(fd, "info source", 1);
}

int commands_run_command(int fd, struct command *com){
    /* Set up data to know the current state */
    switch(com->com_type) {
        case BUFFER_TGDB_COMMAND: data_set_state(INTERNAL_COMMAND);           break;
        case BUFFER_GUI_COMMAND:  data_set_state(GUI_COMMAND);                break;
        case BUFFER_USER_COMMAND: data_set_state(USER_COMMAND);               break;
        case BUFFER_VOID: err_msg("%s:%d switch error", __FILE__, __LINE__);  break;
        default: err_msg("%s:%d switch error", __FILE__, __LINE__);           break;
    }

    /* Run the current command */
    switch(com->com_to_run) {
        case COMMANDS_INFO_SOURCES: 
            return commands_run_info_sources(fd);                   break;
        case COMMANDS_INFO_LIST:
            return commands_run_list(com->data, fd);                break;
        case COMMANDS_INFO_SOURCE:                                  break;
        case COMMANDS_INFO_BREAKPOINTS:
            return commands_run_info_breakpoints(fd);               break;
        case COMMANDS_TTY:
            return commands_run_tty(com->data, fd);                 break;
        case COMMANDS_VOID:                                         break;
        default: err_msg("%s:%d switch error", __FILE__, __LINE__); break;
    }

    return commands_run_com(fd, com->data, 0);
}


void commands_list_command_finished(struct Command ***com, int success){
   /* The file was accepted, lets see if we can get the 
    * absolute path from gdb. It should be ok for tgdb to run this command
    * right away.  */
   if(success)
      commands_run_info_source(masterfd);
   else
      /* The file does not exist and it can not be opened.
       * So we return that information to the gui.  */
      tgdb_append_command(com, ABSOLUTE_SOURCE_DENIED, 
              last_info_source_requested, NULL, NULL);
}

void commands_send_source_absolute_source_file(struct Command ***com){
   /*err_msg("Whats up(%s:%d)\r\n", info_source_buf, info_source_buf_pos);*/

   /* found */
   if(info_source_buf_pos >= source_prefix_length && 
      (strncmp(info_source_buf, source_prefix, source_prefix_length) == 0)){
      char path[MAXLINE];
      memset(path, '\0', MAXLINE);
      strncpy(path, info_source_buf + source_prefix_length, 
              info_source_buf_pos - source_prefix_length);

      /* requesting file */
      if(last_info_source_requested[0] != '\0')
         tgdb_append_command(com, ABSOLUTE_SOURCE_ACCEPTED, path, NULL, NULL);
      else {
         tgdb_append_command(com, SOURCE_FILE_UPDATE, path, NULL, NULL);
         tgdb_append_command(com, LINE_NUMBER_UPDATE, "1", NULL, NULL);
      }
   /* not found */
   } else {
      tgdb_append_command(com, ABSOLUTE_SOURCE_DENIED, 
              last_info_source_requested, NULL, NULL);
   }
}

/* returns 1 if there are commands to run, otherwise 0 */
int commands_has_commnands_to_run(void){
   return (commands == 0)? 0: 1;
}

static void commands_process_source_line(struct Command  ***com){
   int i, length = strlen(sources_buf), start = 0;
   /*err_msg("LINE(%s:%d)\n", sources_buf, length);*/
  
   for(i = 0 ; i < length; ++i){
      if(i > 0 && sources_buf[i - 1] == ',' && sources_buf[i] == ' '){
         sources[sources_pos] = calloc(sizeof(char),i - start) ;
         strncpy(sources[sources_pos], sources_buf + start , i - start - 1);
         /*tgdb_append_command(com, SOURCE_FILE, buf, NULL, NULL);  */
         start += ((i + 1) - start); 
         /*err_msg("FOUND(%s)\n", sources[sources_pos]);*/
         sources_pos++;
      } else if (i == length - 1 ){
         sources[sources_pos] = calloc(sizeof(char),i - start + 2);
         strncpy(sources[sources_pos], sources_buf + start , i - start + 1);
         /*tgdb_append_command(com, SOURCE_FILE, buf, NULL, NULL);  */
         /*err_msg("FINAL FOUND(%s)\n", sources[sources_pos]);*/
         sources_pos++;
      }
   }
}

static void commands_process_info_source(char a){
   static char internal_info_source_buf[MAXLINE];
   static int  internal_info_source_buf_pos = 0;

   if ( a == '\r' )
      return;

   if(a == '\n'){ 
      internal_info_source_buf[internal_info_source_buf_pos] = '\0';

      /* This is the line */
      if ( strncmp(internal_info_source_buf, source_prefix, source_prefix_length) == 0 ) {
         strncpy(info_source_buf, internal_info_source_buf, internal_info_source_buf_pos);
         info_source_buf_pos = internal_info_source_buf_pos;
      }

      /* Delete the old line */
      internal_info_source_buf_pos = 0;
      internal_info_source_buf[0] = '\0';
      return;
   }

   internal_info_source_buf[internal_info_source_buf_pos++] = a;
}

/* process's source files */
static void commands_process_sources(char a, struct Command ***com){
   static const char *sourcesReadyString = "Source files for which symbols ";
   static const int sourcesReadyStringLength = 31;
   buf_add(&sources_buf, a, &sources_buf_pos, &sources_buf_block_size);
   
   if(a == '\n'){
      sources_buf[--sources_buf_pos] = '\0'; /* remove '\r' and null terminate */
      sources_buf[--sources_buf_pos] = '\0'; /* remove '\n' and null terminate */
      /* valid lines are 
       * 1. after the first line,
       * 2. do not end in ':' 
       * 3. and are not empty 
       */

      /*fprintf(stderr, "LINE(%s)BLOCK(%d)BUF_POS(%d)", sources_buf, sources_buf_block_size, sources_buf_pos);*/

      if ( strncmp(sources_buf, sourcesReadyString, sourcesReadyStringLength) == 0 )
         sources_ready = 1;

       /* is this a valid line */
       if(sources_buf_pos > 0 && sources_ready && sources_buf[sources_buf_pos - 1] != ':')
          commands_process_source_line(com); 
      
      sources_buf_pos = 0;
      memset(sources_buf, '\0', sources_buf_block_size*MAX_LINE);
   }
}

void commands_send_gui_sources(struct Command ***com){
   sources_pos--;
   if(sources_pos >= 1){
      tgdb_append_command(com, SOURCES_START, NULL, NULL, NULL);  

      while(sources_pos >= 0){
         tgdb_append_command(com, SOURCE_FILE, sources[sources_pos], NULL, NULL);  
         --sources_pos;   
      }

      tgdb_append_command(com, SOURCES_END, NULL, NULL, NULL);  
   }
}

void commands_process(char a, struct Command ***com){
   if(commands_get_state() == INFO_SOURCES){
      commands_process_sources(a, com);     
   } else if(commands_get_state() == INFO_LIST){
      /* do nothing with data */
   } else if(commands_get_state() == INFO_SOURCE){
      commands_process_info_source(a);   
   } else if(breakpoint_table && cur_command_state == FIELD && cur_field_num == 5){ /* the file name and line num */ 
      if(breakpoint_buffer_pos == MAXLINE*breakpoint_buffer_block_size){
         breakpoint_buffer = (char *)realloc(breakpoint_buffer, ++breakpoint_buffer_block_size*MAXLINE);
         memset(breakpoint_buffer + MAX_LINE, '\0', (breakpoint_buffer_block_size - 1)*MAXLINE);
      }

      breakpoint_buffer[breakpoint_buffer_pos++] = a;   
   } else if(breakpoint_table && cur_command_state == FIELD && cur_field_num == 3 && a == 'y')
         breakpoint_enabled = TRUE;
}
