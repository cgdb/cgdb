#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>
#include <signal.h>

#include "tgdb_init.h"
#include "error.h"
#include "pseudo.h"
#include "types.h"
#include "io.h"
#include "globals.h"
#include "terminal.h"
#include "config.h"
#include "util.h"
#include "macro.h"

/* free_memory: utility function that frees up memory.
 *
 * s:       if NULL, left alone, otherwise pty_release is called on it
 * fd:      if -1, left alone, otherwise close is called on it
 * argc:    The number of items in argv
 * argv:    free is called on each of these
 *
 * Returns -1 if any call fails, otherwise 0
 */
static int free_memory(char *s, int fd, int argc, char *argv[]) {
    int error = 0, i;

    if ( s && pty_release(s) == -1) {
        err_msg("(%s:%d) pty_release failed", __FILE__, __LINE__);
        error = -1;
    }

    if ( fd != -1 && close(fd) == -1) {
        err_msg("(%s:%d) close failed", __FILE__, __LINE__);
        error = -1;
    }

    /* Free the local list */
    for (i = 0; i < argc; i++)
        free(argv[i]);
   
    free(argv);

    return error;
}
   
int tgdb_init_forkAndExecPty(char *debugger, int argc, char *argv[], 
                          char *slavename, int *masterfd,
                          int need_nl_mapping) {
    const char * const GDB               = "gdb";
    const char * const NW               = "--nw";
    const char * const X                 = "-x";
    char *debugger_path = (char *)GDB;

    char **local_argv;  /* Local argument vector to pass to GDB */
    pid_t pid;          /* PID of child process (gdb) to be returned */
    int i, j = 0, extra = 5;
    struct winsize size;
    char gdb_init_file[MAXLINE];

    if ( debugger )
        debugger_path = debugger; 

    /* Not knowing the terminal size is not necesarily an error */
    if(ioctl(STDIN_FILENO, TIOCGWINSZ, &size) < 0)
        err_msg("%s:%d -> Could not get terminal's window size ", __FILE__, __LINE__);

    /* Copy the argv into the local_argv, and NULL terminate it.
     * sneak in the debugger name, the user did not type that */
    local_argv = (char **) xmalloc((argc+extra) * sizeof(char *));
    local_argv[j++] = xstrdup(debugger_path);
   
    /* copy in all the data the user entered */ 
    for (i = 0; i < argc; i++)
        local_argv[j++] = xstrdup(argv[i]);

    /* add the init file that the user did not type */    
    local_argv[j++] = xstrdup(NW);
    local_argv[j++] = xstrdup(X);
    global_get_config_gdb_init_file(gdb_init_file);
    local_argv[j++] = xstrdup(gdb_init_file);
    local_argv[j] = NULL;

    /* Debug output */
    {
        int t;
        for ( t = 0; t < j; t++)
            io_debug_write_fmt("[%s]", local_argv[t]);
        io_debug_write("\r\n");
    }

    /* Fork into two processes with a shared pty pipe */
    pid = pty_fork(masterfd, slavename, SLAVE_SIZE, NULL, NULL);
   
    if ( pid == -1 ) {          /* error, free memory and return  */
        err_msg("(%s:%d) pty_fork failed", __FILE__, __LINE__);
        free_memory(slavename, *masterfd, argc, local_argv);
        return -1;
    } else if ( pid == 0 ) {    /* child */
        if ( close(STDERR_FILENO) == -1 ) {
            err_msg("(%s:%d) close failed", __FILE__, __LINE__);
            free_memory(slavename, *masterfd, argc, local_argv);
            return -1;
        }
      
        if ( dup2(STDOUT_FILENO, STDERR_FILENO) == -1 ) {
            err_msg("(%s:%d) dup2 failed", __FILE__, __LINE__);
            free_memory(slavename, *masterfd, argc, local_argv);
            return -1;
        }

        /* Set gdb's terminal attribute's to not map NL to CR NL */
        if( need_nl_mapping == TRUE) {
            if(tty_output_nl(STDIN_FILENO) == -1)
                err_msg("(%s:%d) Couldn't set pty attributes", __FILE__, __LINE__);
        }

        /*err_msg("R(%d) C(%d) X(%d) Y(%d)\n", size.ws_row, size.ws_col, size.ws_xpixel, size.ws_ypixel);*/

        /* change the child's psuedo terminal's window size to be the same as the
         * parent except the child wants to get infinite height in gdb.  */
        if(pty_change_window_size(STDIN_FILENO, 100000, size.ws_col - 4, size.ws_xpixel, size.ws_ypixel) < 0)
            err_msg("(%s:%d) Couldn't set pty attributes", __FILE__, __LINE__);

        /* Keep GDB from spouting out terminal escape codes */
        if ( putenv("TERM=dumb") == -1 )
            err_msg("(%s:%d) dup2 failed", __FILE__, __LINE__);

        if(execvp(local_argv[0], local_argv) == -1) {
            err_msg("(%s:%d) execvp failed", __FILE__, __LINE__);
            free_memory(slavename, *masterfd, argc, local_argv);
            return -1;
        }
    } // end if 

    if ( free_memory(NULL, -1, argc, local_argv) == -1 )
        return -1;

    return pid;
}


