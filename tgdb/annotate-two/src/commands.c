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
#include "sys_util.h"
#include "queue.h"
#include "ibuf.h"
#include "a2-tgdb.h"
#include "rlctx.h"

static struct string *absolute_path;
static struct string *line_number;


static enum COMMAND_STATE cur_command_state = VOID_COMMAND;
static int cur_field_num = 0;

/* breakpoint information */
struct queue *breakpoint_queue;
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

/* Temporary prototypes */
static void commands_prepare_info_source(enum COMMAND_STATE state);

/* This is used to store all of the entries that are possible to
 * tab complete. It should be blasted each time the completion
 * is run, populated, and then parsed.
 */
//static struct queue *tab_completion_entries = NULL;
static char last_tab_completion_command[MAXLINE];

void commands_init(void) {
    absolute_path       = string_init();
    line_number         = string_init();
    info_source_string  = string_init();
    info_sources_string = string_init();
    breakpoint_string   = string_init();
    breakpoint_queue    = queue_init();
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
   
//    if(tgdb_append_command(q, SOURCE_FILE_UPDATE, file, NULL, NULL) == -1)
//        err_msg("%s:%d -> Could not send command", __FILE__, __LINE__);
//
//    if(tgdb_append_command(q, LINE_NUMBER_UPDATE, line, NULL, NULL) == -1)
//        err_msg("%s:%d -> Could not send command", __FILE__, __LINE__);
    string_clear( absolute_path );
    string_add ( absolute_path, file );
    string_clear( line_number );
    string_add ( line_number, line );

    /* set up the info_source command to get the relative path */
    if ( commands_issue_command ( ANNOTATE_INFO_SOURCE_RELATIVE, NULL, 1 ) == -1 ) {
        err_msg("%s:%d commands_issue_command error", __FILE__, __LINE__);
        return -1;
    }

    return 0;
}

