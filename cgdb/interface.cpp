/**
 * Provides the routines for displaying the interface, and interacting with
 * the user via keystrokes.
 *
 * When the window orientation is set to horizontal, cgdb will displasy as:
 *      ---------------
 *       source window
 *      ---------------
 *       status window
 *      ---------------
 *       gdb window
 *      ---------------
 * In this mode, the winminheight determines how much a window can
 * shrink vertically. The window_shfit variable keeps track of how far
 * the source window has been shifted up or down.
 *
 * When the window orientation is set to vertical, cgdb will display as:
 *
 *      ---------------|------------
 *       source window | gdb window
 *                     |
 *                     |
 *                     |
 *                     |
 *      ---------------|
 *       status window | 
 *      ---------------|------------
 * In this mode, the winminwidth determines how much a window can
 * shrink horizontally. The window_shfit variable keeps track of how
 * far the source window has been shifted left or right.
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

/* System Includes */
#if HAVE_SIGNAL_H
#include <signal.h>
#endif

#if HAVE_STDARG_H
#include <stdarg.h>
#endif /* HAVE_STDARG_H */

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif /* HAVE_STDLIB_H */

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#if HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif /* HAVE_SYS_IOCTL_H */

#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */

#if HAVE_TERMIOS_H
#include <termios.h>
#endif /* HAVE_TERMIOS_H */

#if HAVE_CTYPE_H
#include <ctype.h>
#endif

#include <assert.h>

/* Local Includes */
#include "sys_util.h"
#include "sys_win.h"
#include "cgdb.h"
#include "config.h"
#include "tokenizer.h"
#include "interface.h"
#include "kui_term.h"
#include "scroller.h"
#include "sources.h"
#include "tgdb.h"
#include "filedlg.h"
#include "cgdbrc.h"
#include "highlight.h"
#include "highlight_groups.h"
#include "fs_util.h"
#include "logo.h"

/* ----------- */
/* Prototypes  */
/* ----------- */

/* ------------ */
/* Declarations */
/* ------------ */

extern struct tgdb *tgdb;

/* ----------- */
/* Definitions */
/* ----------- */

/* The minimum number of rows the user wants a window to shrink. */
static int interface_winminheight = 0;

/* The minimum number of columns the user wants a window to shrink. */
static int interface_winminwidth = 0;

/* The offset that determines allows gdb/sources window to grow or shrink */
static int window_shift;

/* Height and width of the terminal */
#define HEIGHT      (screen_size.ws_row)
#define WIDTH       (screen_size.ws_col)

/* Current window split state */
WIN_SPLIT_TYPE cur_win_split = WIN_SPLIT_EVEN;
/* Current window orientation state (horizontal or vertical) */
WIN_SPLIT_ORIENTATION_TYPE cur_split_orientation = WSO_HORIZONTAL;

/* --------------- */
/* Local Variables */
/* --------------- */
static int curses_initialized = 0;  /* Flag: Curses has been initialized */
static struct scroller *gdb_scroller = NULL;
static struct sviewer *src_viewer = NULL;  /* The source viewer window */
static SWINDOW *status_win = NULL;   /* The status line */
static SWINDOW *vseparator_win = NULL;   /* Separator gets own window */
static enum Focus focus = GDB;  /* Which pane is currently focused */
static struct winsize screen_size;  /* Screen size */

struct filedlg *fd;             /* The file dialog structure */

/* The regex the user is entering */
static struct ibuf *regex_cur = NULL;

/* The last regex the user searched for */
static struct ibuf *regex_last = NULL;

/* Direction to search */
static int regex_direction_cur;
static int regex_direction_last;

/* The position of the source file, before regex was applied. */
static int orig_line_regex;

static char last_key_pressed = 0;   /* Last key user entered in cgdb mode */

/* Line number user wants to 'G' to */
static struct ibuf *G_line_number = NULL;

/* The cgdb status bar command */
static struct ibuf *cur_sbc = NULL;

enum StatusBarCommandKind {
    /* This is represented by a : */
    SBC_NORMAL,
    /* This is set when a regular expression is being entered */
    SBC_REGEX
};

static enum StatusBarCommandKind sbc_kind = SBC_NORMAL;

/* --------------- */
/* Local Functions */
/* --------------- */

/* init_curses: Initializes curses and sets up the terminal properly.
 * ------------
 *
 * Return Value: Zero on success, non-zero on failure.
 */
static int init_curses()
{
    char escdelay[] = "ESCDELAY=0";

    if (putenv(escdelay))
    {
        clog_error(CLOG_CGDB, "putenv(\"%s\") failed", escdelay);
    }

    swin_initscr();                  /* Start curses mode */

    if (swin_has_colors()) {
        swin_start_color();
        swin_use_default_colors();
    }

    swin_refresh();                  /* Refresh the initial window once */
    curses_initialized = 1;

    return 0;
}

/* --------------------------------------
 * Theses get the position of each window
 * -------------------------------------- */

/* These are for the source window */
static int get_src_row(void)
{
    return 0;
}

static int get_src_col(void)
{
    return 0;
}

static int get_src_status_height(void)
{
    return 1;
}

static int get_src_height(void)
{
    int result;

    switch (cur_split_orientation) {
        case WSO_HORIZONTAL:
            result = ((screen_size.ws_row + 0.5) / 2) + window_shift;
            break;
        case WSO_VERTICAL:
            result = screen_size.ws_row - get_src_status_height();
            break;
    }

    return result;
}

static int get_src_width(void)
{
    int result;

    switch (cur_split_orientation) {
        case WSO_HORIZONTAL:
            result = screen_size.ws_col;
            break;
        case WSO_VERTICAL:
            result = ((screen_size.ws_col + 0.5) / 2) + window_shift;
            break;
    }

    return result;
}

/* This is for the source window status bar */
static int get_src_status_row(void)
{
    return get_src_row() + get_src_height();
}

static int get_src_status_col(void)
{
    return get_src_col();
}

static int get_src_status_width(void)
{
    return get_src_width();
}

/* These are for the separator line in horizontal split mode */
static int get_sep_row(void)
{
    return 0;
}

static int get_sep_col(void)
{
    return get_src_col() + get_src_width();
}