int tgdb_init_does_gdb_need_mapping(char *debugger) {
    const char * const GDB               = "gdb";
    const char * const NW               = "--nw";
    const char * const NX               = "--nx";
    int masterfd;           /* descriptor to pty */
    int local_need_mapping = -1;   /* return value ( if mapping ) */
    char slavename[SLAVE_SIZE];
    char *debugger_path = (char *)GDB;

    char **local_argv;            /* Local argument vector to pass to GDB */
    int local_argc = 0;
    pid_t pid;                    /* PID of child process (gdb) to be returned */

    if ( debugger )
        debugger_path = debugger; 

    /* Copy the argv into the local_argv, and NULL terminate it */
    /* sneak in the debugger name, the user did not type that */
    local_argv = (char **) xmalloc(4 * sizeof(char *));

    local_argv[local_argc++] = xstrdup(debugger_path);
    local_argv[local_argc++] = xstrdup(NW);
    local_argv[local_argc++] = xstrdup(NX);
    local_argv[local_argc] =   NULL;

    /* Fork into two processes with a shared pty pipe */
    pid = pty_fork(&masterfd, slavename, SLAVE_SIZE, NULL, NULL);
   
    if ( pid == -1 ) {          /* error, free memory and return  */
        err_msg("(%s:%d) pty_fork failed", __FILE__, __LINE__);
        free_memory(slavename, masterfd, local_argc, local_argv);
        return -1;
    } else if ( pid == 0 ) { /* child */
        if ( close(STDERR_FILENO) == -1 ) {
            err_msg("(%s:%d) close failed", __FILE__, __LINE__);
            free_memory(slavename, masterfd, local_argc, local_argv);
            return -1;
        }
      
        if ( dup2(STDOUT_FILENO, STDERR_FILENO) == -1 ) {
            err_msg("(%s:%d) dup2 failed", __FILE__, __LINE__);
            free_memory(slavename, masterfd, local_argc, local_argv);
            return -1;
        }

        /* Keep GDB from spouting out terminal escape codes */
        if ( putenv("TERM=dumb") == -1 )
            err_msg("(%s:%d) dup2 failed", __FILE__, __LINE__);

        execvp(local_argv[0], local_argv);
        err_msg("(%s:%d) execvp failed", __FILE__, __LINE__);
        free_memory(slavename, masterfd, local_argc, local_argv);
        return -1;
    } else { /* Parent */
        char cur = '\0', prev = '\0', prev2 = '\0';
        int status;

        /* Parse output */
        do {
            /*io_display_char(stderr, cur);*/
            if ( cur == '\n' ) {
                if ( prev == '\r' && prev2 == '\r' ) {
                    local_need_mapping = TRUE;
                    break;
                } else if ( prev == '\r' ){
                    local_need_mapping = FALSE;
                    break;
                } else {
                    err_msg("%s:%d -> Unexpected result", __FILE__, __LINE__);
                    local_need_mapping = -1;
                    break;
                }
            }
         
            prev2 = prev;
            prev  = cur;
        } while ( io_read_byte(&cur, masterfd) != -1);


        macro_turn_macro_off();
        io_writen(masterfd, "quit\n", 5); /* Tell gdb to quit */
        macro_turn_macro_on();
        
        /* Read rest of data */
        while( io_read_byte(&cur, masterfd) != -1);

        if (waitpid(pid, &status, 0) == -1) {
            err_msg("(%s:%d) waitpid failed", __FILE__, __LINE__);
            free_memory(slavename, masterfd, local_argc, local_argv);
            return -1;
        }
    } // end if

    if ( free_memory(NULL, -1, local_argc, local_argv) == -1 )
        return -1;

   return local_need_mapping;
}

