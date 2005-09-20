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
#include "tgdb_types.h"
#include "globals.h"
#include "logger.h"
#include "sys_util.h"
#include "queue.h"
#include "ibuf.h"
#include "a2-tgdb.h"
#include "queue.h"
#include "tgdb_list.h"
#include "annotate_two.h"

/**
 * This structure represents most of the I/O parsing state of the 
 * annotate_two subsytem.
 */
struct commands {

	/**
	 * The current absolute path the debugger is at in the inferior.
	 */
	struct ibuf *absolute_path;

	/**
	 * The current line number the debugger is at in the inferior.
	 */
	struct ibuf *line_number;

	/**
	 * The state of the command context.
	 */
	enum COMMAND_STATE cur_command_state;

	/**
	 * This is related to parsing the breakpoint annotations.
	 * It keeps track of the current field we are in.
	 */
	int cur_field_num;

	/**
	 * This is a flag to let us know when the 5th field has been hit.
	 */
    int field_5_newline_hit;

	/** breakpoint information */
	//@{

	/**
	 * A list of breakpoints already parsed.
	 */
	struct tgdb_list *breakpoint_list;

	/**
	 * The current breakpoint being parsed.
	 */
	struct ibuf *breakpoint_string;

	/**
	 * ???
	 */
	int breakpoint_table;

	/**
	 * If the current breakpoint is enabled
	 */
	int breakpoint_enabled;

	/**
	 * ???
	 */
	int breakpoint_started;

	//@}
	
	/** 'info source' information */

	//@{

	/** 
	 * The current info source line being parsed 
	 */
	struct ibuf *info_source_string;

	/** 
	 * Finished parsing the line being looked for.
	 */
	int info_source_ready;

	/**
	 * The name of the file requested to have 'info source' run on.
	 */
	struct ibuf *last_info_source_requested;

	//@}
	
	// info sources information {{{
	//@{

	/** 
	 * ??? Finished parsing the data being looked for.
	 */
	int sources_ready;

	/**
	 * All of the sources.
	 */
	struct ibuf *info_sources_string;

	/**
	 * All of the source, parsed in put in a list, 1 at a time.
	 */
	struct tgdb_list *inferior_source_files;

	//@}
	// }}}
	
	// tab completion information {{{
	//@{

	/** 
	 * ??? Finished parsing the data being looked for.
	 */
	int tab_completion_ready;

	/**
	 * A tab completion item
	 */
	struct ibuf *tab_completion_string;

	/**
	 * All of the tab completion items, parsed in put in a list, 1 at a time.
	 */
	struct tgdb_list *tab_completions;

	//@}
	// }}}

	/** 
	 * The absolute path prefix output by GDB when 'info source' is given
	 */
	const char *source_prefix;

	/** 
	 * The length of the line above.
	 */
	int source_prefix_length;

	/** 
	 * The relative path prefix output by GDB when 'info source' is given
	 */
	const char *source_relative_prefix;

	/** 
	 * The length of the line above.
	 */
	int source_relative_prefix_length;
};

struct commands *commands_initialize(void) {
	struct commands *c = (struct commands *)
		xmalloc ( sizeof ( struct commands ));

    c->absolute_path       	= ibuf_init();
    c->line_number         	= ibuf_init();

	c->cur_command_state 	= VOID_COMMAND;
	c->cur_field_num 		   = 0;
   c->field_5_newline_hit  = 0;

    c->breakpoint_list    	= tgdb_list_init();
    c->breakpoint_string   	= ibuf_init();
	c->breakpoint_table    	= 0;
	c->breakpoint_enabled   = FALSE;
	c->breakpoint_started   = FALSE;

    c->info_source_string  	= ibuf_init();
	c->info_source_ready   	= 0;
	c->last_info_source_requested = ibuf_init();

	c->sources_ready 		= 0;
    c->info_sources_string 	= ibuf_init();
	c->inferior_source_files= tgdb_list_init ();

