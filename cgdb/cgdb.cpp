#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif /* HAVE_FCNTL_H */

#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */

#if HAVE_SIGNAL_H
#include <signal.h>
#endif

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif /* HAVE_STDLIB_H */

#if HAVE_STDIO_H
#include <stdio.h>
#endif /* HAVE_STDIO_H */

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif /* HAVE_SYS_SELECT_H */

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#if HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif /* HAVE_SYS_IOCTL_H */

#if HAVE_ERRNO_H
#include <errno.h>
#endif /* HAVE_ERRNO_H */

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#if HAVE_CTYPE_H
#include <ctype.h>
#endif

#if HAVE_LOCALE_H
#include <locale.h>
#endif

#include <string>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

/* Local Includes */
#include "sys_util.h"
#include "stretchy.h"
#include "sys_win.h"
#include "cgdb.h"
#include "tokenizer.h"
#include "highlight_groups.h"
#include "interface.h"
#include "scroller.h"
#include "sources.h"
#include "tgdb.h"
#include "kui_ctx.h"
#include "kui_map_set.h"
#include "kui_term.h"
#include "kui_manager.h"
#include "fs_util.h"
#include "cgdbrc.h"
#include "io.h"
#include "fork_util.h"
#include "terminal.h"
#include "rline.h"
#include "usage.h"

/* --------- */
/* Constants */
/* --------- */

#define GDB_MAXBUF 4096         /* GDB input buffer size */

/* --------------- */
/* Local Variables */
/* --------------- */

struct tgdb *tgdb;              /* The main TGDB context */

std::string cgdb_home_dir; /* Path to home dir with trailing slash */
std::string cgdb_log_dir;  /* Path to log dir with trailing slash */

static int gdb_console_fd = -1; /* GDB console descriptor */
static int gdb_mi_fd = -1;      /* GDB Machine Interface descriptor */
static bool new_ui_unsupported = false;

static char *debugger_path = NULL;  /* Path to debugger to use */

/* Set to 1 if the user requested cgdb to wait for the debugger to attach. */
static int wait_for_debugger_to_attach = 0;

std::unique_ptr<kui_manager> kui_ctx; /* The key input package */

std::shared_ptr<kui_map_set> kui_map;
std::shared_ptr<kui_map_set> kui_imap;

/**
 * This allows CGDB to know if it is acceptable to read more input from
 * the KUI at this particular moment. Under certain circumstances, CGDB may 
 * have to complete a particular communication message to GDB, in while doing
 * so, may not want to process's the users keystrokes. This flag is purely for
 * CGDB to keep track of if it wants to read from the KUI. The KUI is always
 * ready.
 *
 * If kui_input_acceptable is set to 1, then input can be read from
 * the kui. Otherwise, some part of CGDB is waiting for more events to happen.
 *
 * For example, when the user types 'o' at the CGDB source window, the
 * user is requesting the file dialog to open. CGDB must first ask GDB for 
 * the list of files. While it is retreiving these files, CGDB shouldn't 
 * shoot through the rest of the kui keys available. If the user had
 * typed 'o /main' they would want to open the file dialog and start 
 * searching for a file that had the substring 'main' in it. So, CGDB must
 * first ensure all the files are retrieved, displayed in the file
 * dialog, and ensure the file dialog is ready to receive keys from the 
 * user before the input is allowed to hit the file dialog.
 */
int kui_input_acceptable = 1;

/**
 * This pipe is used for passing SIGWINCH from the handler to the main loop.
 *
 * A separate pipe is used for SIGWINCH because when the user resizes the
 * terminal many signals may be sent to the application. Each time a signal
 * is received it is written to this pipe to be processed in the main loop.
 * The main loop is more efficient if it only has to process the last
 * resize, since all the intermediate signals will be of no use. By creating
 * a separate pipe, the main loop can easily detect if more SIGWINCH signals
 * have been received and ignore them.
 *
 * I'm not sure if this optimization is still necessary as it was needed
 * around the year 2005. It may or may not still matter, but the logic
 * overhead isn't that complex anyways.
 */
int resize_pipe[2] = { -1, -1 };

