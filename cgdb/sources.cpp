/* sources.c:
 * ----------
 * 
 * Source file management routines for the GUI.  Provides the ability to
 * add files to the list, load files, and display within a curses window.
 * Files are buffered in memory when they are displayed, and held in
 * memory for the duration of execution.  If memory consumption becomes a
 * problem, this can be optimized to unload files which have not been
 * displayed recently, or only load portions of large files at once. (May
 * affect syntax highlighting.)
 *
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

/* System Includes */
#if HAVE_STDIO_H
#include <stdio.h>
#endif /* HAVE_STDIO_H */

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif /* HAVE_STDLIB_H */

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */

#if HAVE_CTYPE_H
#include <ctype.h>
#endif

/* Local Includes */
#include "cgdb.h"
#include "highlight.h"
#include "sources.h"
#include "logo.h"
#include "sys_util.h"
#include "fs_util.h"
#include "cgdbrc.h"
#include "highlight_groups.h"

int sources_syntax_on = 1;

// This speeds up loading sqlite.c from 2:48 down to ~2 seconds.
// sqlite3 is 6,596,401 bytes, 188,185 lines.

/* --------------- */
/* Local Functions */
/* --------------- */

/* get_relative_node:  Returns a pointer to the node that matches the 
 * ------------------  given relative path.
 
 *   lpath:  Full path to source file
 *
 * Return Value:  Pointer to the matching node, or NULL if not found.
 */
static struct list_node *get_relative_node(struct sviewer *sview,
        const char *lpath)
{
    struct list_node *cur;

    for (cur = sview->list_head; cur != NULL; cur = cur->next) {
        if (cur->lpath && (strcmp(lpath, cur->lpath) == 0))
            return cur;
    }

    return NULL;
}

/* get_node:  Returns a pointer to the node that matches the given path.
 * ---------
 
 *   path:  Full path to source file
 *
 * Return Value:  Pointer to the matching node, or NULL if not found.
 */
static struct list_node *get_node(struct sviewer *sview, const char *path)
{
    struct list_node *cur;

    for (cur = sview->list_head; cur != NULL; cur = cur->next) {
        if (cur->path && (strcmp(path, cur->path) == 0))
            return cur;
    }

    return NULL;
}

/* stb__sbgrowf: internal stretchy buffer grow function.
 */
int stb__sbgrowf( void **arr, int increment, int itemsize )
{
    int m = *arr ? 2 * stb__sbm( *arr ) + increment : increment + 1; 
    void *p = cgdb_realloc( *arr ? stb__sbraw( *arr ) : 0,
                            itemsize * m + sizeof( int ) * 2 );

    if ( !*arr )
        ( ( int * )p )[ 1 ] = 0; 
    *arr = ( void * )( ( int * )p + 2 ); 
    stb__sbm( *arr ) = m; 

    return 0;
}

/**
 * Get's the timestamp of a particular file.
 *
 * \param path
 * The path to the file to get the timestamp of
 *
 * \param timestamp
 * The timestamp of the file, or 0 on error.
 * 
 * \return
 * 0 on success, -1 on error.
 */
static int get_timestamp(const char *path, time_t * timestamp)
{
    struct stat s;
    int val;

    if (!path)
        return -1;

    if (!timestamp)
        return -1;

    *timestamp = 0;

    val = stat(path, &s);

    if (val)                    /* Error on timestamp */
        return -1;

    *timestamp = s.st_mtime;

    return 0;
}

static void init_file_buffer(struct buffer *buf)
{
    buf->tlines = NULL;
    buf->cur_line = NULL;
    buf->max_width = 0;
    buf->file_data = NULL;
    buf->language = TOKENIZER_LANGUAGE_UNKNOWN;
}

static void release_file_buffer(struct buffer *buf)
{
    if (buf) {
        /* Free entire file buffer if we have that */
        if (buf->file_data) {
            free(buf->file_data);
            buf->file_data = NULL;
        }
        else {
            /* Otherwise free individual file lines */
            int i;
            int count = sbcount(buf->tlines);

            for (i = 0; i < count; ++i) {
                free(buf->tlines[i]);
                buf->tlines[i] = NULL;
            }
        }

        sbfree(buf->tlines);
        buf->tlines = NULL;

        free(buf->cur_line);
        buf->cur_line = NULL;

        buf->max_width = 0;
        buf->language = TOKENIZER_LANGUAGE_UNKNOWN;
    }
}

