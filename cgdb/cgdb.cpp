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

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

/* Local Includes */
#include "sys_util.h"
#include "sys_win.h"
#include "cgdb.h"
#include "tokenizer.h"
#include "interface.h"
#include "scroller.h"
#include "sources.h"
#include "tgdb.h"
#include "kui.h"
#include "kui_term.h"
#include "fs_util.h"
#include "cgdbrc.h"
#include "io.h"
#include "fork_util.h"
#include "terminal.h"
#include "rline.h"
#include "ibuf.h"
#include "usage.h"

/* --------- */
/* Constants */
/* --------- */

#define GDB_MAXBUF 4096         /* GDB input buffer size */

const char *readline_history_filename = "readline_history.txt";

/* --------------- */
/* Local Variables */
/* --------------- */

struct tgdb *tgdb;              /* The main TGDB context */

char cgdb_home_dir[MAXLINE];    /* Path to home directory with trailing slash */
char *readline_history_path;    /* After readline init is called, this will 
                                 * contain the path to the readline history 
                                 * file. */

static int gdb_fd = -1;         /* File descriptor for GDB communication */

/** Master/Slave PTY used to keep readline off of stdin/stdout. */
static pty_pair_ptr pty_pair;

static char *debugger_path = NULL;  /* Path to debugger to use */

/* Set to 1 if the user requested cgdb to wait for the debugger to attach. */
static int wait_for_debugger_to_attach = 0;

struct kui_manager *kui_ctx = NULL; /* The key input package */

struct kui_map_set *kui_map = NULL;
struct kui_map_set *kui_imap = NULL;

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

/* Readline interface */
static struct rline *rline;

/**
 * If this is 1, then CGDB is performing a tab completion in the GDB window.
 * When this is true, there can be no data passed to readline. The completion
 * must be displayed to the user first.
 */
static int is_tab_completing = 0;

/* Current readline line. If the user enters 'b ma\<NL>in<NL>', where 
 * NL is the same as the user hitting the enter key, then 2 commands are
 * received by readline. The first is 'b ma\' and the second is 'in'.
 *
 * In general, the user can put a \ as the last char on the line and GDB
 * accepts this as meaning, "allow me to enter more data on the next line".
 * The trailing \, does not belong in the command to GDB, it is purely there
 * to tell GDB that the user wants to enter more data on the next line.
 *
 * For this reason, CGDB keeps this buffer. It adds the command the user
 * is typing to this buffer. When a line comes that does not end in a \,
 * then the command is sent to GDB.
 */
static struct ibuf *current_line = NULL;

/**
 * When the user is entering a multi-line command in the GDB window
 * via the \ char at the end of line, the prompt get's set to "". This
 * variable is used to remember what the previous prompt was so that it
 * can be set back when the user has finished entering the line.
 */
static char *rline_last_prompt = NULL;

/** 
 * Represents all the different states that can be reached while displaying
 * the tab completion information to the user.
 */
enum tab_completion_state {
  /** Nothing has been done yet */
    TAB_COMPLETION_START,
  /** The query possibilities message was issued, and waiting for response */
    TAB_COMPLETION_QUERY_POSSIBILITIES,
  /** Displaying the tab completion */
    TAB_COMPLETION_COMPLETION_DISPLAY,
  /** Query the user to continue tab completion, and waiting for response */
    TAB_COMPLETION_QUERY_PAGER,
  /** All done */
    TAB_COMPLETION_DONE
};

/**
 * A context needed to allow the displaying of tab completion to take place. 
 * Readline calls back into CGDB with the tab completion information. CGDB can
 * not simply display all the information to the user because the displaying of
 * the tab completion data can be interactive (and is by default). So, this 
 * context keeps track of what has been displayed to the user, and what needs
 * to be displayed while CGDB get's input from the user.
 */
typedef struct tab_completion_ctx {
  /** All of the possible completion matches, valid from [0,num_matches]. */
    char **matches;
  /** The number of completion matches in the 'matches' field. */
    int num_matches;
  /** The longest line in the 'matches' field */
    int max_length;

  /** These variables changed based on the state of this object */

  /** Total number of lines printed so far. */
    int total;
  /** Total number of lines printed so far since last pager. */
    int lines;
  /** The current tab completion state */
    enum tab_completion_state state;
} *tab_completion_ptr;