/**
 * This pipe is used for passing signals from the handler to the main loop.
 *
 * When a signal is sent to CGDB (ie. SIGINT or SIGQUIT), the signal is
 * passed to the signal handler. Only certain functions can be called from
 * a signal handler so some functionality must be handled in the main loop.
 *
 * This pipe is the mechanism that is used to pass to the main loop the fact
 * that a signal was received in a signal handler. This way, when the main
 * loop detects the signal pipe has information it can read the signal and
 * process it safely (by calling any necessary functions).
 */
int signal_pipe[2] = { -1, -1 };

/* Original terminal attributes */
static struct termios term_attributes;

/**
 * Runs a command in the shell.  The shell may be interactive, and CGDB
 * will be paused for the duration of the shell.  Any leading stuff, like
 * 'shell ' or '!' should be removed prior to calling this function.
 *
 * \param command The command to run at the shell.  Empty string or null
 *                means to invoke an interactive shell.
 *
 * \return The exit status of the system() call.
 */
int run_shell_command(const char *command)
{
    int rv;

    /* Cleanly scroll the screen up for a prompt */
    swin_scrl(1);
    swin_move(swin_lines() - 1, 0);
    printf("\n");

    /* Put the terminal in cooked mode and turn on echo */
    swin_endwin();
    tty_set_attributes(STDIN_FILENO, &term_attributes);

    /* NULL or empty string means invoke user's shell */
    if (!command || !command[0]) {
        /* Check for SHELL environment variable */
        char *shell = getenv("SHELL");

        rv = system(shell ? shell : "/bin/sh");
    } else {
        /* Execute the command passed in via system() */
        rv = system(command);
    }

    /* Press any key to continue... */
    fprintf(stderr, "Hit ENTER to continue...");
    while (fgetc(stdin) != '\n') {
    }

    /* Turn off echo and put the terminal back into raw mode */
    tty_cbreak(STDIN_FILENO, &term_attributes);
    if_draw();

    return rv;
}

static void parse_cgdbrc_file()
{
    std::string config_file = fs_util_get_path(cgdb_home_dir, "cgdbrc");
    command_parse_file(config_file.c_str());
}

/* ------------------------ */
/* Initialization functions */
/* ------------------------ */

/* version_info: Returns version information about cgdb.
 * ------------- 
 *
 * Return:  A pointer to a static buffer, containing version info.
 */
static char *version_info(void)
{
    static char buf[MAXLINE];

    snprintf(buf, sizeof(buf), "%s %s\r\n%s", "CGDB", VERSION,
            "Copyright 2002-2022 Bob Rossi and Mike Mueller.\n"
            "CGDB is free software, covered by the GNU General Public License, and you are\n"
            "welcome to change it and/or distribute copies of it under certain conditions.\n"
            "There is absolutely no warranty for CGDB.\n");
    return buf;
}

static void parse_long_options(int *argc, char ***argv)
{
    int c, option_index = 0, n = 1;
    const char *args = "wd:hv";

#ifdef HAVE_GETOPT_H
    static struct option long_options[] = {
        {"version", 0, 0, 0},
        {"help", 0, 0, 0},
        {0, 0, 0, 0}
    };
#endif

    while (1) {
        opterr = 0;
#ifdef HAVE_GETOPT_H
        c = getopt_long(*argc, *argv, args, long_options, &option_index);
#else
        c = getopt(*argc, *argv, args);
#endif
        if (c == -1)
            break;

        if (((char) c) == '?')
            break;

        switch (c) {
            case 0:
                switch (option_index) {
                    case 0:
                        printf("%s", version_info());
                        exit(0);
                    case 1:
                        usage();
                        exit(0);
                    default:
                        break;
                }
                break;
            case 'v':
                printf("%s", version_info());
                exit(0);
            case 'w':
                wait_for_debugger_to_attach = 1;
                n++;
                break;
            case 'd':
                debugger_path = strdup(optarg);
                if (optarg == (*argv)[n + 1]) {
                    /* optarg is in next argv (-d foo) */
                    n += 2;
                } else {
                    /* optarg is in this argv (-dfoo) */
                    n++;
                }
                break;
            case 'h':
                usage();
                exit(0);
            default:
                break;
        }
    }

    *argc -= n;
    *argv += n;

    if (**argv && strcmp(**argv, "--") == 0) {
        (*argc)--;
        (*argv)++;
    }
}