/** 
 * Remove's the memory related to a file.
 *
 * \param node
 * The node who's file buffer data needs to be freed.
 *
 * \return
 * 0 on success, or -1 on error.
 */
static int release_file_memory(struct list_node *node)
{
    if (!node)
        return -1;

    /* Release file buffers */
    release_file_buffer(&node->color_buf);
    release_file_buffer(&node->orig_buf);
    node->buf = NULL;

    return 0;
}

/**
 * Get file size from file pointer.
 *
 * \param file
 * file pointer
 *
 * \return
 * file size on success, or -1 on error.
 */
static long get_file_size(FILE *file)
{
    if (fseek(file, 0, SEEK_END) != -1) {
        long size;

        size = ftell(file);
        fseek(file, 0, SEEK_SET);

        return size;
    }

    return -1;
}

/**
 * Load file and fill tlines line pointers.
 *
 * \param buf
 * struct buffer pointer
 *
 * \param filename
 * name of file to load
 *
 * \return
 * 0 on sucess, 1 on error
 */
static int load_file_buf(struct buffer *buf, const char *filename)
{
    FILE *file;
    long file_size;

    if (!(file = fopen(filename, "rb")))
        return 1;

    file_size = get_file_size(file);
    if ( file_size > 0 ) {
        size_t bytes_read;

        buf->file_data = (char *)cgdb_malloc(file_size + 2);
        bytes_read = fread(buf->file_data, 1, file_size, file);
        if ( bytes_read != file_size ) {
            return 1;
        } else {
            char *line_start = buf->file_data;
            char *line_feed = strchr( line_start, '\n' );

            line_start[file_size] = 0;

            while ( line_feed ) {
                size_t line_len;
                char *line_end = line_feed;

                /* Trim trailing cr-lfs */
                while ( line_end >= line_start && ( *line_end == '\n' || *line_end == '\r' ) )
                    *line_end-- = 0;

                /* Update max length string found */
                line_len = line_end - line_start;
                if (line_len > buf->max_width )
                    buf->max_width = line_len;

                /* Add this line to tlines array */
                sbpush( buf->tlines, line_start );

                line_start = line_feed + 1;
                line_feed = strchr( line_start, '\n' );
            }

            if ( *line_start )
                sbpush( buf->tlines, line_start );
        }

        /* Add two nil bytes in case we use lexer string scanner. */
        buf->file_data[file_size] = 0;
        buf->file_data[file_size + 1] = 0;
    }

    fclose(file);
    return 0;
}

/* load_file:  Loads the file in the list_node into its memory buffer.
 * ----------
 *
 *   node:  The list node to work on
 *
 * Return Value:  Zero on success, non-zero on error.
 */
static int load_file(struct list_node *node)
{
    /* Stat the file to get the timestamp */
    if (get_timestamp(node->path, &(node->last_modification)) == -1)
        return 2;

    node->language = tokenizer_get_default_file_type(strrchr(node->path, '.'));

    /* Add the highlighted lines */
    return source_highlight(node);
}

enum LineKind {
    EXECUTING_LINE,
    SELECTED_LINE
};

/* draw_current_line:  Draws the currently executing source line on the screen
 * ------------------  including the user-selected marker (arrow, highlight,
 *                     etc) indicating this is the executing line.
 *
 *   sview:  The current source viewer
 *   line:   The line number
 *   lwidth: The width of the line number, used to limit printing to the width
 *           of the screen.  Kinda ugly.
 *   kind:   1 if for the executing line, 0 if for 
 */
static void draw_current_line(struct sviewer *sview, int line, int lwidth,
    enum LineKind kind)
{
    int height = 0;             /* Height of curses window */
    int width = 0;              /* Width of curses window */
    int i = 0, j = 0;           /* Iterators */
    struct buffer *buf = NULL;  /* Pointer to the source buffer */
    char *text = NULL;          /* The current line (highlighted) */
    const char *otext = NULL;   /* The current line (unhighlighted) */
    unsigned int length = 0;    /* Length of the line */
    int column_offset = 0;      /* Text to skip due to arrow */
    int arrow_attr;
    int block_attr;
    int highlight_attr;
    enum LineDisplayStyle display_style;
    int highlight_tabstop = cgdbrc_get_int(CGDBRC_TABSTOP);

