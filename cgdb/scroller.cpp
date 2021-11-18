// Some help understanding how searching in the scroller works
//
// - Vterm deals only with what's on the screen
//   It represents rows 0 through vterm height-1, which is 2 below
// - vterminal introduces a scrollback buffer
//   It represents rows -1 through -scrollback height, which is -6 below
// - vterminal also introduces a scrollback delta
//   Allows iterating from 0:height-1 but displaying the scrolled to text
//   The default is 0, which is represented by d0
//   Scrolling back all the way to -6 is represented by d-6
//   Scrolling back partiall to -2 is represented by d-2
// - The scroller has introduced the concept of a search id (sid)
//   The purpose is to iterate easily over all the text (vterm+scrollback)
//
// Example inputs and labels
//   Screen Height: 3
//   Scrollback (sb) size: 6
//   vid: VTerm ID (screen only)
//   tid: Terminal ID (screen + scrollback + scrollback delta)
//   sid: A search ID (for iterating eaisly over all)
//   sb start - scrollback buffer start
//   sb end - scrollback buffer end
//   vt start - vterm buffer start
//   vt end - vterm buffer end
// 
//          sid vid tid d0 d-6 d-2
// sb start  0      -6       0      abc     0
//           1      -5       1         def  1
//           2      -4       2      ghi     2
//           3      -3                 def   
//           4      -2           0  jkl
// sb end    5      -1           1     def
// vt start  6   0  0    0       2  mno
//           7   1  1    1             def
// vt end    8   2  2    2          pqr
//
// Your search will start at the row the scroll cursor is at.
//
// You can loop from 0 to scrollback size + vterm size.
//
// You can convert your cursor position to the sid by doing:
//   sid = cursor_pos + scrollback size + vterminal_scroll_get_delta
// You can convert your sid to a cursor position by doing the following:
//   cursor_pos = sid - scrollback size - vterminal_scroll_get_delta
//
// If your delta is -6, and your cursor is on sid 1, and you find a
// match on sid 7, you'll have to move the display by moving the delta.
// You can move the display to sid by doing the following:
//   delta_offset = sid - scrollback size
// Then, if delta_offset > 0, delta_offset = 0.

/* Local Includes */
#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

/* System Includes */
#if HAVE_CTYPE_H
#include <ctype.h>
#endif

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif /* HAVE_STDLIB_H */

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#include <algorithm>

/* Local Includes */
#include "sys_util.h"
#include "stretchy.h"
#include "sys_win.h"
#include "cgdb.h"
#include "cgdbrc.h"
#include "highlight_groups.h"
#include "scroller.h"
#include "highlight.h"
#include "vterminal.h"

struct scroller {
    // The virtual terminal
    VTerminal *vt;

    // All text sent to the scroller to date.
    // Vterm does not yet support reflow, so when the terminal is resized,
    // or when the cgdb window orientation is changed, vterm can't update
    // the text that well in the scroller. Currently, to work around that,
    // CGDB creates a new vterm on resize and feeds it all the text found
    // to date. When vterm supports reflow, this could go away.
    std::string text;

    // The window the scroller will be displayed on
    //
    // NULL when the height of the scroller is zero
    // This occurs when the terminal has a height of 1 or if the user
    // minimized the height of the scroller manually to zero
    SWINDOW *win;

    // True if in scroll mode, otherwise false
    bool in_scroll_mode;
    // The position of the cursor when in scroll mode
    int scroll_cursor_row, scroll_cursor_col;

    // True if in search mode, otherwise false
    // Can only search when in_scroll_mode is true
    bool in_search_mode;
    // The original delta, cursor row and col. Also the initial search id.
    int delta_init, search_row_init, search_col_init, search_sid_init;
    // True when searching forward, otherwise searching backwards
    bool forward;
    // True when searching case insensitve, false otherwise
    bool icase;

    // The current regex if in_search_mode is true
    struct hl_regex_info *hlregex;
    // The current row, col start and end matching position
    int search_row, search_col_start, search_col_end;
    // The last string regex to be searched for
    std::string last_regex;
};