	c->tab_completion_ready 		= 0;
    c->tab_completion_string 	= ibuf_init();
	c->tab_completions= tgdb_list_init ();

	c->source_prefix 		= "Located in ";
	c->source_prefix_length = 11;

	c->source_relative_prefix 	= "Current source file is ";
	c->source_relative_prefix_length 	= 23;

	return c;
}

int free_breakpoint (void *item) {
  struct tgdb_breakpoint *bp = (struct tgdb_breakpoint *)item;
  if (bp->file) {
    free (bp->file);
    bp->file = NULL;
  }

  if (bp->funcname) {
    free (bp->funcname);
    bp->funcname = NULL;
  }

  free (bp);
  bp = NULL;

  return 0;
}

int free_char_star (void *item) {
  char *s = (char *)item;

  free (s);
  s = NULL;

  return 0;
}

void commands_shutdown ( struct commands *c ) {
	if ( c == NULL )
		return;

	ibuf_free ( c->absolute_path );
	c->absolute_path = NULL;

	ibuf_free ( c->line_number );
	c->line_number = NULL;

	tgdb_list_free (c->breakpoint_list, free_breakpoint);
	tgdb_list_destroy (c->breakpoint_list);

	ibuf_free ( c->breakpoint_string );
	c->breakpoint_string = NULL;

	ibuf_free ( c->info_source_string );
	c->info_source_string = NULL;

	ibuf_free ( c->info_sources_string );
	c->info_sources_string = NULL;

	tgdb_list_free (c->inferior_source_files, free_char_star);
	tgdb_list_destroy (c->inferior_source_files);
	
	/* TODO: free source_files queue */

	free ( c );
	c = NULL;
}

int commands_parse_field(struct commands *c, const char *buf, size_t n, int *field){
   if(sscanf(buf, "field %d", field) != 1)
      logger_write_pos ( logger, __FILE__, __LINE__, "parsing field annotation failed (%s)\n", buf);

   return 0;
}

