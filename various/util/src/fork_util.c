#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_UNISTD_H
#include <unistd.h>
#endif  /* HAVE_UNISTD_H */

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

#if HAVE_SIGNAL_H
#include <signal.h>
#endif /* HAVE_SIGNAL_H */

#if HAVE_ERRNO_H
#include <errno.h>
#endif /* HAVE_ERRNO_H */

#include "fork_util.h"
#include "sys_util.h"
#include "fs_util.h"
#include "pseudo.h"
#include "logger.h"

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
        logger_write_pos ( logger, __FILE__, __LINE__, "pty_release failed");
        error = -1;
    }

    if ( fd != -1 && close(fd) == -1) {
        logger_write_pos ( logger, __FILE__, __LINE__, "close failed");
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

    char **local_argv = NULL;  /* Local argument vector to pass to GDB */
    pid_t pid;          /* PID of child process (gdb) to be returned */
    int i, j = 0, extra = 3;    /* 1 name, 1 NULL */
    int pin[2] = { -1, -1 };

    if ( !name ) {
      logger_write_pos ( logger, __FILE__, __LINE__, "name error");
      return -1;
    } 
    
    /* Create an input pipe to the child program */
    if ( pipe(pin) == -1 ) {
        logger_write_pos ( logger, __FILE__, __LINE__, "pipe failed");
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
        logger_write_pos ( logger, __FILE__, __LINE__, "pty_fork failed");
        pty_free_memory(slavename, *masterfd, argc, local_argv);
        return -1;
    } else if ( pid == 0 ) {    /* child */
        xclose(pin[1]);

        if(execvp(local_argv[0], local_argv) == -1) {
            logger_write_pos ( logger, __FILE__, __LINE__, "execvp failed");
            logger_write_pos ( logger, __FILE__, __LINE__, "%s is not in path", name);
            pty_free_memory(slavename, *masterfd, argc, local_argv);
            exit(-1);
        }

        exit(-1);
    }

    xclose(pin[0]);
    *extra_input = pin[1];

    if ( pty_free_memory(NULL, -1, argc, local_argv) == -1 )
        return -1;

    return pid;
}

int invoke_pty_process_function( 
            char *slavename, int *masterfd, int *extra_input,
            int (*entry)(int)) {

    pid_t pid;          /* PID of child process (gdb) to be returned */
    int pin[2] = { -1, -1 };

    /* Create an input pipe to the child program */
    if ( pipe(pin) == -1 ) {
        logger_write_pos ( logger, __FILE__, __LINE__, "pipe failed");
        return -1;
    }
        
    /* Fork into two processes with a shared pty pipe */
    pid = pty_fork(masterfd, slavename, SLAVE_SIZE, NULL, NULL);
   
    if ( pid == -1 ) {          /* error, free memory and return  */
        logger_write_pos ( logger, __FILE__, __LINE__, "pty_fork failed");
        pty_free_memory(slavename, *masterfd, 0, NULL);
        return -1;
    } else if ( pid == 0 ) {    /* child */
        xclose(pin[1]);

        /* Call entry point to new program */
        entry(pin[0]);
        
        exit(-1);
    }

    xclose(pin[0]);
    *extra_input = pin[1];

    return pid;
}

int util_new_tty(int *masterfd, int *slavefd, char *sname) {
   static char local_slavename[SLAVE_SIZE];

   if ( pty_open(masterfd, slavefd, local_slavename, SLAVE_SIZE, NULL, NULL) == -1){
      logger_write_pos ( logger, __FILE__, __LINE__, "PTY open");
      return -1;   
   }

   strncpy(sname, local_slavename, SLAVE_SIZE);
   return 0;
}

int util_free_tty(int *masterfd, int *slavefd, const char *sname) {

   xclose(*masterfd);
   xclose(*slavefd);

   if ( pty_release(sname) == -1 ) {
      logger_write_pos ( logger, __FILE__, __LINE__, "pty_release error");
      return -1;   
   }
   
   return 0;
}

