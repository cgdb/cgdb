#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#if HAVE_STDLIB_H 
#include <stdlib.h>
#endif  /* HAVE_STDLIB_H */


#include "cgdbrc.h"
#include "command_lexer.h"
#include "tgdb.h"
#include "interface.h"
#include "error.h"
#include "tokenizer.h"
#include "cgdb.h"

extern struct tgdb *tgdb;

/**
 * The general idea is that the configuration will read in the users ~/.cgdbrc
 * file, or ~/.cgdb/config or whatever, and execute each command.  This will
 * also be responsible for processing all : commands in the tui.
 *
 */

extern int regex_icase ;
extern int shortcut_option ;
extern int line_coverage_option;
extern int highlight_tabstop;
extern int config_wrapscan;

enum ConfigType
{
    CONFIG_TYPE_BOOL, /* set ic / set noic */
    CONFIG_TYPE_INT,  /* set tabstop=8 */
    CONFIG_TYPE_STRING,
    CONFIG_TYPE_FUNC_VOID,
    CONFIG_TYPE_FUNC_BOOL,
    CONFIG_TYPE_FUNC_INT,
    CONFIG_TYPE_FUNC_STRING,
};

static int command_set_focus( const char *value );
static int command_set_winsplit( const char *value );
static int command_set_winminheight( int value );
static int command_set_syntax_type( const char *value );
static int command_set_esc_sequence_timeout( int msec );
static int command_set_stc ( int value );

static struct ConfigVariable
{
    const char *name, *s_name;
    enum ConfigType type;
    void *data;
} VARIABLES[] = {
    // keep this stuff sorted! !sort
    /* escdelay   */ 		{ "escdelay", "escdelay", CONFIG_TYPE_FUNC_INT, command_set_esc_sequence_timeout },
    /* focus      */ 		{ "focus", "fo", CONFIG_TYPE_FUNC_STRING, command_set_focus },
    /* ignorecase */ 		{ "ignorecase", "ic", CONFIG_TYPE_BOOL, &regex_icase },
    /* line_coverage */ 	{ "line_coverage", "lc", CONFIG_TYPE_BOOL, &line_coverage_option },
    /* shortcut   */ 		{ "shortcut", "sc", CONFIG_TYPE_BOOL, &shortcut_option },
	/* showtgdbcommands */ 	{ "showtgdbcommands", "stc", CONFIG_TYPE_FUNC_BOOL, &command_set_stc },
    /* syntax */      		{ "syntax", "syn", CONFIG_TYPE_FUNC_STRING, command_set_syntax_type }, 
    /* tabstop   */     	{ "tabstop", "ts", CONFIG_TYPE_INT, &highlight_tabstop },
    /* winminheight */  	{ "winminheight", "wmh", CONFIG_TYPE_FUNC_INT, &command_set_winminheight },
    /* winsplit */      	{ "winsplit", "winsplit", CONFIG_TYPE_FUNC_STRING, command_set_winsplit }, 
    /* wrapscan */          { "wrapscan", "ws", CONFIG_TYPE_BOOL, &config_wrapscan },
};

static int command_focus_cgdb( void );
static int command_focus_gdb( void );
static int command_focus_tty( void );

static int command_do_continue( void );
static int command_do_finish( void );
static int command_do_help( void );
static int command_do_next( void );
static int command_do_quit( void );
static int command_do_quit_force( void );
static int command_do_run( void );
static int command_do_step( void );
static int command_search_next( void );
static int command_start_tty( void );
static int command_toggle_tty( void );

static int command_parse_syntax( void );