/**
 * Attempts to create a config directory in the user's home directory.
 *
 * After being called successfully, both cgdb_home_dir and cgdb_log_dir
 * are set.
 *
 * @return
 * 0 on success or -1 on error
 */
static int init_home_dir(void)
{
    /* Check for a nonstandard cgdb dir location */
    char *cgdb_home_envvar = getenv("CGDB_DIR"); 

    /* Set the cgdb home directory */
    if (cgdb_home_envvar != NULL) { 
        cgdb_home_dir = cgdb_home_envvar;
    } else { 
        cgdb_home_dir = fs_util_get_path(getenv("HOME"), ".cgdb");
    }

    /* Make sure the toplevel cgdb dir exists */
    if (!fs_util_create_dir(cgdb_home_dir)) {
        printf("Exiting, could not create home directory:\n  %s\n",
                cgdb_home_dir.c_str());
        return -1;
    }

    /* Try to create log directory */
    cgdb_log_dir = fs_util_get_path(cgdb_home_dir, "logs");
    if (!fs_util_create_dir(cgdb_log_dir)) {
        printf("Exiting, could not create log directory:\n  %s\n",
                cgdb_log_dir.c_str());
        return -1;
    }

    return 0;
}

/**
 * Console output has returned from GDB, show it.
 *
 * @param context
 * Unused at the moment
 *
 * @param str
 * The console output to display
 */
static void console_output(void *context, const std::string &str) {
    if_print(str.c_str());
}

static void command_response(void *context, struct tgdb_response *response);
            
tgdb_callbacks callbacks = { 
    NULL,       
    console_output,
    command_response
};


/* start_gdb: Starts up libtgdb
 *  Returns:  -1 on error, 0 on success
 */
static int start_gdb(int argc, char *argv[])
{
    // Note, the 40 height by 80 width size here will be overriden
    // once cgdb determines the actual size. This is done in main() in
    // the call to if_layout just after if_init.
    return tgdb_start_gdb(tgdb, debugger_path, argc, argv, get_gdb_height(),
            get_gdb_width(), &gdb_console_fd, &gdb_mi_fd);
}

static void send_key(int focus, char key)
{
    if (focus == 1) {
        tgdb_send_char(tgdb, key);
    }
}

/* user_input: This function will get a key from the user and process it.
 *
 *  Returns:  -1 on error, 0 on success
 */
static int user_input(void)
{
    static int key, val;

    if (if_get_focus() == CGDB)
        kui_ctx->set_map_set(kui_map);
    else if (if_get_focus() == GDB)
        kui_ctx->set_map_set(kui_imap);
    else
        kui_ctx->clear_map_set();

    key = kui_ctx->getkey();
    if (key == -1) {
        clog_error(CLOG_CGDB, "kui_manager_getkey error");
        return -1;
    }

    val = if_input(key);

    if (val == -1) {
        clog_error(CLOG_CGDB, "if_input error");
        return -1;
    } else if (val != 1 && val != 2)
        return 0;

    /* Process the key */
    if (kui_term_is_cgdb_key(key)) {
        const char *seqbuf = kui_term_get_ascii_char_sequence_from_key(key);

        if (seqbuf == NULL) {
            clog_error(CLOG_CGDB,
                "kui_term_get_ascii_char_sequence_from_key error %d", key);
            return -1;
        } else {
            int length = strlen(seqbuf), i;

            for (i = 0; i < length; i++)
                send_key(val, seqbuf[i]);
        }
    } else
        send_key(val, key);

    return 0;
}

/**
 * This will usually process all the input that the KUI has.
 *
 * However, it's possible that one of the keys the user sends to CGDB 
 * switches the state of CGDB in such a way, that CGDB has to do some I/O
 * with GDB before the keys can continue to be sent to CGDB. For this reason,
 * this loop can return before all the KUI's data has been used, in order to
 * give the main loop a chance to run a GDB command.
 *
 * \return
 * 0 on success or -1 on error
 */
