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
#include "queue.h"
#include "tgdb_list.h"
#include "annotate_two.h"

struct commands {
	struct string *absolute_path;
	struct string *line_number;

	enum COMMAND_STATE cur_command_state;
	int cur_field_num;
   int field_5_newline_hit;

	/* breakpoint information */
	struct tgdb_list *breakpoint_list;
	struct string *breakpoint_string;
	int breakpoint_table;
	int breakpoint_enabled;
	int breakpoint_started;

	/* 'info source' information */
	struct string *info_source_string;
	int info_source_ready;
	char last_info_source_requested[MAXLINE];

	/* 'info sources' information */
	int sources_ready;
	struct string *info_sources_string;
	struct tgdb_list *inferior_source_files;

	/* String that is output by gdb to get the absolute path to a file */
	const char *source_prefix;
	int source_prefix_length;

	/* String that is output by gdb to get the relative path to a file */
	const char *source_relative_prefix;
	int source_relative_prefix_length;

	/* This is used to store all of the entries that are possible to
	 * tab complete. It should be blasted each time the completion
	 * is run, populated, and then parsed.
	 */
	//static struct queue *tab_completion_entries = NULL;
	char last_tab_completion_command[MAXLINE];
};

struct commands *commands_initialize(void) {
	struct commands *c = (struct commands *)
		xmalloc ( sizeof ( struct commands ));

    c->absolute_path       	= string_init();
    c->line_number         	= string_init();

	c->cur_command_state 	= VOID_COMMAND;
	c->cur_field_num 		   = 0;
   c->field_5_newline_hit  = 0;

    c->breakpoint_list    	= tgdb_list_init();
    c->breakpoint_string   	= string_init();
	c->breakpoint_table    	= 0;
	c->breakpoint_enabled   = FALSE;
	c->breakpoint_started   = FALSE;

    c->info_source_string  	= string_init();
	c->info_source_ready   	= 0;

	c->sources_ready 		= 0;
    c->info_sources_string 	= string_init();
	c->inferior_source_files= tgdb_list_init ();

	c->source_prefix 		= "Located in ";
	c->source_prefix_length = 11;

	c->source_relative_prefix 	= "Current source file is ";
	c->source_relative_prefix_length 	= 23;

	return c;
}

void commands_shutdown ( struct commands *c ) {
	if ( c == NULL )
		return;

	string_free ( c->absolute_path );
	c->absolute_path = NULL;

	string_free ( c->line_number );
	c->line_number = NULL;

	/* TODO: free breakpoint queue */

	string_free ( c->breakpoint_string );
	c->breakpoint_string = NULL;

	string_free ( c->info_sources_string );
	c->info_sources_string = NULL;
	
	/* TODO: free source_files queue */

	free ( c );
	c = NULL;
}

int commands_parse_field(struct commands *c, const char *buf, size_t n, int *field){
   if(sscanf(buf, "field %d", field) != 1)
      err_msg("%s:%d -> parsing field annotation failed (%s)\n", __FILE__, __LINE__, buf);

   return 0;
}

/* source filename:line:character:middle:addr */
int commands_parse_source(
		struct commands *c, 
		struct queue *command_container,
		const char *buf, size_t n, struct queue *q){
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
     
	/*TODO: I don't think this will work with filenames that contain spaces.
	 * It should be changed. Look at the algorithm in the function below.
	 */
    if(sscanf(copy, "source %s", file) != 1)
        err_msg("%s:%d -> Could not get file name", __FILE__, __LINE__);
   
    string_clear( c->absolute_path );
    string_add ( c->absolute_path, file );
    string_clear( c->line_number );
    string_add ( c->line_number, line );

    /* set up the info_source command to get the relative path */
    if ( commands_issue_command ( 
				c, 
				command_container,
				ANNOTATE_INFO_SOURCE_RELATIVE, 
				NULL, 
				1 ) == -1 ) {
        err_msg("%s:%d commands_issue_command error", __FILE__, __LINE__);
        return -1;
    }

    return 0;
}

/* Unfortunatly, the line that this function has to parse is completly 
 * ambiguous. GDB does not output a line that can be read in a 
 * non-ambiguous way. Therefore, TGDB tries its best to read the line 
 * properly. The line is in this format.
 *
 *  '[io]n .* at .*:number'
 *
 * so, TGDB can parser the ':number' part without a problem. However,
 * it may not be able to get function name and filename correctly. If
 * the filename contains ' at ' then, it TGDB will stop prematurly.
 */

