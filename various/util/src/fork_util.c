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

struct pty_pair {
  int masterfd;
  int slavefd;
  char slavename[SLAVE_SIZE];
};

pty_pair_ptr 
pty_pair_create (void)
{
  int val;
  static char local_slavename[SLAVE_SIZE];
  pty_pair_ptr ptr = (pty_pair_ptr)xmalloc (sizeof (struct pty_pair));
  if (!ptr)
    return NULL;

  ptr->masterfd = -1;
  ptr->slavefd = -1;
  ptr->slavename[0] = 0;

  val = pty_open (&(ptr->masterfd), &(ptr->slavefd), local_slavename, SLAVE_SIZE, NULL, NULL);
  if (val == -1) {
    logger_write_pos ( logger, __FILE__, __LINE__, "PTY open");
    return NULL;   
  }

  strncpy(ptr->slavename, local_slavename, SLAVE_SIZE);

  return ptr;
}

int 
pty_pair_destroy (pty_pair_ptr pty_pair)
{
  if (!pty_pair)
    return -1;

  xclose(pty_pair->masterfd);
  xclose(pty_pair->slavefd);

  if (pty_release (pty_pair->slavename) == -1) {
    logger_write_pos ( logger, __FILE__, __LINE__, "pty_release error");
    return -1;   
  }

  free (pty_pair);

  return 0;
}

int 
pty_pair_get_masterfd (pty_pair_ptr pty_pair)
{
  if (!pty_pair)
    return -1;

  return pty_pair->masterfd;
}

int 
pty_pair_get_slavefd (pty_pair_ptr pty_pair)
{
  if (!pty_pair)
    return -1;

  return pty_pair->slavefd;
}

const char *
pty_pair_get_slavename (pty_pair_ptr pty_pair)
{
  if (!pty_pair)
    return NULL;

  return pty_pair->slavename;
}

int pty_free_process(int *masterfd, char *sname) {

   xclose(*masterfd);

   if ( pty_release(sname) == -1 ) {
      logger_write_pos ( logger, __FILE__, __LINE__, "pty_release error");
      return -1;   
   }
   
   return 0;
}

/** 
 * Utility function that frees up memory.
 *
 * \param argc
 * The number of items in argv
 *
 * \param argv
 * free is called on each of these
 */
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
    int malloc_size = argc + extra;

    /* Copy the argv into the local_argv, and NULL terminate it.
     * sneak in the path name, the user did not type that */
    local_argv = (char **) xmalloc((malloc_size) * sizeof(char *));
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
        free_memory(malloc_size, local_argv);
        return -1;
    }

    if ( pipe(pout) == -1 ) {
        logger_write_pos ( logger, __FILE__, __LINE__, "pipe failed");
        free_memory(malloc_size, local_argv);
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
            free_memory(malloc_size, local_argv);
            return -1;
        }
        xclose(pout[1]);

        /* Make stdout and stderr go to the same fd */
        if ( dup2(STDOUT_FILENO, STDERR_FILENO) == -1 ) {
            logger_write_pos ( logger, __FILE__, __LINE__, "dup2 failed");
            free_memory(malloc_size, local_argv);
            return -1;
        }

        /* Make programs stdin be this pipe */
        if ( (dup2(pin[0], STDIN_FILENO)) == -1) {
            logger_write_pos ( logger, __FILE__, __LINE__, "dup failed");
            free_memory(malloc_size, local_argv);
            return -1;
        }

        xclose(pin[0]);
        execvp(local_argv[0], local_argv);

        /* Will get here if exec failed. This will happen when the 
         * - "gdb" is not on the users path, or if 
         * - user specified a different program via the -d option and it was
         *   not able to be exec'd.
         */

        exit (0);
    }

    *in = pin[1];
    xclose(pin[0]);
    *out = pout[0];
    xclose(pout[1]);
    free_memory(malloc_size, local_argv);
    return pid;
}