typedef int (*action_t)(void);
static struct commands
{
    const char *name;
    // these functions will return 0 on success and 1 on error.  
    // Should the configuration file processing continue after an error?
    action_t action;
} COMMANDS[] = {
    // keep this stuff sorted, you can use !sort in vi
    /* continue    */ { "continue",    command_do_continue },
    /* finish      */ { "finish",      command_do_finish },
    /* help        */ { "help",        command_do_help },
    /* insert      */ { "insert",      command_focus_gdb },
    /* next        */ { "next",        command_do_next },
    /* q           */ { "q",           command_do_quit },
    /* q!          */ { "q!",          command_do_quit_force },
    /* quit        */ { "quit",        command_do_quit },
    /* quit!       */ { "quit!",       command_do_quit_force },
    /* run         */ { "run",         command_do_run },
    /* search_next */ { "search_next", command_search_next },
    /* start_tty   */ { "start_tty",   command_start_tty },
    /* step        */ { "step",        command_do_step },
    /* syntax      */ { "syntax",      command_parse_syntax },
    /* toggle_tty  */ { "toggle_tty",  command_toggle_tty },
    /* tty         */ { "tty",         command_focus_tty },
};

action_t get_command( const char *cmd )
{
    /* FIXME: replace with binary search */
    int i;
    for ( i = 0; i < (sizeof( COMMANDS )/sizeof( COMMANDS[0] )); ++i ) {
        if ( strcmp( cmd, COMMANDS[i].name ) == 0 ) {
            return COMMANDS[i].action;
        }
    }

    return NULL;
}

struct ConfigVariable* get_variable( const char *variable ) 
{
    /* FIXME: replace with binary search */
    int i;
    for ( i = 0; i < (sizeof( VARIABLES )/sizeof( VARIABLES[0])); ++i ) {
        if ( strcmp( variable, VARIABLES[i].name ) == 0 ||
             strcmp( variable, VARIABLES[i].s_name ) == 0 ) {
            return &VARIABLES[i];
        }
    }

    return NULL;
}

int command_set_focus( const char *value )
{
    if( strcasecmp( value, "cgdb" ) == 0 ) {
        command_focus_cgdb();
    } else if (strcasecmp( value, "gdb" ) == 0 ) {
        command_focus_gdb();
    } else if( strcasecmp( value, "tty" ) == 0 ) {
        command_focus_tty();
    } else {
        return 1;
    }

    return 0;
}

static int command_set_stc ( int value ) {
	if ( (value == 0) || (value == 1) ) 
		tgdb_set_verbose_gui_command_output ( tgdb, value );
	else
		return 1;

	return 0;
}

int command_set_esc_sequence_timeout( int msec )
{
	if ( msec >= 0 && msec <= 10000)
		cgdb_set_esc_sequence_timeout ( msec );

    return 0;
}

int command_set_winsplit( const char *value )
{
   if( strcasecmp( value, "top_big" ) == 0 ) {
      if_set_winsplit( WIN_SPLIT_TOP_BIG );
   } else if( strcasecmp( value, "top_full" ) == 0 ) {
      if_set_winsplit( WIN_SPLIT_TOP_FULL );
   } else if( strcasecmp( value, "bottom_big" ) == 0 ) {
      if_set_winsplit( WIN_SPLIT_BOTTOM_BIG );
   } else if( strcasecmp( value, "bottom_full" ) == 0 ) {
      if_set_winsplit( WIN_SPLIT_BOTTOM_FULL );
   } else {
      if_set_winsplit( WIN_SPLIT_EVEN );
   } // end if

   return 0;
}

static int command_set_winminheight( int value ) {
   if ( if_change_winminheight ( value ) == -1 )
      return -1;

    return 0;
}

int command_set_syntax_type ( const char *value )
{
	enum tokenizer_language_support l = TOKENIZER_LANGUAGE_UNKNOWN;

	if( strcasecmp( value, "c" ) == 0 ) {
		l = TOKENIZER_LANGUAGE_C;
	} else if( strcasecmp( value, "ada" ) == 0 ) {
		l = TOKENIZER_LANGUAGE_ADA;
	} else if( strcasecmp( value, "off" ) == 0 ) {
		l = TOKENIZER_LANGUAGE_UNKNOWN;
	}
	
	if_highlight_sviewer ( l );

   	return 0;
}

int command_focus_cgdb( void )
{
    if_set_focus( CGDB );
    return 0;
}

int command_focus_gdb( void )
{
    if_set_focus( GDB );
    return 0;
}

int command_focus_tty( void )
{
    if_set_focus( TTY );
    return 0;
}