/* source filename:line:character:middle:addr */
int commands_parse_source(
		struct commands *c, 
		struct tgdb_list *client_command_list,
		const char *buf, size_t n, 
		struct tgdb_list *list){
    int i = 0;
    char copy[n+1];
    char *cur = copy + n;
	struct ibuf *file = ibuf_init (), *line = ibuf_init ();
    strncpy(copy, buf, n+1); /* modify local copy */
   
    while(cur != copy && i <= 3){
        if(*cur == ':'){
            if(i == 3) {
				int length = strlen ( cur + 1 );
				char *temp = xmalloc ( sizeof ( char ) * ( length + 1 ) );
				
                if(sscanf(cur + 1, "%s", temp) != 1) 
                    logger_write_pos ( logger, __FILE__, __LINE__, "Could not get line number");

				ibuf_add ( line, temp );
				free ( temp );
				temp = NULL;
			}

            *cur = '\0';
            ++i; 
        }
        --cur;
    } /* end while */
     
	/*TODO: I don't think this will work with filenames that contain spaces.
	 * It should be changed. Look at the algorithm in the function below.
	 */
	{
		int length = strlen ( copy );
		char *temp = xmalloc ( sizeof ( char ) * ( length + 1 ) );

		if(sscanf(copy, "source %s", temp) != 1)
        logger_write_pos ( logger, __FILE__, __LINE__, "Could not get file name");
   
		ibuf_add ( file, temp );
		free ( temp );
		temp = NULL;
	}
   
    ibuf_clear( c->absolute_path );
    ibuf_add ( c->absolute_path, ibuf_get ( file ) );
    ibuf_clear( c->line_number );
    ibuf_add ( c->line_number, ibuf_get ( line ) );

	ibuf_free ( file );
	ibuf_free ( line );

    /* set up the info_source command to get the relative path */
    if ( commands_issue_command ( 
				c, 
				client_command_list,
				ANNOTATE_INFO_SOURCE_RELATIVE, 
				NULL, 
				1 ) == -1 ) {
        logger_write_pos ( logger, __FILE__, __LINE__, "commands_issue_command error");
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

static void parse_breakpoint(
		struct commands *c ) {
    unsigned long size = ibuf_length(c->breakpoint_string);
    char copy[size + 1];
    char *cur = copy + size, *fcur;
	struct ibuf *fname, *file, *line;
    static char *info_ptr; 
    struct tgdb_breakpoint *tb;

	fname = ibuf_init ();
	file  = ibuf_init ();
	line  = ibuf_init ();

    info_ptr = ibuf_get(c->breakpoint_string);

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
			int length = strlen ( cur + 1 );
			char *temp = xmalloc ( sizeof ( char ) * ( length + 1 ) );

            if(sscanf(cur + 1, "%s", temp) != 1)
                logger_write_pos ( logger, __FILE__, __LINE__, "Could not get line number");

			ibuf_add ( line, temp );
			free ( temp );
			temp = NULL;

            *cur = '\0';
            break; /* in case of multiple ':' in the line */
        } 

        --cur;
    } /* end while */

    /* Assertion: The string to parse now looks like '[io]n .* at .*'*/
    if ( !((copy[0] == 'i' || copy[0] == 'o') && copy[1] == 'n' && copy[2] == ' ') ){
      logger_write_pos ( logger, __FILE__, __LINE__, "Could not scan function and file name\n"
              "\tWas the program compiled with debug info?");
    }

    fcur = &copy[3];

    /* Assertion: The string to parse now looks like '.* at .*'*/
    if ( ( cur = strstr(fcur, " at " )) == NULL ) {
      logger_write_pos ( logger, __FILE__, __LINE__, "Could not scan function and file name\n"
              "\tWas the program compiled with debug info?");
    }

    *cur='\0';
	ibuf_add ( fname, fcur );

    /* Assertion: The string to parse now looks like ' at .*'*/
    cur += 4;
    /* Assertion: The string to parse now looks like '.*' */
	ibuf_add ( file, cur );

    tb = ( struct tgdb_breakpoint*) xmalloc ( sizeof ( struct tgdb_breakpoint) );
    tb->file = strdup ( ibuf_get ( file ) );
    tb->funcname = strdup ( ibuf_get ( fname ) );
    sscanf (ibuf_get ( line ), "%d", &tb->line);

    if(c->breakpoint_enabled == TRUE)
        tb->enabled = 1;
    else
        tb->enabled = 0;

    tgdb_list_append ( c->breakpoint_list, tb );

	ibuf_free ( fname );
	fname = NULL;

	ibuf_free ( file );
	file = NULL;

	ibuf_free ( line );
	line = NULL;
}


void commands_set_state( 
	struct commands *c, 
	enum COMMAND_STATE state, 
	struct tgdb_list *list){
    c->cur_command_state = state;

    switch(c->cur_command_state){
        case RECORD:  
            if(ibuf_length(c->breakpoint_string) > 0){
                parse_breakpoint ( c );
                ibuf_clear(c->breakpoint_string);
                c->breakpoint_enabled = FALSE;
            }
        break;
        case BREAKPOINT_TABLE_END:  
            if(ibuf_length(c->breakpoint_string) > 0)
                parse_breakpoint ( c );

            /* At this point, annotate needs to send the breakpoints to the gui.
             * All of the valid breakpoints are stored in breakpoint_queue. */
            tgdb_types_append_command ( 
						list, 
						TGDB_UPDATE_BREAKPOINTS, 
						c->breakpoint_list );

            ibuf_clear(c->breakpoint_string);
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
      ibuf_clear(c->breakpoint_string);
      c->field_5_newline_hit = 0;
   }
}

enum COMMAND_STATE commands_get_state( struct commands *c){
   return c->cur_command_state;
}

