/* scroller.c:
 * -----------
 *
 * A scrolling buffer utility.  Able to add and subtract to the buffer.
 * All routines that would require a screen update will automatically refresh
 * the scroller.
 */

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

/* Local Includes */
#include "sys_util.h"
#include "stretchy.h"
#include "sys_win.h"
#include "cgdb.h"
#include "cgdbrc.h"
#include "highlight_groups.h"
#include "scroller.h"
#include "highlight.h"

/* --------------- */
/* Local Functions */
/* --------------- */

/* count: Count the occurrences of a character c in a string s.
 * ------
 *
 *   s:  String to search
 *   c:  Character to search for
 *
 * Return Value:  Number of occurrences of c in s.
 */
static int count(const char *s, int slen, char c)
{
    int i;
    int rv = 0;

    for (i = 0; i < slen; i++) {
        rv += (s[i] == c);
    }

    return rv;
}

/* parse: Translates special characters in a string.  (i.e. backspace, tab...)
 * ------
 *
 *   buf:  The string to parse
 *
 * Return Value:  A newly allocated copy of buf, with modifications made.
 */
static char *parse(struct scroller *scr, struct hl_line_attr **attrs,
    const char *orig, const char *buf, int buflen)
{
    // Read in tabstop settings, but don't change them on the fly as we'd have to
    //  store each previous line and recalculate every one of them.
    static const int tab_size = cgdbrc_get_int(CGDBRC_TABSTOP);
    int tabcount = count(buf, buflen, '\t');
    int orig_len = strlen(orig);
    int length = MAX(orig_len, scr->current.pos) + buflen +
        (tab_size - 1) * tabcount + 1;
    char *rv = (char *) cgdb_calloc(length, 1);
    int i, j;
    int debugwincolor = cgdbrc_get_int(CGDBRC_DEBUGWINCOLOR);
    int width, height;

    height = swin_getmaxy(scr->win);
    width = swin_getmaxx(scr->win);

    /* Copy over original buffer */
    strcpy(rv, orig);

    i = scr->current.pos;

    /* Expand special characters */
    for (j = 0; j < buflen; j++) {
        switch (buf[j]) {
                /* readline wants to ring the bell, ignore */
            case 7:
                break;
                /* Backspace/Delete -> Erase last character */
            case 8:
            case 127:
                if (i > 0)
                    i--;
                break;
                /* Tab -> Translating to spaces */
            case '\t':
                do {
                    rv[i++] = ' ';
                } while (i % tab_size != 0);
                break;
                /* Carriage return -> Move back to the beginning of the line */
            case '\r':
                i = 0;
                if ((buflen - j) >= 1 && buf[j + 1] != '\n') {
                    sbfree(*attrs);
                    *attrs = NULL;
                }
                break;
            case '\033':
                /* Handle ansi escape characters */
                /* see http://ascii-table.com/ansi-escape-sequences-vt-100.php
                 */
                if ((buflen - j) >= 2 && buf[j + 1] == '[' && buf[j + 2] == '?') {
                    /* simply ignore these escape sequences like e.g. "[?1h" */
                    j += 4;
                } else if ((buflen - j) >= 1 && (buf[j + 1] == '=' || buf[j + 1] == '>')) {
                    /* simply ignore these escape sequences */
                    j += 1;
                } else if (hl_ansi_color_support(hl_groups_instance) &&
                    debugwincolor) {
                    int attr;
                    int ansi_count = hl_ansi_get_color_attrs(
                            hl_groups_instance, buf + j, &attr);
                    if (ansi_count) {
                        j += ansi_count - 1;
                        sbpush(*attrs, hl_line_attr(i, attr));
                    }
                } else {
                    rv[i++] = buf[j];
                }
                break;
            default:
                rv[i++] = buf[j];
                break;
        }
    }

    scr->current.pos = i;

    return rv;
}

static void scroller_addline(struct scroller *scr, char *line,
    struct hl_line_attr *attrs)
{
    struct scroller_line sl;

    sl.line = line;
    sl.line_len = strlen(line);
    sl.attrs = attrs;
    sbpush(scr->lines, sl);

    scr->lines_to_display++;
}

/* ----------------- */
/* Exposed Functions */
/* ----------------- */

/* See scroller.h for function descriptions. */

struct scroller *scr_new(SWINDOW *win)
{
    struct scroller *rv;

    rv = (struct scroller *)cgdb_malloc(sizeof(struct scroller));

    rv->current.r = 0;
    rv->current.c = 0;
    rv->current.pos = 0;
    rv->in_scroll_mode = 0;
    rv->lines_to_display = 0;
    rv->win = win;

