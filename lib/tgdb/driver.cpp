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
#include "rline.h"
#include "fork_util.h"

#define MAXLINE 4096

struct tgdb *tgdb;

/** Master/Slave PTY used to keep readline off of stdin/stdout.  */
static pty_pair_ptr pty_pair;

/* Readline interface */
static struct rline *rline;
static int is_tab_completing = 0;
static int gdb_quit = 0;

/* Original terminal attributes */
static struct termios term_attributes;

static void change_prompt(char *prompt)
{
    rline_set_prompt(rline, prompt);
}

static void rlctx_send_user_command(char *line)
{
    /* This happens when rl_callback_read_char gets EOF */
    if (line == NULL)
        return;

    /* Don't add the enter command */
    if (line && *line != '\0')
        rline_add_history(rline, line);

    /* Send this command to TGDB */
    tgdb_request_run_console_command(tgdb, line);

    free(line);
}

static int tab_completion(int a, int b)
{
    char *cur_line;
    int ret;

    is_tab_completing = 1;

    ret = rline_get_current_line(rline, &cur_line);
    if (ret == -1)
        clog_error(CLOG_CGDB, "rline_get_current_line error\n");

    tgdb_request_complete(tgdb, cur_line);
    return 0;
}

/**
 * This is the function responsible for display the readline completions when there
 * is more than 1 item to complete. Currently it prints 1 per line.
 */
static void
readline_completion_display_func(char **matches, int num_matches,
        int max_length)
{
    int i;

    printf("\n");
    for (i = 1; i <= num_matches; ++i) {
        printf("%s", matches[i]);
        printf("\n");
    }
}

int do_tab_completion(char **completions)
{
    if (rline_rl_complete(rline, completions, &readline_completion_display_func) == -1)
    {
        clog_error(CLOG_CGDB, "rline_rl_complete error\n");
        return -1;
    }
    is_tab_completing = 0;

    return 0;
}

