#include "configuration.h"
#include "interface.h"

/**
 * The general idea is that the configuration will read in the users ~/.cgdbrc
 * file, or ~/.cgdb/config or whatever, and execute each command.  This will
 * also be responsible for processing all : commands in the tui.
 *
 */

int regex_icase = 0;
int shortcut_option = 0;


static struct config_variables
{
    const char * name, *s_name;
    int * variable;
} CONFIG_VARIABLES[] = {
    // keep this stuff sorted! !sort
    /* ignorecase */ { "ignorecase", "ic", &regex_icase },
    /* shortcut   */ { "shortcut", "sc", &shortcut_option },
};

static int config_focus_cgdb( void );
static int config_focus_gdb( void );
static int config_focus_tty( void );

static int config_macro_run( void );
static int config_macro_save( void );
static int config_macro_start( void );

static int config_do_continue( void );
static int config_do_finish( void );
static int config_do_help( void );
static int config_do_next( void );
static int config_do_quit( void );
static int config_do_quit_force( void );
static int config_do_run( void );
static int config_do_step( void );
static int config_start_tty( void );
static int config_toggle_tty( void );

static struct config_commands
{
    const char *name;
    // these functions will return 0 on success and 1 on error.  
    // Should the configuration file processing continue after an error?
    int (*action)( void );
} CONFIG_COMMANDS[] = {
    // keep this stuff sorted, you can use !sort in vi
    /* <esc>      */ { "<esc>",      config_focus_cgdb },
    /* continue   */ { "continue",   config_do_continue },
    /* finish     */ { "finish",     config_do_finish },
    /* help       */ { "help",       config_do_help },
    /* insert     */ { "insert",     config_focus_gdb },
    /* macro      */ { "macro",      config_macro_start },
    /* macro_run  */ { "macro_run",  config_macro_run }, 
    /* macro_save */ { "macro_save", config_macro_save },
    /* next       */ { "next",       config_do_next },
    /* q          */ { "q",          config_do_quit },
    /* q!         */ { "q!",         config_do_quit_force },
    /* quit       */ { "quit",       config_do_quit },
    /* quit!      */ { "quit!",      config_do_quit_force },
    /* run        */ { "run",        config_do_run },
    /* start_tty  */ { "start_tty",  config_start_tty },
    /* step       */ { "step",       config_do_step },
    /* toggle_tty */ { "toggle_tty",   config_toggle_tty },
    /* tty        */ { "tty",        config_focus_tty },
};

int config_focus_cgdb( void )
{
    if ( interface_set_focus( CGDB ) ) {
        return 1;
    } else {
        return 0;
    }
}

int config_focus_gdb( void )
{
    if ( interface_set_focus( GDB ) ) {
        return 1;
    } else {
        return 0;
    }
}

int config_focus_tty( void )
{
    if ( interface_set_focus( TTY ) ) {
        return 1;
    } else {
        return 0;
    }
}

int config_macro_run( void )
{
    // FIXME: This should ask for a filename (yylex)
    macro_load( "macro_text.txt" );
    return 0;
}

int config_macro_save( void )
{
    // FIXME: This should ask for a filename (yylex)
    macro_save( "macro_text.txt" );
    return 0;
}

int config_macro_start( void )
{
    macro_start();
    return 0;
}

int config_do_continue( void )
{
    tgdb_run_command( "continue" );
    return 0;
}

int config_do_finish( void )
{
    tgdb_run_command( "finish" );
    return 0;
}

int config_do_help( void )
{
    if_display_help();
    return 0;
}

int config_do_next( void )
{
    tgdb_run_command( "next" );
    return 0;
}


int config_do_quit( void )
{
    /* FIXME: Test to see if debugged program is still running */
    cleanup();
    exit(0);
    return 0;
}


int config_do_quit_force( void )
{
    cleanup();
    exit(0);
    return 0;
}


int config_do_run( void )
{
    /* FIXME: see if there are any other arguments to pass to the run command */
    tgdb_run_command( "run" );
    return 0;
}


int config_do_step( void )
{
    tgdb_run_command( "step" );
    return 0;
}

/* FIXME: I'm not even sure what this is supposed to do */
int config_start_tty( void )
{
    if ( tgdb_new_tty() == -1 ) {
        return 1;
    } else {
        /* FIXME: interface does scr_free( tty_win ), tty_win = NULL, if_layout(); */
        return 0;
    }
}

int config_toggle_tty( void )
{
    if ( tty_win_on ) {
        tty_win_on = 0;
        config_focus_cgdb();
    } else {
        tty_win_on = 1;
        config_focus_tty();
    }

    if_layout();
    return 0;
}