/* ----------------- */
/* Exposed Functions */
/* ----------------- */

static void scr_ring_bell(void *data)
{
    struct scroller *scr = (struct scroller *)data;

    // TODO: Ring the bell
}

// Create a new VTerminal instance
//
// Please note that when the height or width of the scroller is zero,
// than the window (scr->win) will be NULL, as noted in the fields comment.
//
// In this scenario, we allow the height/width of the virtual terminal
// to remain as 1. The virtual terminal requires this. This provides a
// benefit that the user can continue typing into the virtual terminal
// even when it's not visible.
//
// @param scr
// The scroller to operate on
//
// @return
// The new virtual terminal instance
static VTerminal *scr_new_vterminal(struct scroller *scr)
{
    int scrollback_buffer_size = cgdbrc_get_int(CGDBRC_SCROLLBACK_BUFFER_SIZE);

    VTerminalOptions options;
    options.data = (void*)scr;
    // See note in function comments about std::max usage here
    options.width = std::max(swin_getmaxx(scr->win), 1);
    options.height = std::max(swin_getmaxy(scr->win), 1);
    options.scrollback_buffer_size = scrollback_buffer_size;
    options.ring_bell = scr_ring_bell;

    return vterminal_new(options);
}

struct scroller *scr_new(SWINDOW *win)
{
    struct scroller *rv = new scroller();

    rv->in_scroll_mode = false;
    rv->scroll_cursor_row = rv->scroll_cursor_col = 0;
    rv->win = win;

    rv->in_search_mode = false;
    rv->hlregex = NULL;
    rv->search_row = rv->search_col_start = rv->search_col_end = 0;

    rv->vt = scr_new_vterminal(rv);

    return rv;
}

void scr_free(struct scroller *scr)
{
    vterminal_free(scr->vt);

    hl_regex_free(&scr->hlregex);
    scr->hlregex = NULL;

    swin_delwin(scr->win);
    scr->win = NULL;

    /* Release the scroller object */
    delete scr;
}

void scr_set_scroll_mode(struct scroller *scr, bool mode)
{
    // If the request is to enable the scroll mode and it's not already 
    // enabled, then enable it
    if (mode && !scr->in_scroll_mode) {
        scr->in_scroll_mode = true;
        // Start the scroll mode cursor at the same location as the 
        // cursor on the screen
        vterminal_get_cursor_pos(
                scr->vt, scr->scroll_cursor_row, scr->scroll_cursor_col);
    // If the request is to disable the scroll mode and it's currently
    // enabled, then disable it
    } else if (!mode && scr->in_scroll_mode) {
        scr->in_scroll_mode = false;
    }
}

bool scr_scroll_mode(struct scroller *scr)
{
    return scr->in_scroll_mode;
}

void scr_up(struct scroller *scr, int nlines)
{
    // When moving 1 line up
    //   Move the cursor towards the top of the screen
    //   If it hits the top, then start scrolling back
    // Otherwise whem moving many lines up, simply scroll
    if (scr->scroll_cursor_row > 0 && nlines == 1) {
        scr->scroll_cursor_row = scr->scroll_cursor_row - 1;
    } else {
        vterminal_scroll_delta(scr->vt, nlines);
    }
}

void scr_down(struct scroller *scr, int nlines)
{
    int height;
    int width;
    vterminal_get_height_width(scr->vt, height, width);

    // When moving 1 line down
    //   Move the cursor towards the botttom of the screen
    //   If it hits the botttom, then start scrolling forward
    // Otherwise whem moving many lines down, simply scroll
    if (scr->scroll_cursor_row < height - 1 && nlines == 1) {
        scr->scroll_cursor_row = scr->scroll_cursor_row + 1;
    } else {
        vterminal_scroll_delta(scr->vt, -nlines);
    }
}

void scr_home(struct scroller *scr)
{
    int sb_num_rows;
    vterminal_scrollback_num_rows(scr->vt, sb_num_rows);
    vterminal_scroll_delta(scr->vt, sb_num_rows);
}

