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

#include "error.h"
#include "types.h"
#include "util.h"

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

int invoke_debugger(char *path, int argc, char *argv[], int *in, int *out) {
    pid_t pid;
    const char * const GDB               = "gdb";
    const char * const NW                = "--nw";
    const char * const MI                = "--interpreter=mi";
    char **local_argv;
    int i, j = 0, extra = 4;
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
    local_argv[j++] = xstrdup(MI);
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
        /* Make stdout and stderr go to the same fd */
        if ( dup2(STDOUT_FILENO, STDERR_FILENO) == -1 ) {
            err_msg("(%s:%d) dup2 failed", __FILE__, __LINE__);
            free_memory(argc, local_argv);
            return -1;
        }
    
        /* Make the stdout and stderr go to this pipe */
        if ( (dup2(pout[1], STDOUT_FILENO)) == -1) {
            err_msg("(%s:%d) dup failed", __FILE__, __LINE__);
            free_memory(argc, local_argv);
            return -1;
        }
        xclose(pout[1]);

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