    rv->in_search_mode = 0;
    rv->last_hlregex = NULL;
    rv->hlregex = NULL;
    rv->search_r = 0;

    /* Start with a single (blank) line */
    rv->lines = NULL;
    scroller_addline(rv, strdup(""), NULL);

    rv->jump_back_mark.r = -1;
    rv->jump_back_mark.c = -1;
    memset(rv->marks, 0xff, sizeof(rv->marks));

    return rv;
}

void scr_free(struct scroller *scr)
{
    int i;

    /* Release the buffer */
    for (i = 0; i < sbcount(scr->lines); i++) {
        free(scr->lines[i].line);
        sbfree(scr->lines[i].attrs);
    }
    sbfree(scr->lines);
    scr->lines = NULL;

    hl_regex_free(&scr->last_hlregex);
    scr->last_hlregex = NULL;
    hl_regex_free(&scr->hlregex);
    scr->hlregex = NULL;

    swin_delwin(scr->win);
    scr->win = NULL;

    /* Release the scroller object */
    free(scr);
}

static int get_last_col(struct scroller *scr, int row) {
    int width = swin_getmaxx(scr->win);
    return (MAX(scr->lines[row].line_len - 1, 0) / width) * width;
}

static void scr_scroll_lines(struct scroller *scr,
    int *r, int *c, int nlines)
{
    int i;
    int width = swin_getmaxx(scr->win);
    int row = *r;
    int col = (*c / width) * width;
    int amt = (nlines < 0) ? -width : width;

    if (nlines < 0)
        nlines = -nlines;

    for (i = 0; i < nlines; i++) {
        col += amt;
        if (col < 0) {
            if (row <= 0)
                break;
            row--;
            col = get_last_col(scr, row);
        } else if (col >= scr->lines[row].line_len)  {
            if (row >= sbcount(scr->lines) - 1)
                break;

            row++;
            col = 0;
        }
        *r = row;
        *c = col;
    }
}

void scr_up(struct scroller *scr, int nlines)
{
    scr->in_scroll_mode = 1;

    scr_scroll_lines(scr, &scr->current.r, &scr->current.c, -nlines);
}

void scr_down(struct scroller *scr, int nlines)
{
    scr_scroll_lines(scr, &scr->current.r, &scr->current.c, nlines);
}

void scr_home(struct scroller *scr)
{
    scr->current.r = 0;
    scr->current.c = 0;

    scr->in_scroll_mode = 1;
}

void scr_end(struct scroller *scr)
{
    scr->current.r = sbcount(scr->lines) - 1;
    scr->current.c = get_last_col(scr, scr->current.r);
}

static void scr_add_buf(struct scroller *scr, const char *buf)
{
    char *x;
    int distance;

    /* Find next newline in the string */
    x = strchr((char *)buf, '\n');
    distance = x ? x - buf : strlen(buf);

    /* Append to the last line in the buffer */
    if (distance > 0) {
        int is_crlf = (distance == 1) && (buf[0] == '\r');
        if (!is_crlf) {
            int index = sbcount(scr->lines) - 1;
            char *orig = scr->lines[index].line;
            int orig_len = scr->lines[index].line_len;

            scr->lines[index].line = parse(
                scr, &scr->lines[index].attrs, orig, buf, distance);
            scr->lines[index].line_len = strlen(scr->lines[index].line);

            free(orig);
        }
    }

    /* Create additional lines if buf contains newlines */
    while (x != NULL) {
        char *line;
        struct hl_line_attr *attrs = NULL;

        buf = x + 1;
        x = strchr((char *)buf, '\n');
        distance = x ? x - buf : strlen(buf);

        /* Expand the buffer */
        scr->current.pos = 0;

        /* Add the new line */
        line = parse(scr, &attrs, "", buf, distance);
        scroller_addline(scr, line, attrs);
    }

    /* Don't want to exit scroll mode simply because new data received */
    if (!scr->in_scroll_mode) {
        scr_end(scr);
    }
}

void scr_add(struct scroller *scr, const char *buf)
{
    char *tempbuf = NULL;

    scr_add_buf(scr, buf);

    scr_end(scr);

    free(tempbuf);
}

void scr_move(struct scroller *scr, SWINDOW *win)
{
    swin_delwin(scr->win);
    scr->win = win;
}

void scr_clear(struct scroller *scr) {
    scr->lines_to_display = 0;
}

static int wrap_line(struct scroller *scr, int line)
{
    int count = sbcount(scr->lines);

    if (line < 0)
        line = count - 1;
    else if (line >= count)
        line = 0;

    return line;
}