/* tgdb_init_set_home_dir: 
 * Attempts to create a config directory in the user's home directory.
 * Also, it sets the path to the user's home directory globaly.
 */
static int tgdb_init_set_home_dir(void) {
   char buffer[MAXLINE];
   char *env = getenv("HOME"); 
   struct stat st;

#ifdef HAVE_CYGWIN
   char win32_path[MAXLINE];
   extern void cygwin_conv_to_full_win32_path(const char *path, char *win32_path);
#endif
   
   if(env == NULL){
      err_msg("%s:%d -> $HOME is not set", __FILE__, __LINE__);
      return -1;
   } 
   
#ifdef HAVE_CYGWIN
   sprintf( buffer, "%s\\.tgdb", env );
#else
   sprintf( buffer, "%s/.tgdb", env );
#endif

#ifdef HAVE_CYGWIN
   cygwin_conv_to_full_win32_path(buffer, win32_path);
   strncpy( buffer, win32_path, strlen(win32_path));
#endif

    /* Check to see if already exists, if does not exist continue */
    if ( stat( buffer, &st ) == -1 && errno == ENOENT) {
        /* Create home config directory if does not exist */
        if ( access( env, R_OK | W_OK ) == -1 )
            return -1;

        if ( mkdir( buffer, 0755 ) == -1 )
            return -1;
   } 

   global_set_config_dir(buffer);
   return 0;
}

/* tgdb_make_config_file: makes a config file for the user.
 *    Return: -1 on error or 0 on success.
 */
int tgdb_init_setup_config_file(void){
   char gdb_init_file[MAXLINE];

   if ( tgdb_init_set_home_dir() == -1 )
      return -1;
   
   global_get_config_gdb_init_file(gdb_init_file);

   if ( access( gdb_init_file, R_OK || W_OK ) == -1 ) {
      FILE *fp = fopen( gdb_init_file, "w" );
      if ( fp ) {
         fprintf( fp, 
               "set annotate 2\n"
               "set height 0\n"
               "set environment TERM dumb\n"
               "set prompt (tgdb) \n" 
               );
         fclose( fp );
      } else {
         err_msg("%s:%d -> Could not open (%s)", __FILE__, __LINE__, gdb_init_file);
         return -1;
      }
   } else {
      err_msg("%s:%d -> Could not access (%s)", __FILE__, __LINE__, gdb_init_file);
      return -1;
   }

   return 0;
}

int tgdb_init_new_tty(int *masterfd, int *slavefd, char *sname) {
   static char local_slavename[SLAVE_SIZE];

   if ( pty_open(masterfd, slavefd, local_slavename, SLAVE_SIZE, NULL, NULL) == -1){
      err_msg("%s:%d -> Error: PTY open", __FILE__, __LINE__);
      return -1;   
   }

   strncpy(sname, local_slavename, SLAVE_SIZE);
   return 0;
}