static int user_input_loop()
{
    do {
        /* There are reasons that CGDB should wait to get more info from the kui.
         * See the documentation for kui_input_acceptable */
        if (!kui_input_acceptable)
            return 0;

        if (user_input() == -1) {
            clog_error(CLOG_CGDB, "user_input_loop failed");
            return -1;
        }
    } while (kui_ctx->cangetkey());

    return 0;
}

/* This updates all the breakpoints */
static void update_breakpoints(struct tgdb_response *response)
{
    source_set_breakpoints(if_get_sview(),
        response->choice.update_breakpoints.breakpoints);
    if_show_file(NULL, 0, 0);
}

/* This means a source file or line number changed */
static void update_file_position(struct tgdb_response *response)
{
    /**
     * Updating the location that cgdb should point the user to.
     *
     * A variety of different locations can come back from gdb.
     * We currently have the following fields of interest:
     *   path, line number and address.
     *
     * The path may be NULL, relative or absolute. When it is
     * relative or absolute, it might point to a file that does
     * not exist on disk.
     *
     * The address or line number are not always available.
     *
     * So currently, our best bet is to
     * - show the file/line
     * - if file/line unavailable, show the function disassembly
     * - if function disassembly unavailable, show disassembly
     * - if disassembly unavailable, show nothing
     */
    struct tgdb_file_position *tfp;
    struct sviewer *sview = if_get_sview();
    int source_reload_status = -1;

    tfp = response->choice.update_file_position.file_position;
    
    /* Tell source viewer what the current $pc address is. */
    sview->addr_frame = tfp->addr;

    /* If we got a pathname and we're not showing disasm... */
    if (tfp->path && !cgdbrc_get_int(CGDBRC_DISASM)) {

        /**
         * GDB will provide an executing line number even when the program
         * has not been started yet. Checking the frame address as well
         * helps cgdb to not show an executing line unless there is a stack,
         * which tells us the inferior is running.
         */
        int exe_line = sview->addr_frame ? tfp->line_number : -1;

        /* Update the file */
        source_reload_status = 
            source_reload(if_get_sview(), tfp->path, 0);

        /* Show the source file at this line number */
        if_show_file(tfp->path, tfp->line_number, exe_line);
    }

    /**
     * Sometimes gdb provides a path that can not be found
     * on disk. For instance, for glibc where the source isn't
     * available. In this scenario, show the assembly instead.
     */
    if (source_reload_status == -1 && sview->addr_frame) {
        int ret;

        /* Try to show the disasm for the current $pc address */
        ret = source_set_exec_addr(sview, sview->addr_frame);

        if (!ret) {
            if_draw();
        } else if (sview->addr_frame) {
            /* No disasm found - request it */
            tgdb_request_disassemble_func(tgdb,
                DISASSEMBLE_FUNC_SOURCE_LINES);
            // If the disassembly view was not available and is attempting
            // to be loaded, then reload the breakpoints afterwards, so they
            // can be associated with the disassembly view as well
            tgdb_request_breakpoints(tgdb);
        }
    }
}

/* This is a list of all the source files */
static void update_source_files(struct tgdb_response *response)
{
    char **source_files = response->choice.update_source_files.source_files;
    sviewer *sview = if_get_sview();
    struct list_node *cur;
    int added_disasm = 0;

    if_clear_filedlg();

    /* Search for a node which contains this address */
    for (cur = sview->list_head; cur != NULL; cur = cur->next) {
        if (cur->path[0] == '*')
        {
            added_disasm = 1;
            if_add_filedlg_choice(cur->path);
        }
    }

    if (!sbcount(source_files) && !added_disasm) {
        /* No files returned? */
        if_display_message(WIN_REFRESH, "Error:",
            " No sources available! Was the program compiled with debug?");
    } else {
        int i;

        for (i = 0; i < sbcount(source_files); i++) {
            if_add_filedlg_choice(source_files[i]);
        }

        if_set_focus(FILE_DLG);
    }
    kui_input_acceptable = 1;
}