static int get_sep_height(void)
{
    return screen_size.ws_row;
}

static int get_sep_width(void)
{
    return 1;
}

/* This is for the debugger window */
static int get_gdb_row(void)
{
    int result;

    switch (cur_split_orientation) {
        case WSO_HORIZONTAL:
            result = get_src_status_row() + get_src_status_height();
            break;
        case WSO_VERTICAL:
            result = 0;
            break;
    }

    return result;
}

static int get_gdb_col(void)
{
    int result;

    switch (cur_split_orientation) {
        case WSO_HORIZONTAL:
            result = 0;
            break;
        case WSO_VERTICAL:
            result = get_sep_col() + get_sep_width();
            break;
    }

    return result;
}

int get_gdb_height(void)
{
    int result;

    switch (cur_split_orientation) {
        case WSO_HORIZONTAL: {
            int window_size = ((screen_size.ws_row / 2) - window_shift - 1);
            int odd_screen_size = (screen_size.ws_row % 2);

            result = window_size + odd_screen_size;
            break;
        }
        case WSO_VERTICAL:
            result = screen_size.ws_row;
            break;
    }

    return result;
}

static int get_gdb_width(void)
{
    int result;

    switch (cur_split_orientation) {
        case WSO_HORIZONTAL:
            result = screen_size.ws_col;
            break;
        case WSO_VERTICAL: {
            int window_size = ((screen_size.ws_col / 2) - window_shift - 1);
            int odd_screen_size = (screen_size.ws_col % 2);

            result = window_size + odd_screen_size;
            break; 
        }
    }

    return result;
}

/*
 * create_swindow: (re)create window with specified position and size.
 */
static void create_swindow(SWINDOW **win, int nlines, int ncols, int begin_y, int begin_x)
{
    if (*win)
    {
        int x = swin_getbegx(*win);
        int y = swin_getbegy(*win);
        int w = swin_getmaxx(*win);
        int h = swin_getmaxy(*win);

        /* If the window position + size hasn't changed, bail */
        if (x == begin_x && y == begin_y && w == ncols && h == nlines)
            return;

        /* Delete the existing window */
        swin_delwin(*win);
        *win = NULL;
    }

    if ((nlines > 0) && (ncols > 0))
    {
        /* Create the ncurses window */
        *win = swin_newwin(nlines, ncols, begin_y, begin_x);
        swin_werase(*win);
    }
}

static void separator_display(int draw)
{
    int x = get_sep_col();
    int y = get_sep_row();
    int h = y + get_sep_height();
    int w = draw ? 1 : 0;

    /* Make sure our window is created at correct location
     * (or destroyed if draw == 0) */
    create_swindow(&vseparator_win, h, w, y, x);

    if (vseparator_win)
    {
        /* Draw vertical line in window */
        swin_wmove(vseparator_win, 0, 0);
        swin_wvline(vseparator_win, SWIN_SYM_VLINE, h);

        swin_wnoutrefresh(vseparator_win);
    }
}

/* ---------------------------------------
 * Below is the core body of the interface
 * --------------------------------------- */

/* Updates the status bar */
static void update_status_win(enum win_refresh dorefresh)
{
    int pos;
    int attr;

    attr = hl_groups_get_attr(hl_groups_instance, HLG_STATUS_BAR);

    /* Print white background */
    swin_wattron(status_win, attr);

    for (pos = 0; pos < WIDTH; pos++)
        swin_mvwprintw(status_win, 0, pos, " ");
    /* Show the user which window is focused */
    if (focus == GDB)
        swin_mvwprintw(status_win, 0, WIDTH - 1, "*");
    else if (focus == CGDB || focus == CGDB_STATUS_BAR)
        swin_mvwprintw(status_win, 0, WIDTH - 1, " ");

    swin_wattroff(status_win, attr);

    /* Print the regex that the user is looking for Forward */
    if (sbc_kind == SBC_REGEX && regex_direction_cur) {
        if_display_message("/", dorefresh, WIDTH - 1, "%s", ibuf_get(regex_cur));
        swin_curs_set(1);
    }
    /* Regex backwards */
    else if (sbc_kind == SBC_REGEX) {
        if_display_message("?", dorefresh, WIDTH - 1, "%s", ibuf_get(regex_cur));
        swin_curs_set(1);
    }
    /* A colon command typed at the status bar */
    else if (focus == CGDB_STATUS_BAR && sbc_kind == SBC_NORMAL) {
        const char *command = ibuf_get(cur_sbc);

        if (!command)
            command = "";
        if_display_message(":", dorefresh, WIDTH - 1, "%s", command);
        swin_curs_set(1);
    }
    /* Default: Current Filename */
    else {
        /* Print filename */
        const char *filename = source_current_file(src_viewer);

        if (filename) {
            if_display_message("", dorefresh, WIDTH - 1, "%s", filename);
        }
    }

    if (dorefresh == WIN_REFRESH)
        swin_wrefresh(status_win);
    else
        swin_wnoutrefresh(status_win);
}

void if_display_message(const char *msg, enum win_refresh dorefresh, int width, const char *fmt, ...)
{
    va_list ap;
    char va_buf[MAXLINE];
    char buf_display[MAXLINE];
    int pos, error_length, length;
    int attr;

    attr = hl_groups_get_attr(hl_groups_instance, HLG_STATUS_BAR);

    swin_curs_set(0);

    if (!width)
        width = WIDTH;

    /* Get the buffer with format */
    va_start(ap, fmt);
#ifdef   HAVE_VSNPRINTF
    vsnprintf(va_buf, sizeof (va_buf), fmt, ap);    /* this is safe */
#else
    vsprintf(va_buf, fmt, ap);  /* this is not safe */
#endif
    va_end(ap);

    error_length = strlen(msg);
    length = strlen(va_buf);

    if (error_length > width)
        strcat(strncpy(buf_display, msg, width - 1), ">");
    else if (error_length + length > width)
        snprintf(buf_display, sizeof(buf_display), "%s>%s", msg,
                va_buf + (length - (width - error_length) + 1));
    else
        snprintf(buf_display, sizeof(buf_display), "%s%s", msg, va_buf);

    /* Print white background */
    swin_wattron(status_win, attr);
    for (pos = 0; pos < WIDTH; pos++)
        swin_mvwprintw(status_win, 0, pos, " ");

    swin_mvwprintw(status_win, 0, 0, "%s", buf_display);
    swin_wattroff(status_win, attr);
    
    if (dorefresh == WIN_REFRESH)
        swin_wrefresh(status_win);
    else
        swin_wnoutrefresh(status_win);
}