void scr_end(struct scroller *scr)
{
    int sb_num_rows;
    vterminal_scrollback_num_rows(scr->vt, sb_num_rows);
    vterminal_scroll_delta(scr->vt, -sb_num_rows);
}

void scr_left(struct scroller *scr)
{
    if (scr->scroll_cursor_col > 0) {
        scr->scroll_cursor_col--;
    }
}

void scr_right(struct scroller *scr)
{
    int height;
    int width;
    vterminal_get_height_width(scr->vt, height, width);

    if (scr->scroll_cursor_col < width - 1) {
        scr->scroll_cursor_col++;
    }
}

void scr_beginning_of_row(struct scroller *scr)
{
    scr->scroll_cursor_col = 0;
}

void scr_end_of_row(struct scroller *scr)
{
    int height;
    int width;
    vterminal_get_height_width(scr->vt, height, width);

    scr->scroll_cursor_col = width - 1;
}

void scr_push_screen_to_scrollback(struct scroller *scr)
{
    vterminal_push_screen_to_scrollback(scr->vt);
}

void scr_add(struct scroller *scr, const char *buf)
{
    // Keep a copy of all text sent to vterm
    // Vterm doesn't yet support resizing, so we would create a new vterm
    // instance and feed it the same data
    scr->text.append(buf);

    vterminal_write(scr->vt, buf, strlen(buf));
}

void scr_move(struct scroller *scr, SWINDOW *win)
{
    swin_delwin(scr->win);
    scr->win = win;

    // recreate the vterm session with the new size
    vterminal_free(scr->vt);

    scr->vt = scr_new_vterminal(scr);

    vterminal_write(scr->vt, scr->text.data(), scr->text.size());
}

void scr_enable_search(struct scroller *scr, bool forward, bool icase)
{
    if (scr->in_scroll_mode) {
        int delta;
        vterminal_scroll_get_delta(scr->vt, delta);

        int sb_num_rows;
        vterminal_scrollback_num_rows(scr->vt, sb_num_rows);

        scr->in_search_mode = true;
        scr->forward = forward;
        scr->icase = icase;
        scr->delta_init = delta;
        scr->search_sid_init = scr->scroll_cursor_row - delta + sb_num_rows;
        scr->search_row_init = scr->scroll_cursor_row;
        scr->search_col_init = scr->scroll_cursor_col;
    }
}

void scr_disable_search(struct scroller *scr, bool accept)
{
    if (scr->in_search_mode) {
        scr->in_search_mode = false;

        if (accept) {
            scr->scroll_cursor_row = scr->search_row;
            scr->scroll_cursor_col = scr->search_col_start;

            hl_regex_free(&scr->hlregex);
            scr->hlregex = 0;
        } else {
            scr->scroll_cursor_row = scr->search_row_init;
            scr->scroll_cursor_col = scr->search_col_init;
            vterminal_scroll_set_delta(scr->vt, scr->delta_init);
            scr->last_regex.clear();
        }

        scr->search_row = 0;
        scr->search_col_start = 0;
        scr->search_col_end = 0;
    }
}

bool scr_search_mode(struct scroller *scr)
{
    return scr->in_search_mode;
}