    switch (kind) {
        case EXECUTING_LINE:
            display_style =
                cgdbrc_get_displaystyle(CGDBRC_EXECUTING_LINE_DISPLAY);
            arrow_attr = hl_groups_get_attr(hl_groups_instance,
                    HLG_EXECUTING_LINE_ARROW);
            highlight_attr = hl_groups_get_attr(hl_groups_instance,
                    HLG_EXECUTING_LINE_HIGHLIGHT);
            block_attr = hl_groups_get_attr(hl_groups_instance,
                    HLG_EXECUTING_LINE_BLOCK);
            break;
        case SELECTED_LINE:
            display_style =
                cgdbrc_get_displaystyle(CGDBRC_SELECTED_LINE_DISPLAY);
            arrow_attr = hl_groups_get_attr(hl_groups_instance,
                    HLG_SELECTED_LINE_ARROW);
            highlight_attr = hl_groups_get_attr(hl_groups_instance,
                    HLG_SELECTED_LINE_HIGHLIGHT);
            block_attr = hl_groups_get_attr(hl_groups_instance,
                    HLG_SELECTED_LINE_BLOCK);
            break;
    }

    /* Initialize height and width */
    getmaxyx(sview->win, height, width);

    buf = sview->cur->buf;

    /* If the current selected line is the line executing, use cur_line */
    if (line == sview->cur->sel_line && buf->cur_line != NULL) {
        text = buf->cur_line;
    } else {
        text = buf->tlines[line];
    }
    otext = sview->cur->buf->tlines[line];
    length = strlen(otext);

    /* Draw the appropriate arrow, if applicable */
    switch (display_style) {

        case LINE_DISPLAY_SHORT_ARROW:

            wattron(sview->win, arrow_attr);
            waddch(sview->win, ACS_LTEE);
            waddch(sview->win, '>');
            wattroff(sview->win, arrow_attr);
            break;

        case LINE_DISPLAY_LONG_ARROW:

            wattron(sview->win, arrow_attr);
            waddch(sview->win, ACS_LTEE);

            /* Compute the length of the arrow, respecting tab stops, etc. */
            if (isspace(otext[0]) || (otext[i] == hl_get_marker())) {
                for (i = 0; i < length - 1; i++) {
                    /* Skip highlight chars */
                    if ((otext[i] == hl_get_marker()) && otext[i]) {
                        i++;
                        continue;
                    }

                    /* Add one for space or number of spaces to next tabstop */
                    int offset = otext[i] != '\t' ? 1 :
                            highlight_tabstop - (column_offset % highlight_tabstop);

                    column_offset += offset;

                    if (!isspace(otext[i + 1])) {
                        column_offset--;
                        break;
                    }
                }
            }

            column_offset -= sview->cur->sel_col;
            if (column_offset < 0)
                column_offset = 0;
            else {
                /* Now actually draw the arrow */
                for (j = 0; j < column_offset; j++)
                    waddch(sview->win, ACS_HLINE);
            }

            waddch(sview->win, '>');
            wattroff(sview->win, arrow_attr);
            break;

        case LINE_DISPLAY_HIGHLIGHT:
            waddch(sview->win, VERT_LINE);
            waddch(sview->win, ' ');

            wattron(sview->win, highlight_attr);
            for (i = 0; i < width - lwidth - 2; i++) {
                if (i < length) {
                    /* Skip highlight chars */
                    if ( otext[i] == hl_get_marker() )
                        i++;
                    else
                        waddch(sview->win, otext[i]);
                } else {
                    waddch(sview->win, ' ');
                }
            }
            wattroff(sview->win, highlight_attr);

            return;

        case LINE_DISPLAY_BLOCK:

            waddch(sview->win, VERT_LINE);

            /* Compute the length of the arrow, respecting tab stops, etc. */
            if (isspace(otext[0]) || (otext[i] == hl_get_marker())) {
                for (i = 0; i < length - 1; i++) {
                    /* Skip highlight chars */
                    if ((otext[i] == hl_get_marker()) && otext[i]) {
                        i++;
                        continue;
                    }

                    /* Add one for space or number of spaces to next tabstop */
                    int offset = otext[i] != '\t' ? 1 :
                            highlight_tabstop - (column_offset % highlight_tabstop);

                    column_offset += offset;

                    if (!isspace(otext[i + 1])) {
                        column_offset--;
                        break;
                    }
                }
            }


            column_offset -= sview->cur->sel_col;
            if (column_offset < 0) {
                column_offset = 0;
            }

            /* Draw the space to the block */
            for (j = 0; j < column_offset; j++) {
                waddch(sview->win, ' ');
            }

            /* Draw the block */
            wattron(sview->win, block_attr);
            waddch(sview->win, ' ');
            wattroff(sview->win, block_attr);
            break;
    }