static void commands_prepare_info_source(struct annotate_two *a2, struct commands *c, enum COMMAND_STATE state){
   data_set_state(a2, INTERNAL_COMMAND );
   ibuf_clear(c->info_source_string);
   
   if ( state == INFO_SOURCE_ABSOLUTE ) {
        commands_set_state(c, INFO_SOURCE_ABSOLUTE, NULL);
   } else if ( state == INFO_SOURCE_RELATIVE )
        commands_set_state(c, INFO_SOURCE_RELATIVE, NULL);

   c->info_source_ready = 0;
}

void commands_list_command_finished (
	struct commands *c, 
	struct tgdb_list *list, 
	int success){
  /* The file does not exist and it can not be opened.
   * So we return that information to the gui.  */
	struct tgdb_source_file *rejected = (struct tgdb_source_file *)
	  xmalloc ( sizeof ( struct tgdb_source_file ) );

	if ( c->last_info_source_requested == NULL ) 
		rejected->absolute_path = NULL;
	else
		rejected->absolute_path = strdup ( ibuf_get ( c->last_info_source_requested ) );

    tgdb_types_append_command ( list, TGDB_ABSOLUTE_SOURCE_DENIED, rejected);
}

void commands_send_source_absolute_source_file (
	struct commands *c, 
	struct tgdb_list *list){
   /*err_msg("Whats up(%s:%d)\r\n", info_source_buf, info_source_buf_pos);*/
    unsigned long length = ibuf_length(c->info_source_string);
    static char *info_ptr; 
    info_ptr = ibuf_get(c->info_source_string);

   /* found */
   if(length >= c->source_prefix_length && 
      (strncmp(info_ptr, c->source_prefix, c->source_prefix_length) == 0)){
      char *path = info_ptr + c->source_prefix_length;

      /* requesting file */
      if(c->last_info_source_requested == NULL) {
		  	/* This happens only when libtgdb starts */
            ibuf_clear( c->absolute_path );
            ibuf_add ( c->absolute_path, path );
            ibuf_clear( c->line_number );
            ibuf_add ( c->line_number, "1" );
      } else { 
		  struct tgdb_source_file *tsf = (struct tgdb_source_file *)
			  xmalloc ( sizeof ( struct tgdb_source_file ) );
		  tsf->absolute_path = strdup ( path );
          tgdb_types_append_command(list, TGDB_ABSOLUTE_SOURCE_ACCEPTED, tsf);
      }
   /* not found */
   } else {
	  struct tgdb_source_file *rejected = (struct tgdb_source_file *)
		  xmalloc ( sizeof ( struct tgdb_source_file ) );

	  if ( c->last_info_source_requested == NULL )
		  rejected->absolute_path = NULL;
	  else
		  rejected->absolute_path = strdup ( ibuf_get ( c->last_info_source_requested ) );
      tgdb_types_append_command(list, TGDB_ABSOLUTE_SOURCE_DENIED, rejected);
   }
}

static void commands_process_source_line(struct commands *c ){
    unsigned long length = ibuf_length(c->info_sources_string), i, start = 0;
    static char *info_ptr; 
    static char *nfile;
    info_ptr = ibuf_get(c->info_sources_string);
  
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
static void commands_process_info_source (
		struct commands *c, 
		struct tgdb_list *list, 
		char a){
    unsigned long length;
    static char *info_ptr;

    if ( c->info_source_ready  ) /* Already found */
        return;
    
    info_ptr = ibuf_get(c->info_source_string);
    length   = ibuf_length(c->info_source_string);

    if ( a == '\r' )
        return;

    if(a == '\n'){ 
        /* This is the line containing the absolute path to the source file */
        if ( commands_get_state(c) == INFO_SOURCE_ABSOLUTE && 
             length >= c->source_prefix_length && 
             strncmp(info_ptr, c->source_prefix, c->source_prefix_length) == 0 ) {

             commands_send_source_absolute_source_file ( c, list );

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
				tfp->absolute_path = strdup ( ibuf_get ( c->absolute_path ) );
				tfp->relative_path = strdup ( info_ptr + c->source_relative_prefix_length );
				tfp->line_number   = atoi ( ibuf_get ( c->line_number ) );

                tgdb_types_append_command(list, TGDB_UPDATE_FILE_POSITION, tfp );
            }

            c->info_source_ready = 1;
        } else
            ibuf_clear(c->info_source_string);
    } else
        ibuf_addchar(c->info_source_string, a);
}