int pty_free_process(int *masterfd, char *sname) {

   xclose(*masterfd);

   if ( pty_release(sname) == -1 ) {
      logger_write_pos ( logger, __FILE__, __LINE__, "pty_release error");
      return -1;   
   }
   
   return 0;
}

void free_memory(int argc, char *argv[]) {
    int i;

    /* Free the local list */
    for (i = 0; i < argc; i++)
        free(argv[i]);
   
    free(argv);
}

int invoke_debugger(
            const char *path, 
            int argc, char *argv[], 
            int *in, int *out, 
            int choice, char *filename) {
    pid_t pid;
    const char * const GDB               = "gdb";
    const char * const NW                = "--nw";
    const char * const X                 = "-x";
    const char * const ANNOTATE_TWO      = "--annotate=2";
	const char * const GDBMI             = "-i=mi2";
    char *F                              = filename;
    char **local_argv;
    int i, j = 0, extra = 6;
    int pin[2] = { -1, -1 }, pout[2] = { -1, -1 };

    /* Copy the argv into the local_argv, and NULL terminate it.
     * sneak in the path name, the user did not type that */
    local_argv = (char **) xmalloc((argc+extra) * sizeof(char *));
    if ( path )
        local_argv[j++] = xstrdup(path);
    else
        local_argv[j++] = xstrdup(GDB);

	/* NOTE: These options have to come first, since if the user
	 * typed '--args' to GDB, everything at the end of the 
	 * users options become parameters to the inferior.
	 */
    local_argv[j++] = xstrdup(NW);

    /* add the init file that the user did not type */    
	if ( choice == 0 )
		local_argv[j++] = xstrdup(ANNOTATE_TWO);
	else if ( choice == 1 )
		local_argv[j++] = xstrdup(GDBMI);

    local_argv[j++] = xstrdup(X);
    local_argv[j++] = xstrdup(F);

    /* copy in all the data the user entered */ 
    for (i = 0; i < argc; i++)
        local_argv[j++] = xstrdup(argv[i]);

    local_argv[j] = NULL;

    if ( pipe(pin) == -1 ) {
        logger_write_pos ( logger, __FILE__, __LINE__, "pipe failed");
        free_memory(argc, local_argv);
        return -1;
    }

    if ( pipe(pout) == -1 ) {
        logger_write_pos ( logger, __FILE__, __LINE__, "pipe failed");
        free_memory(argc, local_argv);
        return -1;
    }

    if ( (pid = fork()) == -1 ) { /* error, free memory and return  */
        logger_write_pos ( logger, __FILE__, __LINE__, "fork failed");
        return -1;
    } else if ( pid == 0 ) {    /* child */

        xclose(pin[1]);
        xclose(pout[0]);

        /* If this is not called, when user types ^c SIGINT gets sent to gdb */
        setsid();
        
        /* Make the stdout and stderr go to this pipe */
        if ( (dup2(pout[1], STDOUT_FILENO)) == -1) {
            logger_write_pos ( logger, __FILE__, __LINE__, "dup failed");
            free_memory(argc, local_argv);
            return -1;
        }
        xclose(pout[1]);

        /* Make stdout and stderr go to the same fd */
        if ( dup2(STDOUT_FILENO, STDERR_FILENO) == -1 ) {
            logger_write_pos ( logger, __FILE__, __LINE__, "dup2 failed");
            free_memory(argc, local_argv);
            return -1;
        }

        /* Make programs stdin be this pipe */
        if ( (dup2(pin[0], STDIN_FILENO)) == -1) {
            logger_write_pos ( logger, __FILE__, __LINE__, "dup failed");
            free_memory(argc, local_argv);
            return -1;
        }

        xclose(pin[0]);
        if(execvp(local_argv[0], local_argv) == -1) {
            logger_write_pos ( logger, __FILE__, __LINE__, "execvp failed");
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
