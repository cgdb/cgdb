#include "config.h"

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

#include "tgdb_util.h"
#include "error.h"
#include "pseudo.h"
#include "types.h"
#include "util.h"

static char global_config_dir[PATH_MAX];

char *tgdb_util_get_config_dir(void){
    return global_config_dir; 
}

char *tgdb_util_get_config_gdbinit_file(void){
    static char filename[PATH_MAX];
    strncpy(filename, global_config_dir, strlen(global_config_dir) + 1);
#ifdef HAVE_CYGWIN
    strcat( filename, "\\gdb_init");
#else
    strcat( filename, "/gdb_init");
#endif
    return filename;
}

char *tgdb_util_get_config_gdb_debug_file(void){
    static char filename[PATH_MAX];
    strncpy(filename, global_config_dir, strlen(global_config_dir) + 1);
#ifdef HAVE_CYGWIN
    strcat( filename, "\\tgdb_debug");
#else
    strcat( filename, "/tgdb_debug");
#endif
    return filename;
}

int tgdb_util_set_home_dir(void) {
    char tgdb_config_dir_unix_path[MAXLINE];
    char homeDir[MAXLINE];
    char *env = getenv("HOME");
    struct stat st;

#ifdef HAVE_CYGWIN
   char tgdb_config_dir_win_path[MAXLINE];
   char win32_homedir[MAXLINE];
   extern void cygwin_conv_to_full_win32_path(const char *path, char *win32_path);
#endif

   if(env == NULL)
      err_quit("%s:%d -> $HOME is not set", __FILE__, __LINE__);

   sprintf( tgdb_config_dir_unix_path, "%s/.tgdb", env );

#ifdef HAVE_CYGWIN
   cygwin_conv_to_full_win32_path(tgdb_config_dir_unix_path, tgdb_config_dir_win_path);
   strncpy( tgdb_config_dir_unix_path, tgdb_config_dir_win_path, strlen(tgdb_config_dir_win_path) + 1);
   cygwin_conv_to_full_win32_path(env, win32_homedir);
   strncpy( homeDir, win32_homedir, strlen(win32_homedir) + 1);
#else 
   strncpy( homeDir, env, strlen(env) + 1);
#endif

   /* Check to see if already exists, if does not exist continue */
   if ( stat( tgdb_config_dir_unix_path, &st ) == -1 && errno == ENOENT ) {
       /* Create home config directory if does not exist */
       if ( access( env, R_OK | W_OK ) == -1 )
           return -1;

       if ( mkdir( tgdb_config_dir_unix_path, 0755 ) == -1 )
           return -1;
   }

#ifdef HAVE_CYGWIN
   sprintf( global_config_dir, "%s\\", tgdb_config_dir_unix_path );
#else
   sprintf( global_config_dir, "%s/", tgdb_config_dir_unix_path );
#endif

   return 0;
}

int tgdb_util_new_tty(int *masterfd, int *slavefd, char *sname) {
   static char local_slavename[SLAVE_SIZE];

   if ( pty_open(masterfd, slavefd, local_slavename, SLAVE_SIZE, NULL, NULL) == -1){
      err_msg("%s:%d -> Error: PTY open", __FILE__, __LINE__);
      return -1;   
   }

   strncpy(sname, local_slavename, SLAVE_SIZE);
   return 0;
}

int tgdb_util_free_tty(int *masterfd, int *slavefd, char *sname) {

   xclose(*masterfd);
   xclose(*slavefd);

   if ( pty_release(sname) == -1 ) {
      err_msg("%s:%d pty_release error", __FILE__, __LINE__);
      return -1;   
   }
   
   return 0;
}

int tgdb_util_pty_free_process(int *masterfd, char *sname) {

   xclose(*masterfd);

   if ( pty_release(sname) == -1 ) {
      err_msg("%s:%d pty_release error", __FILE__, __LINE__);
      return -1;   
   }
   
   return 0;
}

/* free_memory: utility function that frees up memory.
 *
 * argc:    The number of items in argv
 * argv:    free is called on each of these
 */
static void free_memory(int argc, char *argv[]) {
    int i;

    /* Free the local list */
    for (i = 0; i < argc; i++)
        free(argv[i]);
   
    free(argv);
}

