#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_STDIO_H
#include <stdio.h>
#endif /* HAVE_STDIO_H */

#if HAVE_STDARG_H
#include <stdarg.h>
#endif /* HAVE_STDARG_H */

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

/* Library includes */
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#if HAVE_ERRNO_H
#include <errno.h>
#endif /* HAVE_ERRNO_H */

#if HAVE_SIGNAL_H
#include <signal.h>
#endif

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif /* HAVE_STDLIB_H */

/* Local includes */
#include "tgdb.h"
#include "io.h"
#include "terminal.h"
#include "fork_util.h"
#include "cgdb_clog.h"

#define MAXLINE 4096

struct tgdb *tgdb;

/* Readline interface */
static int gdb_quit = 0;

/* Original terminal attributes */
static struct termios term_attributes;

static void start_logging()
{
    /* Open our cgdb and tgdb io logfiles */
    clog_open(CLOG_CGDB_ID, "%s/cgdb_log%d.txt", ".");
    clog_open(CLOG_GDBIO_ID, "%s/cgdb_gdb_console_io_log%d.txt", ".");
    clog_open(CLOG_GDBMIIO_ID, "%s/cgdb_gdb_mi_io_log%d.txt", ".");

    clog_set_level(CLOG_GDBMIIO_ID, CLOG_DEBUG);
    clog_set_fmt(CLOG_GDBMIIO_ID, CGDB_CLOG_FORMAT);

    /* Puts cgdb in a mode where it writes a debug log of everything
     * that is read from gdb. That is basically the entire session.
     * This info is useful in determining what is going on under tgdb
     * since the gui is good at hiding that info from the user.
     *
     * Change level to CLOG_ERROR to write only error messages.
     *   clog_set_level(CLOG_GDBIO, CLOG_ERROR);
     */
    clog_set_level(CLOG_GDBIO_ID, CLOG_DEBUG);
    clog_set_fmt(CLOG_GDBIO_ID, CGDB_CLOG_FORMAT);

    /* General cgdb logging. Only logging warnings and debug messages
       by default. */
    clog_set_level(CLOG_CGDB_ID, CLOG_DEBUG);
    clog_set_fmt(CLOG_CGDB_ID, CGDB_CLOG_FORMAT);
}

static void signal_handler(int signo)
{
    if (signo == SIGINT || signo == SIGTERM || signo == SIGQUIT)
        tgdb_signal_notification(tgdb, signo);
}

/* Sets up the signal handler for SIGWINCH
 * Returns -1 on error. Or 0 on success */
static int set_up_signal(void)
{
    struct sigaction action;

    action.sa_handler = signal_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    if (sigaction(SIGINT, &action, NULL) < 0) {
        clog_error(CLOG_CGDB, "sigaction failed ");
        return -1;
    }

    if (sigaction(SIGTERM, &action, NULL) < 0) {
        clog_error(CLOG_CGDB, "sigaction failed ");
        return -1;
    }

    if (sigaction(SIGQUIT, &action, NULL) < 0) {
        clog_error(CLOG_CGDB, "sigaction failed ");
        return -1;
    }

    return 0;
}

static int gdb_input(int fd)
{
    int result = tgdb_process(tgdb, fd);

    if (result == -1) {
        clog_error(CLOG_CGDB, "file descriptor closed");
        return -1;
    }

    return 0;
}

static int stdin_input()
{
    char buf[MAXLINE];
    size_t size;
    size_t i;

    size = read(STDIN_FILENO, buf, MAXLINE - 1);
    if (size == -1) {
        clog_error(CLOG_CGDB, "read error");
        return -1;
    }

    buf[size] = 0;

    /* Display GDB output 
     * The strlen check is here so that if_print does not get called
     * when displaying the filedlg. If it does get called, then the 
     * gdb window gets displayed when the filedlg is up
     */
    for (i = 0; i < size; ++i) {
        tgdb_send_char(tgdb, buf[i]);
    }

    return 0;
}

int main_loop(int gdbfd, int mifd)
{
    int max;
    fd_set rfds;
    int result;

    while (!gdb_quit) {
        max = (gdbfd > STDIN_FILENO) ? gdbfd : STDIN_FILENO;
        max = (max > mifd) ? max : mifd;

        /* Clear the set and 
         *
         * READ FROM:
         * stdin          (user or gui) 
         * master         (gdb's stdout)
         * gui_stdout     (gui's stdout sending new info)
         *
         */
        FD_ZERO(&rfds);

        /* Let the terminal emulate the char's when TGDB is busy */
        FD_SET(STDIN_FILENO, &rfds);
        FD_SET(gdbfd, &rfds);
        FD_SET(mifd, &rfds);

        result = select(max + 1, &rfds, NULL, NULL, NULL);

        /* if the signal interrupted system call keep going */
        if (result == -1 && errno == EINTR)
            continue;
        else if (result == -1)  /* on error ... must die -> stupid OS */
            clog_error(CLOG_CGDB, "select failed");

        /* stdin -> readline input */
        if (FD_ISSET(STDIN_FILENO, &rfds)) {
            stdin_input();
        }

        /* gdb's output -> stdout  */
        if (FD_ISSET(gdbfd, &rfds))
            if (gdb_input(gdbfd) == -1)
                return -1;

        /* gdb's mi output -> tgdb  */
        if (FD_ISSET(mifd, &rfds)) {
            if (gdb_input(mifd) == -1)
                return -1;
        }
    }

    return 0;
}

void console_output(void *context, const std::string &str) {
    std::string::const_iterator iter = str.begin();
    for (; iter != str.end(); ++iter) {
        if (write(STDOUT_FILENO, &*iter, 1) != 1) {
            clog_error(CLOG_CGDB, "could not write byte");
        }
    }
}

void command_response(void *context, struct tgdb_response *response)
{
    if (response->header == TGDB_QUIT) {
        fprintf(stderr, "%s:%d TGDB_QUIT\n", __FILE__, __LINE__);
        gdb_quit = 1;
    }
}

tgdb_callbacks callbacks = {
    NULL,
    console_output,
    command_response
};

int main(int argc, char **argv)
{
    int gdb_console_fd, gdb_mi_fd;

#if 0
    int c;

    read(0, &c, 1);
#endif

    start_logging();

    /* Set all clog levels to debug */
    clog_set_level(CLOG_CGDB_ID, CLOG_DEBUG);
    clog_set_level(CLOG_GDBIO_ID, CLOG_DEBUG);
    clog_set_level(CLOG_GDBMIIO_ID, CLOG_DEBUG);

    if (tty_cbreak(STDIN_FILENO, &term_attributes) == -1)
        clog_error(CLOG_CGDB, "tty_cbreak error");

    if ((tgdb = tgdb_initialize(callbacks)) == NULL) {
        clog_error(CLOG_CGDB, "tgdb_start error");
        goto driver_end;
    }

    if (tgdb_start_gdb(tgdb, NULL, argc - 1, argv + 1, 0, 0,
            &gdb_console_fd, &gdb_mi_fd ) == -1) {
        clog_error(CLOG_CGDB, "tgdb_start error");
        goto driver_end;
    }

    set_up_signal();

    main_loop(gdb_console_fd, gdb_mi_fd);

    if (tgdb_shutdown(tgdb) == -1)
        clog_error(CLOG_CGDB, "could not shutdown");

  driver_end:

    if (tty_set_attributes(STDIN_FILENO, &term_attributes) == -1)
        clog_error(CLOG_CGDB, "tty_reset error");

    tgdb_close_logfiles();
    return 0;
}
