/* logo.h:  Routine for displaying an ASCII-art CGDB logo on screen.
 * -------
 */

#ifndef _LOGO_H_
#define _LOGO_H_

#if HAVE_CURSES_H
#include <curses.h>
#elif HAVE_NCURSES_CURSES_H
#include <ncurses/curses.h>
#endif /* HAVE_CURSES_H */

/* ------------------- */
/* Function Prototypes */
/* ------------------- */

/* logo_display:  Chooses a random CGDB logo (only once) and displays it with
 * -------------  some basic help text for first-time users.
 *
 *   win:  The curses window to use.  If the logo won't fit in the given
 *         window, only the title is shown.
 */
void logo_display(WINDOW * win);

#endif