static void update_disassemble(struct tgdb_response *response)
{
    if (response->choice.disassemble_function.error) {
        //$ TODO mikesart: Get module name in here somehow? Passed in when calling tgdb_request_disassemble?
        //      or info sharedlibrary?
        //$ TODO mikesart: Need way to make sure we don't recurse here on error.
        //$ TODO mikesart: 100 lines? Way to load more at end?

        if (response->header == TGDB_DISASSEMBLE_PC) {
            /* Spew out a warning about disassemble failing
             * and disasm next 100 instructions. */
            if_print_message("\nWarning: disassemble address 0x%" PRIx64 " failed.\n",
                response->choice.disassemble_function.addr_start);
        } else {
            tgdb_request_disassemble_pc(tgdb, 100);
        }
    } else {
        uint64_t addr_start = response->choice.disassemble_function.addr_start;
        uint64_t addr_end = response->choice.disassemble_function.addr_end;
        char **disasm = response->choice.disassemble_function.disasm;

        //$ TODO: If addr_start is equal to addr_end of some other
        // buffer, then append it to that buffer?

        //$ TODO: If there is a disassembly view, update the location
        // even if we don't display it? Useful with global marks, etc.

        if (disasm && disasm[0]) {
            int i;
            char *path;
            struct list_node *node;
            sviewer *sview = if_get_sview();

            if (addr_start) {
                path = sys_aprintf(
                    "** %s (%" PRIx64 " - %" PRIx64 ") **",
                    disasm[0], addr_start, addr_end);
            } else {
                path = sys_aprintf("** %s **", disasm[0]);
            }

            node = source_get_node(sview, path);
            if (!node) {
                node = source_add(sview, path);

                //$ TODO mikesart: Add asm colors
                node->language = TOKENIZER_LANGUAGE_ASM;
                node->addr_start = addr_start;
                node->addr_end = addr_end;

                for (i = 0; i < sbcount(disasm); i++) {
                    source_add_disasm_line(node, disasm[i]);
                }

                source_highlight(node);
            }

            source_set_exec_addr(sview, sview->addr_frame);
            if_draw();

            free(path);
        }
    }
}

static void command_response(void *context, struct tgdb_response *response)
{
    switch (response->header)
    {
    case TGDB_UPDATE_BREAKPOINTS:
        update_breakpoints(response);
        break;
    case TGDB_UPDATE_FILE_POSITION:
        update_file_position(response);
        break;
    case TGDB_UPDATE_SOURCE_FILES:
        update_source_files(response);
        break;
    case TGDB_DISASSEMBLE_PC:
    case TGDB_DISASSEMBLE_FUNC:
        update_disassemble(response);
        break;
    case TGDB_QUIT:
        new_ui_unsupported = response->choice.quit.new_ui_unsupported;
        cgdb_cleanup_and_exit(0);
        break;
    }
}

/* gdb_input: Receives data from tgdb:
 *
 *  Returns:  -1 on error, 0 on success
 */
static int gdb_input(int fd)
{
    int result;

    /* Read from GDB */
    result = tgdb_process(tgdb, fd);
    if (result == -1) {
        clog_error(CLOG_CGDB, "tgdb_process error");
        return -1;
    }

    return 0;
}

static int cgdb_resize_term(int fd)
{
    int c, result;

    if (read(fd, &c, sizeof (int)) < sizeof (int)) {
        clog_error(CLOG_CGDB, "read from resize pipe");
        return -1;
    }

    /* If there is more input in the pipe, that means another resize has
     * been received, and we still have not handled this one. So, skip this
     * one and only handle the next one.
     */
    result = io_data_ready(fd, 0);
    if (result == -1) {
        clog_error(CLOG_CGDB, "io_data_ready");
        return -1;
    }

    if (result)
        return 0;

    if (if_resize_term() == -1) {
        clog_error(CLOG_CGDB, "if_resize_term error");
        return -1;
    }

    return 0;
}