/* if_draw: Draws the interface on the screen.
 * --------
 */
void if_draw(void)
{
    if (!curses_initialized)
        return;

    /* Only redisplay the filedlg if it is up */
    if (focus == FILE_DLG) {
        filedlg_display(fd);
        return;
    }

    update_status_win(WIN_NO_REFRESH);

    if (get_src_height() != 0 && get_gdb_height() != 0)
        swin_wnoutrefresh(status_win);

    if (get_src_height() > 0)
        source_display(src_viewer, focus == CGDB, WIN_NO_REFRESH);

    separator_display(cur_split_orientation == WSO_VERTICAL);

    if (get_gdb_height() > 0)
        scr_refresh(gdb_scroller, focus == GDB, WIN_NO_REFRESH);

    /* This check is here so that the cursor goes to the 
     * cgdb window. The cursor would stay in the gdb window 
     * on cygwin */
    if (get_src_height() > 0 && focus == CGDB)
        swin_wnoutrefresh(src_viewer->win);

    swin_doupdate();
}

/* validate_window_sizes:
 * ----------------------
 *
 * This will make sure that the gdb_window, status_bar and source window
 * have appropriate sizes. Each of the windows will not be able to grow
 * smaller than WINMINHEIGHT or WINMINWIDTH in size. It will also restrict the
 * size of windows to being within the size of the terminal.
 */
static void validate_window_sizes(void)
{
    int h_or_w = cur_split_orientation == WSO_HORIZONTAL ? HEIGHT : WIDTH;
    int odd_size = (h_or_w + 1) % 2;
    int max_window_size_shift = (h_or_w / 2) - odd_size;
    int min_window_size_shift = -(h_or_w / 2);

    /* update max and min based off of users winminheight request */
    switch (cur_split_orientation) {
        case WSO_HORIZONTAL:
            min_window_size_shift += interface_winminheight;
            max_window_size_shift -= interface_winminheight;
            break;
        case WSO_VERTICAL:
            min_window_size_shift += interface_winminwidth;
            max_window_size_shift -= interface_winminwidth;
            break;
    }

    /* Make sure that the windows offset is within its bounds: 
     * This checks the window offset.
     * */
    if (window_shift > max_window_size_shift)
        window_shift = max_window_size_shift;
    else if (window_shift < min_window_size_shift)
        window_shift = min_window_size_shift;
}

/* if_layout: Update the layout of the screen based on current terminal size.
 * ----------
 *
 * Return Value: Zero on success, -1 on failure.
 */
static int if_layout()
{
    SWINDOW *gdb_scroller_win = NULL;
    SWINDOW *src_viewer_win = NULL;

    if (!curses_initialized)
        return -1;

    /* Verify the window size is reasonable */
    validate_window_sizes();

    /* Resize the source viewer window */
    create_swindow(&src_viewer_win, get_src_height(), get_src_width(),
        get_src_row(), get_src_col());
    if (src_viewer) {
        source_move(src_viewer, src_viewer_win);
    } else {
        src_viewer = source_new(src_viewer_win);
    }

    /* Resize the GDB I/O window */
    create_swindow(&gdb_scroller_win, get_gdb_height(), get_gdb_width(),
        get_gdb_row(), get_gdb_col());
    if (gdb_scroller) {
        scr_move(gdb_scroller, gdb_scroller_win);
    } else {
        gdb_scroller = scr_new(gdb_scroller_win);
    }

    /* Initialize the status bar window */
    create_swindow(&status_win, get_src_status_height(),
        get_src_status_width(), get_src_status_row(), get_src_status_col());

    /* Redraw the interface */
    if_draw();

    return 0;
}

/* if_resize_term: Checks if a resize event occurred, and updates display if so.
 * ----------
 *
 * Return Value:  Zero on success, non-zero on failure.
 */
int if_resize_term(void)
{
    if (ioctl(fileno(stdout), TIOCGWINSZ, &screen_size) != -1) {
        if (screen_size.ws_row != swin_lines() ||
                screen_size.ws_col != swin_cols()) {
            swin_resizeterm(screen_size.ws_row, screen_size.ws_col);
            swin_refresh();
            rl_resize(screen_size.ws_row, screen_size.ws_col);
            return if_layout();
        }

        rl_resize(screen_size.ws_row, screen_size.ws_col);
        return if_layout();

    }

    return 0;
}

/*
 * increase_win_height: Increase size of the source window
 * ____________________
 *
 * Param jump - if 0, increase source window by 1
 *              if 1, jump source window to next biggest quarter 
 *
 */
static void increase_win_height(int jump)
{
    int height = HEIGHT / 2;
    int old_window_shift = window_shift;

    if (jump) {
        /* user input: '+' */
        if (cur_win_split == WIN_SPLIT_FREE) {
            /* cur position is not on mark, find nearest mark */
            cur_win_split = (WIN_SPLIT_TYPE) ((2 * window_shift) / height);

            /* handle rounding on either side of mid-way mark */
            if (window_shift > 0) {
                cur_win_split = (WIN_SPLIT_TYPE)(cur_win_split + 1);
            }
        } else {
            /* increase to next mark */
            cur_win_split = (WIN_SPLIT_TYPE)(cur_win_split + 1);
        }

        /* check split bounds */
        if (cur_win_split > WIN_SPLIT_SRC_FULL) {
            cur_win_split = WIN_SPLIT_SRC_FULL;
        }

        /* set window height to specified quarter mark */
        window_shift = (int) (height * (cur_win_split / 2.0));
    } else {
        /* user input: '=' */
        cur_win_split = WIN_SPLIT_FREE; /* cur split is not on a mark */
        window_shift++;         /* increase src window size by 1 */

    }

    /* reduce flicker by avoiding unnecessary redraws */
    if (window_shift != old_window_shift) {
        if_layout();
    }
}