static void parse_breakpoint(struct commands *c, struct queue *q){
    unsigned long size = string_length(c->breakpoint_string);
    char copy[size + 1];
    char *cur = copy + size, *fcur;
    char fname[MAXLINE + 2], file[MAXLINE], line[MAXLINE];
    static char *info_ptr; 
    struct tgdb_breakpoint *tb;

    memset(fname, '\0', MAXLINE + 2);
    memset(file, '\0', MAXLINE);
    memset(line, '\0', MAXLINE);

    info_ptr = string_get(c->breakpoint_string);

    strncpy(copy, info_ptr, size + 1); /* modify local copy */

    /* Check to see if this is a watchpoint, if it is, 
     * don't parse for a breakpoint.
     */
    if ( strstr ( copy, " at " ) == NULL )
      return;

    /* This loop starts at the end and traverses backwards, until it finds
     * the ':'. Then, it knows the file number for sure. */
    while(cur != copy){
        if((*cur) == ':'){
            if(sscanf(cur + 1, "%s", line) != 1)
                err_msg("%s:%d -> Could not get line number", __FILE__, __LINE__);

            *cur = '\0';
            break; /* in case of multiple ':' in the line */
        } 

        --cur;
    } /* end while */

    /* Assertion: The string to parse now looks like '[io]n .* at .*'*/
    if ( !((copy[0] == 'i' || copy[0] == 'o') && copy[1] == 'n' && copy[2] == ' ') ){
      err_msg("%s:%d -> Could not scan function and file name\n"
              "\tWas the program compiled with debug info?\n", __FILE__, __LINE__);
    }

    fcur = &copy[3];

    /* Assertion: The string to parse now looks like '.* at .*'*/
    if ( ( cur = strstr(fcur, " at " )) == NULL ) {
      err_msg("%s:%d -> Could not scan function and file name\n"
              "\tWas the program compiled with debug info?\n", __FILE__, __LINE__);
    }

    *cur='\0';
    strcpy ( fname, fcur );

    /* Assertion: The string to parse now looks like ' at .*'*/
    cur += 4;
    /* Assertion: The string to parse now looks like '.*' */
    strcpy(file, cur);

    tb = ( struct tgdb_breakpoint*) xmalloc ( sizeof ( struct tgdb_breakpoint) );
    tb->file = strdup ( file );
    tb->funcname = strdup ( fname );
    sscanf (line, "%d", &tb->line);

    if(c->breakpoint_enabled == TRUE)
        tb->enabled = 1;
    else
        tb->enabled = 0;

    tgdb_list_append ( c->breakpoint_list, tb );
}


