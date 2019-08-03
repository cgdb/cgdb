#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_CURSES_H
#include <curses.h>
#elif HAVE_NCURSES_CURSES_H
#include <ncurses/curses.h>
#endif

#include "cgdb_clog.h"
#include "sys_util.h"
#include "sys_win.h"

const int SWIN_A_NORMAL = A_NORMAL;
const int SWIN_A_STANDOUT = A_STANDOUT;
const int SWIN_A_UNDERLINE = A_UNDERLINE;
const int SWIN_A_REVERSE = A_REVERSE;
const int SWIN_A_BLINK = A_BLINK;
const int SWIN_A_DIM = A_DIM;
const int SWIN_A_BOLD = A_BOLD;

const int SWIN_KEY_BACKSPACE = KEY_BACKSPACE;

SWIN_CHTYPE SWIN_SYM_VLINE;
SWIN_CHTYPE SWIN_SYM_HLINE;
SWIN_CHTYPE SWIN_SYM_LTEE;

static bool initscr_intialized = false;

/**
 * Set the ESCDELAY environment variable.
 *
 * This environment variable is necessary for ncurses to not delay
 * user input when the ESCAPE key is pressed. CGDB does it's own handling
 * of escape.
 *
 * @return
 * True on success or False on failure
 */
static bool swin_set_escdelay(void)
{
    static char escdelay[] = "ESCDELAY=0";
    bool success = putenv(escdelay) == 0;
                
    if (!success)
        clog_error(CLOG_CGDB, "putenv(\"%s\") failed", escdelay);

    return success;
}

bool swin_start()
{
    bool success = swin_set_escdelay();
    if (success) {
        success = swin_initscr() != NULL;
        if (success) {
            if (swin_has_colors()) {
                swin_start_color();
                swin_use_default_colors();
            }

            swin_refresh();
        }
    }

    return success;
}

/* Determines the terminal type and initializes all data structures. */
SWINDOW *swin_initscr()
{
    SWINDOW *win = (SWINDOW *)initscr();

#ifdef HAVE_CYGWIN
    SWIN_SYM_VLINE = ':';
#else
    SWIN_SYM_VLINE = ACS_VLINE;
#endif

    SWIN_SYM_HLINE = ACS_HLINE;
    SWIN_SYM_LTEE = ACS_LTEE;
    
    initscr_intialized = true;

    return win;
}

int swin_endwin()
{
    int result = 0;
    
    if (initscr_intialized)
        result = endwin();

    return result;
}

int swin_lines()
{
    return LINES;
}

int swin_cols()
{
    return COLS;
}

int swin_colors()
{
    return COLORS;
}

int swin_color_pairs()
{
    return COLOR_PAIRS;
}

int swin_has_colors()
{
    return has_colors();
}

int swin_start_color()
{
    int result = start_color();
    if (result == ERR) {
        clog_error(CLOG_CGDB, "start_color failed");
    }
    return result;
}

int swin_use_default_colors()
{
    int result = use_default_colors();
    if (result == ERR) {
        clog_error(CLOG_CGDB, "use_default_colors failed");
    }
    return result;
}

bool swin_supports_default_color_pairs_extension()
{
    int result = init_pair(65, -1, COLOR_BLACK);
    return result != ERR;
}

int swin_resizeterm(int lines, int columns)
{
    return resizeterm(lines, columns);
}

int swin_scrl(int n)
{
    return scrl(n);
}

int swin_keypad(SWINDOW *win, int bf)
{
    return keypad((WINDOW *)win, bf);
}

char *swin_tigetstr(const char *capname)
{
    return tigetstr((char*)capname);
}

int swin_move(int y, int x)
{
    return move(y, x);
}

int swin_wmove(SWINDOW *win, int y, int x)
{
    return wmove((WINDOW *)win, y, x);
}

int swin_wattron(SWINDOW *win, int attrs)
{
    return wattron((WINDOW *)win, attrs);
}

int swin_wattroff(SWINDOW *win, int attrs)
{
    return wattroff((WINDOW *)win, attrs);
}

SWINDOW *swin_newwin(int nlines, int ncols, int begin_y, int begin_x)
{
    return (SWINDOW *)newwin(nlines, ncols, begin_y, begin_x);
}

int swin_delwin(SWINDOW *win)
{
    return delwin((WINDOW *)win);
}

int swin_curs_set(int visibility)
{
    return curs_set(visibility);
}

int swin_getcurx(const SWINDOW *win)
{
    return getcurx((WINDOW *)win);
}

int swin_getcury(const SWINDOW *win)
{
    return getcury((WINDOW *)win);
}

int swin_getbegx(const SWINDOW *win)
{
    return getbegx((WINDOW *)win);
}

int swin_getbegy(const SWINDOW *win)
{
    return getbegy((WINDOW *)win);
}

int swin_getmaxx(const SWINDOW *win)
{
    return getmaxx((WINDOW *)win);
}

int swin_getmaxy(const SWINDOW *win)
{
    return getmaxy((WINDOW *)win);
}

int swin_werase(SWINDOW *win)
{
    return werase((WINDOW *)win);
}

int swin_wvline(SWINDOW *win, SWIN_CHTYPE ch, int n)
{
    return wvline((WINDOW *)win, ch, n);
}

int swin_waddch(SWINDOW *win, const SWIN_CHTYPE ch)
{
    return waddch((WINDOW *)win, ch);
}

int swin_wprintw(SWINDOW *win, const char *fmt, ...)
{
    int ret;
    va_list ap;

    va_start(ap, fmt);
    ret = vw_printw((WINDOW *)win, fmt, ap);
    va_end(ap);

    return ret;
}

int swin_waddnstr(SWINDOW *win, const char *str, int n)
{
    return waddnstr((WINDOW *)win, str, n);
}

int swin_wclrtoeol(SWINDOW *win)
{
    return wclrtoeol((WINDOW *)win);
}

int swin_mvwprintw(SWINDOW *win, int y, int x, const char *fmt, ...)
{
    int ret;

    ret = wmove((WINDOW *)win, y, x);
    if (ret != ERR)
    {
        va_list ap;

        va_start(ap, fmt);
        ret = vw_printw((WINDOW *)win, fmt, ap);
        va_end(ap);
    }

    return ret;
}

int swin_refresh()
{
    return refresh();
}

int swin_wnoutrefresh(SWINDOW *win)
{
    return wnoutrefresh((WINDOW *)win);
}

int swin_wrefresh(SWINDOW *win)
{
    return wrefresh((WINDOW *)win);
}

int swin_doupdate()
{
    return doupdate();
}

int swin_init_pair(int pair, int f, int b)
{
    int result = init_pair(pair, f, b);
    if (result == ERR) {
        clog_error(CLOG_CGDB, "init_pair failed pair=%d f=%d b=%d", pair, f, b);
    }

    return result;
}

int swin_pair_content(int pair, int *fin, int *bin)
{
    int ret;
    short f, b;

    ret = pair_content(pair, &f, &b);
    if (ret == ERR) {
        clog_error(CLOG_CGDB, "pair_content failed pair=%d", pair);
    }

    *fin = f;
    *bin = b;
    return ret;
}

int swin_color_pair(int pair)
{
    return COLOR_PAIR(pair);
}

int swin_raw(void)
{
    return raw();
}
