#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif /* HAVE_STDLIB_H */

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#if HAVE_STDIO_H
#include <stdio.h>
#endif /* HAVE_STDIO_H */

#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#if HAVE_SIGNAL_H
#include <signal.h>
#endif /* HAVE_SIGNAL_H */

#if HAVE_ERRNO_H
#include <errno.h>
#endif /* HAVE_ERRNO_H */

#include "sys_util.h"
#include "fork_util.h"
#include "fs_util.h"
#include "pseudo.h"
#include "terminal.h"

struct pty_pair {
    int masterfd;
    int slavefd;
    char slavename[SLAVE_SIZE];
};

pty_pair_ptr pty_pair_create(void)
{
    int val;
    static char local_slavename[SLAVE_SIZE];
    pty_pair_ptr ptr = (pty_pair_ptr) cgdb_malloc(sizeof (struct pty_pair));

    if (!ptr)
        return NULL;

    ptr->masterfd = -1;
    ptr->slavefd = -1;
    ptr->slavename[0] = 0;

    val = pty_open(&(ptr->masterfd), &(ptr->slavefd), local_slavename,
            SLAVE_SIZE, NULL, NULL);
    if (val == -1) {
        clog_error(CLOG_CGDB, "PTY open");
        return NULL;
    }

    strncpy(ptr->slavename, local_slavename, SLAVE_SIZE);

    return ptr;
}

int pty_pair_destroy(pty_pair_ptr pty_pair)
{
    if (!pty_pair)
        return -1;

    cgdb_close(pty_pair->masterfd);
    cgdb_close(pty_pair->slavefd);

    if (pty_release(pty_pair->slavename) == -1) {
        clog_error(CLOG_CGDB, "pty_release error");
        return -1;
    }

    free(pty_pair);

    return 0;
}

int pty_pair_get_masterfd(pty_pair_ptr pty_pair)
{
    if (!pty_pair)
        return -1;

    return pty_pair->masterfd;
}

int pty_pair_get_slavefd(pty_pair_ptr pty_pair)
{
    if (!pty_pair)
        return -1;

    return pty_pair->slavefd;
}

const char *pty_pair_get_slavename(pty_pair_ptr pty_pair)
{
    if (!pty_pair)
        return NULL;

    return pty_pair->slavename;
}

int pty_free_process(int *masterfd, char *sname)
{

    cgdb_close(*masterfd);

    if (pty_release(sname) == -1) {
        clog_error(CLOG_CGDB, "pty_release error");
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
void free_memory(int argc, char *argv[])
{
    int i;

    /* Free the local list */
    for (i = 0; i < argc; i++)
        free(argv[i]);

    free(argv);
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
static int pty_free_memory(char *s, int fd, int argc, char *argv[])
{
    int error = 0, i;

    if (s && pty_release(s) == -1) {
        clog_error(CLOG_CGDB, "pty_release failed");
        error = -1;
    }

    if (fd != -1 && close(fd) == -1) {
        clog_error(CLOG_CGDB, "close failed");
        error = -1;
    }

    /* Free the local list */
    for (i = 0; i < argc; i++)
        free(argv[i]);

    free(argv);

    return error;
}

int invoke_debugger(const char *path,
        int argc, char *argv[], int gdb_win_rows, int gdb_win_cols,
        int *in, int *out, const char *new_ui_tty)
{
    pid_t pid;
    const char *const GDB = "gdb";
    const char *const NW = "--nw";
    const char *const EX = "-ex";
    char **local_argv;
    int i, j = 0, extra = 5;
    int malloc_size = argc + extra;
    char slavename[64];
    int masterfd;

    struct winsize size, *winsize;
    size.ws_row = gdb_win_rows;
    size.ws_col = gdb_win_cols;
    size.ws_xpixel = 0;
    size.ws_ypixel = 0;

    if (gdb_win_rows == 0 && gdb_win_cols == 0) {
        winsize = 0;
    } else {
        winsize = &size;
    }


    /* Copy the argv into the local_argv, and NULL terminate it.
     * sneak in the path name, the user did not type that */
    local_argv = (char **) cgdb_malloc((malloc_size) * sizeof (char *));
    if (path)
        local_argv[j++] = cgdb_strdup(path);
    else
        local_argv[j++] = cgdb_strdup(GDB);

    /* NOTE: These options have to come first, since if the user
     * typed '--args' to GDB, everything at the end of the 
     * users options become parameters to the inferior.
     */
    local_argv[j++] = cgdb_strdup(NW);

    local_argv[j++] = cgdb_strdup(EX);
    local_argv[j++] = cgdb_strdup(std::string("new-ui mi ").
            append(new_ui_tty).c_str());

    /* copy in all the data the user entered */
    for (i = 0; i < argc; i++)
        local_argv[j++] = cgdb_strdup(argv[i]);

    local_argv[j] = NULL;

    if (fs_util_file_exists_in_path(local_argv[0]) == -1) {
        clog_error(CLOG_CGDB, "Debugger \"%s\" not found", local_argv[0]);
        pty_free_memory(slavename, masterfd, argc, local_argv);
        return -1;
    }

    /* Log the gdb invocation line */
    clog_info(CLOG_GDBIO, "Invoking program:");
    for (i = 0; i < j; i++) {
        clog_info(CLOG_GDBIO, "  argv[%d]=%s ", i, local_argv[i]);
    }

    /* Fork into two processes with a shared pty pipe */
    pid = pty_fork(&masterfd, slavename, SLAVE_SIZE, NULL, winsize);

    if (pid == -1) {            /* error, free memory and return  */
        pty_free_memory(slavename, masterfd, argc, local_argv);
        clog_error(CLOG_CGDB, "fork failed");
        return -1;
    } else if (pid == 0) {      /* child */
        execvp(local_argv[0], local_argv);

        /* Will get here if exec failed. This will happen when the 
         * - "gdb" is not on the users path, or if 
         * - user specified a different program via the -d option and it was
         *   not able to be exec'd.
         */
        exit(0);
    }

    *in = masterfd;
    *out = masterfd;

    free_memory(malloc_size, local_argv);
    return pid;
}