static int scr_search_regex_forward(struct scroller *scr, const char *regex)
{
    int sb_num_rows;
    vterminal_scrollback_num_rows(scr->vt, sb_num_rows);

    int height;
    int width;
    vterminal_get_height_width(scr->vt, height, width);

    int delta;
    vterminal_scroll_get_delta(scr->vt, delta);

    int wrapscan_enabled = cgdbrc_get_int(CGDBRC_WRAPSCAN);

    int count = sb_num_rows + height;
    int regex_matched = 0;

    if (!scr || !regex) {
        // TODO: LOG ERROR
        return -1;
    }

    scr->last_regex = regex;

    // The starting search row and column
    int search_row = scr->search_sid_init;
    int search_col = scr->search_col_init;

    // Increment the column by 1 to get the starting row/column
    if (search_col < width - 1) {
        search_col++;
    } else {
        search_row++;
        if (search_row >= count) {
            search_row = 0;
        }
        search_col = 0;
    }

    for (;;)
    {
        int start, end;
        // convert from sid to cursor position taking into account delta
        int vfr = search_row - sb_num_rows + delta;
        std::string utf8buf;
        vterminal_fetch_row(scr->vt, vfr, search_col, width, utf8buf);
        regex_matched = hl_regex_search(&scr->hlregex, utf8buf.c_str(),
                regex, scr->icase, &start, &end);
        if (regex_matched > 0) {
            // Need to scroll the terminal if the search is not in view
            if (count - delta - height <= search_row &&
                search_row < count - delta) {
            } else {
                delta = search_row - sb_num_rows;
                if (delta > 0) {
                    delta = 0;
                }
                delta = -delta;
                vterminal_scroll_set_delta(scr->vt, delta);
            }

            // convert from sid to cursor position taking into account delta
            scr->search_row = search_row - sb_num_rows + delta;
            scr->search_col_start = start + search_col;
            scr->search_col_end = end + search_col;
            break;
        }

        // Stop searching when made it back to original position
        if (wrapscan_enabled &&
            search_row == scr->search_sid_init && search_col == 0) {
            break;
        // Or if wrapscan is disabled and searching hit the end
        } else if (!wrapscan_enabled && search_row == count - 1) {
            break;
        }

        search_row++;
        if (search_row >= count) {
            search_row = 0;
        }
        search_col = 0;
    }

    return regex_matched;
}

static int scr_search_regex_backwards(struct scroller *scr, const char *regex)
{
    int sb_num_rows;
    vterminal_scrollback_num_rows(scr->vt, sb_num_rows);

    int height;
    int width;
    vterminal_get_height_width(scr->vt, height, width);

    int delta;
    vterminal_scroll_get_delta(scr->vt, delta);

    int wrapscan_enabled = cgdbrc_get_int(CGDBRC_WRAPSCAN);

    int count = sb_num_rows + height;
    int regex_matched = 0;

    if (!scr || !regex) {
        // TODO: LOG ERROR
        return -1;
    }

    scr->last_regex = regex;

    // The starting search row and column
    int search_row = scr->search_sid_init;
    int search_col = scr->search_col_init;

    // Decrement the column by 1 to get the starting row/column
    if (search_col > 0) {
        search_col--;
    } else {
        search_row--;
        if (search_row < 0) {
            search_row = count - 1;
        }
        search_col = width - 1;
    }

    for (;;)
    {
        int start = 0, end = 0;
        int vfr = search_row - sb_num_rows + delta;

        // Searching in reverse is more difficult
        // The idea is to search right to left, however the regex api
        // doesn't support that. Need to mimic this by searching left
        // to right to find all the matches on the line, and then 
        // take the right most match.
        for (int c = 0;;) {
            std::string utf8buf;
            vterminal_fetch_row(scr->vt, vfr, c, width, utf8buf);

            int _start, _end, result;
            result = hl_regex_search(&scr->hlregex, utf8buf.c_str(),
                    regex, scr->icase, &_start, &_end);
            if ((result == 1) && (c + _start <= search_col)) {
                regex_matched = 1;
                start = c + _start;
                end = c + _end;
                c = start + 1;
            } else {
                break;
            }
        }

        if (regex_matched > 0) {
            // Need to scroll the terminal if the search is not in view
            if (count - delta - height <= search_row &&
                search_row < count - delta) {
            } else {
                delta = search_row - sb_num_rows;
                if (delta > 0) {
                    delta = 0;
                }
                delta = -delta;
                vterminal_scroll_set_delta(scr->vt, delta);
            }

            scr->search_row = search_row - sb_num_rows + delta;
            scr->search_col_start = start;
            scr->search_col_end = end;
            break;
        }

        // Stop searching when made it back to original position
        if (wrapscan_enabled &&
            search_row == scr->search_sid_init &&
            search_col == width - 1) {
            break;
        // Or if wrapscan is disabled and searching hit the top
        } else if (!wrapscan_enabled && search_row == 0) {
            break;
        }

        search_row--;
        if (search_row < 0) {
            search_row = count - 1;
        }
        search_col = width - 1;
    }

    return regex_matched;
}