/*
 * decrease_win_height: Decrease size of the source window
 * ____________________
 *
 * Param jump - if 0, decrease source window by 1
 *              if 1, jump source window to next smallest quarter 
 *
 */
static void decrease_win_height(int jump)
{
    int height = HEIGHT / 2;
    int old_window_shift = window_shift;

    if (jump) {
        /* user input: '_' */
        if (cur_win_split == WIN_SPLIT_FREE) {
            /* cur position is not on mark, find nearest mark */
            cur_win_split = (WIN_SPLIT_TYPE) ((2 * window_shift) / height);

            /* handle rounding on either side of mid-way mark */
            if (window_shift < 0) {
                cur_win_split = (WIN_SPLIT_TYPE)(cur_win_split - 1);
            }
        } else {
            /* decrease to next mark */
            cur_win_split = (WIN_SPLIT_TYPE)(cur_win_split - 1);
        }

        /* check split bounds */
        if (cur_win_split < WIN_SPLIT_GDB_FULL) {
            cur_win_split = WIN_SPLIT_GDB_FULL;
        }

        /* set window height to specified quarter mark */
        window_shift = (int) (height * (cur_win_split / 2.0));
    } else {
        /* user input: '-' */
        cur_win_split = WIN_SPLIT_FREE; /* cur split is not on a mark */
        window_shift--;         /* decrease src window size by 1 */

    }

    /* reduce flicker by avoiding unnecessary redraws */
    if (window_shift != old_window_shift) {
        if_layout();
    }
}

/**
 * The signal handler for CGDB.
 *
 * Pass any signals along from the signal handler to the main loop by
 * writing the signal value to a pipe, which is later read and interpreted.
 *
 * Since it's non trivial to do error handling from the signal handler if an
 * error occurs the program will terminate. Hopefully this doesn't occur.
 */
static void signal_handler(int signo)
{
    extern int resize_pipe[2];
    extern int signal_pipe[2];
    int fdpipe;

    if (signo == SIGWINCH) {
        fdpipe = resize_pipe[1];
    } else {
        fdpipe = signal_pipe[1];
    }

    assert(write(fdpipe, &signo, sizeof(signo)) == sizeof(signo));
}

static void if_run_command(struct sviewer *sview, struct ibuf *ibuf_command)
{
    char *command = ibuf_get(ibuf_command);

    /* refresh and return if the user entered no data */
    if (ibuf_length(ibuf_command) == 0) {
        if_draw();
        return;
    }

    if (command_parse_string(command)) {
        if_display_message("Unknown command: ", WIN_NO_REFRESH, 0, "%s", command);
    } else {
        update_status_win(WIN_NO_REFRESH);
    }

    if_draw();
}

/**
 * Capture a regular expression from the user, one key at a time.
 * This modifies the global variables regex_cur and regex_last.
 *
 * \param sview
 * The source viewer.
 *
 * \return
 * 0 if user gave a regex, otherwise 1.
 */
static int gdb_input_regex_input(struct scroller *scr, int key)
{
    int regex_icase = cgdbrc_get_int(CGDBRC_IGNORECASE);

    /* Flag to indicate we're done with regex mode, need to switch back */
    int done = 0;

    /* Receive a regex from the user. */
    switch (key)
    {
    case '\r':
    case '\n':
    case CGDB_KEY_CTRL_M:
        /* Save for future searches via 'n' or 'N' */
        ibuf_free(regex_last);
        regex_last = ibuf_dup(regex_cur);

        regex_direction_last = regex_direction_cur;
        scr_search_regex(scr, ibuf_get(regex_last), 2,
            regex_direction_last, regex_icase);
        if_draw();
        done = 1;
        break;
    case 8:
    case 127:
        /* Backspace or DEL key */
        if (ibuf_length(regex_cur) == 0)
        {
            done = 1;
            scr_search_regex(scr, "", 2,
                regex_direction_cur, regex_icase);
        }
        else
        {
            ibuf_delchar(regex_cur);
            scr_search_regex(scr, ibuf_get(regex_cur), 1,
                regex_direction_cur, regex_icase);
            if_draw();
            update_status_win(WIN_REFRESH);
        }
        break;
    default:
        if (kui_term_is_cgdb_key(key))
        {
            const char *keycode = kui_term_get_keycode_from_cgdb_key(key);
            int length = strlen(keycode), i;

            for (i = 0; i < length; i++)
                ibuf_addchar(regex_cur, keycode[i]);
        }
        else
        {
            ibuf_addchar(regex_cur, key);
        }
        scr_search_regex(scr, ibuf_get(regex_cur), 1,
            regex_direction_cur, regex_icase);
        if_draw();
        update_status_win(WIN_REFRESH);
    };

    if (done)
    {
        gdb_scroller->in_search_mode = 0;

        ibuf_free(regex_cur);
        regex_cur = NULL;

        sbc_kind = SBC_NORMAL;
        if_set_focus(GDB);
    }

    return 0;
}

/* gdb_input: Handles user input to the GDB window.
 * ----------
 *
 *   key:  Keystroke received.
 *
 * Return Value:    0 if internal key was used, 
 *                  1 if input to gdb or ...
 *                  -1        : Error resizing terminal -- terminal too small
 */
