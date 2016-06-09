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

static int get_line_leading_ws_count(const char *otext, int length, int tabstop)
{
    int i;
    int column_offset = 0;      /* Text to skip due to arrow */

    for (i = 0; i < length - 1; i++) {
        int offset;

        /* Skip highlight chars (HL_CHAR / CHAR_MAX) */
        if (otext[i] == hl_get_marker()) {
            i++;
            continue;
        }

        /* Bail if we hit a non whitespace character */
        if (!isspace(otext[i]))
            break;

        /* Add one for space or number of spaces to next tabstop */
        offset = (otext[i] == '\t') ? (tabstop - (column_offset % tabstop)) : 1;
        column_offset += offset;
    }

    return column_offset;
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
    if ( !node->lflags ) {
        int count = sbcount(node->buf->tlines);
        sbsetcount( node->lflags, count );

        memset(node->lflags, 0, sbcount(node->lflags));
    }

    if (node->buf && node->buf->tlines)
        return 0;

    return -1;
}


struct sviewer *source_new(int pos_r, int pos_c, int height, int width)
{
    struct sviewer *rv;

    /* Allocate a new structure */
    rv = (struct sviewer *)cgdb_malloc(sizeof (struct sviewer));

    /* Initialize the structure */
    rv->win = newwin(height, width, pos_r, pos_c);
    rv->cur = NULL;
    rv->list_head = NULL;

    /* Initialize global marks */
    memset(rv->global_marks, 0, sizeof(rv->global_marks));
    rv->jump_back_mark.node = NULL;
    rv->jump_back_mark.line = -1;

    return rv;
}

