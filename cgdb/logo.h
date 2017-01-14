/* logo.h:  Routine for displaying an ASCII-art CGDB logo on screen.
 * -------
 */

#ifndef _LOGO_H_
#define _LOGO_H_

/* ------------------- */
/* Function Prototypes */
/* ------------------- */

/* logo_display:  Chooses a random CGDB logo (only once) and displays it with
 * -------------  some basic help text for first-time users.
 *
 *   win:  The curses window to use.  If the logo won't fit in the given
 *         window, only the title is shown.
 */
void logo_display(SWINDOW *win);

/**
 * This chooses another logo to be displayed on the next call to logo_display.
 */
void logo_reset();

#endif