static int gdb_input(int key, int *last_key)
{
    int result = 0;

    if (gdb_scroller->in_search_mode)
        return gdb_input_regex_input(gdb_scroller, key);

    if (gdb_scroller->in_scroll_mode) {

        /* Handle setting (mX) and going ('X) to gdb buffer marks */
        if (last_key_pressed == 'm' || last_key_pressed == '\'') {
            int ret = 0;

            if (last_key_pressed == 'm')
                ret = scr_set_mark(gdb_scroller, key);
            else if(last_key_pressed == '\'')
                ret = scr_goto_mark(gdb_scroller, key);

            if (ret) {
                *last_key = 0;
                if_draw();
            }
            return 0;
        }

        /* In scroll mode, all extra characters are not passed to
         * the active GDB command. result = 0 above ensures that. */
        switch (key) {
            
            case 'm':
            case '\'':
                /* Mark keys - ignore them */
                break;
            case CGDB_KEY_CTRL_U:
                scr_up(gdb_scroller, get_gdb_height() / 2);
                break;
            case CGDB_KEY_PPAGE:
                scr_up(gdb_scroller, get_gdb_height() - 1);
                break;
            case CGDB_KEY_CTRL_D:
                scr_down(gdb_scroller, get_gdb_height() / 2);
                break;
            case CGDB_KEY_NPAGE:
                scr_down(gdb_scroller, get_gdb_height() - 1);
                break;
            case CGDB_KEY_HOME:
            case CGDB_KEY_F11:
                scr_home(gdb_scroller);
                break;
            case 'G':
            case CGDB_KEY_END:
            case CGDB_KEY_F12:
                scr_end(gdb_scroller);
                break;
            case 'k':
            case CGDB_KEY_UP:
            case CGDB_KEY_CTRL_P:
                scr_up(gdb_scroller, 1);
                break;
            case 'j':
            case CGDB_KEY_DOWN:
            case CGDB_KEY_CTRL_N:
                scr_down(gdb_scroller, 1);
                break;
            case 'g':
                if (last_key_pressed == 'g') {
                    scr_home(gdb_scroller);
                }
                break;
            case 'q':
            case 'i':
            case '\r':
            case '\n':
            case CGDB_KEY_CTRL_M:
                scr_end(gdb_scroller);
                gdb_scroller->in_scroll_mode = 0;
                break;
            case 'n':
                scr_search_regex(gdb_scroller, ibuf_get(regex_last), 2,
                    regex_direction_last, cgdbrc_get_int(CGDBRC_IGNORECASE));
                break;
            case 'N':
                scr_search_regex(gdb_scroller, ibuf_get(regex_last), 2,
                    !regex_direction_last, cgdbrc_get_int(CGDBRC_IGNORECASE));
                break;
            case '/':
            case '?':
                /* Capturing regular expressions */
                regex_cur = ibuf_init();
                regex_direction_cur = ('/' == key);
                orig_line_regex = gdb_scroller->current.r;

                sbc_kind = SBC_REGEX;

                scr_search_regex_init(gdb_scroller);
                break;
        }

    } else {
        switch (key) {
            case CGDB_KEY_PPAGE:
                scr_up(gdb_scroller, get_gdb_height() - 1);
                break;
            case CGDB_KEY_CTRL_L:
                scr_clear(gdb_scroller);

                /* The return 1 tells readline that gdb did not handle the
                 * Ctrl-l. That way readline will handle it. Because
                 * readline uses TERM=dumb, that means that it will clear
                 * a single line and put out the prompt. */
                result = 1;
                break;
            default:
                /* This tells the input to go to active GDB command */
                result = 1;
        }
    }

    if_draw();

    return result;
}

/**
 * Capture a regular expression from the user, one key at a time.
 * This modifies the global variables regex_cur and regex_last.
 *
 * \param sview
 * The source viewer.
 *
 * \return
 * 0 if user gave a regex, otherwise 1.
 */
static int status_bar_regex_input(struct sviewer *sview, int key)
{
    int regex_icase = cgdbrc_get_int(CGDBRC_IGNORECASE);

    /* Flag to indicate we're done with regex mode, need to switch back */
    int done = 0;

    /* Receive a regex from the user. */
    switch (key) {
        case '\r':
        case '\n':
        case CGDB_KEY_CTRL_M:
            /* Save for future searches via 'n' or 'N' */
            ibuf_free(regex_last);
            regex_last = ibuf_dup(regex_cur);

            regex_direction_last = regex_direction_cur;
            source_search_regex(sview, ibuf_get(regex_last), 2,
                    regex_direction_last, regex_icase);
            if_draw();
            done = 1;
            break;
        case 8:
        case 127:
            /* Backspace or DEL key */
            if (ibuf_length(regex_cur) == 0) {
                done = 1;
                source_search_regex(sview, "", 2,
                        regex_direction_cur, regex_icase);
            } else {
                ibuf_delchar(regex_cur);
                source_search_regex(sview, ibuf_get(regex_cur), 1,
                        regex_direction_cur, regex_icase);
                if_draw();
                update_status_win(WIN_REFRESH);
            }
            break;
        default:
            if (kui_term_is_cgdb_key(key)) {
                const char *keycode = kui_term_get_keycode_from_cgdb_key(key);
                int length = strlen(keycode), i;

                for (i = 0; i < length; i++)
                    ibuf_addchar(regex_cur, keycode[i]);
            } else {
                ibuf_addchar(regex_cur, key);
            }
            source_search_regex(sview, ibuf_get(regex_cur), 1,
                    regex_direction_cur, regex_icase);
            if_draw();
            update_status_win(WIN_REFRESH);
    };

    if (done) {
        ibuf_free(regex_cur);
        regex_cur = NULL;

        sbc_kind = SBC_NORMAL;
        if_set_focus(CGDB);
    }

    return 0;
}

static int status_bar_normal_input(int key)
{
    /* Flag to indicate we're done with status mode, need to switch back */
    int done = 0;

    /* The goal of this state is to receive a command from the user. */
    switch (key) {
        case '\r':
        case '\n':
        case CGDB_KEY_CTRL_M:
            /* Found a command */
            if_run_command(src_viewer, cur_sbc);
            done = 1;
            break;
        case 8:
        case 127:
            /* Backspace or DEL key */
            if (ibuf_length(cur_sbc) == 0) {
                done = 1;
            } else {
                ibuf_delchar(cur_sbc);
                update_status_win(WIN_REFRESH);
            }
            break;
        default:
            if (kui_term_is_cgdb_key(key)) {
                const char *keycode = kui_term_get_keycode_from_cgdb_key(key);
                int length = strlen(keycode), i;

                for (i = 0; i < length; i++)
                    ibuf_addchar(cur_sbc, keycode[i]);
            } else {
                ibuf_addchar(cur_sbc, key);
            }
            update_status_win(WIN_REFRESH);
            break;
    };

    if (done) {
        ibuf_free(cur_sbc);
        cur_sbc = NULL;
        if_set_focus(CGDB);
    }

    return 0;
}

static int status_bar_input(struct sviewer *sview, int key)
{
    switch (sbc_kind) {
        case SBC_NORMAL:
            return status_bar_normal_input(key);
        case SBC_REGEX:
            return status_bar_regex_input(sview, key);
    };

    return -1;
}

