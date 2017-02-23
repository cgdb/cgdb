#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_CURSES_H
#include <curses.h>
#elif HAVE_NCURSES_CURSES_H
#include <ncurses/curses.h>
#endif

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
    return win;
}

int swin_endwin()
{
    return endwin();
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
    return start_color();
}

int swin_use_default_colors()
{
    return use_default_colors();
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
    ret = vwprintw((WINDOW *)win, fmt, ap);
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
        ret = vwprintw((WINDOW *)win, fmt, ap);
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
    return init_pair(pair, f, b);
}

int swin_pair_content(int pair, int *fin, int *bin)
{
    int ret;
    short f, b;

    ret = pair_content(pair, &f, &b);

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