/**
 * Handles the signals that were received from the main loop.
 *
 * Initially the signals are sent to the signal handler. The signal handler
 * writes those values to a pipe which are detected in the main loop and
 * sent here.
 *
 * @param fd
 * The file descriptor to read the signal number from.
 *
 * @return
 * 0 on success or -1 on error
 */
static int cgdb_handle_signal_in_main_loop(int fd)
{
    int signo;

    if (read(fd, &signo, sizeof(int)) < sizeof(int)) {
        clog_error(CLOG_CGDB, "read from signal pipe");
        return -1;
    }

    tgdb_signal_notification(tgdb, signo);

    return 0;
}

static int main_loop(void)
{
    fd_set rset;
    int max;

    /* Main (infinite) loop:
     *   Sits and waits for input on either stdin (user input) or the
     *   GDB file descriptor.  When input is received, wrapper functions
     *   are called to process the input, and handle it appropriately.
     *   This will result in calls to the curses interface, typically. */

    for (;;) {
        max = (gdb_console_fd > STDIN_FILENO) ? gdb_console_fd : STDIN_FILENO;
        max = (max > resize_pipe[0]) ? max : resize_pipe[0];
        max = (max > signal_pipe[0]) ? max : signal_pipe[0];
        max = (max > gdb_mi_fd) ? max :gdb_mi_fd;

        /* Reset the fd_set, and watch for input from GDB or stdin */
        FD_ZERO(&rset);
        FD_SET(STDIN_FILENO, &rset);
        FD_SET(gdb_console_fd, &rset);
        FD_SET(resize_pipe[0], &rset);
        FD_SET(signal_pipe[0], &rset);
        FD_SET(gdb_mi_fd, &rset);

        /* Wait for input */
        if (select(max + 1, &rset, NULL, NULL, NULL) == -1) {
            if (errno == EINTR)
                continue;
            else {
                clog_error(CLOG_CGDB, "select failed: %s", strerror(errno));
                return -1;
            }
        }

        /* A signal occurred (besides SIGWINCH) */
        if (FD_ISSET(signal_pipe[0], &rset))
            if (cgdb_handle_signal_in_main_loop(signal_pipe[0]) == -1)
                return -1;

        /* A resize signal occurred */
        if (FD_ISSET(resize_pipe[0], &rset))
            if (cgdb_resize_term(resize_pipe[0]) == -1)
                return -1;

        /* Input received:  Handle it */
        if (FD_ISSET(STDIN_FILENO, &rset)) {
            int val = user_input_loop();

            /* The below condition happens on cygwin when user types ctrl-z
             * select returns (when it shouldn't) with the value of 1. the
             * user input loop gets called, the kui gets called and does a
             * non blocking read which returns EAGAIN. The kui then passes
             * the -1 up the stack with out making any more system calls. */
            if (val == -1 && errno == EAGAIN)
                continue;
            else if (val == -1)
                return -1;
        }

        /* gdb's output -> stdout */
        if (FD_ISSET(gdb_console_fd, &rset)) {
            if (gdb_input(gdb_console_fd) == -1) {
                return -1;
            }

            /* When the file dialog is opened, the user input is blocked, 
             * until GDB returns all the files that should be displayed,
             * and the file dialog can open, and be prepared to receive 
             * input. So, if we are in the file dialog, and are no longer
             * waiting for the gdb command, then read the input.
             */
            if (kui_ctx->cangetkey()) {
                user_input_loop();
            }
        }

        if (FD_ISSET(gdb_mi_fd, &rset)) {
            if (gdb_input(gdb_mi_fd) == -1) {
                return -1;
            }
        }
    }
    return 0;
}

/* ----------------- */
/* Exposed Functions */
/* ----------------- */

/* cgdb_cleanup_and_exit: Invoked by the various err_xxx funtions when dying.
 * -------- */
