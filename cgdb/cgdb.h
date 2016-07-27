/* cgdb.h:
 * -------
 *
 * Contains macros for any of the sources here to use.
 */

#ifndef _CGDB_H_
#define _CGDB_H_

/* ----------- */
/* Definitions */
/* ----------- */

#define MAXLINE 4096

/* From the ncurses doupdate() man page:
 *
 * The wnoutrefresh and doupdate routines allow multiple updates with more
 * efficiency than wrefresh alone. In addition to all the window structures,
 * curses keeps two data structures representing the terminal screen: a
 * physical screen, describing what is actually on the screen, and a virtual
 * screen, describing what the programmer wants to have on the screen. 
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
 *
 * So we use the win_refresh option to tell the routine whether to use
 * wrefresh() or wnoutrefresh(). This eliminates quite a bit of flashing as
 * well.
 */
enum win_refresh {
    /* Tells CGDB to use wnoutrefresh */
    WIN_NO_REFRESH,
    /* Tells CGDB to use wrefresh */
    WIN_REFRESH
};

/* Clean cgdb up (when exiting) */
void cgdb_cleanup_and_exit(int val);

/*
 * See documentation in cgdb.c.
 */
int run_shell_command(const char *command);

void rl_resize(int rows, int cols);

#endif