void commands_set_state( struct commands *c, enum COMMAND_STATE state, struct queue *q){
    c->cur_command_state = state;

    switch(c->cur_command_state){
        case RECORD:  
            if(string_length(c->breakpoint_string) > 0){
                parse_breakpoint(c, q);
                string_clear(c->breakpoint_string);
                c->breakpoint_enabled = FALSE;
            }
        break;
        case BREAKPOINT_TABLE_END:  
            if(string_length(c->breakpoint_string) > 0)
                parse_breakpoint(c, q);

            /* At this point, annotate needs to send the breakpoints to the gui.
             * All of the valid breakpoints are stored in breakpoint_queue. */
            tgdb_append_command ( q, TGDB_UPDATE_BREAKPOINTS, c->breakpoint_list );

            string_clear(c->breakpoint_string);
            c->breakpoint_enabled = FALSE;

            c->breakpoint_started = FALSE;
            break;
        case BREAKPOINT_HEADERS: 
            c->breakpoint_table = 0; 
            break;
        case BREAKPOINT_TABLE_BEGIN: 
            
            /* The breakpoint queue should be empty at this point */
            c->breakpoint_table = 1; 
            c->breakpoint_started = TRUE;
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

void commands_set_field_num( struct commands *c, int field_num){
   c->cur_field_num = field_num;

   /* clear buffer and start over */
   if(c->breakpoint_table && c->cur_command_state == FIELD && c->cur_field_num == 5) {
      string_clear(c->breakpoint_string);
      c->field_5_newline_hit = 0;
   }
}

enum COMMAND_STATE commands_get_state( struct commands *c){
   return c->cur_command_state;
}

static void commands_prepare_info_source(struct annotate_two *a2, struct commands *c, enum COMMAND_STATE state){
   data_set_state(a2, INTERNAL_COMMAND );
   string_clear(c->info_source_string);
   
   if ( state == INFO_SOURCE_ABSOLUTE ) {
        commands_set_state(c, INFO_SOURCE_ABSOLUTE, NULL);
   } else if ( state == INFO_SOURCE_RELATIVE )
        commands_set_state(c, INFO_SOURCE_RELATIVE, NULL);

   c->info_source_ready = 0;
}

void commands_list_command_finished(struct commands *c, struct queue *q, int success){
  /* The file does not exist and it can not be opened.
   * So we return that information to the gui.  */
    struct string *rej = string_init();
    string_add ( rej, c->last_info_source_requested );
    tgdb_append_command(q, TGDB_ABSOLUTE_SOURCE_DENIED, rej);
}

void commands_send_source_absolute_source_file(struct commands *c, struct queue *q){
   /*err_msg("Whats up(%s:%d)\r\n", info_source_buf, info_source_buf_pos);*/
    unsigned long length = string_length(c->info_source_string);
    static char *info_ptr; 
    info_ptr = string_get(c->info_source_string);

   /* found */
   if(length >= c->source_prefix_length && 
      (strncmp(info_ptr, c->source_prefix, c->source_prefix_length) == 0)){
      char *path = info_ptr + c->source_prefix_length;

      /* requesting file */
      if(c->last_info_source_requested[0] != '\0') {
          struct string *accepted = string_init();
          string_add ( accepted, path );
          tgdb_append_command(q, TGDB_ABSOLUTE_SOURCE_ACCEPTED, accepted);
      } else { /* This happens only when libtgdb starts */
            string_clear( c->absolute_path );
            string_add ( c->absolute_path, path );
            string_clear( c->line_number );
            string_add ( c->line_number, "1" );
      }
   /* not found */
   } else {
      struct string *rej = string_init();
      string_add ( rej, c->last_info_source_requested );
      tgdb_append_command(q, TGDB_ABSOLUTE_SOURCE_DENIED, rej);
   }
}

static void commands_process_source_line(struct commands *c ){
    unsigned long length = string_length(c->info_sources_string), i, start = 0;
    static char *info_ptr; 
    static char *nfile;
    info_ptr = string_get(c->info_sources_string);
  
    for(i = 0 ; i < length; ++i){
        if(i > 0 && info_ptr[i - 1] == ',' && info_ptr[i] == ' '){
            nfile = calloc(sizeof(char),i - start) ;
            strncpy(nfile, info_ptr + start , i - start - 1);
            start += ((i + 1) - start); 
			tgdb_list_append ( c->inferior_source_files, nfile );
        } else if (i == length - 1 ){
            nfile = calloc(sizeof(char),i - start + 2);
            strncpy(nfile, info_ptr + start , i - start + 1);
			tgdb_list_append ( c->inferior_source_files, nfile );
        }
    }
}

/* commands_process_info_source:
 * -----------------------------
 *
 * This function is capable of parsing the output of 'info source'.
 * It can get both the absolute and relative path to the source file.
 */
static void commands_process_info_source(struct commands *c, struct queue *q, char a){
    unsigned long length;
    static char *info_ptr;

    if ( c->info_source_ready  ) /* Already found */
        return;
    
    info_ptr = string_get(c->info_source_string);
    length   = string_length(c->info_source_string);

    if ( a == '\r' )
        return;

    if(a == '\n'){ 
        /* This is the line containing the absolute path to the source file */
        if ( commands_get_state(c) == INFO_SOURCE_ABSOLUTE && 
             length >= c->source_prefix_length && 
             strncmp(info_ptr, c->source_prefix, c->source_prefix_length) == 0 ) {

             commands_send_source_absolute_source_file ( c, q );

            c->info_source_ready = 1;
        } else if ( /* This is the line contatining the relative path to the source file */
            commands_get_state(c) == INFO_SOURCE_RELATIVE && 
            length >= c->source_relative_prefix_length && 
            strncmp(info_ptr, c->source_relative_prefix, c->source_relative_prefix_length) == 0 ) {

            /* So far, INFO_SOURCE_RELATIVE is only used when a 
             * TGDB_UPDATE_FILE_POSITION is needed.
             */
            {
				/* This section allocates a new structure to add into the queue 
				 * All of its members will need to be freed later.
				 */
				struct tgdb_file_position *tfp = (struct tgdb_file_position *)
					xmalloc ( sizeof ( struct tgdb_file_position ) );
				tfp->absolute_path = string_dup ( c->absolute_path );
				tfp->relative_path = string_init ();
				string_add ( tfp->relative_path, info_ptr + c->source_relative_prefix_length );
				tfp->line_number   = atoi ( string_get ( c->line_number ) );

                tgdb_append_command(q, TGDB_UPDATE_FILE_POSITION, tfp );
            }

            c->info_source_ready = 1;
        } else
            string_clear(c->info_source_string);
    } else
        string_addchar(c->info_source_string, a);
}

/* process's source files */
static void commands_process_sources(struct commands *c, char a){
    static const char *sourcesReadyString = "Source files for which symbols ";
    static const int sourcesReadyStringLength = 31;
    static char *info_ptr;
    string_addchar(c->info_sources_string, a);
   
    if(a == '\n'){
        string_delchar(c->info_sources_string);     /* remove '\n' and null terminate */
        /* valid lines are 
         * 1. after the first line,
         * 2. do not end in ':' 
         * 3. and are not empty 
         */
        info_ptr = string_get(c->info_sources_string);

        if ( strncmp(info_ptr, sourcesReadyString, sourcesReadyStringLength) == 0 )
            c->sources_ready = 1;

        /* is this a valid line */
        if(string_length(c->info_sources_string) > 0 && c->sources_ready && info_ptr[string_length(c->info_sources_string) - 1] != ':')
            commands_process_source_line(c); 

        string_clear(c->info_sources_string);
    }
}

void commands_free(struct commands *c, void *item) {
    free((char*)item);
}

void commands_send_gui_sources(struct commands *c, struct queue *q){
	/* If the inferior program was not compiled with debug, then no sources
	 * will be available. If no sources are available, do not return the
	 * TGDB_UPDATE_SOURCE_FILES command. */
	if ( tgdb_list_size ( c->inferior_source_files ) > 0 )
		tgdb_append_command ( q, TGDB_UPDATE_SOURCE_FILES, c->inferior_source_files );
}

void commands_process(struct commands *c, char a, struct queue *q){
    if(commands_get_state(c) == INFO_SOURCES){
        commands_process_sources(c, a);     
    } else if(commands_get_state(c) == INFO_LIST){
        /* do nothing with data */
    } else if(commands_get_state(c) == INFO_SOURCE_ABSOLUTE 
            ||commands_get_state(c) == INFO_SOURCE_RELATIVE){
        commands_process_info_source(c, q, a);   
    } else if(c->breakpoint_table && c->cur_command_state == FIELD && c->cur_field_num == 5){ /* the file name and line num */ 
        if ( a == '\n' || a == '\r' )
            c->field_5_newline_hit = 1;
        
        if ( !c->field_5_newline_hit )
            string_addchar(c->breakpoint_string, a);
    } else if(c->breakpoint_table && c->cur_command_state == FIELD && c->cur_field_num == 3 && a == 'y') {
        c->breakpoint_enabled = TRUE;
    } else if ( commands_get_state(c) == TAB_COMPLETE ) {
        /*commands_process_tab_completion ( c, a );*/
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
static void commands_prepare_info_breakpoints( struct commands *c ) {
    string_clear(c->breakpoint_string);
}

/* commands_prepare_tab_completion:
 * --------------------------------
 *
 * This prepares the tab completion command
 */
static void commands_prepare_tab_completion ( struct annotate_two *a2, struct commands *c ) {
    /* TODO: Make tab completion work with readline */
//    if ( tab_completion_entries != NULL ) {
//        /* TODO: Free the old entries */
//    }
//        
//    tab_completion_entries = queue_init();
//    commands_set_state ( TAB_COMPLETE, NULL );
    data_set_state ( a2, USER_COMMAND );
}

/* commands_prepare_info_sources: 
 * ------------------------------
 *
 *  This prepares the command 'info sources' by setting certain variables.
 */
static void commands_prepare_info_sources ( struct annotate_two *a2, struct commands *c ){
    c->sources_ready = 0;
    string_clear(c->info_sources_string);
    commands_set_state(c, INFO_SOURCES, NULL);
	global_set_start_info_sources ( a2->g );
}

/* commands_prepare_list: 
 * -----------------------------
 *  This runs the command 'list filename:1' and then runs
 *  'info source' to find out what the absolute path to filename is.
 * 
 *    filename -> The name of the file to check the absolute path of.
 */
static void commands_prepare_list ( struct annotate_two *a2, struct commands *c, char *filename ){
   commands_set_state(c, INFO_LIST, NULL);
   global_set_start_list(a2->g);
   c->info_source_ready = 0;
}

void commands_finalize_command ( struct commands *c, struct queue *q ) {
    switch ( commands_get_state (c) ) {
        case TAB_COMPLETE:
                /* TODO: Make tab completion work with readline */ 
//            tgdb_complete_command ( tab_completion_entries, last_tab_completion_command ); 
            break;
        case INFO_SOURCE_RELATIVE:
        case INFO_SOURCE_ABSOLUTE:

            if ( c->info_source_ready == 0 ) {
                struct string *rej = string_init();
                string_add ( rej, c->last_info_source_requested );
                tgdb_append_command(q, TGDB_ABSOLUTE_SOURCE_DENIED, rej );
            }

            break;
        default: break;
    }
}

int commands_prepare_for_command ( struct annotate_two *a2, struct commands *c, struct command *com ) {
    enum annotate_commands *a_com = ( enum annotate_commands *) com->client_data;

    /* Set the commands state to nothing */
    commands_set_state(c , VOID, NULL);

	/* The list command is no longer running */
    global_list_finished( a2->g);

    if ( global_list_had_error (a2->g) == TRUE && commands_get_state(c) == INFO_LIST )  {
        global_set_list_error ( a2->g, FALSE );
        return -1;
    }

    if ( a_com == NULL ) {
        data_set_state ( a2, USER_COMMAND );
        return 0;
    }

    switch ( *a_com ) {
        case ANNOTATE_INFO_SOURCES:
            commands_prepare_info_sources ( a2, c );
            break;
        case ANNOTATE_LIST:
            commands_prepare_list ( a2, c, com -> data );
            break;
        case ANNOTATE_INFO_SOURCE_RELATIVE:
            commands_prepare_info_source ( a2, c, INFO_SOURCE_RELATIVE );
            break;
        case ANNOTATE_INFO_SOURCE_ABSOLUTE:
            commands_prepare_info_source ( a2, c, INFO_SOURCE_ABSOLUTE );
            break;
        case ANNOTATE_INFO_BREAKPOINTS:
            commands_prepare_info_breakpoints (c);
            break;
        case ANNOTATE_TTY:
            break;  /* Nothing to do */
        case ANNOTATE_COMPLETE:
            commands_prepare_tab_completion ( a2, c );
            io_debug_write_fmt("<%s\n>", com->data);
            return 0;
            break;  /* Nothing to do */
        case ANNOTATE_VOID:
        default:
            err_msg ( "%s:%d commands_prepare_for_command error", __FILE__, __LINE__ );
            break;
    };
   
    data_set_state(a2, INTERNAL_COMMAND );
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
	struct commands *c,
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
                    c->last_info_source_requested[0] = '\0';
                else
                    strcpy ( c->last_info_source_requested, temp_file_name );

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
            strcpy ( c->last_tab_completion_command, data );
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

int commands_user_ran_command ( struct commands *c, struct queue *command_container ) {
    if ( commands_issue_command ( c, command_container, ANNOTATE_INFO_BREAKPOINTS, NULL, 0 ) == -1 ) {
        err_msg("%s:%d commands_issue_command error", __FILE__, __LINE__);
        return -1;
    }

    return 0;
}

int commands_issue_command ( 
		struct commands *c, 
		struct queue *command_container,
		enum annotate_commands com, 
		const char *data, 
		int oob) {
    const char *ncom = commands_create_command ( c, com, data );
    struct command *command = NULL;
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

	/* Append to the command_container the commands */
	queue_append ( command_container, command );

    return 0;
}