int command_do_continue( void )
{
    if ( tgdb_run_debugger_command ( tgdb, TGDB_CONTINUE ) == -1 ) 
		return -1;

    return 0;
}

int command_do_finish( void )
{
    if ( tgdb_run_debugger_command ( tgdb, TGDB_FINISH ) == -1 )
		return -1;

    return 0;
}

int command_do_help( void )
{
    if_display_help();
    return 0;
}

int command_do_next( void )
{
    if ( tgdb_run_debugger_command  ( tgdb, TGDB_NEXT ) == -1 )
		return -1;

    return 0;
}

int command_do_quit( void )
{
    /* FIXME: Test to see if debugged program is still running */
    cleanup();
    exit(0);
    return 0;
}

int command_do_quit_force( void )
{
    cleanup();
    exit(0);
    return 0;
}

int command_do_run( void )
{
    /* FIXME: see if there are any other arguments to pass to the run command */
    if ( tgdb_run_debugger_command  ( tgdb, TGDB_RUN ) == -1 )
		return -1;

    return 0;
}

int command_do_step( void )
{
    if ( tgdb_run_debugger_command  ( tgdb, TGDB_STEP ) == -1 )
		return -1;

    return 0;
}

int command_search_next( void )
{
    if_search_next();
    return 0;
}

int command_start_tty( void )
{
    /* FIXME: Find out what this is supposed to do and implement */
    return 1;
//    if ( tgdb_new_tty() == -1 ) {
//        return 1;
//    } else {
//        /* FIXME: interface does scr_free( tty_win ), tty_win = NULL, if_layout(); */
//        return 0;
//    }
}

int command_toggle_tty( void )
{
    if_tty_toggle();
    return 0;
}

int command_parse_syntax( void )
{
  /* This is something like: 
     :syntax
     :syntax on
     :syntax off
  */

  int rv = 1;
  switch( (rv = yylex()) ) {
  case EOL: {
    /* TODO: Print out syntax info (like vim?) */
  } break;
  case BOOLEAN: 
  case IDENTIFIER: {
    extern int sources_syntax_on;
    const char *value = get_token();
    if( strcasecmp( value, "on" ) == 0 || strcasecmp( value, "yes") == 0 )
        sources_syntax_on = 1;
    else if ( strcasecmp( value, "no" ) == 0 || strcasecmp( value, "off") == 0 )
        sources_syntax_on = 0;

    if_draw(); 
  } break;
  default:
    break;
  }

  return 0;
}

