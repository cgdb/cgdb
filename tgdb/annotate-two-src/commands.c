#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_STDLIB_H 
#include <stdlib.h>
#endif  /* HAVE_STDLIB_H */

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

/* Local includes */
#include "commands.h"
#include "data.h"
#include "io.h"
#include "types.h"
#include "globals.h"
#include "error.h"
#include "util.h"
#include "queue.h"

extern int masterfd;
static int commands = 0;

static enum COMMAND_STATE cur_command_state = VOID_COMMAND;
static int cur_field_num = 0;

/* breakpoint information */
struct string *breakpoint_string;
static int breakpoint_table = 0;
static int breakpoint_enabled = FALSE;
static int breakpoint_started = FALSE;

/* 'info source' information */
static struct string *info_source_string;
static int info_source_ready = 0;
static char last_info_source_requested[MAXLINE];

/* 'info sources' information */
static int sources_ready = 0;
static struct string *info_sources_string;
static struct queue *source_files; /* The queue of current files */

/* String that is output by gdb to get the absolute path to a file */
static char *source_prefix = "Located in ";
static int source_prefix_length = 11;

/* String that is output by gdb to get the relative path to a file */
static char *source_relative_prefix = "Current source file is ";
static int source_relative_prefix_length = 23;


extern struct queue *oob_input_queue;
extern struct rlctx *rl;

void commands_init(void) {
    info_source_string  = string_init();
    info_sources_string = string_init();
    breakpoint_string   = string_init();
    source_files        = queue_init();
}

int commands_parse_field(const char *buf, size_t n, int *field){
   if(sscanf(buf, "field %d", field) != 1)
      err_msg("%s:%d -> parsing field annotation failed (%s)\n", __FILE__, __LINE__, buf);

   return 0;
}

/* source filename:line:character:middle:addr */
int commands_parse_source(const char *buf, size_t n, struct queue *q){
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
   
   if(tgdb_append_command(q, SOURCE_FILE_UPDATE, file, NULL, NULL) == -1)
      err_msg("%s:%d -> Could not send command", __FILE__, __LINE__);

   if(tgdb_append_command(q, LINE_NUMBER_UPDATE, line, NULL, NULL) == -1)
      err_msg("%s:%d -> Could not send command", __FILE__, __LINE__);

   tgdb_setup_buffer_command_to_run(
            oob_input_queue,
            NULL , 
            BUFFER_TGDB_COMMAND, 
            COMMANDS_HIDE_OUTPUT, 
            COMMANDS_INFO_SOURCE_RELATIVE);

   return 0;
}

static void parse_breakpoint(struct queue *q){
    unsigned long size = string_length(breakpoint_string);
    char copy[size + 1];
    char *cur = copy + size, *fcur;
    char fname[MAXLINE + 2], file[MAXLINE], line[MAXLINE];
    static char *info_ptr; 

    memset(fname, '\0', MAXLINE + 2);
    memset(file, '\0', MAXLINE);
    memset(line, '\0', MAXLINE);

    info_ptr = string_get(breakpoint_string);

    strncpy(copy, info_ptr, size + 1); /* modify local copy */

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

    if(tgdb_append_command(q, BREAKPOINT, fname, line, file) == -1)
        err_msg("%s:%d -> Could not send command", __FILE__, __LINE__);
}


void commands_set_state(enum COMMAND_STATE state, struct queue *q){
    cur_command_state = state;

    switch(cur_command_state){
        case RECORD:  
            if(string_length(breakpoint_string) > 0){
                parse_breakpoint(q);
                string_clear(breakpoint_string);
                breakpoint_enabled = FALSE;
            }
        break;
        case BREAKPOINT_TABLE_END:  
            if(string_length(breakpoint_string) > 0)
                parse_breakpoint(q);

            if(tgdb_append_command(q, BREAKPOINTS_END, NULL, NULL, NULL) == -1)
                err_msg("%s:%d -> Could not send command", __FILE__, __LINE__);

            string_clear(breakpoint_string);
            breakpoint_enabled = FALSE;

            if(breakpoint_started == FALSE){
                if(tgdb_append_command(q, BREAKPOINTS_BEGIN, NULL, NULL, NULL) == -1)
                    err_msg("%s:%d -> Could not send command", __FILE__, __LINE__);
            
                if(tgdb_append_command(q, BREAKPOINTS_END, NULL, NULL, NULL) == -1)
                    err_msg("%s:%d -> Could not send command", __FILE__, __LINE__);
            }

            breakpoint_started = FALSE;
            break;
        case BREAKPOINT_HEADERS: 
            breakpoint_table = 0; 
            break;
        case BREAKPOINT_TABLE_BEGIN: 
            if(tgdb_append_command(q, BREAKPOINTS_BEGIN, NULL, NULL, NULL) == -1)
                err_msg("%s:%d -> Could not send command", __FILE__, __LINE__);
            breakpoint_table = 1; 
            breakpoint_started = TRUE;
            break;
        case INFO_SOURCE_ABSOLUTE:
            break;
        case INFO_SOURCE_RELATIVE:
            break;
        case INFO_SOURCES:
            break;
        default: break;
    }
}