    /* Finally, print the source line */
    hl_wprintw(sview->win, text, width - lwidth - 2,
            sview->cur->sel_col + column_offset);
}

/* --------- */
/* Functions */
/* --------- */

/* Descriptive comments found in header file: sources.h */

int source_highlight(struct list_node *node)
{
    int do_color = sources_syntax_on &&
                   (node->language != TOKENIZER_LANGUAGE_UNKNOWN) &&
                   has_colors();

    node->buf = NULL;

    /* If we're doing color and we haven't already loaded this file
     * with this language, then load and highlight it.
     */
    if (do_color && (node->color_buf.language != node->language))
    {
        /* Release previously loaded data */
        release_file_buffer(&node->color_buf);

        node->color_buf.language = node->language;
        if ( highlight_node(node->path, &node->color_buf) )
        {
            /* Error - free this and try loading no color buffer */
            release_file_buffer( &node->color_buf );
            do_color = 0;
        }
    }

    if (!do_color && !sbcount(node->orig_buf.tlines))
        load_file_buf(&node->orig_buf, node->path);

    /* If we're doing color, use color_buf, otherwise original buf */
    node->buf = do_color ? &node->color_buf : &node->orig_buf;

    /* Allocate the breakpoints array */
    if ( !node->breakpts ) {
        int count = sbcount(node->buf->tlines);
        sbsetcount( node->breakpts, count );

        memset(node->breakpts, 0, sbcount(node->breakpts));
    }

    if (node->buf && node->buf->tlines)
        return 0;

    return -1;
}


struct sviewer *source_new(int pos_r, int pos_c, int height, int width)
{
    struct sviewer *rv;

    /* Allocate a new structure */
    if ((rv = (struct sviewer *)malloc(sizeof (struct sviewer))) == NULL)
        return NULL;

    /* Initialize the structure */
    rv->win = newwin(height, width, pos_r, pos_c);
    rv->cur = NULL;
    rv->list_head = NULL;

    return rv;
}

int source_add(struct sviewer *sview, const char *path)
{
    struct list_node *new_node;

    new_node = (struct list_node *)malloc(sizeof (struct list_node));
    new_node->path = strdup(path);
    new_node->lpath = NULL;

    init_file_buffer(&new_node->orig_buf);
    init_file_buffer(&new_node->color_buf);

    new_node->buf = NULL;
    new_node->breakpts = NULL;
    new_node->sel_line = 0;
    new_node->sel_col = 0;
    new_node->sel_col_rbeg = 0;
    new_node->sel_col_rend = 0;
    new_node->sel_rline = 0;
    new_node->exe_line = 0;
    new_node->last_modification = 0;    /* No timestamp yet */

    if (sview->list_head == NULL) {
        /* List is empty, this is the first node */
        new_node->next = NULL;
        sview->list_head = new_node;
    } else {
        /* Insert at the front of the list (easy) */
        new_node->next = sview->list_head;
        sview->list_head = new_node;
    }

    return 0;
}

int source_set_relative_path(struct sviewer *sview,
        const char *path, const char *lpath)
{
    struct list_node *node = sview->list_head;

    while (node != NULL) {
        if (strcmp(node->path, path) == 0) {
            free(node->lpath);
            node->lpath = strdup(lpath);
            return 0;
        }

        node = node->next;
    }

    return -1;
}

