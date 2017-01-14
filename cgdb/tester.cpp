/* tester.c:
 * ---------
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

/* System Includes */

/* Colors */
#define CGDB_COLOR_GREEN            1
#define CGDB_COLOR_RED              2
#define CGDB_COLOR_CYAN             3
#define CGDB_COLOR_WHITE            4
#define CGDB_COLOR_MAGENTA          5
#define CGDB_COLOR_BLUE             6
#define CGDB_COLOR_YELLOW           7

#define CGDB_COLOR_INVERSE_GREEN    8
#define CGDB_COLOR_INVERSE_RED      9
#define CGDB_COLOR_INVERSE_CYAN     10
#define CGDB_COLOR_INVERSE_WHITE    11
#define CGDB_COLOR_INVERSE_MAGENTA  12
#define CGDB_COLOR_INVERSE_BLUE     13
#define CGDB_COLOR_INVERSE_YELLOW   14
#define CGDB_COLOR_STATUS_BAR       15

/* Local Includes */
#include "sys_win.h"
#include "sources.h"
#include "cgdb.h"

/* --------------- */
/* Local Variables */
/* --------------- */

static int curses_initialized = 0;  /* Set when curses has been started */
static int curses_colors = 0;   /* Set if terminal supports color */
static char *my_name = NULL;    /* Name of this application (argv[0]) */

/* --------- */
/* Functions */
/* --------- */

int init_curses()
{
    swin_initscr();             /* Start curses mode */
    cbreak();                   /* Line buffering disabled */
    noecho();                   /* Do not echo characters typed by user */
    swin_keypad(stdscr, TRUE);  /* Translate arrow keys, Fn keys, etc. */

    if ((curses_colors = swin_has_colors())) {
        start_color();
        swin_init_pair(CGDB_COLOR_BLACK, COLOR_BLACK, COLOR_BLACK);
        swin_init_pair(CGDB_COLOR_GREEN, COLOR_GREEN, COLOR_BLACK);
        swin_init_pair(CGDB_COLOR_RED, COLOR_RED, COLOR_BLACK);
        swin_init_pair(CGDB_COLOR_CYAN, COLOR_CYAN, COLOR_BLACK);
        swin_init_pair(CGDB_COLOR_WHITE, COLOR_WHITE, COLOR_BLACK);
        swin_init_pair(CGDB_COLOR_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
        swin_init_pair(CGDB_COLOR_BLUE, COLOR_BLUE, COLOR_BLACK);
        swin_init_pair(CGDB_COLOR_YELLOW, COLOR_YELLOW, COLOR_BLACK);
    }

    curses_initialized = 1;
    return 0;
}

/* cleanup: Invoked by the various err_xxx funtions when dying.
 * -------- */
void cleanup()
{
    /* Shut down curses cleanly */
    if (curses_initialized) {
        swin_endwin();
    }
}

int main(int argc, char *argv[])
{
    /* NOTE:  This whole main is pretty much garbage right now.  Its only
     *        purpose is to show 2 source files using the sources module,
     *        for testing purposes only. */

    int c = 0;
    int num = 12;
    int line1 = 7, line2 = 7, *line = &line1;
    char *bar = "________________________________________"
            "________________________________________";

    /* Set up some data */
    my_name = argv[0];

    if (argc < 3)
        err_quit("  USAGE: %s <file 1> <file 2>", my_name);

    /* Initialize the display */
    if (init_curses())
        err_quit("%s: Unable to initialize the curses library\n", my_name);

    source_add(argv[1]);
    source_add(argv[2]);
    swin_move(0, 0);
    attron(SWIN_A_BOLD);
    swin_wprintw(stdscr, "ESC                  : Exit\n");
    swin_wprintw(stdscr, "TAB                  : Switch panes\n");
    swin_wprintw(stdscr, "UP, DOWN, PGUP, PGDN : Scroll current pane\n");
    swin_mvwprintw(stdscr, 4, 0, bar);
    swin_mvwprintw(stdscr, 17, 0, bar);
    swin_attroff(SWIN_A_BOLD);
    swin_mvwprintw(stdscr, 19, 0, bar);
    swin_mvwprintw(stdscr, 32, 0, bar);
    do {
        switch (c) {
            case 9:
                if (line == &line1) {
                    swin_mvwprintw(stdscr, 4, 0, bar);
                    swin_mvwprintw(stdscr, 17, 0, bar);
                    swin_attron(SWIN_A_BOLD);
                    swin_mvwprintw(stdscr, 19, 0, bar);
                    swin_mvwprintw(stdscr, 32, 0, bar);
                    swin_attroff(SWIN_A_BOLD);
                    line = &line2;
                } else {
                    swin_attron(SWIN_A_BOLD);
                    swin_mvwprintw(stdscr, 4, 0, bar);
                    swin_mvwprintw(stdscr, 17, 0, bar);
                    swin_attroff(SWIN_A_BOLD);
                    swin_mvwprintw(stdscr, 19, 0, bar);
                    swin_mvwprintw(stdscr, 32, 0, bar);
                    line = &line1;
                }
                break;
            case KEY_DOWN:
                (*line)++;
                break;
            case KEY_UP:
                (*line)--;
                break;
            case KEY_PPAGE:
                (*line) -= 10;
                break;
            case KEY_NPAGE:
                (*line) += 10;
                break;
        }
        if (*line > source_length(argv[line == &line1 ? 1 : 2]) - 5)
            *line = source_length(argv[line == &line1 ? 1 : 2]) - 5;
        if (*line < 7)
            *line = 7;
        swin_move(5, 0);
        source_display(argv[1], stdscr, line1, num, 1);
        swin_move(20, 0);
        source_display(argv[2], stdscr, line2, num, 1);
    } while ((c = getch()) != 27);
    source_del(argv[1]);
    source_del(argv[2]);

    cleanup();
    return 0;
}