/* process's source files */
static void commands_process_sources(struct commands *c, char a){
    static const char *sourcesReadyString = "Source files for which symbols ";
    static const int sourcesReadyStringLength = 31;
    static char *info_ptr;
    ibuf_addchar(c->info_sources_string, a);
   
    if(a == '\n'){
        ibuf_delchar(c->info_sources_string);     /* remove '\n' and null terminate */
        /* valid lines are 
         * 1. after the first line,
         * 2. do not end in ':' 
         * 3. and are not empty 
         */
        info_ptr = ibuf_get(c->info_sources_string);

        if ( strncmp(info_ptr, sourcesReadyString, sourcesReadyStringLength) == 0 )
            c->sources_ready = 1;

        /* is this a valid line */
        if(ibuf_length(c->info_sources_string) > 0 && c->sources_ready && info_ptr[ibuf_length(c->info_sources_string) - 1] != ':')
            commands_process_source_line(c); 

        ibuf_clear(c->info_sources_string);
    }
}

static void 
commands_process_completion(struct commands *c ){
  const char *ptr = ibuf_get (c->tab_completion_string);
  tgdb_list_append (c->tab_completions, strdup (ptr));
}

/* process's source files */
static void 
commands_process_complete (struct commands *c, char a)
{
  ibuf_addchar(c->tab_completion_string, a);
   
  if(a == '\n'){
    ibuf_delchar(c->tab_completion_string); /* remove '\n' and null terminate */

    if (ibuf_length (c->tab_completion_string) > 0)
      commands_process_completion(c); 

    ibuf_clear(c->tab_completion_string);
  }
}

void commands_free(struct commands *c, void *item) {
    free((char*)item);
}

void commands_send_gui_sources (
		struct commands *c, 
		struct tgdb_list *list){
  /* If the inferior program was not compiled with debug, then no sources
   * will be available. If no sources are available, do not return the
   * TGDB_UPDATE_SOURCE_FILES command. */
  if (tgdb_list_size ( c->inferior_source_files ) > 0)
    tgdb_types_append_command (list, TGDB_UPDATE_SOURCE_FILES, c->inferior_source_files);
}


void commands_send_gui_completions (
		struct commands *c, 
		struct tgdb_list *list){
  /* If the inferior program was not compiled with debug, then no sources
   * will be available. If no sources are available, do not return the
   * TGDB_UPDATE_SOURCE_FILES command. */
//  if (tgdb_list_size ( c->tab_completions ) > 0)
    tgdb_types_append_command (list, TGDB_UPDATE_COMPLETIONS, c->tab_completions);
}