int scr_search_regex(struct scroller *scr, const char *regex, int opt,
    int direction, int icase)
{
    if (regex && *regex) {
        int line;
        int line_end;
        int line_inc = direction ? +1 : -1;
        int line_start = scr->search_r;

        line = wrap_line(scr, line_start + line_inc);

        if (cgdbrc_get_int(CGDBRC_WRAPSCAN))
        {
            // Wrapping is on so stop at the line we started on.
            line_end = line_start;
        }
        else
        {
            // No wrapping. Stop at line 0 if searching down and last line
            // if searching up.
            line_end = direction ? 0 : sbcount(scr->lines) - 1;
        }

        for (;;)
        {
            int ret;
            int start, end;
            char *line_str = scr->lines[line].line;

            ret = hl_regex_search(&scr->hlregex, line_str, regex, icase, &start, &end);
            if (ret > 0)
            {
                /* Got a match */
                scr->current.r = line;
                scr->current.c = get_last_col(scr, line);

                /* Finalized match - move to this location */
                if (opt == 2) {
                    scr->search_r = line;

                    hl_regex_free(&scr->hlregex);
                    scr->last_hlregex = scr->hlregex;
                    scr->hlregex = 0;
                }
                return 1;
            }

            line = wrap_line(scr, line + line_inc);
            if (line == line_end)
                break;
        }
    }

    /* Nothing found - go back to original line */
    scr->current.r = scr->search_r;
    scr->current.c = get_last_col(scr, scr->search_r);
    return 0;
}

void scr_search_regex_init(struct scroller *scr)
{
    scr->in_search_mode = 1;

    /* Start searching at the beginning of the selected line */
    scr->search_r = scr->current.r;
}

int scr_set_mark(struct scroller *scr, int key)
{
    if (key >= 'a' && key <= 'z')
    {
        /* Local buffer mark */
        scr->marks[key - 'a'].r = scr->current.r;
        scr->marks[key - 'a'].c = scr->current.c;
        return 1;
    }

    return 0;
}

int scr_goto_mark(struct scroller *scr, int key)
{
    scroller_mark mark_temp;
    scroller_mark *mark = NULL;

    if (key >= 'a' && key <= 'z')
    {
        /* Local buffer mark */
        mark = &scr->marks[key - 'a'];
    }
    else if (key == '\'')
    {
        /* Jump back to where we last jumped from */
        mark_temp = scr->jump_back_mark;
        mark = &mark_temp;
    }
    else if (key == '.')
    {
        /* Jump to last line */
        mark_temp.r = sbcount(scr->lines) - 1;
        mark_temp.c = get_last_col(scr, scr->current.r);
        mark = &mark_temp;
    }

    if (mark && (mark->r >= 0))
    {
        scr->jump_back_mark.r = scr->current.r;
        scr->jump_back_mark.c = scr->current.c;

        scr->current.r = mark->r;
        scr->current.c = mark->c;
        return 1;
    }

    return 0;
}

/**
 * Determine the proper number of scroller rows to display.
 *
 * The user is allowed to clear the window with Ctrl-l. After this,
 * the scroller starts at the top and begins moving down again like
 * clearing a terminal screen when using gdb.
 *
 * The scroller knows how many lines should be displayed to the user.
 * Some lines are longer than the terminal and wrap.
 * This function determines how many rows are necessary to display
 * in the scroller to show the proper lines.
 *
 * Note that scr->lines_to_display <= rows_to_display <= height.
 *
 * @param scr
 * The scroller object
 *
 * @param height
 * The height of the scroller window
 *
 * @param width
 * The width of the scroller window
 *
 * @return
 * The number of scroller rows to display
 */
static int number_rows_to_display(struct scroller *scr, int height, int width)
{
    int rows_to_display = 0;
    int row, col;
    int nlines;

    if (scr->lines_to_display > height) {
        scr->lines_to_display = height;
    }

    row = scr->current.r;
    col = scr->current.c;

    /**
     * For each line to display, determine the number or rows it takes up.
     */
    for (nlines = 0; nlines < scr->lines_to_display;) {
        if (col >= width)
            col -= width;
        else {
            nlines++;
            row--;
            if (row >= 0) {
                int length = scr->lines[row].line_len;
                if (length > width)
                    col = ((length - 1) / width) * width;
            }
        }

        rows_to_display += 1;
    }

    if (rows_to_display > height) {
        rows_to_display = height;
    }

    return rows_to_display;
}

