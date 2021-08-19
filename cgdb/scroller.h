/* scroller.h:
 * -----------
 *
 * A scrolling buffer utility.  Able to add and subtract to the buffer.
 * All routines that would require a screen update will automatically redraw
 * the scroller.  There is no "draw" function.
 */

#ifndef _SCROLLER_H_
#define _SCROLLER_H_

#include "sys_win.h"

/* Count of marks */
#define MARK_COUNT 26

/* --------------- */
/* Data Structures */
/* --------------- */

struct VTerminal;

struct scroller_mark
{
    int r;
    int c;
};

struct scroller {
    // True if in scroll mode, otherwise false
    bool in_scroll_mode;
    // The position of the cursor when in scroll mode
    int scroll_cursor_row, scroll_cursor_col;

    // True if in search mode, otherwise false
    // Can only search when in_scroll_mode is true
    bool in_search_mode;
    /** The last regex searched for */
    struct hl_regex_info *last_hlregex;
    /** The current regex if in_search_mode is true */
    struct hl_regex_info *hlregex;
    // The original delta when starting a search, will move back to this on fail
    int delta_init;
    int search_sid_init, search_col_init;
    /** The original row, or last selected row, when searching.  */
    int search_row, search_col_start, search_col_end;

    SWINDOW *win; /* The scoller's own window */

    scroller_mark marks[MARK_COUNT]; /* Local a-z marks */
    scroller_mark jump_back_mark;    /* Location where last jump occurred from */

    // the virtual terminal
    VTerminal *vt;

    // All text sent to the virtual terminal so far
    std::string text;
};

/* --------- */
/* Functions */
/* --------- */

/* scr_new: Creates and initializes a new scroller
 * --------
 *
 * Return Value: A pointer to a new scroller.
 */
struct scroller *scr_new(SWINDOW *window);

/* scr_free: Releases the memory allocated by a scroller
 * ---------
 *
 *   scr:  Pointer to the scroller object
 */
void scr_free(struct scroller *scr);

// Enable or disable the scroll mode
//
// @param scr
// The scroller to operate on
// 
// @param mode
// When true, scroll mode is enabled. When false, scroll mode is disabled
void scr_set_scroll_mode(struct scroller *scr, bool mode);

// Move up a number of lines in scroll mode
// Will enable scroll mode if not yet enabled
//
// @param scr
// The scroller to operate on
//
// @nlines
// Number of lines to scroll back; will not scroll past beginning
void scr_up(struct scroller *scr, int nlines);

// Move down a number of lines in scroll mode
// Will enable scroll mode if not yet enabled
//
// @param scr
// The scroller to operate on
//
// @nlines
// Number of lines to scroll down; will not scroll past end
void scr_down(struct scroller *scr, int nlines);

// Jump to the beginning (the top) of the scrollback buffer in scroll mode
// Will enable scroll mode if not yet enabled
//
// @param scr
// The scroller to operate on
void scr_home(struct scroller *scr);

// Jump to the end (the bottom) of the scrollback buffer in scroll mode
// Will enable scroll mode if not yet enabled
//
// @param scr
// The scroller to operate on
void scr_end(struct scroller *scr);

// Move the cursor left a position in scroll mode
//
// @param scr
// The scroller to operate on
void scr_left(struct scroller *scr);

// Move the cursor right a position in scroll mode
//
// @param scr
// The scroller to operate on
void scr_right(struct scroller *scr);

// Move the cursor to the beginning of the row in scroll mode
//
// @param scr
// The scroller to operate on
void scr_beginning_of_row(struct scroller *scr);

// Move the cursor to the end of the row in scroll mode
//
// @param scr
// The scroller to operate on
void scr_end_of_row(struct scroller *scr);

void scr_push_screen_to_scrollback(struct scroller *scr);

/* scr_add:  Append a string to the buffer.
 * --------
 *
 *   scr:  Pointer to the scroller object
 *   buf:  Buffer to append -- \b characters will be treated as backspace!
 */
void scr_add(struct scroller *scr, const char *buf);

/* Reposition the buffer on the screen
 *
 * @param scr
 * Pointer to the scroller object
 *
 * @param win
 * The new window
 */
void scr_move(struct scroller *scr, SWINDOW *win);

/* scr_refresh: Refreshes the scroller on the screen, in case the caller
 * ------------ damages the screen area where the scroller is written (or,
 *              perhaps the terminal size has changed, and you wish to redraw).
 *
 *   scr:    Pointer to the scroller object
 *   focus:  If the window has focus
 */
void scr_refresh(struct scroller *scr, int focus, enum win_refresh dorefresh);

/**
 * Should be called before scr_search_regex
 *
 * This function initializes scr before it can search for a regex
 *
 * @param scr
 * Pointer to the scroller object
 */
void scr_search_regex_init(struct scroller *scr);
void scr_search_regex_final(struct scroller *scr);

/**
 * Searches for regex in current scroller and displays line.
 *
 * @param scr
 * Pointer to the scroller object
 *
 * @param regex
 * The regular expression to search for. If NULL, then no regex will be tried.
 *
 * @param opt
 * If 1, Then the search is temporary ( User has not hit enter )
 * If 2, The search is perminant
 *
 * @param direction
 * If 0 then forward, else reverse
 *
 * @param icase
 * If 0 ignore case.
 *
 * @return
 * Zero on match, 
 * -1 if sview->cur is NULL
 * -2 if regex is NULL
 * -3 if regcomp fails
 * non-zero on failure.
 */
int scr_search_regex(struct scroller *scr, const char *regex, int opt,
    int direction, int icase);

int scr_set_mark(struct scroller *scr, int key);
int scr_goto_mark(struct scroller *scr, int key);

#endif