void cgdb_cleanup_and_exit(int val)
{
    /* Cleanly scroll the screen up for a prompt */
    swin_scrl(1);
    swin_move(swin_lines() - 1, 0);
    printf("\n");

    /* The order of these is important. They each must restore the terminal
     * the way they found it. Thus, the order in which curses/readline is 
     * started, is the reverse order in which they should be shutdown 
     */

    swin_endwin();

    /* Shut down interface */
    if_shutdown();

    hl_groups_shutdown(hl_groups_instance);

    /* Finally, should display the errors. 
     * TGDB guarantees the logger to be open at this point.
     * So, we can get the filename directly from the logger 
     */

    /* Shut down debugger */
    tgdb_shutdown(tgdb);

    if (tty_set_attributes(STDIN_FILENO, &term_attributes) == -1)
        clog_error(CLOG_CGDB, "tty_reset error");

    /* Close our logfiles */
    tgdb_close_logfiles();

    /**
     * If the cgdb log file has non-zero size, alert the user.
     */
    if (clog_did_error_occur())
    {
        fprintf(stderr,
            "CGDB had unexpected results."
            " Search the logs for more details.\n"
            " CGDB log directory: %s\n"
            " Lines beginning with ERROR: are an issue.\n",
            cgdb_log_dir.c_str());
    }

    if (new_ui_unsupported) {
        fprintf(stderr, "cgdb requires gdb 7.12 or later\n");
    }

    exit(val);
}

int init_resize_pipe(void)
{
    if (pipe(resize_pipe) == -1) {
        clog_error(CLOG_CGDB, "pipe error");
        return -1;
    }

    return 0;
}

int init_signal_pipe(void)
{
    int result = pipe(signal_pipe);

    if (result == -1) {
        clog_error(CLOG_CGDB, "pipe error");
    }

    return result;
}

static void rlctx_send_user_command(char *line)
{
}
static int tab_completion(int a, int b)
{
    return 0;
}

int update_kui(cgdbrc_config_option_ptr option)
{
    kui_ctx->set_terminal_escape_sequence_timeout(
            cgdbrc_get_key_code_timeoutlen());
    kui_ctx->set_key_mapping_timeout(
            cgdbrc_get_mapped_key_timeoutlen());

    return 0;
}

int add_readline_key_sequence(const char *readline_str, enum cgdb_key key)
{
    int result;
    std::list<std::string> keyseq;

    result = rline_get_keyseq(readline_str, keyseq);
    if (result == 0) {
         result = kui_ctx->get_terminal_keys_kui_map(key, keyseq);
    }

    return result;
}

int init_kui(void)
{
    kui_ctx = kui_manager::create(STDIN_FILENO,
                                  cgdbrc_get_key_code_timeoutlen(),
                                  cgdbrc_get_mapped_key_timeoutlen());
    if (!kui_ctx) {
        clog_error(CLOG_CGDB, "Unable to initialize input library");
        cgdb_cleanup_and_exit(-1);
    }

    kui_map = std::make_shared<kui_map_set>();
    kui_imap = std::make_shared<kui_map_set>();

    kui_ctx->set_map_set(kui_map);

    /* Combine the cgdbrc config package with libkui. If any of the options
     * below change, update the KUI.  Currently, the handles are not kept around,
     * because CGDB never plans on detaching. */
    cgdbrc_attach(CGDBRC_TIMEOUT, &update_kui);
    cgdbrc_attach(CGDBRC_TIMEOUT_LEN, &update_kui);
    cgdbrc_attach(CGDBRC_TTIMEOUT, &update_kui);
    cgdbrc_attach(CGDBRC_TTIMEOUT_LEN, &update_kui);

    /* It's important that CGDB uses readline's view of 
     * Home and End keys. A few distros I've run into (redhat e3
     * and ubuntu) provide incorrect terminfo entries for xterm.
     * So, Home and End do not work. The distro's fixed readline
     * by modifying /etc/inputrc to hard code the terminal sequences.
     * I have no idea why they wouldn't just fix the terminfo 
     * database, but they didn't! Therefor, readline, bash, gdb all
     * work but cgdb doesn't. So, I'm going to simply ask readline
     * what it thinks the Home and End keys are and add them to 
     * CGDB's mappings.
     */

    /* For now, I've decided it's OK for these functions to fail as they
     * only add functionality to CGDB. */

    /* Home key */
    add_readline_key_sequence("beginning-of-line", CGDB_KEY_HOME);
    /* End key */
    add_readline_key_sequence("end-of-line", CGDB_KEY_END);
    /* Backword-Word */
    add_readline_key_sequence("backward-word", CGDB_KEY_BACKWARD_WORD);
    /* Forward-word */
    add_readline_key_sequence("forward-word", CGDB_KEY_FORWARD_WORD);
    /* Backword-Kill-Word */
    add_readline_key_sequence(
            "backward-kill-word", CGDB_KEY_BACKWARD_KILL_WORD);
    /* Forward-Kill-word */
    add_readline_key_sequence(
            "kill-word", CGDB_KEY_FORWARD_KILL_WORD);

    return 0;
}

