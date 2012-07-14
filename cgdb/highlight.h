#ifndef _HIGHLIGHT_H_
#define _HIGHLIGHT_H_

/* highlight.h:
 * ------------
 * 
 * Syntax highlighting routines.
 *
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_CURSES_H
#include <curses.h>
#elif HAVE_NCURSES_CURSES_H
#include <ncurses/curses.h>
#endif /* HAVE_CURSES_H */

/* Local Includes */
#include "sources.h"

/* --------- */
/* Functions */
/* --------- */

/* highlight:  Inserts the highlighting tags into the buffer.  Lines in
 * ----------  this file should be displayed with hl_wprintw from now on...
 *
 *   node:  The node containing the file buffer to highlight.
 */
void highlight(struct list_node *node);

/* hl_wprintw:  Prints a given line using the embedded highlighting commands
 * -----------  to dictate how to color the given line.
 *
 *   win:     The ncurses window to which the line will be written
 *   line:    The line to print
 *   width:   The maximum width of a line
 *   offset:  Character (in line) to start at (0..length-1)
 */
void hl_wprintw(WINDOW * win, const char *line, int width, int offset);

/* hl_regex: Matches a regular expression to some lines.
 * ---------
 *
 *  regex:          The regular expression to match.
 *  tlines:         The lines of text to search.
 *  length:         The number of lines.
 *  cur_line:       This line is returned with highlighting embedded into it.
 *  sel_line:       The current line the user is on.
 *  sel_rline:      The current line the regular expression is on.
 *  sel_col_rbeg:   The beggining index of the last match.
 *  sel_col_rend:   The ending index of the last match.
 *  opt:            1 -> incremental match, 2 -> perminant match
 *  direction:      1 if forward, 0 if reverse
 *  icase:          1 if case insensitive, 0 otherwise
 */
int hl_regex(const char *regex, const char **highlighted_lines, const char **tlines, const int length, char **cur_line, /* Returns the correct highlighted line */
        int *sel_line,          /* Returns new cur line if regex matches */
        int *sel_rline,         /* Used for internal purposes */
        int *sel_col_rbeg,
        int *sel_col_rend, int opt, int direction, int icase);

#endif /* _HIGHLIGHT_H_ */
