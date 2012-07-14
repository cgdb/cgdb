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
#include "cgdb.h"
#include "scroller.h"

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
static int count(const char *s, char c)
{
    int rv = 0;
    char *x = strchr(s, c);

    while (x) {
        rv++;
        x = strchr(x + 1, c);
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
static char *parse(struct scroller *scr, const char *orig, const char *buf)
{
    const int tab_size = 8;
    int length = strlen(orig) + strlen(buf) + (tab_size - 1) * count(buf, '\t');
    char *rv = (char *) malloc(length + 1);
    int i, j;

    /* Zero out the string */
    memset(rv, 0, length + 1);
    strcpy(rv, orig);
    i = scr->current.pos;

    /* Expand special characters */
    for (j = 0; j < strlen(buf); j++) {
        switch (buf[j]) {
                /* Backspace/Delete -> Erase last character */
            case 8:
            case 127:
                if (i > 0)
                    i--;
                break;
                /* Tab -> Translating to spaces */
            case '\t':
                do
                    rv[i++] = ' ';
                while (i % tab_size != 0);
                break;
                /* Carriage return -> Move back to the beginning of the line */
            case '\r':
                i = 0;
                break;
                /* Default case -> Only keep printable characters */
            default:
                if (isprint((int) buf[j])) {
                    rv[i] = buf[j];
                    i++;
                }
                break;
        }
    }

    scr->current.pos = i;
    /* Remove trailing space from the line */
    for (j = strlen(rv) - 1; j > i && isspace((int) rv[j]); j--);
    rv[j + 1] = 0;

    return realloc(rv, strlen(rv) + 1);
}

/* ----------------- */
/* Exposed Functions */
/* ----------------- */

/* See scroller.h for function descriptions. */

struct scroller *scr_new(int pos_r, int pos_c, int height, int width)
{
    struct scroller *rv;

    if ((rv = malloc(sizeof (struct scroller))) == NULL)
        return NULL;

    rv->current.r = 0;
    rv->current.c = 0;
    rv->current.pos = 0;
    rv->win = newwin(height, width, pos_r, pos_c);

    /* Start with a single (blank) line */
    rv->buffer = malloc(sizeof (char *));
    rv->buffer[0] = strdup("");
    rv->length = 1;

    return rv;
}

void scr_free(struct scroller *scr)
{
    int i;

    /* Release the buffer */
    if (scr->length) {
        for (i = 0; i < scr->length; i++)
            free(scr->buffer[i]);
        free(scr->buffer);
    }
    delwin(scr->win);

    /* Release the scroller object */
    free(scr);
}

void scr_up(struct scroller *scr, int nlines)
{
    int height, width;
    int length;
    int i;

    /* Sanity check */
    getmaxyx(scr->win, height, width);
    if (scr->current.c > 0) {
        if (scr->current.c % width != 0)
            scr->current.c = (scr->current.c / width) * width;
    }

    for (i = 0; i < nlines; i++) {
        /* If current column is positive, drop it by 'width' */
        if (scr->current.c > 0)
            scr->current.c -= width;

        /* Else, decrease the current row number, and set column accordingly */
        else {
            if (scr->current.r > 0) {
                scr->current.r--;
                if ((length = strlen(scr->buffer[scr->current.r])) > width)
                    scr->current.c = ((length - 1) / width) * width;
            } else {
                /* At top */
                break;
            }
        }
    }
}

void scr_down(struct scroller *scr, int nlines)
{
    int height, width;
    int length;
    int i;

    /* Sanity check */
    getmaxyx(scr->win, height, width);
    if (scr->current.c > 0) {
        if (scr->current.c % width != 0)
            scr->current.c = (scr->current.c / width) * width;
    }

    for (i = 0; i < nlines; i++) {
        /* If the current line wraps to the next, then advance column number */
        length = strlen(scr->buffer[scr->current.r]);
        if (scr->current.c < length - width)
            scr->current.c += width;

        /* Otherwise, advance row number, and set column number to 0. */
        else {
            if (scr->current.r < scr->length - 1) {
                scr->current.r++;
                scr->current.c = 0;
            } else {
                /* At bottom */
                break;
            }
        }
    }
}

void scr_home(struct scroller *scr)
{
    scr->current.r = 0;
    scr->current.c = 0;
}

void scr_end(struct scroller *scr)
{
    int height, width;

    getmaxyx(scr->win, height, width);

    scr->current.r = scr->length - 1;
    scr->current.c = (strlen(scr->buffer[scr->current.r]) / width) * width;
}

void scr_add(struct scroller *scr, const char *buf)
{
    int distance;               /* Distance to next new line character */
    int length;                 /* Length of the current line */
    char *x;                    /* Pointer to next new line character */

    /* Find next newline in the string */
    x = strchr(buf, '\n');
    length = strlen(scr->buffer[scr->length - 1]);
    distance = x ? x - buf : strlen(buf);

    /* Append to the last line in the buffer */
    if (distance > 0) {
        char *temp = scr->buffer[scr->length - 1];
        char *buf2 = malloc(distance + 1);

        strncpy(buf2, buf, distance);
        buf2[distance] = 0;
        scr->buffer[scr->length - 1] = parse(scr, temp, buf2);
        free(temp);
        free(buf2);
    }

    /* Create additional lines if buf contains newlines */
    while (x != NULL) {
        char *newbuf;

        buf = x + 1;
        x = strchr(buf, '\n');
        distance = x ? x - buf : strlen(buf);

        /* Create a new buffer that stops at the next newline */
        newbuf = malloc(distance + 1);
        memset(newbuf, 0, distance + 1);
        strncpy(newbuf, buf, distance);

        /* Expand the buffer */
        scr->length++;
        scr->buffer = realloc(scr->buffer, sizeof (char *) * scr->length);
        scr->current.pos = 0;

        /* Add the new line */
        scr->buffer[scr->length - 1] = parse(scr, "", newbuf);
        free(newbuf);
    }

    scr_end(scr);
}

void scr_move(struct scroller *scr, int pos_r, int pos_c, int height, int width)
{
    delwin(scr->win);
    scr->win = newwin(height, width, pos_r, pos_c);
    wclear(scr->win);
}

void scr_refresh(struct scroller *scr, int focus)
{
    int length;                 /* Length of current line */
    int nlines;                 /* Number of lines written so far */
    int r;                      /* Current row in scroller */
    int c;                      /* Current column in row */
    int width, height;          /* Width and height of window */
    char *buffer;               /* Current line segment to print */

    /* Sanity check */
    getmaxyx(scr->win, height, width);

    if (scr->current.c > 0) {
        if (scr->current.c % width != 0)
            scr->current.c = (scr->current.c / width) * width;
    }
    r = scr->current.r;
    c = scr->current.c;
    buffer = malloc(width + 1);
    buffer[width] = 0;

    /* Start drawing at the bottom of the viewable space, and work our way up */
    for (nlines = 1; nlines <= height; nlines++) {

        /* Print the current line [segment] */
        memset(buffer, ' ', width);
        if (r >= 0) {
            length = strlen(scr->buffer[r] + c);
            memcpy(buffer, scr->buffer[r] + c, length < width ? length : width);
        }
        mvwprintw(scr->win, height - nlines, 0, "%s", buffer);

        /* Update our position */
        if (c >= width)
            c -= width;
        else {
            r--;
            if (r >= 0) {
                length = strlen(scr->buffer[r]);
                if (length > width)
                    c = ((length - 1) / width) * width;
            }
        }
    }

    length = strlen(scr->buffer[scr->current.r] + scr->current.c);
    if (focus && scr->current.r == scr->length - 1 && length <= width) {
        /* We're on the last line, draw the cursor */
        curs_set(1);
        wmove(scr->win, height - 1, scr->current.pos % width);
    } else {
        /* Hide the cursor */
        curs_set(0);
    }

    free(buffer);
    wrefresh(scr->win);
}