/**
 * toggle a breakpoint
 * 
 * \param sview
 * The source view
 *
 * \param t
 * The action to take
 *
 * \return
 * 0 on success, -1 on error.
 */
static int
toggle_breakpoint(struct sviewer *sview, enum tgdb_breakpoint_action t)
{
    int line;
    uint64_t addr = 0;
    char *path = NULL;

    if (!sview || !sview->cur || !sview->cur->path)
        return -1;

    line = sview->cur->sel_line;

    if (sview->cur->path[0] == '*')
    {
        addr = sview->cur->file_buf.addrs[line];
        if (!addr)
            return -1;
    }
    else
    {

        /* Get filename (strip path off -- GDB is dumb) */
        path = strrchr(sview->cur->path, '/') + 1;
        if (path == (char *)NULL + 1)
            path = sview->cur->path;
    }

    /* delete an existing breakpoint */
    if (sview->cur->lflags[line].breakpt)
        t = TGDB_BREAKPOINT_DELETE;

    tgdb_request_modify_breakpoint(tgdb, path, line + 1, addr, t);
    return 0;
}

/* source_input: Handles user input to the source window.
 * -------------
 *
 *   sview:     Source viewer object
 *   key:       Keystroke received.
 */
static void source_input(struct sviewer *sview, int key)
{
    switch (key) {
        case CGDB_KEY_UP:
        case 'k': {             /* VI-style up-arrow */
            int lineno = 1;
            cgdb_string_to_int(ibuf_get(G_line_number), &lineno);
            source_vscroll(sview, -lineno);
            break;
        }
        case CGDB_KEY_DOWN:
        case 'j': {             /* VI-style down-arrow */
            int lineno = 1;
            cgdb_string_to_int(ibuf_get(G_line_number), &lineno);
            source_vscroll(sview, lineno);
            break;
        }
        case CGDB_KEY_LEFT:
        case 'h':
            source_hscroll(sview, -1);
            break;
        case CGDB_KEY_RIGHT:
        case 'l':
            source_hscroll(sview, 1);
            break;
        case CGDB_KEY_CTRL_U:  /* VI-style 1/2 page up */
            source_vscroll(sview, -(get_src_height() / 2));
            break;
        case CGDB_KEY_PPAGE:
        case CGDB_KEY_CTRL_B:  /* VI-style page up */
            source_vscroll(sview, -(get_src_height() - 1));
            break;
        case CGDB_KEY_CTRL_D:  /* VI-style 1/2 page down */
            source_vscroll(sview, (get_src_height() / 2));
            break;
        case CGDB_KEY_NPAGE:
        case CGDB_KEY_CTRL_F:  /* VI-style page down */
            source_vscroll(sview, get_src_height() - 1);
            break;
        case 'g':              /* beginning of file */
            if (last_key_pressed == 'g')
                source_set_sel_line(sview, 1);
            break;
        case 'G': {              /* end of file or a line number */
            int lineno = -1;
            cgdb_string_to_int(ibuf_get(G_line_number), &lineno);
            source_set_sel_line(sview, lineno);
            break;
        }
        case '=':
            /* inc window by 1 */
            increase_win_height(0);
            break;
        case '-':
            /* dec window by 1 */
            decrease_win_height(0);
            break;
        case '+':
            increase_win_height(1);
            break;
        case '_':
            decrease_win_height(1);
            break;
        case 'o':
            /* Causes file dialog to be opened */
        {
            extern int kui_input_acceptable;

            kui_input_acceptable = 0;

            tgdb_request_inferiors_source_files(tgdb);
        }
            break;
        case ' ':
        {
            enum tgdb_breakpoint_action t = TGDB_BREAKPOINT_ADD;

            toggle_breakpoint(sview, t);
        }
            break;
        case 't':
        {
            enum tgdb_breakpoint_action t = TGDB_TBREAKPOINT_ADD;

            toggle_breakpoint(sview, t);
        }
            break;
        default:
            break;
    }

    /* Store digits into G_line_number for 'G' command. */
    if (key >= '0' && key <= '9') {
        ibuf_addchar(G_line_number, key);
    } else {
        ibuf_clear(G_line_number);
    }

    /* Some extended features that are set by :set sc */
    if_draw();
}

/* Sets up the signal handler for SIGWINCH
 * Returns -1 on error. Or 0 on success */
static int set_up_signal(void)
{
    struct sigaction action;

    action.sa_handler = signal_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    if (sigaction(SIGWINCH, &action, NULL) < 0) {
        clog_error(CLOG_CGDB, "sigaction failed ");
        return -1;
    }

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

    if (sigaction(SIGCHLD, &action, NULL) < 0) {
        clog_error(CLOG_CGDB, "sigaction failed ");
        return -1;
    }

    return 0;
}

/* ----------------- */
/* Exposed Functions */
/* ----------------- */

/* See interface.h for function descriptions. */
int if_init(void)
{
    if (init_curses())
    {
        clog_error(CLOG_CGDB, "Unable to initialize the ncurses library");
        return -1;
    }

    hl_groups_instance = hl_groups_initialize();
    if (!hl_groups_instance)
    {
        clog_error(CLOG_CGDB, "Unable to setup highlighting groups");
        return -1;
    }

    if (hl_groups_setup(hl_groups_instance) == -1)
    {
        clog_error(CLOG_CGDB, "Unable to setup highlighting groups");
        return -1;
    }

    /* Set up the signal handler to catch SIGWINCH */
    if (set_up_signal() == -1)
    {
        clog_error(CLOG_CGDB, "Unable to handle signal: SIGWINCH");
        return -1;
    }

    if (ioctl(fileno(stdout), TIOCGWINSZ, &screen_size) == -1) {
        screen_size.ws_row = swin_lines();
        screen_size.ws_col = swin_cols();
    }

    /* Create the file dialog object */
    fd = filedlg_new(0, 0, HEIGHT, WIDTH);

    /* Set up window layout */
    window_shift = (int) ((HEIGHT / 2) * (cur_win_split / 2.0));
    switch (if_layout()) {
        case 2:
            return 4;
    }

    G_line_number = ibuf_init();

    return 0;
}

