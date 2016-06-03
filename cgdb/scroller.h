/* scroller.h:
 * -----------
 *
 * A scrolling buffer utility.  Able to add and subtract to the buffer.
 * All routines that would require a screen update will automatically redraw
 * the scroller.  There is no "draw" function.
 */

#ifndef _SCROLLER_H_
#define _SCROLLER_H_

/* Local Includes */
#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_CURSES_H
#include <curses.h>
#elif HAVE_NCURSES_CURSES_H
#include <ncurses/curses.h>
#endif /* HAVE_CURSES_H */

/* --------------- */
/* Data Structures */
/* --------------- */

struct scroller_line {
    char *line;
    int line_len;
    int tty;
    struct hl_line_attr *attrs;
};

struct scroller {
    struct scroller_line *lines;

    char *last_tty_line;
    int last_tty_attr;

    int in_scroll_mode;         /* Currently in scroll mode? */
    /**
     * The number of lines to display in the cursor.
     *
     * This starts at zero, and only the prompt is displayed in the
     * scroller. Run cgdb -q to see this.
     *
     * Every time another line is added to the scroller, this is increased,
     * so that more lines are dispalyed. It will ultimatley hit the height
     * of the scroller and max out.
     */

    int lines_to_display;

    /**
     * The last row that the cursor was drawn at.
     *
     * There is currently an open issue with the cursor not being removed
     * from the screen properly by ncurses.
     *   http://lists.gnu.org/archive/html/bug-ncurses/2016-08/msg00001.html
     *
     * When redrawing the window, if the cursor has moved, the previous
     * cursor is not cleared from the screen with a call to wclrtoeol.
     *
     * For now, we call wclear if the cursor was on a line different than
     * the line we are about to put it on. This variable can be removed
     * when that is no longer necessary.
     */
    int last_cursor_row;

    struct {
        int r;                  /* Current line (row) number */
        int c;                  /* Current column number */
        int pos;                /* Cursor position in last line */
    } current;
    WINDOW *win;                /* The scoller's own window */
};

/* --------- */
/* Functions */
/* --------- */

/* scr_new: Creates and initializes a new scroller
 * --------
 *
 *   pos_r:   Position on screen -- row
 *   pos_c:   Position on screen -- column
 *   height:  Height of the scroller on the screen (rows)
 *   width:   Width of the scroller on the screen (columns)
 *
 * Return Value: A pointer to a new scroller, or NULL on error.
 */
struct scroller *scr_new(int pos_r, int pos_c, int height, int width);

/* scr_free: Releases the memory allocated by a scroller
 * ---------
 *
 *   scr:  Pointer to the scroller object
 */
void scr_free(struct scroller *scr);

/* scr_up: Move up a number of lines
 * -------
 *
 *   scr:    Pointer to the scroller object
 *   nlines: Number of lines to scroll back; will not scroll past beginning
 */
void scr_up(struct scroller *scr, int nlines);

/* scr_down: Move down a number of lines
 * ---------
 *
 *   scr:    Pointer to the scroller object
 *   nlines: Number of lines to scroll down; will not scroll past end
 */
void scr_down(struct scroller *scr, int nlines);

/* scr_home: Jump to the top line of the buffer
 * ---------
 *
 *   scr:  Pointer to the scroller object
 */
void scr_home(struct scroller *scr);

/* scr_end: Jump to the bottom line of the buffer
 * --------
 *
 *   scr:  Pointer to the scroller object
 */
void scr_end(struct scroller *scr);

/* scr_add:  Append a string to the buffer.
 * --------
 *
 *   scr:  Pointer to the scroller object
 *   buf:  Buffer to append -- \b characters will be treated as backspace!
 */
void scr_add(struct scroller *scr, const char *buf, int tty);

/* scr_move: Reposition the buffer on the screen
 * ---------
 *
 *   scr:     Pointer to the scroller object
 *   pos_r:   Position on screen -- row
 *   pos_c:   Position on screen -- column
 *   height:  Height of the scroller on the screen (rows)
 *   width:   Width of the scroller on the screen (columns)
 */
void scr_move(struct scroller *scr,
        int pos_r, int pos_c, int height, int width);

/**
 * Clear the scroller.
 *
 * When the scroller is cleared, only the prompt will be displayed
 * at the top of the gdb scroller window.
 *
 * @param scr
 * The scroller to clear.
 */
void scr_clear(struct scroller *scr);

/* scr_refresh: Refreshes the scroller on the screen, in case the caller
 * ------------ damages the screen area where the scroller is written (or,
 *              perhaps the terminal size has changed, and you wish to redraw).
 *
 *   scr:    Pointer to the scroller object
 *   focus:  If the window has focus
 */
void scr_refresh(struct scroller *scr, int focus, enum win_refresh dorefresh);

#endif