int source_del(struct sviewer *sview, const char *path)
{
    struct list_node *cur;
    struct list_node *prev = NULL;

    /* Find the target node */
    for (cur = sview->list_head; cur != NULL; cur = cur->next) {
        if (strcmp(path, cur->path) == 0)
            break;
        prev = cur;
    }

    if (cur == NULL)
        return 1;               /* Node not found */

    /* Release file buffers */
    release_file_buffer(&cur->orig_buf);
    release_file_buffer(&cur->color_buf);
    cur->buf = NULL;

    /* Release file name */
    free(cur->path);
    cur->path = NULL;

    sbfree(cur->breakpts);
    cur->breakpts = NULL;

    /* Release local file name */
    if (cur->lpath) {
        free(cur->lpath);
        cur->lpath = NULL;
    }

    /* Remove link from list */
    if (cur == sview->list_head)
        sview->list_head = sview->list_head->next;
    else
        prev->next = cur->next;

    /* Free the node */
    free(cur);

    return 0;
}

int source_length(struct sviewer *sview, const char *path)
{
    struct list_node *cur = get_node(sview, path);

    if (!cur) {
        /* Load the file if it's not already */
        if (!cur->buf && load_file(cur))
            return -1;
    }

    return sbcount(cur->buf->tlines);
}

char *source_current_file(struct sviewer *sview, char *path)
{
    if (sview == NULL || sview->cur == NULL)
        return NULL;

    strcpy(path, sview->cur->path);
    return path;
}

int source_display(struct sviewer *sview, int focus, enum win_refresh dorefresh)
{
    char fmt[5];
    int width, height;
    int lwidth;
    int line;
    int i;
    int count;
    int attr = 0, sellineno;
    int exelinearrow, sellinearrow;
    int enabled_bp, disabled_bp;

    exelinearrow = hl_groups_get_attr(hl_groups_instance, HLG_EXECUTING_LINE_ARROW);
    sellinearrow = hl_groups_get_attr(hl_groups_instance, HLG_SELECTED_LINE_ARROW);
    sellineno = hl_groups_get_attr(hl_groups_instance, HLG_SELECTED_LINE_NUMBER);
    enabled_bp = hl_groups_get_attr(hl_groups_instance, HLG_ENABLED_BREAKPOINT);
    disabled_bp = hl_groups_get_attr(hl_groups_instance, HLG_DISABLED_BREAKPOINT);

    /* Check that a file is loaded */
    if (sview->cur == NULL || sview->cur->buf->tlines == NULL) {
        logo_display(sview->win);
        wrefresh(sview->win);
        return 0;
    }

    /* Make sure cursor is visible */
    curs_set( !!focus );

    /* Initialize variables */
    getmaxyx(sview->win, height, width);

    /* Set starting line number (center source file if it's small enough) */
    count = sbcount(sview->cur->buf->tlines);
    if (count < height)
        line = (count - height) / 2;
    else {
        line = sview->cur->sel_line - height / 2;
        if (line > count - height)
            line = count - height;
        else if (line < 0)
            line = 0;
    }

    /* Print 'height' lines of the file, starting at 'line' */
    lwidth = log10_uint(count) + 1;
    sprintf(fmt, "%%%dd", lwidth);

    for (i = 0; i < height; i++, line++) {
        wmove(sview->win, i, 0);

        if (!has_colors()) {
            wprintw(sview->win, "%s\n", sview->cur->buf->tlines[line]);
            continue;
        }

        /* Outside of file, just finish drawing the vertical line */
        if (line < 0 || line >= count) {
            int j;

            for (j = 1; j < lwidth; j++)
                waddch(sview->win, ' ');
            waddch(sview->win, '~');

            if (focus)
                wattron(sview->win, A_BOLD);
            waddch(sview->win, VERT_LINE);
            if (focus)
                wattroff(sview->win, A_BOLD);

            for (j = 2 + lwidth; j < width; j++)
                waddch(sview->win, ' ');

            /* Mark the current (executing) line with an arrow */
        } else if ( line == sview->cur->exe_line ) {
            switch (sview->cur->breakpts[line]) {
                case 0:
                    attr = exelinearrow;
                    break;
                case 1:
                    attr = enabled_bp;
                    break;
                case 2:
                    attr = disabled_bp;
                    break;
            }
            wattron(sview->win, attr);
            wprintw(sview->win, fmt, line + 1);
            wattroff(sview->win, attr);

            draw_current_line(sview, line, lwidth, EXECUTING_LINE);

            /* Mark the current line with an arrow */
        } else if ( line == sview->cur->sel_line ) {
            switch (sview->cur->breakpts[line]) {
                case 0:
                    attr = sellinearrow;
                    break;
                case 1:
                    attr = enabled_bp;
                    break;
                case 2:
                    attr = disabled_bp;
                    break;
            }
            wattron(sview->win, attr);
            wprintw(sview->win, fmt, line + 1);
            wattroff(sview->win, attr);

            draw_current_line(sview, line, lwidth, SELECTED_LINE);

            /* Look for breakpoints */
        } else if (sview->cur->breakpts[line]) {
            if (sview->cur->breakpts[line] == 1)
                attr = enabled_bp;
            else
                attr = disabled_bp;

            wattron(sview->win, attr);
            wprintw(sview->win, fmt, line + 1);
            wattroff(sview->win, attr);

            if (focus)
                wattron(sview->win, A_BOLD);
            waddch(sview->win, VERT_LINE);
            if (focus)
                wattroff(sview->win, A_BOLD);
            waddch(sview->win, ' ');

            if (line == sview->cur->sel_line && sview->cur->buf->cur_line != NULL) {
                hl_wprintw(sview->win, sview->cur->buf->cur_line,
                        width - lwidth - 2, sview->cur->sel_col);

            } else {
                hl_wprintw(sview->win, sview->cur->buf->tlines[line],
                        width - lwidth - 2, sview->cur->sel_col);
            }
        }
        /* Ordinary lines */
        else {
            if (focus && sview->cur->sel_line == line)
                wattron(sview->win, sellineno);

            wprintw(sview->win, fmt, line + 1);

            if (focus && sview->cur->sel_line == line)
                wattroff(sview->win, sellineno);

            if (focus)
                wattron(sview->win, A_BOLD);
            waddch(sview->win, VERT_LINE);
            if (focus)
                wattroff(sview->win, A_BOLD);
            waddch(sview->win, ' ');

            /* No special line information */
            if (line == sview->cur->sel_line && sview->cur->buf->cur_line != NULL) {
                hl_wprintw(sview->win, sview->cur->buf->cur_line,
                        width - lwidth - 2, sview->cur->sel_col);

            } else {
                hl_wprintw(sview->win, sview->cur->buf->tlines[line],
                        width - lwidth - 2, sview->cur->sel_col);
            }
        }
    }

    wmove(sview->win, height - (line - sview->cur->sel_line), lwidth + 2);

    switch(dorefresh) {
        case WIN_NO_REFRESH:
            wnoutrefresh(sview->win);
            break;
        case WIN_REFRESH:
            wrefresh(sview->win);
            break;
    }

    return 0;
}