/**
 * Send input to the CGDB source window.
 * 
 * @param key
 * The key to send to the CGDB source window.
 *
 * @param last_key
 * An output parameter. When set, that will tell cgdb to use the set value,
 * instead of the current key, as the "last_key_pressed" in the next
 * call to cgdb_input. This is useful to set mainly when the current input
 * has consumed more than one character, and the "last_key_pressed" should
 * be not set on the next call to cgdb_input.
 *
 * @return
 * Currently only returns 0.
 */
static int cgdb_input(int key, int *last_key)
{
    int regex_icase = cgdbrc_get_int(CGDBRC_IGNORECASE);

    if (src_viewer && src_viewer->cur) {
        int ret = 0;

        /* Handle setting (mX) and going ('X) to source buffer marks */
        if (last_key_pressed == 'm')
            ret = source_set_mark(src_viewer, key);
        else if (last_key_pressed == '\'')
            ret = source_goto_mark(src_viewer, key);

        if (ret) {
            /* When m[a-zA-Z] matches, don't let the marker char
             * be treated as the last key. That would allow the
             * chars mgg, to set the marker g, and then move to the top
             * of the file via gg.
             * CGDB should see those as mg (set a local mark g), and then
             * an individual g.
             */
            *last_key = 0;
            if_draw();
            return 0;
        }
    }

    switch (key) {
        case 's':
            gdb_scroller->in_scroll_mode = 1;
            if_set_focus(GDB);
            return 0;
        case 'i':
            if_set_focus(GDB);
            return 0;
        case ':':
            /* Set the type of the command the user is typing in the status bar */
            sbc_kind = SBC_NORMAL;
            if_set_focus(CGDB_STATUS_BAR);
            /* Since the user is about to type in a command, allocate a buffer
                         * in which this command can be stored. */
            cur_sbc = ibuf_init();
            return 0;
        case '/':
        case '?':
            if (src_viewer->cur) {
                regex_cur = ibuf_init();
                regex_direction_cur = ('/' == key);
                orig_line_regex = src_viewer->cur->sel_line;

                sbc_kind = SBC_REGEX;
                if_set_focus(CGDB_STATUS_BAR);

                /* Capturing regular expressions */
                source_search_regex_init(src_viewer);

                /* Initialize the function for finding a regex and tell user */
                if_draw();
            }
            return 0;
        case 'n':
            source_search_regex(src_viewer, ibuf_get(regex_last), 2,
                                regex_direction_last, regex_icase);
            if_draw();
            break;
        case 'N':
            source_search_regex(src_viewer, ibuf_get(regex_last), 2,
                                !regex_direction_last, regex_icase);
            if_draw();
            break;
        case CGDB_KEY_CTRL_T:
            if (tgdb_tty_new(tgdb) == -1) {
                /* Error */
            } else {
                if_layout();
            }

            break;
        case CGDB_KEY_CTRL_W:
            switch (cur_split_orientation) {
                case WSO_HORIZONTAL:
                    cur_split_orientation = WSO_VERTICAL;
                    break;
                case WSO_VERTICAL:
                    cur_split_orientation = WSO_HORIZONTAL;
                    break;
            }

            if_layout();

            break;
        case CGDB_KEY_F1:
            if_display_help();
            return 0;
        case CGDB_KEY_F5:
            /* Issue GDB run command */
            tgdb_request_run_debugger_command(tgdb, TGDB_RUN);
            return 0;
        case CGDB_KEY_F6:
            /* Issue GDB continue command */
            tgdb_request_run_debugger_command(tgdb, TGDB_CONTINUE);
            return 0;
        case CGDB_KEY_F7:
            /* Issue GDB finish command */
            tgdb_request_run_debugger_command(tgdb, TGDB_FINISH);
            return 0;
        case CGDB_KEY_F8:
            /* Issue GDB next command */
            tgdb_request_run_debugger_command(tgdb, TGDB_NEXT);
        case CGDB_KEY_F10:
            /* Issue GDB step command */
            tgdb_request_run_debugger_command(tgdb, TGDB_STEP);
            return 0;
        case CGDB_KEY_CTRL_L:
            if_layout();
            return 0;
    }

    source_input(src_viewer, key);
    return 0;
}

int internal_if_input(int key, int *last_key)
{
    /* Normally, CGDB_KEY_ESC, but can be configured by the user */
    int cgdb_mode_key = cgdbrc_get_int(CGDBRC_CGDB_MODE_KEY);

    /* The cgdb mode key, puts the debugger into command mode */
    if (focus != CGDB && key == cgdb_mode_key) {
        enum Focus new_focus = CGDB;

        /* Depending on which cgdb was in, it can free some memory here that
         * it was previously using. */
        if (focus == CGDB_STATUS_BAR && sbc_kind == SBC_NORMAL) {
            ibuf_free(cur_sbc);
            cur_sbc = NULL;
        } else if (focus == CGDB_STATUS_BAR && sbc_kind == SBC_REGEX) {
            ibuf_free(regex_cur);
            regex_cur = NULL;

            hl_regex_free(&src_viewer->hlregex);

            src_viewer->cur->sel_rline = orig_line_regex;
            src_viewer->cur->sel_line = orig_line_regex;
            sbc_kind = SBC_NORMAL;
        }
        else if (focus == GDB && sbc_kind == SBC_REGEX)
        {
            ibuf_free(regex_cur);
            regex_cur = NULL;

            gdb_scroller->in_search_mode = 0;
            sbc_kind = SBC_NORMAL;

            new_focus = GDB;
        }

        if_set_focus(new_focus);
        return 0;
    }
    /* If you are already in cgdb mode, the cgdb mode key does nothing */
    else if (key == cgdb_mode_key)
        return 0;

    /* Check for global keystrokes */
    switch (focus) {
        case CGDB:
            return cgdb_input(key, last_key);
        case GDB:
            return gdb_input(key, last_key);
        case FILE_DLG:
        {
            char filedlg_file[MAX_LINE];
            int ret = filedlg_recv_char(fd, key, filedlg_file, last_key_pressed);

            /* The user cancelled */
            if (ret == -1) {
                if_set_focus(CGDB);
                return 0;
                /* Needs more data */
            } else if (ret == 0) {
                return 0;
                /* The user picked a file */
            } else if (ret == 1) {
                if_show_file(filedlg_file, 0, 0);
                if_set_focus(CGDB);
                return 0;
            }
        }
            return 0;
        case CGDB_STATUS_BAR:
            return status_bar_input(src_viewer, key);
    }

    /* Never gets here */
    return 0;
}

