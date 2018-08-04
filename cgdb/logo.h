/* logo.h:  Routine for displaying an ASCII-art CGDB logo on screen.
 * -------
 */

#ifndef _LOGO_H_
#define _LOGO_H_

/* Local Includes */
#include "highlight_groups.h"

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

#ifdef TESTING
void tst_set_logo_index(int index);
int tst_get_logo_index();
int tst_get_logo_height();
int tst_get_logo_width();
int tst_logos_available();
int tst_get_cgdb_num_usage();
void tst_center_line(SWINDOW *win, int row, int width, const char* data,
                     int datawidth, enum hl_group_kind group_kind);
#endif // TESTING

#endif // _LOGO_H_
