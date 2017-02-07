#ifndef __SYS_WIN_H__
#define __SYS_WIN_H__

typedef struct SWINDOW SWINDOW;
typedef unsigned long SWIN_CHTYPE;

#define COLOR_BLACK 0
#define COLOR_RED 1
#define COLOR_GREEN 2
#define COLOR_YELLOW 3
#define COLOR_BLUE 4
#define COLOR_MAGENTA 5
#define COLOR_CYAN 6
#define COLOR_WHITE 7

/* attributes */
extern const int SWIN_A_NORMAL;
extern const int SWIN_A_STANDOUT;
extern const int SWIN_A_UNDERLINE;
extern const int SWIN_A_REVERSE;
extern const int SWIN_A_BLINK;
extern const int SWIN_A_DIM;
extern const int SWIN_A_BOLD;

extern const int SWIN_KEY_BACKSPACE; /* backspace key */

extern SWIN_CHTYPE SWIN_SYM_VLINE; /* vertical line */
extern SWIN_CHTYPE SWIN_SYM_HLINE; /* horizontal line */
extern SWIN_CHTYPE SWIN_SYM_LTEE;  /* tee pointing right */

/* Determines the terminal type and initializes all data structures. */
SWINDOW *swin_initscr();
/* The program must call endwin for each terminal being used before exiting. */
int swin_endwin();

int swin_lines();       /* height of screen */
int swin_cols();        /* width of screen */
int swin_colors();      /* number of colors supported */
int swin_color_pairs(); /* number of color pairs supported */

int swin_has_colors();
int swin_start_color();
int swin_use_default_colors();

/* Resizes the standard and current windows to the specified dimensions, and
   adjusts other bookkeeping data used by the ncurses library that record the
   window dimensions such as the LINES and COLS variables. */
int swin_resizeterm(int lines, int columns);

SWINDOW *swin_newwin(int nlines, int ncols, int begin_y, int begin_x);
int swin_delwin(SWINDOW *win);

/* Scroll window up n lines */
int swin_scrl(int n);   

/* The keypad option enables the keypad of the user's terminal. If enabled 
   the user can press a function key (such as an arrow key) and wgetch returns
   a single value representing the function key, as in KEY_LEFT. If disabled
   curses does not treat function keys specially and the program has to
   interpret the escape sequences itself. */
int swin_keypad(SWINDOW *win, int bf);

/* Get terminfo value capname capability */
char *swin_tigetstr(const char *capname);

/* Move cursor */
int swin_move(int y, int x);
int swin_wmove(SWINDOW *win, int y, int x);

/* Set cursor state */
int swin_curs_set(int visibility);

/* Turns on/off named attributes */
int swin_wattron(SWINDOW *win, int attrs);
int swin_wattroff(SWINDOW *win, int attrs);

int swin_getcurx(const SWINDOW *win);
int swin_getcury(const SWINDOW *win);

int swin_getbegx(const SWINDOW *win);
int swin_getbegy(const SWINDOW *win);
int swin_getmaxx(const SWINDOW *win);
int swin_getmaxy(const SWINDOW *win);

/* Copy blanks to every position in the window */
int swin_werase(SWINDOW *win);

/* Draw a vertical (top to bottom) line using ch starting at the current cursor
   position in the window. The current cursor position is not changed. The line is
   at most n characters long, or as many as fit into the window. */
int swin_wvline(SWINDOW *win, SWIN_CHTYPE ch, int n);
/* Draw ch and advance cursor */
int swin_waddch(SWINDOW *win, const SWIN_CHTYPE ch);
/* Erase the current line to the right of the cursor, inclusive, to the end of the
   current line. */
int swin_wclrtoeol(SWINDOW *win);

int swin_waddnstr(SWINDOW *win, const char *str, int n);
int swin_wprintw(SWINDOW *win, const char *fmt, ...) ATTRIBUTE_PRINTF(2, 3);
int swin_mvwprintw(SWINDOW *win, int y, int x, const char *fmt, ...) ATTRIBUTE_PRINTF(4, 5);

/* From the ncurses doupdate() man page:
 *
 * The routine wrefresh works by first calling wnoutrefresh, which copies the
 * named window to the virtual screen, and then calling doupdate, which
 * compares the virtual screen to the physical screen and does the actual
 * update. If the programmer wishes to output several windows at once, a
 * series of calls to wrefresh results in alternating calls to wnoutrefresh
 * and doupdate, causing several bursts of output to the screen. By first
 * calling wnoutrefresh for each window, it is then possible to call doupdate
 * once, resulting in only one burst of output, with fewer total characters
 * transmitted and less CPU time used.
 */
int swin_refresh();
int swin_wrefresh(SWINDOW *win);
int swin_wnoutrefresh(SWINDOW *win);
int swin_doupdate();

/*
 * Curses supports color attributes on terminals with that capability. To use
 * these routines start_color must be called, usually right after initscr.
 * Colors are always used in pairs (referred to as color-pairs). A color-pair
 * consists of a foreground color (for characters) and a background color
 * (for the blank field on which the characters are displayed). A programmer
 * initializes a color-pair with the routine init_pair. After it has been
 * initialized, COLOR_PAIR(n), a macro defined in <curses.h>, can be used as
 * a new video attribute.
 */
int swin_init_pair(int pair, int f, int b);
int swin_pair_content(int pair, int *f, int *b);
int swin_color_pair(int pair); /* COLOR_PAIR(n) */

/**
 * Put the keyboard into raw mode.
 *
 * This is currently only used by the kui_driver.
 *
 * @return
 * 0 on succes or -1 on error
 */
int swin_raw(void);

#endif