static void signal_handler(int signo)
{
    if (signo == SIGINT)
        rline_clear(rline);

    if (signo == SIGINT || signo == SIGTERM || signo == SIGQUIT)
        tgdb_signal_notification(tgdb, signo);

    is_tab_completing = 0;
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

static void driver_prompt_change(const char *new_prompt)
{
    char *nprompt = (char *) new_prompt;

    change_prompt(nprompt);
}

static int gdb_input(void)
{
    int result = tgdb_process(tgdb);

    if (result == -1) {
        clog_error(CLOG_CGDB, "file descriptor closed");
        return -1;
    }

    return 0;
}

static void tty_input(void)
{
    char buf[MAXLINE];
    size_t size;
    size_t i;

    if ((size = tgdb_recv_inferior_data(tgdb, buf, MAXLINE)) == -1) {
        clog_error(CLOG_CGDB, "file descriptor closed");
        return;
    }
    /* end if */
    for (i = 0; i < size; ++i)
        if (write(STDOUT_FILENO, &(buf[i]), 1) != 1) {
            clog_error(CLOG_CGDB, "could not write byte");
            return;
        }
}

static int readline_input()
{
    char buf[MAXLINE];
    size_t size;
    size_t i;

    int masterfd = pty_pair_get_masterfd(pty_pair);

    if (masterfd == -1) {
        clog_error(CLOG_CGDB, "pty_pair_get_masterfd error");
        return -1;
    }

    size = read(masterfd, buf, MAXLINE - 1);
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
    for (i = 0; i < size; ++i)
        if (write(STDOUT_FILENO, &(buf[i]), 1) != 1) {
            clog_error(CLOG_CGDB, "could not write byte");
            return -1;
        }

    return 0;
}

static int stdin_input()
{
    char buf[MAXLINE];
    size_t size;
    size_t i;

    int masterfd = pty_pair_get_masterfd(pty_pair);

    if (masterfd == -1) {
        clog_error(CLOG_CGDB, "pty_pair_get_masterfd error");
        return -1;
    }

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
    for (i = 0; i < size; ++i)
        if (write(masterfd, &(buf[i]), 1) != 1) {
            clog_error(CLOG_CGDB, "could not write byte");
            return -1;
        }

    return 0;
    return 0;
}

int main_loop(int gdbfd)
{
    int max;
    fd_set rfds;
    int result;

    int masterfd, slavefd, childfd;

    masterfd = pty_pair_get_masterfd(pty_pair);
    if (masterfd == -1) {
        clog_error(CLOG_CGDB, "pty_pair_get_masterfd error");
        return -1;
    }

    slavefd = pty_pair_get_slavefd(pty_pair);
    if (slavefd == -1) {
        clog_error(CLOG_CGDB, "pty_pair_get_slavefd error");
        return -1;
    }

    /* When TGDB is ready, we read from STDIN, otherwise, leave the data buffered. */

    while (!gdb_quit) {

        /* get max fd  for select loop */
        childfd = tgdb_get_inferior_fd(tgdb);

        max = (gdbfd > STDIN_FILENO) ? gdbfd : STDIN_FILENO;
        max = (max > childfd) ? max : childfd;
        max = (max > slavefd) ? max : slavefd;
        max = (max > masterfd) ? max : masterfd;

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
        FD_SET(slavefd, &rfds);
        FD_SET(masterfd, &rfds);

        if (childfd != -1)
            FD_SET(childfd, &rfds);

        result = select(max + 1, &rfds, NULL, NULL, NULL);

        /* if the signal interrupted system call keep going */
        if (result == -1 && errno == EINTR)
            continue;
        else if (result == -1)  /* on error ... must die -> stupid OS */
            clog_error(CLOG_CGDB, "select failed");

        /* Input received through the pty:  Handle it
         * Readline read from slavefd, and it wrote to the masterfd. 
         */
        if (FD_ISSET(masterfd, &rfds))
            if (readline_input() == -1)
                return -1;

        if (FD_ISSET(slavefd, &rfds))
            rline_rl_callback_read_char(rline);

        /* stdin -> readline input */
        if (FD_ISSET(STDIN_FILENO, &rfds)) {
            stdin_input();
        }

        /* child's output -> stdout */
        if (childfd != -1 && FD_ISSET(childfd, &rfds)) {
            tty_input();
            continue;
        }

        /* gdb's output -> stdout  */
        if (FD_ISSET(gdbfd, &rfds))
            if (gdb_input() == -1)
                return -1;
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

void console_ready(void *context)
{
    rline_rl_forced_update_display(rline);
}

void request_sent(void *context, struct tgdb_request *request,
        const std::string &command)
{
    if (request->header == TGDB_REQUEST_CONSOLE_COMMAND &&
        request->choice.console_command.queued) {
        char *prompt;
        rline_get_prompt(rline, &prompt);
        if (prompt) {
            printf("%s", prompt);
        }
        printf("%s\n", request->choice.console_command.command);
    }
}

void command_response(void *context, struct tgdb_response *response)
{
    if (response->header == TGDB_UPDATE_COMPLETIONS)
    {
        char **completions = response->choice.update_completions.completions;

        do_tab_completion(completions);
    }

    if (response->header == TGDB_UPDATE_CONSOLE_PROMPT_VALUE)
        driver_prompt_change(response->choice.update_console_prompt_value.
                prompt_value);

    if (response->header == TGDB_QUIT) {
        fprintf(stderr, "%s:%d TGDB_QUIT\n", __FILE__, __LINE__);
        gdb_quit = 1;
    }
}

tgdb_callbacks callbacks = {
    NULL,
    console_output,
    console_ready,
    request_sent,
    command_response
};

int main(int argc, char **argv)
{
    int gdb_fd, slavefd, masterfd;

#if 0
    int c;

    read(0, &c, 1);
#endif

    if (tty_cbreak(STDIN_FILENO, &term_attributes) == -1)
        clog_error(CLOG_CGDB, "tty_cbreak error");

    pty_pair = pty_pair_create();
    if (!pty_pair) {
        fprintf(stderr, "%s:%d Unable to create PTY pair", __FILE__, __LINE__);
        exit(-1);
    }

    slavefd = pty_pair_get_slavefd(pty_pair);
    if (slavefd == -1) {
        fprintf(stderr, "%s:%d Unable to get slavefd", __FILE__, __LINE__);
        exit(-1);
    }

    masterfd = pty_pair_get_masterfd(pty_pair);
    if (masterfd == -1) {
        fprintf(stderr, "%s:%d Unable to get masterfd", __FILE__, __LINE__);
        exit(-1);
    }

    if (tty_off_xon_xoff(masterfd) == -1)
        exit(-1);

    rline = rline_initialize(slavefd, rlctx_send_user_command, tab_completion,
            getenv("TERM"));

    if ((tgdb = tgdb_initialize(NULL, argc - 1, argv + 1, &gdb_fd,
            callbacks)) == NULL) {
        clog_error(CLOG_CGDB, "tgdb_start error");
        goto driver_end;
    }

    /* Set all clog levels to debug */
    clog_set_level(CLOG_CGDB_ID, CLOG_DEBUG);
    clog_set_level(CLOG_GDBIO_ID, CLOG_DEBUG);

    set_up_signal();

    main_loop(gdb_fd);

    if (tgdb_shutdown(tgdb) == -1)
        clog_error(CLOG_CGDB, "could not shutdown");

  driver_end:

    if (tty_set_attributes(STDIN_FILENO, &term_attributes) == -1)
        clog_error(CLOG_CGDB, "tty_reset error");

    tgdb_close_logfiles();
    return 0;
}