int command_parse_set( void )
{
    /* commands could look like the following:
     * set ignorecase
     * set noignorecase
     * set focus=gdb
     * set tabstop=8
     */

    int rv = 1;
    int boolean = 1;
    const char * value = NULL;
    
    switch ( (rv = yylex()) ) {
    case EOL: {
        /* TODO: Print out all the variables that have been set. */
    } break;
    case IDENTIFIER: {
        const char *token = get_token();
        int length = strlen( token );
        struct ConfigVariable *variable = NULL;

        if ( length > 2 && token[0] == 'n' && token[1] == 'o' ) {
            value = token + 2;
            boolean = 0;
        } else {
            value = token;
        }

        if ( (variable = get_variable( value )) != NULL ) {
            rv = 0;
            if( boolean == 0 &&
                variable->type != CONFIG_TYPE_BOOL ) {
                /* this is an error, you cant' do:
                 * set notabstop 
                 */
                rv = 1;
            }

            switch( variable->type ) {
            case CONFIG_TYPE_BOOL:
                *(int*)(variable->data) = boolean;
                break;
            case CONFIG_TYPE_INT: {
                if( yylex() == '=' &&
                    yylex() == NUMBER ) {
                    int data = strtol( get_token(), NULL, 10 );
                    *(int*)(variable->data) = data;
                } else {
                    rv = 1;
                }
            } break;
            case CONFIG_TYPE_STRING: {
                if( yylex() == '=' &&
                   (rv = yylex(), rv == STRING || rv == IDENTIFIER) ) {
                    /* BAM! comma operator */
                    char * data = (char*)get_token();
                    if( rv == STRING ) {
                        /* get rid of quotes */
                        data = data + 1;
                        data[ strlen( data ) - 1 ] = '\0';
                    } 
                    if( variable->data ) 
                    { free( variable->data ); }
                    (char *)variable->data = strdup( data );
                } else {
                    rv = 1;
                }
            } break;
            case CONFIG_TYPE_FUNC_VOID: {
                int(*functor)( void ) = (int(*)(void))variable->data;
                if( functor )
                { rv = functor(); }
                else 
                { rv = 1; }
            } break;
            case CONFIG_TYPE_FUNC_BOOL: {
                int (*functor)( int ) = (int(*)(int))variable->data;
                    if( functor )
                    { rv = functor( boolean ); }
                    else
                    { rv = 1; }
            } break;
            case CONFIG_TYPE_FUNC_INT: {
                int (*functor)( int ) = (int(*)(int))variable->data;
                if( yylex() == '=' &&
                    yylex() == NUMBER ) {
                    int data = strtol( get_token(), NULL, 10 );
                    if( functor )
                    { rv = functor( data ); }
                    else
                    { rv = 1; }
                } else {
                    rv = 1;
                }
            } break;
            case CONFIG_TYPE_FUNC_STRING: {
                int (*functor)( const char* ) = (int(*)(const char*))variable->data;
                if( yylex() == '=' &&
                   (rv = yylex(), rv == STRING || rv == IDENTIFIER) ) {
                    /* BAM! comma operator */
                    char * data = (char*)get_token();
                    if( rv == STRING ) {
                        /* get rid of quotes */
                        data = data + 1;
                        data[ strlen( data ) - 1 ] = '\0';
                    } 
                    if( functor )
                    { rv = functor(data); }
                    else
                    { rv = 1; }
                } else 
                { rv = 1; }
            } break;
            default: 
                rv = 1;
                break;
            }
        }
    } break;
    default:
        break;
    }

    return rv;
}

int command_parse_string( const char *buffer )
{
    typedef struct yy_buffer_state* YY_BUFFER_STATE;
    extern YY_BUFFER_STATE yy_scan_string( const char *yy_str );
    extern void yy_delete_buffer( YY_BUFFER_STATE state );
    int rv = 1;
    YY_BUFFER_STATE state = yy_scan_string( (char*)buffer );

    switch ( yylex() ) {
    case SET:
        /* get the next token */
        rv = command_parse_set();
        break;

    case UNSET:
    case BIND:
    case MACRO:
        /* ignore this stuff for now. */
        rv = 1;
        break;

    case NUMBER: {
        const char *number = get_token();
        if ( number[0] == '+' ) {
            source_vscroll( if_get_sview(), atoi( number+1 ));
            rv = 0;
        } else if ( number[0] == '-' ) {
            source_vscroll( if_get_sview(), -atoi( number+1 ));
            rv = 0;
        } else {
            source_set_sel_line(if_get_sview(), atoi( number ));
            rv = 0;
        }
        if_draw();
    } break;

    case IDENTIFIER: {
        action_t action = get_command( get_token() );
        if ( action ) {
            action();
            rv = 0;
        } else {
            rv = 1;
        }
    } break;

    case EOL: 
        /* basically just an empty line, don't do nothin. */
        rv = 0;
        break;

    default:
        rv = 1;
        break;
    }

    yy_delete_buffer( state );
    return rv;
}

int command_parse_file( FILE *fp )
{
    char buffer[4096];
    char *p = buffer;
    int linenumber = 0;

    while( linenumber++, fgets( p, sizeof(buffer) - ( p - buffer ), fp ) )
    {
        int bufferlen = strlen( buffer );
        if( buffer[bufferlen - 2] == '\\' ) 
        {
            /* line continuation character, read another line into the buffer */
            linenumber--;
            p = buffer + bufferlen - 2;
            continue;
        }

        if( command_parse_string( buffer ) )
        {
            /* buffer already has an \n */
            if_print_message ("Error parsing line %d: %s", linenumber, buffer );
            /* return -1; don't return, lets keep parsing the file. */
        }

        p = buffer;
    }

    return 0;
}