int source_add(struct sviewer *sview, const char *path)
{
    struct list_node *new_node;

    new_node = (struct list_node *)cgdb_malloc(sizeof (struct list_node));
    new_node->path = strdup(path);

    init_file_buffer(&new_node->orig_buf);
    init_file_buffer(&new_node->color_buf);

    new_node->buf = NULL;
    new_node->lflags = NULL;
    new_node->sel_line = 0;
    new_node->sel_col = 0;
    new_node->sel_col_rbeg = 0;
    new_node->sel_col_rend = 0;
    new_node->sel_rline = 0;
    new_node->exe_line = -1;
    new_node->last_modification = 0;    /* No timestamp yet */

    /* Initialize all local marks to -1 */
    memset(new_node->local_marks, 0xff, sizeof(new_node->local_marks));

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

int source_del(struct sviewer *sview, const char *path)
{
    int i;
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

    sbfree(cur->lflags);
    cur->lflags = NULL;

    /* Remove link from list */
    if (cur == sview->list_head)
        sview->list_head = sview->list_head->next;
    else
        prev->next = cur->next;

    /* Free the node */
    free(cur);

    /* Free any global marks pointing to this bugger */
    for (i = 0; i < sizeof(sview->global_marks) / sizeof(sview->global_marks[0]); i++) {
        if (sview->global_marks[i].node == cur)
            sview->global_marks[i].node = NULL;
    }

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

/* source_get_mark_char:  Return mark char for line.
 * --------------------
 *
 *   sview:  The source viewer object
 *   line: line to check for mark
 *   return: -1 on error, 0 if no char exists on line, otherwise char
 */
static int source_get_mark_char(struct sviewer *sview,
    struct list_node *node, int line)
{
    if (!node || (line < 0) || (line >= sbcount(node->lflags)))
        return -1;

    if (node->lflags[line].has_mark) {
        int i;

        for (i = 0; i < MARK_COUNT; i++) {
            if (sview->global_marks[i].line == line)
                return 'A' + i;
        }

        for (i = 0; i < MARK_COUNT; i++) {
            if (node->local_marks[i] == line)
                return 'a' + i;
        }
    }

    return 0;
}

int source_set_mark(struct sviewer *sview, int key)
{
    int ret = 0;
    int old_line;
    struct list_node *old_node;
    int sel_line = sview->cur->sel_line;

    if (key >= 'a' && key <= 'z') {
        /* Local buffer mark */
        old_line = sview->cur->local_marks[key - 'a'];
        old_node = sview->cur;
        sview->cur->local_marks[key - 'a'] = sel_line;
        ret = 1;
    } else if (key >= 'A' && key <= 'Z') {
        /* Global buffer mark */
        old_line = sview->global_marks[key - 'A'].line;
        old_node = sview->global_marks[key - 'A'].node;
        sview->global_marks[key - 'A'].line = sel_line;
        sview->global_marks[key - 'A'].node = sview->cur;
        ret = 1;
    }

    if (ret) {
        /* Just added a mark to the selected line, flag it */
        sview->cur->lflags[sel_line].has_mark = 1;

        /* Check if the old line still has a mark */
        if (source_get_mark_char(sview, old_node, old_line) == 0)
            old_node->lflags[old_line].has_mark = 0;
    }

    return ret;
}

int source_goto_mark(struct sviewer *sview, int key)
{
    int line;
    struct list_node *node = NULL;

    if (key >= 'a' && key <= 'z') {
        /* Local buffer mark */
        line = sview->cur->local_marks[key - 'a'];
        node = (line >= 0) ? sview->cur : NULL;
    } else if (key >= 'A' && key <= 'Z') {
        /* Global buffer mark */
        line = sview->global_marks[key - 'A'].line;
        node = sview->global_marks[key - 'A'].node;
    } else if (key == '\'' ) {
        /* Jump back to where we jumped from */
        line = sview->jump_back_mark.line;
        node = sview->jump_back_mark.node;
    } else if (key == '.') {
        /* Jump to currently executing line if it's set */
        line = sview->cur->exe_line;
        node = (line >= 0) ? sview->cur : NULL;
    }

    if (node) {
        sview->jump_back_mark.line = sview->cur->sel_line;
        sview->jump_back_mark.node = sview->cur;

        sview->cur = node;
        source_set_sel_line(sview, line + 1);
        return 1;
    }

    return 0;
}

/** 
 * Display the source.
 *
 * A line in the source viewer looks like,
 *   # │ marker text
 * where,
 *   # is the line number to display or ~ if no line number
 *   │ is the divider between the line number or it is a mark
 *   marker is shortarrow, longarrow, highlight, block, etc
 *   text is the source code to display
 *
 * The syntax highlighting works as follows,
 *
 * The #
 * - If breakpoint is set, use Breakpoint
 * - If breakpoint is disabled, use DisabledBreakpoint
 * - If selected line, use SelectedLineNr
 * - If executing line, use ExecutingLineNr
 * - Otherwise, no highlighting group
 *
 * The │ 
 * - When source window is in focus, the character is bolded, otherwise normal
 * - If the user has a mark set, the mark will be displayed instead of any
 *   other character.
 * - Edge case: When the marker is long or short arrow, CGDB prints ├
 *   instead of │ the ├ is colored based on highlighting group for 
 *   the selected or executing arrow.
 *
 * The marker
 * - The marker is the shortarrow, longarrow, highlight or block
 * - The color is based off the corresponding highlighting group
 *
 * The text
 * - The syntax highlighting source code to display
 * - Will be colored with SelectedLineHighlight or ExecutingLineHighlight
 *   if the line is the selected or executing line and the display is set
 *   to highlight.
 */

int source_display(struct sviewer *sview, int focus, enum win_refresh dorefresh)
{
    char fmt[5];
    int width, height;
    int lwidth;
    int line;
    int i;
    int count;
    enum LineDisplayStyle exe_display_style, sel_display_style;
    int attr = 0;
    int sellineno, exelineno;
    int enabled_bp, disabled_bp;
    int exe_line_display, sel_line_display;
    int exe_line_display_is_arrow, sel_line_display_is_arrow;
    int exe_arrow_attr, sel_arrow_attr;
    int exe_highlight_attr, sel_highlight_attr;
    int exe_block_attr, sel_block_attr;
    int focus_attr = focus ? A_BOLD : 0;
    int is_sel_line, is_exe_line;
    int highlight_tabstop = cgdbrc_get_int(CGDBRC_TABSTOP);
    const char *cur_line;
    int showmarks = cgdbrc_get_int(CGDBRC_SHOWMARKS);
    int mark_attr;

    /* Check that a file is loaded */
    if (sview->cur == NULL || sview->cur->buf->tlines == NULL) {
        logo_display(sview->win);

        if (dorefresh == WIN_REFRESH)
            wrefresh(sview->win);
        else
            wnoutrefresh(sview->win);
        return 0;
    }

    sellineno = hl_groups_get_attr(
        hl_groups_instance, HLG_SELECTED_LINE_NUMBER);
    exelineno = hl_groups_get_attr(
        hl_groups_instance, HLG_EXECUTING_LINE_NUMBER);
    enabled_bp = hl_groups_get_attr(
        hl_groups_instance, HLG_ENABLED_BREAKPOINT);
    disabled_bp = hl_groups_get_attr(
        hl_groups_instance, HLG_DISABLED_BREAKPOINT);

    exe_display_style = cgdbrc_get_displaystyle(CGDBRC_EXECUTING_LINE_DISPLAY);
    exe_arrow_attr = hl_groups_get_attr(
        hl_groups_instance, HLG_EXECUTING_LINE_ARROW);
    exe_highlight_attr = hl_groups_get_attr(
        hl_groups_instance, HLG_EXECUTING_LINE_HIGHLIGHT);
    exe_block_attr = hl_groups_get_attr(
        hl_groups_instance, HLG_EXECUTING_LINE_BLOCK);

    sel_display_style = cgdbrc_get_displaystyle(CGDBRC_SELECTED_LINE_DISPLAY);
    sel_arrow_attr = hl_groups_get_attr(
        hl_groups_instance, HLG_SELECTED_LINE_ARROW);
    sel_highlight_attr = hl_groups_get_attr(
        hl_groups_instance, HLG_SELECTED_LINE_HIGHLIGHT);
    sel_block_attr = hl_groups_get_attr(
        hl_groups_instance, HLG_SELECTED_LINE_BLOCK);

    exe_line_display_is_arrow =
        exe_display_style == LINE_DISPLAY_SHORT_ARROW ||
        exe_display_style == LINE_DISPLAY_LONG_ARROW;
    sel_line_display_is_arrow = 
        sel_display_style == LINE_DISPLAY_SHORT_ARROW ||
        sel_display_style == LINE_DISPLAY_LONG_ARROW;

    mark_attr = hl_groups_get_attr(hl_groups_instance, HLG_MARK);

    /* Make sure cursor is visible */
    curs_set(!!focus);

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
    snprintf(fmt, sizeof(fmt), "%%%dd", lwidth);

    for (i = 0; i < height; i++, line++) {
        int column_offset = 0;
        int line_highlight_attr = 0;

        wmove(sview->win, i, 0);

        if (!has_colors()) {
            wprintw(sview->win, "%s\n", sview->cur->buf->tlines[line]);
            continue;
        }

        is_sel_line = line == sview->cur->sel_line;
        is_exe_line = line == sview->cur->exe_line;
        cur_line = (is_sel_line && sview->cur->buf->cur_line) ?
            sview->cur->buf->cur_line : sview->cur->buf->tlines[line];

        /* Print the line number */
        if (line < 0 || line >= count) {
            for (int j = 1; j < lwidth; j++)
                waddch(sview->win, ' ');
            waddch(sview->win, '~');
        } else {
            int line_attr = 0;
            int bp_val = sview->cur->lflags[line].breakpt;
            if (bp_val == 1) {
                line_attr = enabled_bp;
            } else if (bp_val == 2) {
                line_attr = disabled_bp;
            } else if (bp_val == 0 && is_exe_line) {
                line_attr = exelineno;
            } else if (bp_val == 0 && is_sel_line) {
                line_attr = sellineno;
            }

            wattron(sview->win, line_attr);
            wprintw(sview->win, fmt, line + 1);
            wattroff(sview->win, line_attr);
        }

        /* Print the vertical bar or mark */
        {
            chtype vert_bar_char;
            int vert_bar_attr;
            int mc;
            
            if (showmarks &&
                ((mc = source_get_mark_char(sview, sview->cur, line)) > 0)) {
                vert_bar_char = mc;
                vert_bar_attr = mark_attr;
            } else if (is_exe_line && exe_line_display_is_arrow) {
                vert_bar_attr = exe_arrow_attr;
                vert_bar_char = ACS_LTEE;
            } else if (is_sel_line && sel_line_display_is_arrow) {
                vert_bar_attr = sel_arrow_attr;
                vert_bar_char = ACS_LTEE;
            } else {
                vert_bar_attr = focus_attr;
                vert_bar_char = VERT_LINE;
            }

            wattron(sview->win, vert_bar_attr);
            waddch(sview->win, vert_bar_char);
            wattroff(sview->win, vert_bar_attr);
        }

        /* Print the marker */
        if (is_exe_line || is_sel_line) {
            enum LineDisplayStyle display_style;
            int arrow_attr, block_attr, highlight_attr;

            if (is_exe_line) {
                display_style = exe_display_style;
                arrow_attr = exe_arrow_attr;
                block_attr = exe_block_attr;
                highlight_attr = exe_highlight_attr;
            } else {
                display_style = sel_display_style;
                arrow_attr = sel_arrow_attr;
                block_attr = sel_block_attr;
                highlight_attr = sel_highlight_attr;
            }

            switch (display_style) {
                case LINE_DISPLAY_SHORT_ARROW:
                    wattron(sview->win, arrow_attr);
                    waddch(sview->win, '>');
                    wattroff(sview->win, arrow_attr);
                    break;
                case LINE_DISPLAY_LONG_ARROW:
                    wattron(sview->win, arrow_attr);
                    column_offset = get_line_leading_ws_count(
                        sview->cur->buf->tlines[line],
                        strlen(sview->cur->buf->tlines[line]),
                        highlight_tabstop);
                    column_offset -= (sview->cur->sel_col + 1);
                    if (column_offset < 0)
                        column_offset = 0;

                    /* Now actually draw the arrow */
                    for (int j = 0; j < column_offset; j++)
                        waddch(sview->win, ACS_HLINE);

                    waddch(sview->win, '>');
                    wattroff(sview->win, arrow_attr);
                    
                    break;
                case LINE_DISPLAY_HIGHLIGHT:
                    waddch(sview->win, ' ');
                    line_highlight_attr = highlight_attr;
                    break;
                case LINE_DISPLAY_BLOCK:
                    column_offset = get_line_leading_ws_count(
                        sview->cur->buf->tlines[line],
                        strlen(sview->cur->buf->tlines[line]),
                        highlight_tabstop);
                    column_offset -= (sview->cur->sel_col + 1);
                    if (column_offset < 0)
                        column_offset = 0;

                    /* Now actually draw the space to the block */
                    for (int j = 0; j < column_offset; j++)
                        waddch(sview->win, ' ');

                    /* Draw the block */
                    wattron(sview->win, block_attr);
                    waddch(sview->win, ' ');
                    wattroff(sview->win, block_attr);
                    break;
            }
        } else {
            waddch(sview->win, ' ');
        }

        /* Print the text */
        if (line < 0 || line >= count) {
            for (int j = 2 + lwidth; j < width; j++)
                waddch(sview->win, ' ');
        } else {
            hl_wprintw(sview->win, cur_line, width - lwidth - 2,
                    sview->cur->sel_col + column_offset, line_highlight_attr);
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

static int clamp_line(struct sviewer *sview, int line)
{
    if (line < 0)
        line = 0;
    if (line >= sbcount(sview->cur->buf->tlines))
        line = sbcount(sview->cur->buf->tlines) - 1;

    return line;
}

void source_vscroll(struct sviewer *sview, int offset)
{
    if (sview->cur) {
        sview->cur->sel_line = clamp_line(sview, sview->cur->sel_line + offset);
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
            sview->cur->sel_line = clamp_line(sview, line - 1);
        }

        sview->cur->sel_rline = sview->cur->sel_line;
    }
}

int source_set_exec_line(struct sviewer *sview, const char *path, int sel_line, int exe_line)
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
    if (sel_line--) {
        sview->cur->sel_line = clamp_line(sview, sel_line);

        /* Set executing line if passed a valid value */
        if (exe_line > 0)
            sview->cur->exe_line = clamp_line(sview, exe_line - 1);
    }

    return 0;
}

void source_free(struct sviewer *sview)
{
    /* Free all file buffers */
    while (sview->list_head != NULL)
        source_del(sview, sview->list_head->path);

    delwin(sview->win);
    sview->win = NULL;

    free(sview);
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
            free(sview->cur->buf->cur_line);
            sview->cur->buf->cur_line = NULL;
        }
        return -1;
    }

    if (!sbcount(sview->cur->orig_buf.tlines))
        load_file_buf(&sview->cur->orig_buf, sview->cur->path);

    return hl_regex(regex,
            (const char **) sview->cur->buf->tlines,
            (const char **) sview->cur->orig_buf.tlines,
            sbcount(sview->cur->buf->tlines),
            &sview->cur->buf->cur_line, &sview->cur->sel_line,
            &sview->cur->sel_rline, &sview->cur->sel_col_rbeg,
            &sview->cur->sel_col_rend, opt, direction, icase);
}

void source_enable_break(struct sviewer *sview, const char *path, 
        const char *fullname, int line, int enabled)
{
    struct list_node *node = 0;

    if (fullname) {
        node = get_node(sview, fullname);
    }

    if (!node && path) {
        node = get_node(sview, path);
    }

    if (!node || (!node->buf && load_file(node)))
        return;

    if (line > 0 && line < sbcount(node->lflags)) {
        node->lflags[line - 1].breakpt = (enabled == 0)?2:1;
    }
}

void source_clear_breaks(struct sviewer *sview)
{
    struct list_node *node;

    for (node = sview->list_head; node != NULL; node = node->next)
    {
        int i;
        for (i = 0; i < sbcount(node->lflags); i++)
            node->lflags[i].breakpt = 0;
    }
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