/** 
 * The current tab completion display context. 
 * If non-null, currently doing a display */
static tab_completion_ptr completion_ptr = NULL;

/* Original terminal attributes */
static struct termios term_attributes;

static int is_gdb_tui_command(const char* line)
{
    size_t i;
    static const char *tui_commands[] =
    {
        "wh",
        "wi",
        "win",
        "winh",
        "winhe",
        "winhei",
        "winheig",
        "winheigh",
        "winheight",
        "foc",
        "focu",
        "focus",
        "la",
        "lay",
        "layo",
        "layou",
        "layout",
        "tui" /* tui enable */
    };

    /* Skip leading white space */
    while (isspace(*line))
        line++;
    if (*line) {
        size_t cmd_len;
        const char *cmd_end = line + 1;

        /* Find end of command */
        while (*cmd_end && !isspace(*cmd_end))
            cmd_end++;

        cmd_len = cmd_end - line;
        for (i = 0; i < sizeof(tui_commands) / sizeof(tui_commands[0]); i++)
        {
            size_t len = strlen(tui_commands[i]);

            if ((len == cmd_len) && !strncasecmp(line, tui_commands[i], len))
                return 1;
        }
    }

    return 0;
}

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
    char config_file[FSUTIL_PATH_MAX];

    fs_util_get_path(cgdb_home_dir, "cgdbrc", config_file);
    command_parse_file(config_file);
}

/* readline code {{{*/

/* Please forgive me for adding all the comment below. This function
 * has some strange bahaviors that I thought should be well explained.
 */
