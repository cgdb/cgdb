#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#if HAVE_STDLIB_H 
#include <stdlib.h>
#endif  /* HAVE_STDLIB_H */


#include "commands.h"
#include "command_lexer.h"
#include "tgdb.h"
#include "interface.h"
#include "error.h"

/**
 * The general idea is that the configuration will read in the users ~/.cgdbrc
 * file, or ~/.cgdb/config or whatever, and execute each command.  This will
 * also be responsible for processing all : commands in the tui.
 *
 */

extern int regex_icase ;
extern int shortcut_option ;
extern int line_coverage_option;


static struct variables
{
    const char * name, *s_name;
    int * variable;
} VARIABLES[] = {
    // keep this stuff sorted! !sort
    /* ignorecase */ 	{ "ignorecase", "ic", &regex_icase },
    /* line coverage */ { "line_coverage", "lc", &line_coverage_option },
    /* shortcut   */ 	{ "shortcut", "sc", &shortcut_option },
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

int * get_variable( const char *variable ) 
{
    /* FIXME: replace with binary search */
    int i;
    for ( i = 0; i < (sizeof( VARIABLES )/sizeof( VARIABLES[0])); ++i ) {
        if ( strcmp( variable, VARIABLES[i].name ) == 0 ||
             strcmp( variable, VARIABLES[i].s_name ) == 0 ) {
            return VARIABLES[i].variable;
        }
    }

    return NULL;
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
    if_print(tgdb_send( "continue", 2 ));
    return 0;
}

int command_do_finish( void )
{
    if_print(tgdb_send( "finish", 2 ));
    return 0;
}

int command_do_help( void )
{
    if_display_help();
    return 0;
}

int command_do_next( void )
{
    if_print(tgdb_send( "next", 2 ));
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
    if_print(tgdb_send( "run", 2 ));
    return 0;
}

int command_do_step( void )
{
    if_print(tgdb_send( "step", 2 ));
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
    if ( tgdb_new_tty() == -1 ) {
        return 1;
    } else {
        /* FIXME: interface does scr_free( tty_win ), tty_win = NULL, if_layout(); */
        return 0;
    }
}

int command_toggle_tty( void )
{
    if_tty_toggle();
    return 0;
}

int command_parse_set( void )
{
    int rv = 1;
    int val = 1;
    const char * value = NULL;
    
    switch ( (rv = yylex()) ) {
    case IDENTIFIER: {
        const char *token = get_token();
        int length = strlen( token );
        int *variable = NULL;
        if ( length > 2 && token[0] == 'n' && token[1] == 'o' ) {
            value = token + 2;
            val = 0;
        } else {
            value = token;
        }

        if ( (variable = get_variable( value )) != NULL ) {
            rv = 0;
            *variable = val;
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
    int rv = 0;
    YY_BUFFER_STATE state = yy_scan_string( (char*)buffer );
    do {
        switch ( (rv = yylex()) ) {
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
            goto bail;
        } break;
        default:
            rv = 1;
            goto bail;
        }
    } while( rv );
    rv = 0;

bail:
    yy_delete_buffer( state );
    return rv;
}