static void parse_breakpoint(struct queue *q){
    unsigned long size = string_length(breakpoint_string);
    char copy[size + 1];
    char *cur = copy + size, *fcur;
    char fname[MAXLINE + 2], file[MAXLINE], line[MAXLINE];
    static char *info_ptr; 
    struct string *s;

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

    s = string_init ();

    string_add ( s, fname );
    string_add ( s, " " );
    string_add ( s, line );
    string_add ( s, " " );
    string_add ( s, file );

    queue_append ( breakpoint_queue, s );

//    if(tgdb_append_command(q, BREAKPOINT, ) == -1)
//        err_msg("%s:%d -> Could not send command", __FILE__, __LINE__);
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

//            if(tgdb_append_command(q, BREAKPOINTS_END, NULL, NULL, NULL) == -1)
//                err_msg("%s:%d -> Could not send command", __FILE__, __LINE__);

            /* At this point, annotate needs to send the breakpoints to the gui.
             * All of the valid breakpoints are stored in breakpoint_queue. */
            tgdb_append_command ( q, TGDB_UPDATE_BREAKPOINTS, breakpoint_queue );

            string_clear(breakpoint_string);
            breakpoint_enabled = FALSE;

            /* Whats the point of this? */
//            if(breakpoint_started == FALSE){
//                if(tgdb_append_command(q, BREAKPOINTS_BEGIN, NULL, NULL, NULL) == -1)
//                    err_msg("%s:%d -> Could not send command", __FILE__, __LINE__);
//            
//                if(tgdb_append_command(q, BREAKPOINTS_END, NULL, NULL, NULL) == -1)
//                    err_msg("%s:%d -> Could not send command", __FILE__, __LINE__);
//            }

            breakpoint_started = FALSE;
            break;
        case BREAKPOINT_HEADERS: 
            breakpoint_table = 0; 
            break;
        case BREAKPOINT_TABLE_BEGIN: 
//            if(tgdb_append_command(q, BREAKPOINTS_BEGIN, NULL, NULL, NULL) == -1)
//                err_msg("%s:%d -> Could not send command", __FILE__, __LINE__);

            /* The breakpoint queue should be empty at this point */
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

static void commands_prepare_info_source(enum COMMAND_STATE state){
   data_set_state(INTERNAL_COMMAND);
   string_clear(info_source_string);
   
   if ( state == INFO_SOURCE_ABSOLUTE ) {
        commands_set_state(INFO_SOURCE_ABSOLUTE, NULL);
        global_set_start_info_source();
   } else if ( state == INFO_SOURCE_RELATIVE )
        commands_set_state(INFO_SOURCE_RELATIVE, NULL);

   info_source_ready = 0;
}

void commands_list_command_finished(struct queue *q, int success){
  /* The file does not exist and it can not be opened.
   * So we return that information to the gui.  */
    struct string *rej = string_init();
    string_add ( rej, last_info_source_requested );
    tgdb_append_command(q, TGDB_ABSOLUTE_SOURCE_DENIED, rej);
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
      if(last_info_source_requested[0] != '\0') {
          struct string *accepted = string_init();
          string_add ( accepted, path );
          tgdb_append_command(q, TGDB_ABSOLUTE_SOURCE_ACCEPTED, accepted);
      } else { /* This happens only when libtgdb starts */
//         tgdb_append_command(q, SOURCE_FILE_UPDATE, path, NULL, NULL);
//         tgdb_append_command(q, LINE_NUMBER_UPDATE, "1", NULL, NULL);
            string_clear( absolute_path );
            string_add ( absolute_path, path );
            string_clear( line_number );
            string_add ( line_number, "1" );
      }
   /* not found */
   } else {
      struct string *rej = string_init();
      string_add ( rej, last_info_source_requested );
      tgdb_append_command(q, TGDB_ABSOLUTE_SOURCE_DENIED, rej);
   }
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

/* commands_process_info_source:
 * -----------------------------
 *
 * This function is capable of parsing the output of 'info source'.
 * It can get both the absolute and relative path to the source file.
 */
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
        /* This is the line containing the absolute path to the source file */
        if ( commands_get_state() == INFO_SOURCE_ABSOLUTE && 
             length >= source_prefix_length && 
             strncmp(info_ptr, source_prefix, source_prefix_length) == 0 ) {

             commands_send_source_absolute_source_file ( q );

            info_source_ready = 1;
        } else if ( /* This is the line contatining the relative path to the source file */
            commands_get_state() == INFO_SOURCE_RELATIVE && 
            length >= source_relative_prefix_length && 
            strncmp(info_ptr, source_relative_prefix, source_relative_prefix_length) == 0 ) {

            /* So far, INFO_SOURCE_RELATIVE is only used when a 
             * TGDB_UPDATE_FILE_POSITION is needed.
             */
            {
                struct queue *update = queue_init();
                struct string *relative_path = string_init();
                string_add ( relative_path, info_ptr + source_relative_prefix_length );
                queue_append ( update, absolute_path );
                queue_append ( update, relative_path );
                queue_append ( update, line_number );

                tgdb_append_command(q, TGDB_UPDATE_FILE_POSITION, update );
            }

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

/* commands_process_tab_completion:
 * --------------------------------
 *
 * c:    The character to process
 */
static void commands_process_tab_completion(char c){
    /* TODO: Make tab completion work with readline */
//    static struct string *tab_completion_entry = NULL;
//
//    if ( tab_completion_entry == NULL )
//        tab_completion_entry = string_init ();
//
    /* Append new completion to list */
//    if ( c == '\n' && data_get_state () == INTERNAL_COMMAND ) {
//        queue_append ( tab_completion_entries, tab_completion_entry );
//        tab_completion_entry = string_init ();
//    } else {
//        string_addchar ( tab_completion_entry, c ); 
//    }
}


void commands_free(void *item) {
    free((char*)item);
}

void commands_send_gui_sources(struct queue *q){
    tgdb_append_command ( q, TGDB_UPDATE_SOURCE_FILES, source_files );
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
    } else if(breakpoint_table && cur_command_state == FIELD && cur_field_num == 3 && a == 'y') {
        breakpoint_enabled = TRUE;
    } else if ( commands_get_state() == TAB_COMPLETE ) {
        commands_process_tab_completion ( a );
    }
}

/*******************************************************************************
 * This must be translated to just return the proper command.
 ******************************************************************************/

/* commands_prepare_info_breakpoints: 
 * ----------------------------------
 *  
 *  This prepares the command 'info breakpoints' 
 */
static void commands_prepare_info_breakpoints( void ) {
    string_clear(breakpoint_string);
}

/* commands_prepare_tab_completion:
 * --------------------------------
 *
 * This prepares the tab completion command
 */
static void commands_prepare_tab_completion ( void ) {
    /* TODO: Make tab completion work with readline */
//    if ( tab_completion_entries != NULL ) {
//        /* TODO: Free the old entries */
//    }
//        
//    tab_completion_entries = queue_init();
//    commands_set_state ( TAB_COMPLETE, NULL );
    data_set_state ( USER_COMMAND );
}

/* commands_prepare_info_sources: 
 * ------------------------------
 *
 *  This prepares the command 'info sources' by setting certain variables.
 */
static void commands_prepare_info_sources ( void ){
    sources_ready = 0;
    string_clear(info_sources_string);
    commands_set_state(INFO_SOURCES, NULL);
    global_set_start_info_sources();
}

/* commands_prepare_list: 
 * -----------------------------
 *  This runs the command 'list filename:1' and then runs
 *  'info source' to find out what the absolute path to filename is.
 * 
 *    filename -> The name of the file to check the absolute path of.
 */
static void commands_prepare_list ( char *filename ){
   commands_set_state(INFO_LIST, NULL);
   global_set_start_list();
   info_source_ready = 0;
}

void commands_finalize_command ( struct queue *q ) {
    switch ( commands_get_state () ) {
        case TAB_COMPLETE:
                /* TODO: Make tab completion work with readline */ 
//            tgdb_complete_command ( tab_completion_entries, last_tab_completion_command ); 
            break;
        case INFO_SOURCE_RELATIVE:
        case INFO_SOURCE_ABSOLUTE:

            if ( info_source_ready == 0 ) {
                struct string *rej = string_init();
                string_add ( rej, last_info_source_requested );
                tgdb_append_command(q, TGDB_ABSOLUTE_SOURCE_DENIED, rej );
            }

            break;
        default: break;
    }
}

int commands_prepare_for_command ( struct command *com ) {
    enum annotate_commands *a_com = ( enum annotate_commands *) com->client_data;
    extern int COMMAND_ALREADY_GIVEN; 

    /* Set the commands state to nothing */
    commands_set_state(VOID, NULL);

    if ( global_list_had_error () == TRUE && commands_get_state() == INFO_LIST )  {
        global_set_list_error ( FALSE );
        return -1;
    }

    COMMAND_ALREADY_GIVEN = 1;
    if ( a_com == NULL ) {
        data_set_state ( USER_COMMAND );
        return 0;
    }

    switch ( *a_com ) {
        case ANNOTATE_INFO_SOURCES:
            commands_prepare_info_sources ();
            break;
        case ANNOTATE_LIST:
            commands_prepare_list ( com -> data );
            break;
        case ANNOTATE_INFO_SOURCE_RELATIVE:
            commands_prepare_info_source ( INFO_SOURCE_RELATIVE );
            break;
        case ANNOTATE_INFO_SOURCE_ABSOLUTE:
            commands_prepare_info_source ( INFO_SOURCE_ABSOLUTE );
            break;
        case ANNOTATE_INFO_BREAKPOINTS:
            commands_prepare_info_breakpoints ();
            break;
        case ANNOTATE_TTY:
            break;  /* Nothing to do */
        case ANNOTATE_COMPLETE:
            commands_prepare_tab_completion ();
            io_debug_write_fmt("<%s\n>", com->data);
            return 0;
            break;  /* Nothing to do */
        case ANNOTATE_VOID:
        default:
            err_msg ( "%s:%d commands_prepare_for_command error", __FILE__, __LINE__ );
            break;
    };
   
    data_set_state(INTERNAL_COMMAND);
    io_debug_write_fmt("<%s\n>", com->data);

    return 0;
}

/* commands_create_command:
 * ------------------------
 *
 * This is responsible for creating a command to run through the debugger.
 *
 * com  - The annotate command to run
 * data - Information that may be needed to create the command
 *
 * Returns - A command ready to be run through the debugger.
 *           NULL on error
 */
static const char *commands_create_command ( 
    enum annotate_commands com,
    const char *data) {

    char *ncom = NULL;
    
    switch ( com ) {
        case ANNOTATE_INFO_SOURCES: 
            ncom = strdup ( "server info sources\n" );   
            break;
        case ANNOTATE_LIST:
            {
                static char temp_file_name [MAXLINE];
                if ( data == NULL )
                    temp_file_name[0] = 0;
                else
                    strcpy ( temp_file_name, data );

                if ( data == NULL )
                    ncom = (char *)xmalloc( sizeof ( char ) * ( 16 ));
                else
                    ncom = (char *)xmalloc( sizeof ( char ) * ( 16 + strlen (data )));
                strcpy ( ncom, "server list " );

                /* This should only happen for the initial 'list' */
                if ( temp_file_name[0] != '\0' ) {
                    strcat ( ncom, temp_file_name );
                    strcat ( ncom, ":1" );
                } 

                if ( temp_file_name[0] == '\0' )
                    last_info_source_requested[0] = '\0';
                else
                    strcpy ( last_info_source_requested, temp_file_name );

                strcat ( ncom, "\n" );
                break;
            }
        case ANNOTATE_INFO_SOURCE_RELATIVE:                         
            ncom = strdup ( "server info source\n" );   
            break;
        case ANNOTATE_INFO_SOURCE_ABSOLUTE:                         
            ncom = strdup ( "server info source\n" );   
            break;
        case ANNOTATE_INFO_BREAKPOINTS:
            ncom = strdup ( "server info breakpoints\n" );   
            break;
        case ANNOTATE_TTY:
            {
                static char temp_tty_name [MAXLINE];
                strcpy ( temp_tty_name, data );
                ncom = (char *)xmalloc( sizeof ( char ) * ( 13 + strlen( data )));
                strcpy ( ncom, "server tty " );
                strcat ( ncom, temp_tty_name );
                strcat ( ncom, "\n" );
                break;
            }
        case ANNOTATE_COMPLETE:
            ncom = (char *)xmalloc( sizeof ( char ) * ( 18 + strlen( data )));
            strcpy ( ncom, "server complete " );
            strcat ( ncom, data );
            strcat ( ncom, "\n" );

            /* A hack to save the last tab completion command */
            strcpy ( last_tab_completion_command, data );
            break;
        case ANNOTATE_SET_PROMPT:
            ncom = strdup ( data );   
            break;
        case ANNOTATE_VOID:
        default: 
            err_msg("%s:%d switch error", __FILE__, __LINE__);
            break;
    };

    return ncom;
}

int commands_user_ran_command ( void ) {
    if ( commands_issue_command ( ANNOTATE_INFO_BREAKPOINTS, NULL, 0 ) == -1 ) {
        err_msg("%s:%d commands_issue_command error", __FILE__, __LINE__);
        return -1;
    }

    return 0;
}

int commands_issue_command ( enum annotate_commands com, const char *data, int oob) {
    const char *ncom = commands_create_command ( com, data );
    struct command *command;
    enum annotate_commands *nacom = (enum annotate_commands *)xmalloc ( sizeof ( enum annotate_commands ) );
    
    *nacom = com;

    if ( ncom == NULL ) {
        err_msg("%s:%d commands_issue_command error", __FILE__, __LINE__);
        return -1;
    }

    /* This should send the command to tgdb-base to handle */ 
    if ( oob == 1 ) {
        command = tgdb_interface_new_command ( 
                ncom,
                BUFFER_OOB_COMMAND,
                COMMANDS_HIDE_OUTPUT,
                COMMANDS_VOID,
                (void*) nacom ); 
    } else if ( oob == 2 ) {
        command = tgdb_interface_new_command ( 
                ncom,
                BUFFER_READLINE_COMMAND,
                COMMANDS_HIDE_OUTPUT,
                COMMANDS_SET_PROMPT,
                NULL ); 
    } else if ( oob == 0 ){
        command = tgdb_interface_new_command ( 
                ncom,
                BUFFER_TGDB_COMMAND,
                COMMANDS_HIDE_OUTPUT,
                COMMANDS_VOID,
                (void*) nacom ); 
    } else if ( oob == 3 ) {
        command = tgdb_interface_new_command ( 
                ncom,
                BUFFER_TGDB_COMMAND,
                COMMANDS_SHOW_USER_OUTPUT,
                COMMANDS_VOID,
                (void*) nacom ); 
    } else if ( oob == 4 ) {
        command = tgdb_interface_new_command ( 
                ncom,
                BUFFER_GUI_COMMAND,
                COMMANDS_SHOW_USER_OUTPUT,
                COMMANDS_VOID,
                (void*) nacom ); 
    }

    if ( tgdb_dispatch_command ( command ) == -1 ) {
        err_msg("%s:%d tgdb_dispatch_command error", __FILE__, __LINE__);
        return -1;
    }

    return 0;
}