void commands_process ( struct commands *c, char a, struct tgdb_list *list){
    if(commands_get_state(c) == INFO_SOURCES){
        commands_process_sources(c, a);     
    } else if(commands_get_state(c) == COMPLETE){
        commands_process_complete(c, a);     
    } else if(commands_get_state(c) == INFO_LIST){
        /* do nothing with data */
    } else if(commands_get_state(c) == INFO_SOURCE_ABSOLUTE 
            ||commands_get_state(c) == INFO_SOURCE_RELATIVE){
        commands_process_info_source(c, list, a);   
    } else if(c->breakpoint_table && c->cur_command_state == FIELD && c->cur_field_num == 5){ /* the file name and line num */ 
        if ( a == '\n' || a == '\r' )
            c->field_5_newline_hit = 1;
        
        if ( !c->field_5_newline_hit )
            ibuf_addchar(c->breakpoint_string, a);
    } else if(c->breakpoint_table && c->cur_command_state == FIELD && c->cur_field_num == 3 && a == 'y') {
        c->breakpoint_enabled = TRUE;
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
    ibuf_clear(c->breakpoint_string);
}

/* commands_prepare_tab_completion:
 * --------------------------------
 *
 * This prepares the tab completion command
 */
static void commands_prepare_tab_completion ( struct annotate_two *a2, struct commands *c ) {
  c->tab_completion_ready = 0;
  ibuf_clear(c->tab_completion_string);
  commands_set_state(c, COMPLETE, NULL);
  global_set_start_completion ( a2->g );
}

/* commands_prepare_info_sources: 
 * ------------------------------
 *
 *  This prepares the command 'info sources' by setting certain variables.
 */
static void commands_prepare_info_sources ( struct annotate_two *a2, struct commands *c ){
    c->sources_ready = 0;
    ibuf_clear(c->info_sources_string);
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

void commands_finalize_command ( 
		struct commands *c, 
		struct tgdb_list *list ) {
    switch ( commands_get_state (c) ) {
        case INFO_SOURCE_RELATIVE:
        case INFO_SOURCE_ABSOLUTE:

            if ( c->info_source_ready == 0 ) {
				struct tgdb_source_file *rejected = (struct tgdb_source_file *)
				  xmalloc ( sizeof ( struct tgdb_source_file ) );
				if ( c->last_info_source_requested == NULL )
					rejected->absolute_path = NULL;
				else
					rejected->absolute_path = strdup ( ibuf_get ( c->last_info_source_requested ) );
                tgdb_types_append_command(list, TGDB_ABSOLUTE_SOURCE_DENIED, rejected );
            }

            break;
        default: break;
    }
}

int commands_prepare_for_command ( 
		struct annotate_two *a2, 
		struct commands *c, 
		struct tgdb_client_command *com ) {

    enum annotate_commands *a_com = ( enum annotate_commands *) com->tgdb_client_private_data;

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
            commands_prepare_list ( a2, c, com->tgdb_client_command_data );
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
            io_debug_write_fmt("<%s\n>", com->tgdb_client_command_data);
            break;  /* Nothing to do */
	case ANNOTATE_INFO_SOURCE:
	case ANNOTATE_SET_PROMPT:
        case ANNOTATE_VOID:
			break;
        default:
            logger_write_pos ( logger, __FILE__, __LINE__, "commands_prepare_for_command error");
            break;
    };
   
    data_set_state(a2, INTERNAL_COMMAND );
    io_debug_write_fmt("<%s\n>", com->tgdb_client_command_data);

    return 0;
}

/** 
 * This is responsible for creating a command to run through the debugger.
 *
 * \param com 
 * The annotate command to run
 *
 * \param data 
 * Information that may be needed to create the command
 *
 * \return
 * A command ready to be run through the debugger or NULL on error.
 * The memory is malloc'd, and must be freed.
 */
static char *commands_create_command ( 
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
				struct ibuf *temp_file_name = NULL;
                if ( data != NULL ) {
					temp_file_name = ibuf_init ();
					ibuf_add ( temp_file_name, data );
				}

                if ( data == NULL )
                    ncom = (char *)xmalloc( sizeof ( char ) * ( 16 ));
                else
                    ncom = (char *)xmalloc( sizeof ( char ) * ( 16 + strlen (data )));
                strcpy ( ncom, "server list " );

                /* This should only happen for the initial 'list' */
                if ( temp_file_name != NULL ) {
                    strcat ( ncom, ibuf_get ( temp_file_name ) );
                    strcat ( ncom, ":1" );
                } 

                if ( temp_file_name == NULL ) {
					ibuf_free ( c->last_info_source_requested );
					c->last_info_source_requested = NULL;
				} else {
					if ( c->last_info_source_requested == NULL )
						c->last_info_source_requested = ibuf_init ();

					ibuf_clear ( c->last_info_source_requested );
					ibuf_add ( c->last_info_source_requested, ibuf_get ( temp_file_name ) );
				}

                strcat ( ncom, "\n" );

				ibuf_free ( temp_file_name );
				temp_file_name = NULL;
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
				struct ibuf *temp_tty_name = ibuf_init ();
				ibuf_add ( temp_tty_name, data );
                ncom = (char *)xmalloc( sizeof ( char ) * ( 13 + strlen( data )));
                strcpy ( ncom, "server tty " );
                strcat ( ncom, ibuf_get  (temp_tty_name ) );
                strcat ( ncom, "\n" );

				ibuf_free ( temp_tty_name );
				temp_tty_name = NULL;
                break;
            }
        case ANNOTATE_COMPLETE:
            ncom = (char *)xmalloc( sizeof ( char ) * ( 18 + strlen( data )));
            strcpy ( ncom, "server complete " );
            strcat ( ncom, data );
            strcat ( ncom, "\n" );
            break;
        case ANNOTATE_SET_PROMPT:
            ncom = strdup ( data );   
            break;
        case ANNOTATE_VOID:
        default: 
            logger_write_pos ( logger, __FILE__, __LINE__, "switch error");
            break;
    };

    return ncom;
}

