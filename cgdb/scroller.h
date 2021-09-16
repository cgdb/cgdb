// A GDB scroller

#ifndef _SCROLLER_H_
#define _SCROLLER_H_

#include "sys_win.h"

struct scroller;

// Creates and initializes a new scroller
//
// @return
// A pointer to a new scroller.
struct scroller *scr_new(SWINDOW *window);

// Releases the memory allocated by a scroller
//
// @param scr
// The scroller to operate on
void scr_free(struct scroller *scr);

// Enable or disable the scroll mode
//
// @param scr
// The scroller to operate on
// 
// @param mode
// When true, scroll mode is enabled. When false, scroll mode is disabled
void scr_set_scroll_mode(struct scroller *scr, bool mode);

// Determine the scroll mode
//
// @param scr
// The scroller to operate on
//
// @return
// True in scroll mode, otherwise false
bool scr_scroll_mode(struct scroller *scr);

// Move up a number of lines in scroll mode. Scroll mode must be enabled.
//
// @param scr
// The scroller to operate on
//
// @nlines
// Number of lines to scroll back; will not scroll past beginning
void scr_up(struct scroller *scr, int nlines);

// Move down a number of lines in scroll mode. Scroll mode must be enabled.
//
// @param scr
// The scroller to operate on
//
// @nlines
// Number of lines to scroll down; will not scroll past end
void scr_down(struct scroller *scr, int nlines);

// Jump to the beginning (the top) of the scrollback buffer in scroll mode
// Scroll mode must be enabled.
//
// @param scr
// The scroller to operate on
void scr_home(struct scroller *scr);

// Jump to the end (the bottom) of the scrollback buffer in scroll mode
// Scroll mode must be enabled.
//
// @param scr
// The scroller to operate on
void scr_end(struct scroller *scr);

// Move the cursor left a position in scroll mode. Scroll mode must be enabled.
//
// @param scr
// The scroller to operate on
void scr_left(struct scroller *scr);

// Move the cursor right a position in scroll mode. Scroll mode must be enabled.
//
// @param scr
// The scroller to operate on
void scr_right(struct scroller *scr);

// Move the cursor to the beginning of the row in scroll mode
// Scroll mode must be enabled
//
// @param scr
// The scroller to operate on
void scr_beginning_of_row(struct scroller *scr);

// Move the cursor to the end of the row in scroll mode
// Scroll mode must be enabled
//
// @param scr
// The scroller to operate on
void scr_end_of_row(struct scroller *scr);

// Enable the search mode. Scroll mode must be enabled.
//
// The current cursor location and delta within the scrollback mode are saved.
// Once enabled, you can incrementally search by calling scr_search_regex
// repeatedly, and then cancel/accept the search via scr_disable_search.
// You cancel cancel the search without calling scr_search_regex as well.
//
// @param scr
// The scroller to operate on
//
// @param forward
// If true then forward, else in reverse
//
// @param icase
// If true then ignore case, else consider case
void scr_enable_search(struct scroller *scr, bool forward, bool icase);

// Disable the search mode. Search mode must be enabled.
//
// @param scr
// The scroller to operate on
//
// @param accept
// True to accept the search position/cursor found by scr_search_regex
// False will reset to the original position/cursor position
void scr_disable_search(struct scroller *scr, bool accept);

// Determine the search mode
//
// @param scr
// The scroller to operate on
//
// @return
// True in search mode, otherwise false
bool scr_search_mode(struct scroller *scr);

// Searches for regex in current scroller and displays line.
// Search mode must be enabled.
//
// @param scr
// Pointer to the scroller object
//
// @param regex
// The regular expression to search for. If NULL, then no regex will be tried.
//
// @return
// Zero on success with no match, 1 on success with match or -1 on error
int scr_search_regex(struct scroller *scr, const char *regex);

// Repeat the last search made. Search mode must be enabled.
//
// This requires at least one call to scr_disable_search with accept=true.
// If this hasn't occured yet, than this will be a no-op.
//
// @param scr
// Pointer to the scroller object
//
// @param forward
// If true then forward, else in reverse
//
// @param icase
// If true then ignore case, else consider case
void scr_search_next(struct scroller *scr, bool forward, bool icase);

// Push the contents of the screen into the scrollback buffer
// This is used when the user types control-l
//
// @param scr
// Pointer to the scroller object
void scr_push_screen_to_scrollback(struct scroller *scr);

// Add output from gdb to the scroller buffer
// 
// @param scr
// The scroller to operate on
//
// @param buf
// The buffer to append
void scr_add(struct scroller *scr, const char *buf);

// Give the scroller a new window to display itself in
//
// @param scr
// The scroller to operate on
//
// @param win
// The window to place the scroller into
void scr_move(struct scroller *scr, SWINDOW *win);

// Refreshes the scroller on the screen
//
// @param scr
// The scroller to operate on
//
// @param focus
// True if the scroller has focus, false otherwise
//
// @param dorefresh
// Controls how the scroller should update the screen
void scr_refresh(struct scroller *scr, int focus, enum win_refresh dorefresh);


#endif