int if_input(int key)
{
    int last_key = key;
    int result = internal_if_input(key, &last_key);

    last_key_pressed = last_key;
    return result;
}

static void if_print_internal(const char *buf, enum ScrInputKind kind)
{
    if (!gdb_scroller) {
        clog_error(CLOG_CGDB, "%s", buf);
        return;
    }

    /* Print it to the scroller */
    scr_add(gdb_scroller, buf, kind);

    if (get_gdb_height() > 0) {
        scr_refresh(gdb_scroller, focus == GDB, WIN_NO_REFRESH);

        /* Make sure cursor reappears in source window if focus is there */
        if (focus == CGDB)
            swin_wnoutrefresh(src_viewer->win);

        swin_doupdate();
    }

}

void if_tty_print(const char *buf)
{
    /* Send output to the gdb buffer */
    if_print_internal(buf, SCR_INPUT_INFERIOR);
}

void if_print(const char *buf)
{
    if_print_internal(buf, SCR_INPUT_DEBUGGER);
}

void if_rl_print(const char *buf)
{
    if_print_internal(buf, SCR_INPUT_READLINE);
}

void if_sdc_print(const char *buf)
{
    if_print_message("cgdb sdc:%s", buf);
}

void if_print_message(const char *fmt, ...)
{
    va_list ap;
    char va_buf[MAXLINE];

    /* Get the buffer with format */
    va_start(ap, fmt);
#ifdef   HAVE_VSNPRINTF
    vsnprintf(va_buf, sizeof (va_buf), fmt, ap);    /* this is safe */
#else
    vsprintf(va_buf, fmt, ap);  /* this is not safe */
#endif
    va_end(ap);

    if_print(va_buf);
}

void if_show_file(char *path, int sel_line, int exe_line)
{
    if (source_set_exec_line(src_viewer, path, sel_line, exe_line) == 0)
        if_draw();
}

void if_display_help(void)
{
    char cgdb_help_file[FSUTIL_PATH_MAX];
    int ret_val = 0;

    fs_util_get_path(PKGDATADIR, "cgdb.txt", cgdb_help_file);

    /* File doesn't exist. Try to find cgdb.txt in the build dir in case
     * the user is running a built cgdb binary directly. */
    if (!fs_verify_file_exists(cgdb_help_file))
        fs_util_get_path(TOPBUILDDIR, "doc/cgdb.txt", cgdb_help_file);

    ret_val = source_set_exec_line(src_viewer, cgdb_help_file, 1, 0);

    if (ret_val == 0)
    {
        src_viewer->cur->language = TOKENIZER_LANGUAGE_CGDBHELP;
        source_highlight(src_viewer->cur);
        if_draw();
    }
    else if (ret_val == 5)      /* File does not exist */
        if_display_message("No such file: ", WIN_REFRESH, 0, "%s", cgdb_help_file);
}

void if_display_logo(int reset)
{
    if (reset)
        logo_reset();

    src_viewer->cur = NULL;
}

struct sviewer *if_get_sview()
{
    return src_viewer;
}

void if_clear_filedlg(void)
{
    filedlg_clear(fd);
}

void if_add_filedlg_choice(const char *filename)
{
    filedlg_add_file_choice(fd, filename);
}

void if_filedlg_display_message(char *message)
{
    filedlg_display_message(fd, message);
}

void if_shutdown(void)
{
    /* Shut down curses cleanly */
    if (curses_initialized)
        swin_endwin();

    if (status_win) {
        swin_delwin(status_win);
        status_win = NULL;
    }

    if (gdb_scroller) {
        scr_free(gdb_scroller);
        gdb_scroller = NULL;
    }

    if (src_viewer) {
        source_free(src_viewer);
        src_viewer = NULL;
    }

    if (vseparator_win) {
        swin_delwin(vseparator_win);
        vseparator_win = NULL;
    }

    if (G_line_number) {
        ibuf_free(G_line_number);
        G_line_number = 0;
    }
}

void if_set_focus(Focus f)
{
    switch (f) {
        case GDB:
            focus = f;
            if_draw();
            break;
        case CGDB:
            focus = f;
            if_draw();
            break;
        case FILE_DLG:
            focus = f;
            if_draw();
            break;
        case CGDB_STATUS_BAR:
            focus = f;
            if_draw();
        default:
            return;
    }
}

Focus if_get_focus(void)
{
    return focus;
}

void reset_window_shift(void)
{
    int h_or_w = cur_split_orientation == WSO_HORIZONTAL ? HEIGHT : WIDTH;

    window_shift = (int) ((h_or_w / 2) * (cur_win_split / 2.0));
    if_layout();
}

void if_set_winsplitorientation(WIN_SPLIT_ORIENTATION_TYPE new_orientation)
{
    cur_split_orientation = new_orientation;
    reset_window_shift();
}

void if_set_winsplit(WIN_SPLIT_TYPE new_split)
{
    cur_win_split = new_split;
    reset_window_shift();
}

void if_highlight_sviewer(enum tokenizer_language_support l)
{
    /* src_viewer->cur is NULL when reading cgdbrc */
    if (src_viewer && src_viewer->cur) {
        if ( l == TOKENIZER_LANGUAGE_UNKNOWN )
            l = tokenizer_get_default_file_type(strrchr(src_viewer->cur->path, '.'));

        src_viewer->cur->language = l;
        source_highlight(src_viewer->cur);
        if_draw();
    }
}

int if_change_winminheight(int value)
{
    if (value < 0)
        return -1;
    else if (value > HEIGHT / 2)
        return -1;

    interface_winminheight = value;
    if_layout();

    return 0;
}

int if_change_winminwidth(int value)
{
    if (value < 0)
        return -1;
    else if (value > WIDTH / 2)
        return -1;

    interface_winminwidth = value;
    if_layout();

    return 0;
}

int if_clear_line()
{
    std::string line;

    line.push_back('\r');
    line.append(get_gdb_width(), ' ');
    line.push_back('\r');

    if_print(line.c_str());

    return 0;
}