void commands_set_field_num(int field_num){
   cur_field_num = field_num;

   /* clear buffer and start over */
   if(breakpoint_table && cur_command_state == FIELD && cur_field_num == 5)
      string_clear(breakpoint_string);
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
   io_debug_write_fmt("<%s\n>", com);

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
   string_clear(breakpoint_string);
   return commands_run_com(fd, "info breakpoints", 1);
}

static int commands_run_tty(char *tty, int fd){
   char line[MAXLINE];
   strcpy(line, "tty ");
   strcat(line, tty);
   return commands_run_com(fd, line, 1);
}

static int commands_run_tab_completion(char *com, int fd){
   char line[MAXLINE];
   data_set_state(INTERNAL_COMMAND);
   strcpy(line, "complete ");
   strcat(line, com);
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
   string_clear(info_sources_string);
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
   info_source_ready = 0;
   return commands_run_com(fd, local_com, 1);
}

static int commands_run_info_source(enum COMMAND_STATE state, int fd){
   data_set_state(INTERNAL_COMMAND);
   string_clear(info_source_string);
   
   if ( state == INFO_SOURCE_ABSOLUTE ) {
        commands_set_state(INFO_SOURCE_ABSOLUTE, NULL);
        global_set_start_info_source();
   } else if ( state == INFO_SOURCE_RELATIVE )
        commands_set_state(INFO_SOURCE_RELATIVE, NULL);

   info_source_ready = 0;
   return commands_run_com(fd, "info source", 1);
}

int commands_run_readline_change_prompt(char *prompt) {
    if ( rlctx_change_prompt(rl, prompt) == -1 ) {
        err_msg("%s:%d rlctx_change_prompt failed", __FILE__, __LINE__);
        return -1;
    }

    return 0;
}

int commands_run_readline_redisplay(void) {
    if ( rlctx_redisplay(rl) == -1 ) {
        err_msg("%s:%d rlctx_change_prompt failed", __FILE__, __LINE__);
        return -1;
    }

    return 0;
}

int commands_run_command(int fd, struct command *com){
    /* Set up data to know the current state */
    switch(com->com_type) {
        case BUFFER_TGDB_COMMAND: data_set_state(INTERNAL_COMMAND);           break;
        case BUFFER_GUI_COMMAND:  data_set_state(GUI_COMMAND);                break;
        case BUFFER_USER_COMMAND: data_set_state(USER_COMMAND);               break;
        case BUFFER_READLINE_COMMAND: 
            data_set_state(INTERNAL_COMMAND);               
            break;
        case BUFFER_VOID: err_msg("%s:%d switch error", __FILE__, __LINE__);  break;
        default: err_msg("%s:%d switch error", __FILE__, __LINE__);           break;
    }

    /* Run the current command */
    switch(com->com_to_run) {
        case COMMANDS_INFO_SOURCES: 
            return commands_run_info_sources(fd);                   break;
        case COMMANDS_INFO_LIST:
            return commands_run_list(com->data, fd);                break;
        case COMMANDS_INFO_SOURCE_RELATIVE:                         
            return commands_run_info_source(
                    INFO_SOURCE_RELATIVE,
                    fd);                                            break;
        case COMMANDS_INFO_SOURCE_ABSOLUTE:                         break;
        case COMMANDS_INFO_BREAKPOINTS:
            return commands_run_info_breakpoints(fd);               break;
        case COMMANDS_TTY:
            return commands_run_tty(com->data, fd);                 break;
        case COMMANDS_TAB_COMPLETION:
            return commands_run_tab_completion(com->data, fd);      break;
        case COMMANDS_SET_PROMPT:
            return commands_run_readline_change_prompt(com->data);  break;
        case COMMANDS_REDISPLAY:
            return commands_run_readline_redisplay();               break;
        case COMMANDS_VOID:                                         break;
        default: err_msg("%s:%d switch error", __FILE__, __LINE__); break;
    }

    return commands_run_com(fd, com->data, 0);
}


void commands_list_command_finished(struct queue *q, int success){
   /* The file was accepted, lets see if we can get the 
    * absolute path from gdb. It should be ok for tgdb to run this command
    * right away.  */
   if(success)
      commands_run_info_source(INFO_SOURCE_ABSOLUTE, masterfd);
   else
      /* The file does not exist and it can not be opened.
       * So we return that information to the gui.  */
      tgdb_append_command(q, ABSOLUTE_SOURCE_DENIED, 
              last_info_source_requested, NULL, NULL);
}