int invoke_debugger(char *path, int argc, char *argv[], int *in, int *out, int choice) {
    pid_t pid;
    const char * const GDB               = "gdb";
    const char * const NW                = "--nw";
    const char * const X                 = "-x";
    char *F                              = tgdb_util_get_config_gdbinit_file();
    char **local_argv;
    int i, j = 0, extra = 5;
    int pin[2] = { -1, -1 }, pout[2] = { -1, -1 };

    /* Copy the argv into the local_argv, and NULL terminate it.
     * sneak in the path name, the user did not type that */
    local_argv = (char **) xmalloc((argc+extra) * sizeof(char *));
    if ( path )
        local_argv[j++] = xstrdup(path);
    else
        local_argv[j++] = xstrdup(GDB);

    /* copy in all the data the user entered */ 
    for (i = 0; i < argc; i++)
        local_argv[j++] = xstrdup(argv[i]);

    /* add the init file that the user did not type */    
    local_argv[j++] = xstrdup(NW);
    local_argv[j++] = xstrdup(X);
    local_argv[j++] = xstrdup(F);
    local_argv[j] = NULL;

    if ( pipe(pin) == -1 ) {
        err_msg("(%s:%d) pipe failed", __FILE__, __LINE__);
        free_memory(argc, local_argv);
        return -1;
    }

    if ( pipe(pout) == -1 ) {
        err_msg("(%s:%d) pipe failed", __FILE__, __LINE__);
        free_memory(argc, local_argv);
        return -1;
    }

    if ( (pid = fork()) == -1 ) { /* error, free memory and return  */
        err_msg("(%s:%d) fork failed", __FILE__, __LINE__);
        return -1;
    } else if ( pid == 0 ) {    /* child */

        xclose(pin[1]);
        xclose(pout[0]);

        /* If this is not called, when user types ^c SIGINT gets sent to gdb */
        setsid();
        
        /* Make the stdout and stderr go to this pipe */
        if ( (dup2(pout[1], STDOUT_FILENO)) == -1) {
            err_msg("(%s:%d) dup failed", __FILE__, __LINE__);
            free_memory(argc, local_argv);
            return -1;
        }
        xclose(pout[1]);

        /* Make stdout and stderr go to the same fd */
        if ( dup2(STDOUT_FILENO, STDERR_FILENO) == -1 ) {
            err_msg("(%s:%d) dup2 failed", __FILE__, __LINE__);
            free_memory(argc, local_argv);
            return -1;
        }

        /* Make programs stdin be this pipe */
        if ( (dup2(pin[0], STDIN_FILENO)) == -1) {
            err_msg("(%s:%d) dup failed", __FILE__, __LINE__);
            free_memory(argc, local_argv);
            return -1;
        }

        xclose(pin[0]);
        if(execvp(local_argv[0], local_argv) == -1) {
            err_msg("(%s:%d) execvp failed", __FILE__, __LINE__);
            free_memory(argc, local_argv);
            return -1;
        }
    }

    *in = pin[1];
    xclose(pin[0]);
    *out = pout[0];
    xclose(pout[1]);
    free_memory(argc, local_argv);
    return pid;
}

/* free_memory: utility function that frees up memory.
 *
 * s:       if NULL, left alone, otherwise pty_release is called on it
 * fd:      if -1, left alone, otherwise close is called on it
 * argc:    The number of items in argv
 * argv:    free is called on each of these
 *
 * Returns -1 if any call fails, otherwise 0
 */
static int pty_free_memory(char *s, int fd, int argc, char *argv[]) {
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
   
int invoke_pty_process(
    char *name, 
    int argc, char *argv[], 
    char *slavename, int *masterfd,
    int *extra_input) {

    char **local_argv;  /* Local argument vector to pass to GDB */
    pid_t pid;          /* PID of child process (gdb) to be returned */
    int i, j = 0, extra = 3;    /* 1 name, 1 NULL */
    int pin[2] = { -1, -1 };

    if ( !name ) {
      err_msg("%s:%d name error", __FILE__, __LINE__);
      return -1;
    } 
    
    /* Create an input pipe to the child program */
    if ( pipe(pin) == -1 ) {
        err_msg("(%s:%d) pipe failed", __FILE__, __LINE__);
        free_memory(argc, local_argv);
        return -1;
    }
        
    /* Copy the argv into the local_argv, and NULL terminate it. */
    local_argv = (char **) xmalloc((argc+extra) * sizeof(char *));
    local_argv[j++] = xstrdup(name);
   
    /* copy in all the data the user entered */ 
    for (i = 0; i < argc; i++)
        local_argv[j++] = xstrdup(argv[i]);

    /* Add the read only pipe descriptor */
    {
        char num[16];
        sprintf(num, "%d", pin[0]);
        local_argv[j++] = xstrdup(num);
    }

    local_argv[j] = NULL;

    /* Fork into two processes with a shared pty pipe */
    pid = pty_fork(masterfd, slavename, SLAVE_SIZE, NULL, NULL);
   
    if ( pid == -1 ) {          /* error, free memory and return  */
        err_msg("(%s:%d) pty_fork failed", __FILE__, __LINE__);
        pty_free_memory(slavename, *masterfd, argc, local_argv);
        return -1;
    } else if ( pid == 0 ) {    /* child */
        xclose(pin[1]);

        if(execvp(local_argv[0], local_argv) == -1) {
            err_msg("(%s:%d) execvp failed", __FILE__, __LINE__);
            err_msg("(%s:%d) %s is not in path", __FILE__, __LINE__, name);
            pty_free_memory(slavename, *masterfd, argc, local_argv);
            return -1;
        }

        return -1;
    } // end if 

    xclose(pin[0]);
    *extra_input = pin[1];

    if ( pty_free_memory(NULL, -1, argc, local_argv) == -1 )
        return -1;

    return pid;
}