void scr_refresh(struct scroller *scr, int focus, enum win_refresh dorefresh)
{
    int length;                 /* Length of current line */
    int nlines;                 /* Number of lines written so far */
    int r;                      /* Current row in scroller */
    int c;                      /* Current column in row */
    int width, height;          /* Width and height of window */
    int highlight_attr;
    int hlsearch = cgdbrc_get_int(CGDBRC_HLSEARCH);
    int rows_to_display = 0;

    /* Steal line highlight attribute for our scroll mode status */
    highlight_attr = hl_groups_get_attr(hl_groups_instance,
        HLG_SCROLL_MODE_STATUS);

    /* Sanity check */
    height = swin_getmaxy(scr->win);
    width = swin_getmaxx(scr->win);

    if (scr->current.c > 0) {
        if (scr->current.c % width != 0)
            scr->current.c = (scr->current.c / width) * width;
    }
    r = scr->current.r;
    c = scr->current.c;


    rows_to_display = number_rows_to_display(scr, height, width);

    /**
     * Printing the scroller to the gdb window.
     *
     * The gdb window has a certain dimension (height and width).
     * The scroller has a certain number of rows to print.
     *
     * When starting cgdb, or when the user clears the screen with Ctrl-L,
     * the entire scroller buffer is not displayed in the gdb window.
     *
     * The gdb readline input will be displayed just below the last
     * line of output in the scroller.
     *
     * In order to display the scroller buffer, first determine how many
     * lines should be displayed. Then start drawing from the bottom of
     * the viewable space and work our way up.
     */
    for (nlines = 1; nlines <= height; nlines++) {
        /* Empty lines below the scroller prompt should be empty.
         * When in scroller mode, there should be no empty lines on the
         * bottom. */
        if (!scr->in_scroll_mode && nlines <= height - rows_to_display) {
            swin_wmove(scr->win, height - nlines, 0);
            swin_wclrtoeol(scr->win);
        } 
        /* Print the current line [segment] */
        else if (r >= 0) {
            struct scroller_line *sline = &scr->lines[r];

            hl_printline(scr->win, sline->line, sline->line_len, sline->attrs, 0, height - nlines, c, width);

            /* If we're searching right now or we finished search
             * and have focus... */
            if (hlsearch && scr->last_hlregex && focus) {
                struct hl_line_attr *attrs = hl_regex_highlight(
                    &scr->last_hlregex, sline->line, HLG_SEARCH);

                if (sbcount(attrs)) {
                    hl_printline_highlight(scr->win, sline->line,
                        sline->line_len, attrs, 0, height - nlines, c, width);
                    sbfree(attrs);
                }
            }

            if (scr->hlregex && scr->current.r == r) {
                struct hl_line_attr *attrs = hl_regex_highlight(
                    &scr->hlregex, sline->line, HLG_INCSEARCH);

                if (sbcount(attrs)) {
                    hl_printline_highlight(scr->win, sline->line,
                        sline->line_len, attrs, 0, height - nlines, c, width);
                    sbfree(attrs);
                }
            }

            /* Update our position */
            if (c >= width)
                c -= width;
            else {
                r--;
                if (r >= 0) {
                    length = scr->lines[r].line_len;
                    if (length > width)
                        c = ((length - 1) / width) * width;
                }
            }
        /* Empty lines above the first line in the scroller should be empty.
         * Since the scroller starts at the top, this should only occur when
         * in the scroll mode. */
        } else {
            swin_wmove(scr->win, height - nlines, 0);
            swin_wclrtoeol(scr->win);
        }

        /* If we're in scroll mode and this is the top line, spew status on right */
        if (scr->in_scroll_mode && (nlines == height)) {
            char status[ 64 ];
            size_t status_len;

            snprintf(status, sizeof(status), "[%d/%d]", scr->current.r + 1, sbcount(scr->lines));

            status_len = strlen(status);
            if ( status_len < width ) {
                swin_wattron(scr->win, highlight_attr);
                swin_mvwprintw(scr->win, height - nlines, width - status_len, "%s", status);
                swin_wattroff(scr->win, highlight_attr);
            }
        }
    }

    length = scr->lines[scr->current.r].line_len - scr->current.c;

    /* Only show the cursor when
     * - the scroller is in focus
     * - on the last line
     * - when it is within the width of the screen
     * - when not in scroller mode
     */ 
    if (focus && scr->current.r == sbcount(scr->lines) - 1 &&
        length <= width && !scr->in_scroll_mode) {
        swin_curs_set(1);
        swin_wmove(scr->win, rows_to_display-1, scr->current.pos % width);
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