/**
 * Create the log files.
 */
static void cgdb_start_logging()
{
    /* Open our cgdb and tgdb io logfiles */
    clog_open(CLOG_CGDB_ID, "%s/cgdb_log%d.txt", cgdb_log_dir);
    clog_open(CLOG_GDBIO_ID, "%s/cgdb_gdb_console_io_log%d.txt", cgdb_log_dir);
    clog_open(CLOG_GDBMIIO_ID, "%s/cgdb_gdb_mi_io_log%d.txt", cgdb_log_dir);

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

int main(int argc, char *argv[])
{
    parse_long_options(&argc, &argv);

    // Create the home directory and the log directory
    if (init_home_dir() == -1) {
        exit(-1);
    }

    /* Debugging helper - wait for debugger to attach to us before continuing */
    if (wait_for_debugger_to_attach) {
        if (cgdb_supports_debugger_attach_detection()) {
            printf("Waiting for debugger to attach...\n");
            while (cgdb_is_debugger_attached() == 0) {
                sleep(1);
            }
        } else {
            int c;
            printf("Press any key to continue execution...\n");
            read(0, &c, 1);
        }
    }

    cgdb_start_logging();

    /* Initialize default option values */
    cgdbrc_init();

    tgdb = tgdb_initialize(callbacks);
    if (!tgdb) {
        fprintf(stderr, "%s:%d Unable to initialize tgdb\n",
                __FILE__, __LINE__);
        exit(-1);
    }

    /* From here on, the logger is initialized */

    setlocale(LC_CTYPE, "");

    if (tty_cbreak(STDIN_FILENO, &term_attributes) == -1) {
        clog_error(CLOG_CGDB, "tty_cbreak error");
        cgdb_cleanup_and_exit(-1);
    }

    if (init_kui() == -1) {
        clog_error(CLOG_CGDB, "init_kui error");
        cgdb_cleanup_and_exit(-1);
    }

    /* Initialize curses */
    if (!swin_start()) {
        clog_error(CLOG_CGDB, "Unable to start curses");
    }

    /* Initialize the highlighting groups */
    hl_groups_instance = hl_groups_initialize();
    if (!hl_groups_instance)
    {
        clog_error(CLOG_CGDB, "Unable to setup highlighting groups");
        cgdb_cleanup_and_exit(-1);
    }

    /* Parse the cgdbrc file. Note that we are doing this before
       if_init() is called so windows and highlight groups haven't
       been created yet. We need to do this here because some options
       can disable color, ansi escape parsing, or set Logo color.
    */
    parse_cgdbrc_file();

    /* Initialize the display */
    if (if_init() == -1)
    {
        clog_error(CLOG_CGDB, "if_init() failed.");
        cgdb_cleanup_and_exit(-1);
    }

    if_layout();

    // Starting gdb after the height/width of the gdb window has been decided 
    if (start_gdb(argc, argv) == -1) {
        cgdb_cleanup_and_exit(-1);
    }

    /* Initialize the pipe that is used for resize */
    if (init_resize_pipe() == -1) {
        clog_error(CLOG_CGDB, "init_resize_pipe error");
        cgdb_cleanup_and_exit(-1);
    }

    /* Initialize the pipe that is used for signals */
    if (init_signal_pipe() == -1) {
        clog_error(CLOG_CGDB, "init_signal_pipe error");
        cgdb_cleanup_and_exit(-1);
    }

    /* Enter main loop */
    main_loop();

    /* Shut down curses and exit */
    cgdb_cleanup_and_exit(0);
}