int scr_search_regex(struct scroller *scr, const char *regex)
{
    int result;

    if (scr->forward) {
        result = scr_search_regex_forward(scr, regex);
    } else {
        result = scr_search_regex_backwards(scr, regex);
    }

    return result;
}

void scr_search_next(struct scroller *scr, bool forward, bool icase)
{
    if (scr->last_regex.size() > 0) {
        scr_enable_search(scr, forward, icase);
        scr_search_regex(scr, scr->last_regex.c_str());
        scr_disable_search(scr, true);
    }
}

void scr_refresh(struct scroller *scr, int focus, enum win_refresh dorefresh)
{
    int height;
    int width;
    vterminal_get_height_width(scr->vt, height, width);

    int vterm_cursor_row, vterm_cursor_col;
    vterminal_get_cursor_pos(scr->vt, vterm_cursor_row, vterm_cursor_col);

    int sb_num_rows;
    vterminal_scrollback_num_rows(scr->vt, sb_num_rows);

    int delta;
    vterminal_scroll_get_delta(scr->vt, delta);

    int highlight_attr, search_attr;

    int cursor_row, cursor_col;

    if (scr->in_scroll_mode) {
        cursor_row = scr->scroll_cursor_row;
        cursor_col = scr->scroll_cursor_col;
    } else {
        cursor_row = vterm_cursor_row;
        cursor_col = vterm_cursor_col;
    }

    /* Steal line highlight attribute for our scroll mode status */
    highlight_attr = hl_groups_get_attr(hl_groups_instance,
        HLG_SCROLL_MODE_STATUS);

    search_attr = hl_groups_get_attr(hl_groups_instance, HLG_INCSEARCH);

    for (int r = 0; r < height; ++r) {
        for (int c = 0; c < width; ) {
            std::string utf8buf;
            int attr = 0;
            int cellwidth;
            int in_search = scr->in_search_mode && scr->search_row == r &&
                    c >= scr->search_col_start && c < scr->search_col_end;

            vterminal_fetch_row_col(scr->vt, r, c, utf8buf, attr, cellwidth);
            swin_wmove(scr->win, r,  c);
            swin_wattron(scr->win, attr);
            if (in_search)
                swin_wattron(scr->win, search_attr);

            // print the cell utf8 data or an empty char
            // If nothing is written at all, then the cell will not be colored
            if (utf8buf.size()) {
                swin_waddnstr(scr->win, utf8buf.data(), utf8buf.size());
            } else {
                swin_waddnstr(scr->win, " ", 1);
            }

            if (in_search)
                swin_wattroff(scr->win, search_attr);
            swin_wattroff(scr->win, attr);
            swin_wclrtoeol(scr->win);
            c += cellwidth;
        }

        // If in scroll mode, overlay the percent the scroller is scrolled
        // back on the top right of the scroller display.
        if (scr->in_scroll_mode && r == 0) {
            char status[ 64 ];
            size_t status_len;

            snprintf(status, sizeof(status), "[%d/%d]", delta, sb_num_rows);

            status_len = strlen(status);
            if ( status_len < width ) {
                swin_wattron(scr->win, highlight_attr);
                swin_mvwprintw(scr->win, r, width - status_len, "%s", status);
                swin_wattroff(scr->win, highlight_attr);
            }
        }
    }

    // Show the cursor when the scroller is in focus
    if (focus) {
        swin_wmove(scr->win, cursor_row, cursor_col);
        swin_curs_set(1);
    } else {
        /* Hide the cursor */
        swin_curs_set(0);
    }

    switch(dorefresh) {
        case WIN_NO_REFRESH:
            swin_wnoutrefresh(scr->win);
            break;
        case WIN_REFRESH:
            swin_wrefresh(scr->win);
            break;
    }
}