void rlctx_send_user_command(char *line)
{
    const char *cline;
    int length;
    char *rline_prompt;

    if (line == NULL) {
        /* NULL line means rl_callback_read_char received EOF */
        ibuf_add(current_line, "quit");
    } else {
        /* Add the line passed in to the current line */
        ibuf_add(current_line, line);
    }

    /* Get current line, and current line length */
    cline = ibuf_get(current_line);
    length = ibuf_length(current_line);

    /* Check to see if the user is escaping the line, to use a 
     * multi line command. If so, return so that the user can
     * continue the command. This data should not go into the history
     * buffer, or be sent to gdb yet. 
     *
     * Also, notice the length > 0 condition. (length == 0) can happen 
     * when the user simply hits Enter on the keyboard. */
    if (length > 0 && cline[length - 1] == '\\') {
        /* The \ char is for continuation only, it is not meant to be sent
         * to GDB as a character. */
        ibuf_delchar(current_line);

        /* Only need to change the prompt the first time the \ char is used.
         * Doing it a second time would erase the real rline_last_prompt,
         * since the prompt has already been set to "".
         */
        if (!rline_last_prompt) {
            rline_get_prompt(rline, &rline_prompt);
            rline_last_prompt = strdup(rline_prompt);
            /* GDB set's the prompt to nothing when doing continuation.
             * This is here just for compatibility. */
            rline_set_prompt(rline, "");
        }

        free(line);
        return;
    }

    /* If here, a full command has been sent. Restore the prompt. */
    if (rline_last_prompt) {
        rline_set_prompt(rline, rline_last_prompt);
        free(rline_last_prompt);
        rline_last_prompt = NULL;
    }

    /* Don't add the enter command to the history */
    if (length > 0)
        rline_add_history(rline, cline);

    if (is_gdb_tui_command(cline)) {
        if_print_message("\nWARNING: Not executing GDB TUI command: %s", cline);
        rline_clear(rline);
        rline_rl_forced_update_display(rline);
    } else {
        /* Send this command to TGDB */
        tgdb_request_run_console_command(tgdb, cline);
    }

    ibuf_clear(current_line);

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
 * Create a tab completion context.
 *
 * \param matches
 * See tab_completion field documentation
 * 
 * \param num_matches
 * See tab_completion field documentation
 *
 * \param max_length
 * See tab_completion field documentation
 *
 * \return
 * The next context, or NULL on error.
 */
static tab_completion_ptr
tab_completion_create(char **matches, int num_matches, int max_length)
{
    int i;
    tab_completion_ptr comptr;

    comptr = (tab_completion_ptr) cgdb_malloc(sizeof (struct
                    tab_completion_ctx));

    comptr->matches = (char **)cgdb_malloc(sizeof (char *) * (num_matches + 1));
    for (i = 0; i <= num_matches; ++i)
        comptr->matches[i] = cgdb_strdup(matches[i]);

    comptr->num_matches = num_matches;
    comptr->max_length = max_length;
    comptr->total = 1;
    comptr->lines = 0;
    comptr->state = TAB_COMPLETION_START;

    return comptr;
}

/**
 * Destroy the tab completion context.
 *
 * \param comptr
 * This object to destroy.
 */
static void tab_completion_destroy(tab_completion_ptr comptr)
{
    int i;

    if (!comptr)
        return;

    if (comptr->matches == 0)
        return;

    for (i = 0; i <= comptr->num_matches; ++i) {
        free(comptr->matches[i]);
        comptr->matches[i] = NULL;
    }

    free(comptr->matches);
    comptr->matches = NULL;

    free(comptr);
    comptr = NULL;
}

/**
 * Get a yes or no from the user.
 *
 * \param key
 * The key that the user pressed
 *
 * \param for_pager
 * Will be non-zero if this is for the pager, otherwise 0.
 *
 * \return
 * 0 for no, 1 for yes, 2 for single line yes, -1 for nothing.
 */
static int cgdb_get_y_or_n(int key, int for_pager)
{
    if (key == 'y' || key == 'Y' || key == ' ')
        return 1;
    if (key == 'n' || key == 'N')
        return 0;
    if (for_pager && (key == '\r' || key == '\n' || key == CGDB_KEY_CTRL_M))
        return 2;
    if (for_pager && (key == 'q' || key == 'Q'))
        return 0;
    if (key == CGDB_KEY_CTRL_G)
        return 0;

    return -1;
}

/**
 * The goal of this function is to display the tab completion information
 * to the user in an asychronous and potentially interactive manner.
 *
 * It will output to the screen as much as can be until user input is needed. 
 * If user input is needed, then this function must stop, and wait until 
 * that data has been received.
 *
 * If this function is called a second time with the same completion_ptr
 * parameter, it will continue outputting the tab completion data from
 * where it left off.
 *
 * \param completion_ptr
 * The tab completion data to output to the user
 *
 * \param key
 * The key the user wants to pass to the query command that was made.
 * If -1, then this is assummed to be the first time this function has been
 * called.
 *
 * \return
 * 0 on success or -1 on error
 */
static int handle_tab_completion_request(tab_completion_ptr comptr, int key)
{
    int query_items;
    int gdb_window_size;

    if (!comptr)
        return -1;

    query_items = rline_get_rl_completion_query_items(rline);
    gdb_window_size = get_gdb_height();

    if (comptr->state == TAB_COMPLETION_START) {
        if_print("\n");

        if (query_items > 0 && comptr->num_matches >= query_items) {
            if_print_message("Display all %d possibilities? (y or n)\n",
                    comptr->num_matches);
            comptr->state = TAB_COMPLETION_QUERY_POSSIBILITIES;
            return 0;
        }

        comptr->state = TAB_COMPLETION_COMPLETION_DISPLAY;
    }

    if (comptr->state == TAB_COMPLETION_QUERY_POSSIBILITIES) {
        int val = cgdb_get_y_or_n(key, 0);

        if (val == 1)
            comptr->state = TAB_COMPLETION_COMPLETION_DISPLAY;
        else if (val == 0)
            comptr->state = TAB_COMPLETION_DONE;
        else
            return 0;           /* stay at the same state */
    }

    if (comptr->state == TAB_COMPLETION_QUERY_PAGER) {
        int i = cgdb_get_y_or_n(key, 1);

        if_clear_line();        /* Clear the --More-- */
        if (i == 0)
            comptr->state = TAB_COMPLETION_DONE;
        else if (i == 2) {
            comptr->lines--;
            comptr->state = TAB_COMPLETION_COMPLETION_DISPLAY;
        } else {
            comptr->lines = 0;
            comptr->state = TAB_COMPLETION_COMPLETION_DISPLAY;
        }
    }

    if (comptr->state == TAB_COMPLETION_COMPLETION_DISPLAY) {
        for (; comptr->total <= comptr->num_matches; comptr->total++) {
            if_print(comptr->matches[comptr->total]);
            if_print("\n");

            comptr->lines++;
            if (comptr->lines >= (gdb_window_size - 1) &&
                    comptr->total < comptr->num_matches) {
                if_print("--More--");
                comptr->state = TAB_COMPLETION_QUERY_PAGER;
                comptr->total++;
                return 0;
            }
        }

        comptr->state = TAB_COMPLETION_DONE;
    }

    if (comptr->state == TAB_COMPLETION_DONE) {
        tab_completion_destroy(completion_ptr);
        completion_ptr = NULL;
        is_tab_completing = 0;
    }

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
    /* Create the tab completion item, and attempt to display it to the user */
    completion_ptr = tab_completion_create(matches, num_matches, max_length);
    if (!completion_ptr)
        clog_error(CLOG_CGDB, "tab_completion_create error\n");

    if (handle_tab_completion_request(completion_ptr, -1) == -1)
        clog_error(CLOG_CGDB, "handle_tab_completion_request error\n");
}

int do_tab_completion(char **completions)
{
    int ret;

    ret = rline_rl_complete(rline, completions, &readline_completion_display_func);
    if (ret == -1)
    {
        clog_error(CLOG_CGDB, "rline_rl_complete error\n");
        return -1;
    }

    /** 
     * If completion_ptr is non-null, then this means CGDB still has to display
     * the completion to the user. is_tab_completing can not be turned off until
     * the completions are displayed to the user. */
    if (!completion_ptr)
        is_tab_completing = 0;

    return 0;
}

void rl_sigint_recved(void)
{
    rline_clear(rline);
    is_tab_completing = 0;
}

void rl_resize(int rows, int cols)
{
    rline_resize_terminal_and_redisplay(rline, rows, cols);
}

static void change_prompt(const char *new_prompt)
{
    rline_set_prompt(rline, new_prompt);
}

/* }}}*/

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
            "Copyright 2002-2010 Bob Rossi and Mike Mueller.\n"
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

/* init_home_dir: Attempts to create a config directory in the user's home
 * -------------  directory.
 *
 * Return:  0 on success or -1 on error
 */

static int init_home_dir(void)
{
    /* Get the home directory */
    char *home_dir = getenv("HOME");
    const char *cgdb_dir = ".cgdb";

    /* Create the config directory */
    if (!fs_util_create_dir_in_base(home_dir, cgdb_dir)) {
        clog_error(CLOG_CGDB, "fs_util_create_dir_in_base error");
        return -1;
    }

    fs_util_get_path(home_dir, cgdb_dir, cgdb_home_dir);

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

/**
 * The console is ready for another command.
 */
static void console_ready(void *context)
{
    rline_rl_forced_update_display(rline);
}

static void request_sent(void *context, struct tgdb_request *request,
        const std::string &command) 
{
    if (request->header == TGDB_REQUEST_CONSOLE_COMMAND &&
        request->choice.console_command.queued) {
        char *prompt;
        rline_get_prompt(rline, &prompt);
        if (prompt) {
            if_print(prompt);
        }
        if_print(request->choice.console_command.command);
        if_print("\n");
    } else if (request->header == TGDB_REQUEST_DEBUGGER_COMMAND) {
        if_print("\n");
    }
    
    if (cgdbrc_get_int(CGDBRC_SHOWDEBUGCOMMANDS)) {
        if_sdc_print(command.c_str());
    }
}


static void command_response(void *context, struct tgdb_response *response);
            
tgdb_callbacks callbacks = { 
    NULL,       
    console_output,
    console_ready,
    request_sent,
    command_response
};


/* start_gdb: Starts up libtgdb
 *  Returns:  -1 on error, 0 on success
 */
static int start_gdb(int argc, char *argv[])
{
    tgdb = tgdb_initialize(debugger_path, argc, argv, &gdb_fd, callbacks);
    if (tgdb == NULL)
        return -1;

    return 0;
}

static void send_key(int focus, char key)
{
    if (focus == 1) {
        int size;
        int masterfd;

        masterfd = pty_pair_get_masterfd(pty_pair);
        if (masterfd == -1)
            clog_error(CLOG_CGDB, "send_key error");
        size = write(masterfd, &key, sizeof (char));
        if (size != 1)
            clog_error(CLOG_CGDB, "send_key error");
    } else if (focus == 2) {
        tgdb_send_inferior_char(tgdb, key);
    }
}

/* user_input: This function will get a key from the user and process it.
 *
 *  Returns:  -1 on error, 0 on success
 */
static int user_input(void)
{
    static int key, val;

    val = kui_manager_clear_map_set(kui_ctx);
    if (val == -1) {
        clog_error(CLOG_CGDB, "Could not clear the map set");
        return -1;
    }

    if (if_get_focus() == CGDB)
        val = kui_manager_set_map_set(kui_ctx, kui_map);
    else if (if_get_focus() == GDB)
        val = kui_manager_set_map_set(kui_ctx, kui_imap);

    key = kui_manager_getkey(kui_ctx);
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

    if (val == 1 && completion_ptr)
        return handle_tab_completion_request(completion_ptr, key);

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
    } while (kui_manager_cangetkey(kui_ctx));

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
        if_display_message("Error:", WIN_REFRESH, 0,
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

static void update_completions(struct tgdb_response *response)
{
    do_tab_completion(response->choice.update_completions.completions);
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
            if_print_message("\nWarning: dissasemble address 0x%" PRIx64 " failed.\n",
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

static void update_prompt(struct tgdb_response *response)
{
    change_prompt(response->choice.update_console_prompt_value.prompt_value);
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
    case TGDB_UPDATE_COMPLETIONS:
        update_completions(response);
        break;
    case TGDB_DISASSEMBLE_PC:
    case TGDB_DISASSEMBLE_FUNC:
        update_disassemble(response);
        break;
    case TGDB_UPDATE_CONSOLE_PROMPT_VALUE:
        update_prompt(response);
        break;
    case TGDB_QUIT:
        cgdb_cleanup_and_exit(0);
        break;
    default:
        break;
    }
}

/* gdb_input: Receives data from tgdb:
 *
 *  Returns:  -1 on error, 0 on success
 */
static int gdb_input()
{
    int result;

    /* Read from GDB */
    result = tgdb_process(tgdb);
    if (result == -1) {
        clog_error(CLOG_CGDB, "tgdb_process error");
        return -1;
    }

    return 0;
}

static int readline_input()
{
    char buf[GDB_MAXBUF + 1];
    int size;

    int masterfd = pty_pair_get_masterfd(pty_pair);

    if (masterfd == -1) {
        clog_error(CLOG_CGDB, "pty_pair_get_masterfd error");
        return -1;
    }

    size = read(masterfd, buf, GDB_MAXBUF);
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
    if (size > 0)
        if_rl_print(buf);

    return 0;
}

/* child_input: Receives data from the child application:
 *
 *  Returns: -1 on error, 0 on EOF or number of bytes handled from child.
 */
static ssize_t child_input()
{
    ssize_t size;
    char buf[GDB_MAXBUF + 1];

    /* Read from GDB */
    size = tgdb_recv_inferior_data(tgdb, buf, GDB_MAXBUF);
    if (size == -1) {
        clog_error(CLOG_CGDB, "tgdb_recv_inferior_data error ");
        return -1;
    }
    buf[size] = 0;

    /* Display CHILD output */
    if_tty_print(buf);
    return size;
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

    if (signo == SIGINT) {
        rl_sigint_recved();
    }

    tgdb_signal_notification(tgdb, signo);

    return 0;
}

static int main_loop(void)
{
    fd_set rset;
    int max;
    int masterfd, slavefd;

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


    /* Main (infinite) loop:
     *   Sits and waits for input on either stdin (user input) or the
     *   GDB file descriptor.  When input is received, wrapper functions
     *   are called to process the input, and handle it appropriately.
     *   This will result in calls to the curses interface, typically. */

    for (;;) {
        /**
         * tty_fd can vary during the program. Some GDB variants, or perhaps
         * OS's allow the inferior to close the terminal descriptor.
         *
         * CGDB reallocates a new one in this situation, for the next run of
         * the inferior to have a place to send it's output.
         *
         * This varies the value of tty_fd, and forces us to compute the max
         * each time in the loop.
         */
        int tty_fd = tgdb_get_inferior_fd(tgdb);

        max = (gdb_fd > STDIN_FILENO) ? gdb_fd : STDIN_FILENO;
        max = (max > tty_fd) ? max : tty_fd;
        max = (max > resize_pipe[0]) ? max : resize_pipe[0];
        max = (max > signal_pipe[0]) ? max : signal_pipe[0];
        max = (max > slavefd) ? max : slavefd;
        max = (max > masterfd) ? max : masterfd;

        /* Reset the fd_set, and watch for input from GDB or stdin */
        FD_ZERO(&rset);
        FD_SET(STDIN_FILENO, &rset);
        FD_SET(gdb_fd, &rset);
        FD_SET(tty_fd, &rset);
        FD_SET(resize_pipe[0], &rset);
        FD_SET(signal_pipe[0], &rset);

        /* No readline activity allowed while displaying tab completion */
        if (!is_tab_completing) {
            FD_SET(slavefd, &rset);
            FD_SET(masterfd, &rset);
        }

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

        /* Input received through the pty:  Handle it 
         * Wrote to masterfd, now slavefd is ready, tell readline */
        if (FD_ISSET(slavefd, &rset))
            rline_rl_callback_read_char(rline);

        /* Input received through the pty:  Handle it
         * Readline read from slavefd, and it wrote to the masterfd. 
         */
        if (FD_ISSET(masterfd, &rset))
            if (readline_input() == -1)
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

        /**
         * Handle the debugged programs standard output.
         * (Otherwise known as the inferior)
         * child's ouptut -> stdout
         * 
         * The continue is important. It allows all of the child
         * output to get written to stdout before tgdb's next command.
         * This is because sometimes they are both ready.
         *
         * In the case that the tty_fd has been closed, do not continue
         * or an infinite loop will occur (as the select loop is always
         * activated on EOF). Instead fall through and let the remaining
         * file descriptors get handled.
         */
        if (FD_ISSET(tty_fd, &rset)) {
            ssize_t result = child_input();
            if (result == -1) {
                return -1;
            } else if (result == 0) {
                if (tgdb_tty_new(tgdb) == -1) {
                    return -1;
                }
            } else {
                continue;
            }
        }

        /* gdb's output -> stdout */
        if (FD_ISSET(gdb_fd, &rset)) {
            if (gdb_input() == -1) {
                return -1;
            }

            /* When the file dialog is opened, the user input is blocked, 
             * until GDB returns all the files that should be displayed,
             * and the file dialog can open, and be prepared to receive 
             * input. So, if we are in the file dialog, and are no longer
             * waiting for the gdb command, then read the input.
             */
            if (kui_manager_cangetkey(kui_ctx)) {
                user_input_loop();
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
    ibuf_free(current_line);

    /* Cleanly scroll the screen up for a prompt */
    swin_scrl(1);
    swin_move(swin_lines() - 1, 0);
    printf("\n");

    rline_write_history(rline, readline_history_path);

    /* The order of these is important. They each must restore the terminal
     * the way they found it. Thus, the order in which curses/readline is 
     * started, is the reverse order in which they should be shutdown 
     */

    /* Shut down interface */
    if_shutdown();

#if 0
    if (masterfd != -1)
        util_free_tty(&masterfd, &slavefd, tty_name);
#endif

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
    const char *log_filename = clog_filename(CLOG_CGDB_ID);
    long log_bytes_written = get_file_size_by_name(log_filename);
    if (log_bytes_written > 0)
    {
        fprintf(stderr, "CGDB had unexpected results, see %s for details.\n",
            log_filename);
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

int init_readline(void)
{
    int slavefd, masterfd;
    int length;

    slavefd = pty_pair_get_slavefd(pty_pair);
    if (slavefd == -1)
        return -1;

    masterfd = pty_pair_get_masterfd(pty_pair);
    if (masterfd == -1)
        return -1;

    if (tty_off_xon_xoff(slavefd) == -1)
        return -1;

    /* The 16 is because I don't know how many char's the directory separator 
     * is going to be, I expect it to be 1, but who knows. */
    length = strlen(cgdb_home_dir) + strlen(readline_history_filename) + 16;
    readline_history_path = (char *) cgdb_malloc(sizeof (char) * length);
    fs_util_get_path(cgdb_home_dir, readline_history_filename,
            readline_history_path);
    rline = rline_initialize(slavefd, rlctx_send_user_command, tab_completion,
            "dumb");
    rline_read_history(rline, readline_history_path);
    return 0;
}

int create_and_init_pair()
{
    struct winsize size;
    int slavefd;

    pty_pair = pty_pair_create();
    if (!pty_pair) {
        fprintf(stderr, "%s:%d Unable to create PTY pair", __FILE__, __LINE__);
        return -1;
    }

    slavefd = pty_pair_get_slavefd(pty_pair);
    if (slavefd == -1) {
        clog_error(CLOG_CGDB, "pty_pair_get_slavefd error");
        return -1;
    }

    /* Set the pty winsize to the winsize of stdout */
    if (ioctl(0, TIOCGWINSZ, &size) < 0)
        return -1;

    if (ioctl(slavefd, TIOCSWINSZ, &size) < 0)
        return -1;

    return 0;
}

int update_kui(cgdbrc_config_option_ptr option)
{
    kui_manager_set_terminal_escape_sequence_timeout(kui_ctx,
            cgdbrc_get_key_code_timeoutlen());
    kui_manager_set_key_mapping_timeout(kui_ctx,
            cgdbrc_get_mapped_key_timeoutlen());

    return 0;
}

static int destroyReadlineKeySeq(void *data)
{
    char *keyseq = (char *) data;

    free(keyseq);
    return 0;
}

int add_readline_key_sequence(const char *readline_str, enum cgdb_key key)
{
    int ret_val;

    std_list_ptr keyseq_list = std_list_create(destroyReadlineKeySeq);

    ret_val = rline_get_keyseq(rline, readline_str, keyseq_list);
    if (ret_val == -1) {
        std_list_destroy(keyseq_list);
        return -1;
    }

    ret_val = kui_manager_get_terminal_keys_kui_map(kui_ctx, key, keyseq_list);
    if (ret_val == -1) {
        std_list_destroy(keyseq_list);
        return -1;
    }
    std_list_destroy(keyseq_list);

    return 0;
}

int init_kui(void)
{
    kui_ctx = kui_manager_create(STDIN_FILENO, cgdbrc_get_key_code_timeoutlen(),
            cgdbrc_get_mapped_key_timeoutlen());
    if (!kui_ctx) {
        clog_error(CLOG_CGDB, "Unable to initialize input library");
        cgdb_cleanup_and_exit(-1);
    }

    kui_map = kui_ms_create();
    if (!kui_map) {
        clog_error(CLOG_CGDB, "Unable to initialize input library");
        cgdb_cleanup_and_exit(-1);
    }

    kui_imap = kui_ms_create();
    if (!kui_imap) {
        clog_error(CLOG_CGDB, "Unable to initialize input library");
        cgdb_cleanup_and_exit(-1);
    }

    if (kui_manager_set_map_set(kui_ctx, kui_map) == -1) {
        clog_error(CLOG_CGDB, "Unable to initialize input library");
        cgdb_cleanup_and_exit(-1);
    }

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

int main(int argc, char *argv[])
{
    parse_long_options(&argc, &argv);

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

    current_line = ibuf_init();

    cgdbrc_init();

    if (create_and_init_pair() == -1) {
        fprintf(stderr, "%s:%d Unable to create PTY pair\n",
                __FILE__, __LINE__);
        exit(-1);
    }

    /* First create tgdb, because it has the error log */
    if (start_gdb(argc, argv) == -1) {
        fprintf(stderr, "%s:%d Unable to invoke debugger: %s\n",
                __FILE__, __LINE__, debugger_path ? debugger_path : "gdb");
        exit(-1);
    }

    /* From here on, the logger is initialized */

    /* Create the home directory */
    if (init_home_dir() == -1) {
        clog_error(CLOG_CGDB, "Unable to create home dir ~/.cgdb");
        cgdb_cleanup_and_exit(-1);
    }

    if (init_readline() == -1) {
        clog_error(CLOG_CGDB, "Unable to init readline");
        cgdb_cleanup_and_exit(-1);
    }

    if (tty_cbreak(STDIN_FILENO, &term_attributes) == -1) {
        clog_error(CLOG_CGDB, "tty_cbreak error");
        cgdb_cleanup_and_exit(-1);
    }

    if (init_kui() == -1) {
        clog_error(CLOG_CGDB, "init_kui error");
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