void source_move(struct sviewer *sview,
        int pos_r, int pos_c, int height, int width)
{
    delwin(sview->win);
    sview->win = newwin(height, width, pos_r, pos_c);
    wclear(sview->win);
}

void source_vscroll(struct sviewer *sview, int offset)
{
    if (sview->cur) {
        sview->cur->sel_line += offset;
        if (sview->cur->sel_line < 0)
            sview->cur->sel_line = 0;
        if (sview->cur->sel_line >= sbcount(sview->cur->buf->tlines))
            sview->cur->sel_line = sbcount(sview->cur->buf->tlines) - 1;

        sview->cur->sel_rline = sview->cur->sel_line;
    }
}

void source_hscroll(struct sviewer *sview, int offset)
{
    int lwidth;
    int max_width;
    int width, height;

    if (sview->cur) {
        getmaxyx(sview->win, height, width);

        lwidth = log10_uint(sbcount(sview->cur->buf->tlines)) + 1;
        max_width = sview->cur->buf->max_width - width + lwidth + 6;

        sview->cur->sel_col += offset;
        if (sview->cur->sel_col > max_width)
            sview->cur->sel_col = max_width;
        if (sview->cur->sel_col < 0)
            sview->cur->sel_col = 0;
    }
}

void source_set_sel_line(struct sviewer *sview, int line)
{
    if (sview->cur) {
        if (line == -1) {
            sview->cur->sel_line = sbcount(sview->cur->buf->tlines) - 1;
        } else {
            /* Set line (note correction for 0-based line counting) */
            sview->cur->sel_line = line - 1;
            if (sview->cur->sel_line < 0)
                sview->cur->sel_line = 0;
            if (sview->cur->sel_line >= sbcount(sview->cur->buf->tlines))
                sview->cur->sel_line = sbcount(sview->cur->buf->tlines) - 1;
            }

        sview->cur->sel_rline = sview->cur->sel_line;
    }
}

