#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_STDLIB_H 
#include <stdlib.h>
#endif  /* HAVE_STDLIB_H */

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#if HAVE_STDIO_H
#include <stdio.h> 
#endif /* HAVE_STDIO_H */

#if HAVE_UNISTD_H
#include <unistd.h>
#endif  /* HAVE_UNISTD_H */

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#if HAVE_LIMITS_H
#include <limits.h>
#endif /* HAVE_LIMITS_H */

#if HAVE_SIGNAL_H
#include <signal.h>
#endif /* HAVE_SIGNAL_H */

#if HAVE_ERRNO_H
#include <errno.h>
#endif /* HAVE_ERRNO_H */

#include "a2-tgdb.h"
#include "fork_util.h"
#include "fs_util.h"
#include "pseudo.h"
#include "error.h"
#include "io.h"
#include "state_machine.h"
#include "data.h"
#include "commands.h"
#include "globals.h"
#include "types.h"
#include "queue.h"
#include "sys_util.h"
#include "ibuf.h"

/* This is set when tgdb has initialized itself */
static int tgdb_initialized = 0;
int masterfd = -1;                     /* master fd of the pseudo-terminal */
static pid_t debugger_pid;             /* pid of child process */
unsigned short tgdb_partially_run_command = 0;
extern sig_atomic_t control_c;

int a2_tgdb_completion_callback(const char *line) {

    if ( commands_issue_command ( ANNOTATE_COMPLETE, line, 4 ) == -1 ) {
        err_msg("%s:%d commands_issue_command error", __FILE__, __LINE__);
        return -1;
    }

    return 0;
}

/* tgdb_is_debugger_ready: Determines if a command can be sent directly to gdb.
 * 
 * Returns: FALSE if gdb is busy running a command or a command can not be run.
 *          TRUE  if gdb can currently recieve a command.
 */
int a2_tgdb_is_debugger_ready(void) {
    extern int COMMAND_ALREADY_GIVEN;
    if ( !tgdb_initialized )
        return FALSE;

    /* If the user is at the prompt and the raw queue is empty */
    if ( data_get_state() == USER_AT_PROMPT && !COMMAND_ALREADY_GIVEN)
        return TRUE;

    return FALSE;
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

/* The config directory and the init file for gdb */
static char config_dir[PATH_MAX];
static char a2_gdb_init_file[PATH_MAX];

/* tgdb_setup_config_file: 
 * -----------------------
 *  Creates a config file for the user.
 *
 *  Pre: The directory already has read/write permissions. This should have
 *       been checked by tgdb-base.
 *
 *  Return: 1 on success or 0 on error
 */
static int tgdb_setup_config_file ( char *dir ) {
    FILE *fp;

    strncpy ( config_dir , dir , strlen ( dir ) + 1 );

    fs_util_get_path ( dir, "a2_gdb_init", a2_gdb_init_file );

    if ( (fp = fopen ( a2_gdb_init_file, "w" )) ) {
        fprintf( fp, 
            "set annotate 2\n"
            "set height 0\n"
            "set prompt (tgdb) \n");
        fclose( fp );
    } else {
        err_msg("%s:%d fopen error '%s'", __FILE__, __LINE__, a2_gdb_init_file);
        return 0;
    }

    return 1;
}

int a2_find_valid_debugger ( 
            char *debugger,
            int argc, char **argv,
            int *gdb_stdin, int *gdb,
            char *config_dir) {

    char a2_debug_file[PATH_MAX];

    if ( !tgdb_setup_config_file( config_dir ) ) {
        err_msg("%s:%d tgdb_init_config_file error", __FILE__, __LINE__);
        return -1;
    }

    /* Initialize the debug file that a2_tgdb writes to */
    fs_util_get_path ( config_dir, "a2_tgdb_debug.txt", a2_debug_file );
    io_debug_init(a2_debug_file);

    debugger_pid = invoke_debugger(debugger, argc, argv, &masterfd, gdb, 0, a2_gdb_init_file);

    *gdb_stdin  = masterfd;

    if ( debugger_pid == -1 )
        return -1;

    return 1;
}

int a2_set_inferior_tty ( const char *inferior_tty_name ) {

    if ( commands_issue_command ( ANNOTATE_TTY, inferior_tty_name, 0 ) == -1 ) {
        err_msg("%s:%d commands_issue_command error", __FILE__, __LINE__);
        return -1;
    }

    return 0;
}

int a2_tgdb_init( const char *inferior_tty_name ){
    commands_init();

    tgdb_setup_signals();

    a2_set_inferior_tty ( inferior_tty_name );
   
    a2_tgdb_get_source_absolute_filename(NULL);

    if ( commands_issue_command ( ANNOTATE_INFO_SOURCE_RELATIVE, NULL, 0 ) == -1 ) {
        err_msg("%s:%d commands_issue_command error", __FILE__, __LINE__);
        return -1;
    }

    tgdb_initialized = 1;

    return 0;
}

int a2_tgdb_shutdown(void){
    close(masterfd);
    return 0;
}

int a2_tgdb_get_source_absolute_filename(char *file){

    if ( commands_issue_command ( ANNOTATE_LIST, file, 0 ) == -1 ) {
        err_msg("%s:%d commands_issue_command error", __FILE__, __LINE__);
        return -1;
    }

    if ( commands_issue_command ( ANNOTATE_INFO_SOURCE_ABSOLUTE, file, 0 ) == -1 ) {
        err_msg("%s:%d commands_issue_command error", __FILE__, __LINE__);
        return -1;
    }

    return 0;
}

int a2_tgdb_get_sources(void){
    if ( commands_issue_command ( ANNOTATE_INFO_SOURCES, NULL, 0 ) == -1 ) {
        err_msg("%s:%d commands_issue_command error", __FILE__, __LINE__);
        return -1;
    }

    return 0;
}

void a2_command_typed_at_prompt ( int i ) {
    extern int COMMAND_TYPED_AT_PROMPT;
    if ( i ) {
        tgdb_partially_run_command = 0;
        COMMAND_TYPED_AT_PROMPT = 1;
    } else
        tgdb_partially_run_command = 1;
}

char *a2_tgdb_err_msg(void) {
    return err_get();
}

int a2_tgdb_change_prompt(char *prompt) {
    /* Must call a callback to change the prompt */
    if ( commands_issue_command ( ANNOTATE_SET_PROMPT, prompt, 2 ) == -1 ) {
        err_msg("%s:%d commands_issue_command error", __FILE__, __LINE__);
        return -1;
    }
            
    return 0;
}