int commands_user_ran_command ( 
		struct commands *c, 
		struct tgdb_list *client_command_list ) {
    if ( commands_issue_command ( 
				c, 
				client_command_list, 
				ANNOTATE_INFO_BREAKPOINTS, NULL, 0 ) == -1 ) {
        logger_write_pos ( logger, __FILE__, __LINE__, "commands_issue_command error");
        return -1;
    }

    return 0;
}

int commands_issue_command ( 
		struct commands *c, 
		struct tgdb_list *client_command_list,
		enum annotate_commands com, 
		const char *data, 
		int oob) {
    char *ncom = commands_create_command ( c, com, data );
    struct tgdb_client_command *client_command = NULL;
    enum annotate_commands *nacom = (enum annotate_commands *)xmalloc ( sizeof ( enum annotate_commands ) );
    
    *nacom = com;

    if ( ncom == NULL ) {
        logger_write_pos ( logger, __FILE__, __LINE__, "commands_issue_command error");
        return -1;
    }

    /* This should send the command to tgdb-base to handle */ 
	if ( oob == 0 ) {
        client_command = tgdb_client_command_create ( 
                ncom,
                TGDB_CLIENT_COMMAND_NORMAL,
                TGDB_CLIENT_COMMAND_DISPLAY_NOTHING,
                TGDB_CLIENT_COMMAND_ACTION_NONE,
                (void*) nacom ); 
	} else if ( oob == 1 ) {
        client_command = tgdb_client_command_create ( 
                ncom,
                TGDB_CLIENT_COMMAND_PRIORITY,
                TGDB_CLIENT_COMMAND_DISPLAY_NOTHING,
				TGDB_CLIENT_COMMAND_ACTION_NONE,
                (void*) nacom ); 
    } else if ( oob == 2 ) {
        client_command = tgdb_client_command_create ( 
                ncom,
                TGDB_CLIENT_COMMAND_TGDB_BASE,
                TGDB_CLIENT_COMMAND_DISPLAY_NOTHING,
                TGDB_CLIENT_COMMAND_ACTION_CONSOLE_SET_PROMPT,
                NULL ); 
    } else if ( oob == 4 ) {
        client_command = tgdb_client_command_create ( 
                ncom,
                TGDB_CLIENT_COMMAND_NORMAL,
                TGDB_CLIENT_COMMAND_DISPLAY_NOTHING,
                TGDB_CLIENT_COMMAND_ACTION_NONE,
                (void*) nacom ); 
    }

    if (ncom) {
      free (ncom);
      ncom = NULL;
    }

	/* Append to the command_container the commands */
	tgdb_list_append ( client_command_list, client_command );

    return 0;
}