int source_set_exec_line(struct sviewer *sview, const char *path, int line)
{
    if (path && !fs_verify_file_exists(path))
        return 5;

    /* Locate node, if path has changed */
    if (path != NULL && !(sview->cur = get_node(sview, path))) {
        /* Not found -- attempt to add */
        if (source_add(sview, path))
            return 1;
        else if (!(sview->cur = get_node(sview, path)))
            return 2;
    } else if (path == NULL && sview->cur == NULL)
        return 3;

    /* Buffer the file if it's not already */
    if (!sview->cur->buf && load_file(sview->cur))
        return 4;

    /* Update line, if set */
    if (line--) {
        /* Check bounds of line */
        if (line < 0)
            line = 0;
        if (line >= sbcount(sview->cur->buf->tlines))
            line = sbcount(sview->cur->buf->tlines) - 1;
        sview->cur->sel_line = sview->cur->exe_line = line;
    }

    return 0;
}

void source_free(struct sviewer *sview)
{
    /* Free all file buffers */
    while (sview->list_head != NULL)
        source_del(sview, sview->list_head->path);

    delwin(sview->win);
}

void source_search_regex_init(struct sviewer *sview)
{
    if (sview == NULL || sview->cur == NULL)
        return;

    /* Start from beginning of line if not at same line */
    if (sview->cur->sel_rline != sview->cur->sel_line) {
        sview->cur->sel_col_rend = 0;
        sview->cur->sel_col_rbeg = 0;
    }

    /* Start searching at the beginning of the selected line */
    sview->cur->sel_rline = sview->cur->sel_line;
}

int source_search_regex(struct sviewer *sview,
        const char *regex, int opt, int direction, int icase)
{
    if (sview == NULL || sview->cur == NULL ||
        regex == NULL || strlen(regex) == 0) {

        if (sview && sview->cur) {
            free( sview->cur->buf->cur_line );
            sview->cur->buf->cur_line = NULL;
        }
        return -1;
    }

    if ( !sbcount(sview->cur->orig_buf.tlines ) )
        load_file_buf(&sview->cur->orig_buf, sview->cur->path);

    return hl_regex(regex,
            (const char **) sview->cur->buf->tlines,
            (const char **) sview->cur->orig_buf.tlines,
            sbcount(sview->cur->buf->tlines),
            &sview->cur->buf->cur_line, &sview->cur->sel_line,
            &sview->cur->sel_rline, &sview->cur->sel_col_rbeg,
            &sview->cur->sel_col_rend, opt, direction, icase);
}

void source_disable_break(struct sviewer *sview, const char *path, int line)
{
    struct list_node *node;

    if ((node = get_relative_node(sview, path)) == NULL)
        return;

    if (!node->buf && load_file(node))
        return;

    if (line > 0 && line <= sbcount(node->buf->tlines))
        node->breakpts[line - 1] = 2;
}

void source_enable_break(struct sviewer *sview, const char *path, int line)
{
    struct list_node *node;

    if ((node = get_relative_node(sview, path)) == NULL)
        return;

    if (!node->buf && load_file(node))
        return;

    if (line > 0 && line <= sbcount(node->buf->tlines)) {
        node->breakpts[line - 1] = 1;
    }
}

void source_clear_breaks(struct sviewer *sview)
{
    struct list_node *node;

    for (node = sview->list_head; node != NULL; node = node->next)
        memset(node->breakpts, 0, sbcount(node->breakpts));
}

int source_reload(struct sviewer *sview, const char *path, int force)
{
    time_t timestamp;
    struct list_node *cur;
    struct list_node *prev = NULL;
    int auto_source_reload = cgdbrc_get_int(CGDBRC_AUTOSOURCERELOAD);

    if (!path)
        return -1;

    if (get_timestamp(path, &timestamp) == -1)
        return -1;

    /* Find the target node */
    for (cur = sview->list_head; cur != NULL; cur = cur->next) {
        if (strcmp(path, cur->path) == 0)
            break;
        prev = cur;
    }

    if (cur == NULL)
        return 1;               /* Node not found */

    if ((auto_source_reload || force) && cur->last_modification < timestamp) {

        if (release_file_memory(sview->cur) == -1)
            return -1;

        if (load_file(cur))
            return -1;
    }

    return 0;
}