void commands_send_source_absolute_source_file(struct queue *q){
   /*err_msg("Whats up(%s:%d)\r\n", info_source_buf, info_source_buf_pos);*/
    unsigned long length = string_length(info_source_string);
    static char *info_ptr; 
    info_ptr = string_get(info_source_string);

   /* found */
   if(length >= source_prefix_length && 
      (strncmp(info_ptr, source_prefix, source_prefix_length) == 0)){
      char *path = info_ptr + source_prefix_length;

      /* requesting file */
      if(last_info_source_requested[0] != '\0')
         tgdb_append_command(q, ABSOLUTE_SOURCE_ACCEPTED, path, NULL, NULL);
      else {
         tgdb_append_command(q, SOURCE_FILE_UPDATE, path, NULL, NULL);
         tgdb_append_command(q, LINE_NUMBER_UPDATE, "1", NULL, NULL);
      }
   /* not found */
   } else {
      tgdb_append_command(q, ABSOLUTE_SOURCE_DENIED, 
              last_info_source_requested, NULL, NULL);
   }
}

/* returns 1 if there are commands to run, otherwise 0 */
int commands_has_commnands_to_run(void){
   return (commands == 0)? 0: 1;
}

static void commands_process_source_line(void){
    unsigned long length = string_length(info_sources_string), i, start = 0;
    static char *info_ptr; 
    static char *nfile;
    info_ptr = string_get(info_sources_string);
  
    for(i = 0 ; i < length; ++i){
        if(i > 0 && info_ptr[i - 1] == ',' && info_ptr[i] == ' '){
            nfile = calloc(sizeof(char),i - start) ;
            strncpy(nfile, info_ptr + start , i - start - 1);
            start += ((i + 1) - start); 
            queue_append(source_files, nfile);
        } else if (i == length - 1 ){
            nfile = calloc(sizeof(char),i - start + 2);
            strncpy(nfile, info_ptr + start , i - start + 1);
            queue_append(source_files, nfile);
        }
    }
}

static void commands_process_info_source(struct queue *q, char a){
    unsigned long length;
    static char *info_ptr;

    if ( info_source_ready  ) /* Already found */
        return;
    
    info_ptr = string_get(info_source_string);
    length   = string_length(info_source_string);

    if ( a == '\r' )
        return;

    if(a == '\n'){ 
        /* This is the line of interest */
        if ( commands_get_state() == INFO_SOURCE_ABSOLUTE && 
             length >= source_prefix_length && 
             strncmp(info_ptr, source_prefix, source_prefix_length) == 0 ) {
            info_source_ready = 1;
        } else if ( 
             commands_get_state() == INFO_SOURCE_RELATIVE && 
             length >= source_relative_prefix_length && 
             strncmp(info_ptr, source_relative_prefix, source_relative_prefix_length) == 0 ) {

             if(tgdb_append_command(q, CURRENT_FILE_UPDATE, info_ptr + source_relative_prefix_length, NULL, NULL) == -1)
                err_msg("%s:%d -> Could not send command", __FILE__, __LINE__);

             info_source_ready = 1;
        } else
            string_clear(info_source_string);
    } else
        string_addchar(info_source_string, a);
}

/* process's source files */
static void commands_process_sources(char a){
    static const char *sourcesReadyString = "Source files for which symbols ";
    static const int sourcesReadyStringLength = 31;
    static char *info_ptr;
    string_addchar(info_sources_string, a);
   
    if(a == '\n'){
        string_delchar(info_sources_string);     /* remove '\n' and null terminate */
        /* valid lines are 
         * 1. after the first line,
         * 2. do not end in ':' 
         * 3. and are not empty 
         */
        info_ptr = string_get(info_sources_string);

        if ( strncmp(info_ptr, sourcesReadyString, sourcesReadyStringLength) == 0 )
            sources_ready = 1;

        /* is this a valid line */
        if(string_length(info_sources_string) > 0 && sources_ready && info_ptr[string_length(info_sources_string) - 1] != ':')
            commands_process_source_line(); 

        string_clear(info_sources_string);
    }
}

/* TODO: This variable is here becuase the travere command for the queue only
 * passes along the item to the function. It would be better if it could pass
 * along other data 
 */
static struct queue *hackq;

void commands_send_gui_source(void *item) {
    tgdb_append_command(hackq, SOURCE_FILE, (char*)item, NULL, NULL);  
}

void commands_free(void *item) {
    free((char*)item);
}

void commands_send_gui_sources(struct queue *q){
    hackq = q;
    tgdb_append_command(q, SOURCES_START, NULL, NULL, NULL);  
    queue_traverse_list(source_files, commands_send_gui_source);
    tgdb_append_command(q, SOURCES_END, NULL, NULL, NULL);  
    queue_free_list(source_files, commands_free);
    hackq = NULL;
}

void commands_process(char a, struct queue *q){
    if(commands_get_state() == INFO_SOURCES){
        commands_process_sources(a);     
    } else if(commands_get_state() == INFO_LIST){
        /* do nothing with data */
    } else if(commands_get_state() == INFO_SOURCE_ABSOLUTE 
            ||commands_get_state() == INFO_SOURCE_RELATIVE){
        commands_process_info_source(q, a);   
    } else if(breakpoint_table && cur_command_state == FIELD && cur_field_num == 5){ /* the file name and line num */ 
        string_addchar(breakpoint_string, a);
    } else if(breakpoint_table && cur_command_state == FIELD && cur_field_num == 3 && a == 'y')
        breakpoint_enabled = TRUE;
}
