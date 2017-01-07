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

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

/* Local Includes */
#include "sys_util.h"
#include "sys_win.h"
#include "cgdb.h"
#include "highlight.h"
#include "tokenizer.h"
#include "sources.h"
#include "logo.h"
#include "fs_util.h"
#include "cgdbrc.h"
#include "highlight_groups.h"
#include "interface.h"
#include "tgdb_types.h"

int sources_syntax_on = 1;

// This speeds up loading sqlite.c from 2:48 down to ~2 seconds.
// sqlite3 is 6,596,401 bytes, 188,185 lines.

/* --------------- */
/* Local Functions */
/* --------------- */

/* source_get_node:  Returns a pointer to the node that matches the given path.
 * ---------
 *   path:  Full path to source file
 *
 * Return Value:  Pointer to the matching node, or NULL if not found.
 */
struct list_node *source_get_node(struct sviewer *sview, const char *path)
{
    if (sview && path && path[0]) {
        struct list_node *cur;

        for (cur = sview->list_head; cur != NULL; cur = cur->next) {
            if (cur->path && (strcmp(path, cur->path) == 0))
                return cur;
        }
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
    int val;
    struct stat s;

    /* Special buffer not backed by file */
    if (path[0] == '*') {
        *timestamp = 0;
        return 0;
    }

    val = path ? stat(path, &s) : -1;
    *timestamp = val ? 0 : s.st_mtime;
    return val;
}

static void init_file_buffer(struct buffer *buf)
{
    buf->lines = NULL;
    buf->addrs = NULL;
    buf->max_width = 0;
    buf->file_data = NULL;
    buf->tabstop = cgdbrc_get_int(CGDBRC_TABSTOP);
    buf->language = TOKENIZER_LANGUAGE_UNKNOWN;
}

static void release_file_buffer(struct buffer *buf)
{
    if (buf) {
        int i;

        for (i = 0; i < sbcount(buf->lines); i++) {
            sbfree(buf->lines[i].attrs);
            buf->lines[i].attrs = NULL;

            if (!buf->file_data) {
                sbfree(buf->lines[i].line);
                buf->lines[i].line = NULL;
            }
        }

        /* Free entire file buffer */
        sbfree(buf->file_data);
        buf->file_data = NULL;

        sbfree(buf->lines);
        buf->lines = NULL;

        sbfree(buf->addrs);
        buf->addrs = NULL;

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
    release_file_buffer(&node->file_buf);

    return 0;
}

static char *detab_buffer(char *buffer, int tabstop)
{
    int i;
    int dst = 0;
    char *newbuf = NULL;
    int size = sbcount(buffer);

    char *tab = strchr(buffer, '\t');
    if (!tab)
        return buffer;

    for (i = 0; i < size; i++) {
        if (buffer[i] == '\t') {
            int spaces = tabstop - dst % tabstop;

            while(spaces--) {
                sbpush(newbuf, ' ');
                dst++;
            }
        } else {
            sbpush(newbuf, buffer[i]);
            dst++;
        }

        if (buffer[i] == '\n' || buffer[i] == '\r')
            dst = 0;
    }

    sbfree(buffer);
    return newbuf;
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
 * 0 on sucess, -1 on error
 */
static int load_file_buf(struct buffer *buf, const char *filename)
{
    FILE *file;
    long file_size;
    int ret = -1;

    /* Special buffer not backed by file */
    if (filename[0] == '*')
        return 0;

    file = fopen(filename, "rb");
    if (!file)
        return -1;

    file_size = get_file_size(file);
    if (file_size > 0) {
        size_t bytes_read;

        /* Set the stretchy buffer size to our file size plus one for nil */
        sbsetcount(buf->file_data, file_size + 1);

        /* Read in the entire file */
        bytes_read = fread(buf->file_data, 1, file_size, file);

        /* If we had a partial read, bail */
        if (bytes_read != file_size) {
            sbfree(buf->file_data);
            buf->file_data = NULL;

            fclose(file);
            return -1;
        }

        /* Zero terminate buffer */
        buf->file_data[bytes_read] = 0;

        /* Convert tabs to spaces */
        buf->tabstop = cgdbrc_get_int(CGDBRC_TABSTOP);
        buf->file_data = detab_buffer(buf->file_data, buf->tabstop);

        {
            char *line_start = buf->file_data;
            char *line_feed = strchr(line_start, '\n');

            while (line_feed)
            {
                size_t line_len;
                char *line_end = line_feed;

                /* Trim trailing cr-lfs */
                while (line_end >= line_start && (*line_end == '\n' || *line_end == '\r'))
                    line_end--;

                /* Update max length string found */
                line_len = line_end - line_start + 1;
                if (line_len > buf->max_width)
                    buf->max_width = line_len;

                struct source_line sline;
                sline.line = line_start;
                sline.len = line_len;
                sline.attrs = NULL;

                /* Add this line to lines array */
                sbpush(buf->lines, sline);

                line_start = line_feed + 1;
                line_feed = strchr(line_start, '\n');
            }

            if (*line_start) {
                struct source_line sline;
                sline.line = line_start;
                sline.len = strlen(line_start);
                sline.attrs = NULL;

                sbpush(buf->lines, sline);
            }

            ret = 0;
        }
    }

    fclose(file);
    return ret;
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
    /* No node pointer? */
    if (!node)
        return -1;

    /* File already loaded - success! */
    if (node->file_buf.lines)
        return 0;

    /* Stat the file to get the timestamp */
    if (get_timestamp(node->path, &(node->last_modification)) == -1)
        return -1;

    node->language = tokenizer_get_default_file_type(strrchr(node->path, '.'));

    /* Add the highlighted lines */
    return source_highlight(node);
}

/* --------- */
/* Functions */
/* --------- */

/* Descriptive comments found in header file: sources.h */

/* Returns HLG_LAST on error */
static enum hl_group_kind hlg_from_tokenizer_type(enum tokenizer_type type, const char *tok_data)
{
    switch(type) {
        case TOKENIZER_KEYWORD: return HLG_KEYWORD;
        case TOKENIZER_TYPE: return HLG_TYPE;
        case TOKENIZER_LITERAL: return HLG_LITERAL;
        case TOKENIZER_NUMBER: return HLG_TEXT;
        case TOKENIZER_COMMENT: return HLG_COMMENT;
        case TOKENIZER_DIRECTIVE: return HLG_DIRECTIVE;
        case TOKENIZER_TEXT: return HLG_TEXT;
        case TOKENIZER_NEWLINE: return HLG_LAST;
        case TOKENIZER_ERROR: return HLG_TEXT;
        case TOKENIZER_SEARCH: return HLG_SEARCH;
        case TOKENIZER_STATUS_BAR: return HLG_STATUS_BAR;
        case TOKENIZER_EXECUTING_LINE_ARROW: return HLG_EXECUTING_LINE_ARROW;
        case TOKENIZER_SELECTED_LINE_ARROW: return HLG_SELECTED_LINE_ARROW;
        case TOKENIZER_EXECUTING_LINE_HIGHLIGHT: return HLG_EXECUTING_LINE_HIGHLIGHT;
        case TOKENIZER_SELECTED_LINE_HIGHLIGHT: return HLG_SELECTED_LINE_HIGHLIGHT;
        case TOKENIZER_EXECUTING_LINE_BLOCK: return HLG_EXECUTING_LINE_BLOCK;
        case TOKENIZER_SELECTED_LINE_BLOCK: return HLG_SELECTED_LINE_BLOCK;
        case TOKENIZER_ENABLED_BREAKPOINT: return HLG_ENABLED_BREAKPOINT;
        case TOKENIZER_DISABLED_BREAKPOINT: return HLG_DISABLED_BREAKPOINT;
        case TOKENIZER_SELECTED_LINE_NUMBER: return HLG_SELECTED_LINE_NUMBER;
        case TOKENIZER_SCROLL_MODE_STATUS: return HLG_SCROLL_MODE_STATUS;
        case TOKENIZER_LOGO: return HLG_LOGO;
        case TOKENIZER_COLOR: return hl_get_color_group(tok_data);
    }

    return HLG_TEXT;
}

static int highlight_node(struct list_node *node)
{
    int i;
    int ret;
    int line = 0;
    int length = 0;
    int lasttype = -1;
    struct token_data tok_data;
    struct tokenizer *t = tokenizer_init();
    struct buffer *buf = &node->file_buf;

    for (i = 0; i < sbcount(buf->lines); i++) {
        sbfree(buf->lines[i].attrs);
        buf->lines[i].attrs = NULL;
    }

    if (!buf->file_data) {
        for (line = 0; line < sbcount(buf->lines); line++) {
            struct source_line *sline = &buf->lines[line];

            tokenizer_set_buffer(t, sline->line, buf->language);

            length = 0;
            lasttype = -1;
            while ((ret = tokenizer_get_token(t, &tok_data)) > 0) {
                if (tok_data.e == TOKENIZER_NEWLINE)
                    break;

                enum hl_group_kind hlg = hlg_from_tokenizer_type(tok_data.e, tok_data.data);

                /* Add attribute if highlight group has changed */
                if (lasttype != hlg) {
                    struct hl_line_attr line_attr;

                    line_attr.col = length;
                    line_attr.attr = hl_groups_get_attr(
                        hl_groups_instance, hlg);

                    sbpush(buf->lines[line].attrs, line_attr);

                    lasttype = hlg;
                }

                /* Add the text and bump our length */
                length += strlen(tok_data.data);
            }
        }

    } else {
        if (tokenizer_set_buffer(t, buf->file_data, buf->language) == -1) {
            if_print_message("%s:%d tokenizer_set_buffer error", __FILE__, __LINE__);
            return -1;
        }

        while ((ret = tokenizer_get_token(t, &tok_data)) > 0) {
            if (tok_data.e == TOKENIZER_NEWLINE) {
                if (length > buf->max_width)
                    buf->max_width = length;

                length = 0;
                lasttype = -1;
                line++;
            } else {
                enum hl_group_kind hlg = hlg_from_tokenizer_type(tok_data.e, tok_data.data);

                if (hlg == HLG_LAST) {
                    clog_error(CLOG_CGDB, "Bad hlg_type for '%s', e==%d\n", tok_data.data, tok_data.e);
                    hlg = HLG_TEXT;
                }

                /* Add attribute if highlight group has changed */
                if (lasttype != hlg) {
                    struct hl_line_attr line_attr;

                    line_attr.col = length;
                    line_attr.attr = hl_groups_get_attr(
                        hl_groups_instance, hlg);

                    sbpush(buf->lines[line].attrs, line_attr);

                    lasttype = hlg;
                }

                /* Add the text and bump our length */
                length += strlen(tok_data.data);
            }
        }
    }

    tokenizer_destroy(t);
    return 0;
}

int source_highlight(struct list_node *node)
{
    int do_color = sources_syntax_on &&
                   (node->language != TOKENIZER_LANGUAGE_UNKNOWN) &&
                   swin_has_colors();

    /* Load the entire file */
    if (!sbcount(node->file_buf.lines))
        load_file_buf(&node->file_buf, node->path);

    /* If we're doing color and we haven't already loaded this file
     * with this language, then load and highlight it.
     */
    if (do_color && (node->file_buf.language != node->language)) {
        node->file_buf.language = node->language;
        highlight_node(node);
    }

    /* Allocate the breakpoints array */
    if (!node->lflags) {
        int count = sbcount(node->file_buf.lines);
        sbsetcount(node->lflags, count);

        memset(node->lflags, 0, sbcount(node->lflags));
    }

    if (node->file_buf.lines)
        return 0;

    return -1;
}

struct sviewer *source_new(SWINDOW *win)
{
    struct sviewer *rv;

    /* Allocate a new structure */
    rv = (struct sviewer *)cgdb_malloc(sizeof (struct sviewer));

    /* Initialize the structure */
    rv->win = win;
    rv->cur = NULL;
    rv->list_head = NULL;

    /* Initialize global marks */
    memset(rv->global_marks, 0, sizeof(rv->global_marks));
    rv->jump_back_mark.node = NULL;
    rv->jump_back_mark.line = -1;

    rv->addr_frame = 0;

    rv->hlregex = NULL;
    rv->last_hlregex = NULL;

    return rv;
}

struct list_node *source_add(struct sviewer *sview, const char *path)
{
    struct list_node *new_node;

    new_node = source_get_node(sview, path);
    if (new_node)
        return new_node;

    new_node = (struct list_node *)cgdb_malloc(sizeof (struct list_node));
    new_node->path = strdup(path);

    init_file_buffer(&new_node->file_buf);

    new_node->lflags = NULL;
    new_node->sel_line = -1;
    new_node->sel_col = 0;
    new_node->sel_rline = 0;
    new_node->exe_line = -1;
    new_node->last_modification = 0;    /* No timestamp yet */
    new_node->language = TOKENIZER_LANGUAGE_UNKNOWN;
    new_node->addr_start = 0;
    new_node->addr_end = 0;

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

    return new_node;
}

void source_add_disasm_line(struct list_node *node, const char *line)
{
    uint64_t addr = 0;
    struct source_line sline;
    char *colon = 0, colon_char = 0;

    sline.line = NULL;
    sbsetcount(sline.line, strlen(line) + 1);
    strcpy(sline.line, line);
    sline.line = detab_buffer(sline.line, node->file_buf.tabstop);

    sline.attrs = NULL;
    sline.len = sbcount(sline.line);

    colon = strchr((char*)line, ':');
    if (colon) {
        colon_char = *colon;
        *colon = 0;
    }

    cgdb_hexstr_to_u64(line, &addr);

    if (colon) {
        *colon = colon_char;
    }

    sbpush(node->file_buf.addrs, addr);

    struct line_flags lf = { 0, 0 };
    sbpush(node->file_buf.lines, sline);
    sbpush(node->lflags, lf);
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
    release_file_buffer(&cur->file_buf);

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
    struct list_node *cur = source_get_node(sview, path);

    /* Load the file if it's not already */
    if (load_file(cur))
        return -1;

    return sbcount(cur->file_buf.lines);
}

char *source_current_file(struct sviewer *sview)
{
    return (sview && sview->cur) ? sview->cur->path : NULL;
}

/* source_get_mark_char:  Return mark char for line.
 * --------------------
 *
 *   sview:  The source viewer object
 *   node: 
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

static int get_line_leading_ws_count(const char *otext, int length)
{
    int i;
    int column_offset = 0; /* Text to skip due to arrow */

    for (i = 0; i < length - 1; i++) {
        /* Bail if we hit a non whitespace character */
        if (!isspace(otext[i]))
            break;

        column_offset++;
    }

    return column_offset;
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
    int i;
    int lwidth;
    int line;
    int count;

    enum LineDisplayStyle exe_display_style, sel_display_style;
    int sellineno, exelineno;
    int enabled_bp, disabled_bp;
    int exe_line_display_is_arrow, sel_line_display_is_arrow;
    int exe_arrow_attr, sel_arrow_attr;
    int exe_highlight_attr, sel_highlight_attr;
    int exe_block_attr, sel_block_attr;
    int search_attr;
    int inc_search_attr;
    char fmt[16];
    int width, height;
    int focus_attr = focus ? SWIN_A_BOLD : 0;
    int showmarks = cgdbrc_get_int(CGDBRC_SHOWMARKS);
    int hlsearch = cgdbrc_get_int(CGDBRC_HLSEARCH);
    int mark_attr;

    struct hl_line_attr *sel_highlight_attrs = 0;
    struct hl_line_attr *exe_highlight_attrs = 0;
    struct hl_line_attr tmp_attr;

    /* Check that a file is loaded */
    if (!sview->cur || !sview->cur->file_buf.lines) {
        logo_display(sview->win);

        if (dorefresh == WIN_REFRESH)
            swin_wrefresh(sview->win);
        else
            swin_wnoutrefresh(sview->win);

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

    search_attr = hl_groups_get_attr(hl_groups_instance, HLG_SEARCH);
    inc_search_attr = hl_groups_get_attr(hl_groups_instance, HLG_INCSEARCH);

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

    tmp_attr.col = 0;
    tmp_attr.attr = sel_highlight_attr;
    sbpush(sel_highlight_attrs, tmp_attr);

    tmp_attr.col = 0;
    tmp_attr.attr = exe_highlight_attr;
    sbpush(exe_highlight_attrs, tmp_attr);
    
    /* Make sure cursor is visible */
    swin_curs_set(!!focus);

    /* Initialize variables */
    height = swin_getmaxy(sview->win);
    width = swin_getmaxx(sview->win);

    /* Set starting line number (center source file if it's small enough) */
    count = sbcount(sview->cur->file_buf.lines);
    if (count < height) {
        line = (count - height) / 2;
    } else {
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
        /* Is this the current selected line? */
        int is_sel_line = (line >= 0 && sview->cur->sel_line == line);
        /* Is this the current executing line */
        int is_exe_line = (line >= 0 && sview->cur->exe_line == line);
        struct source_line *sline = (line < 0 || line >= count)?
            NULL:&sview->cur->file_buf.lines[line];
        struct hl_line_attr *printline_attrs = (sline)?sline->attrs:0;

        swin_wmove(sview->win, i, 0);

        /* Print the line number */
        if (line < 0 || line >= count) {
            for (int j = 1; j < lwidth; j++)
                swin_waddch(sview->win, ' ');
            swin_waddch(sview->win, '~');
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

            swin_wattron(sview->win, line_attr);
            swin_wprintw(sview->win, fmt, line + 1);
            swin_wattroff(sview->win, line_attr);
        }

        if (!swin_has_colors()) {
            /* TODO:
            swin_wprintw(sview->win, "%.*s\n",
                    sview->cur->file_buf.lines[line].line,
                    sview->cur->file_buf.lines[line].len);
            */
            continue;
        }

        /* Print the vertical bar or mark */
        {
            SWIN_CHTYPE vert_bar_char;
            int vert_bar_attr;
            int mc;

            if (showmarks &&
                ((mc = source_get_mark_char(sview, sview->cur, line)) > 0)) {
                vert_bar_char = mc;
                vert_bar_attr = mark_attr;
            } else if (is_exe_line && exe_line_display_is_arrow) {
                vert_bar_attr = exe_arrow_attr;
                vert_bar_char = SWIN_SYM_LTEE;
            } else if (is_sel_line && sel_line_display_is_arrow) {
                vert_bar_attr = sel_arrow_attr;
                vert_bar_char = SWIN_SYM_LTEE;
            } else {
                vert_bar_attr = focus_attr;
                vert_bar_char = SWIN_SYM_VLINE;
            }

            swin_wattron(sview->win, vert_bar_attr);
            swin_waddch(sview->win, vert_bar_char);
            swin_wattroff(sview->win, vert_bar_attr);
        }

        /* Print the marker */
        if (is_exe_line || is_sel_line) {
            enum LineDisplayStyle display_style;
            int arrow_attr, block_attr;
            struct hl_line_attr *highlight_attr;

            if (is_exe_line) {
                display_style = exe_display_style;
                arrow_attr = exe_arrow_attr;
                block_attr = exe_block_attr;
                highlight_attr = exe_highlight_attrs;
            } else {
                display_style = sel_display_style;
                arrow_attr = sel_arrow_attr;
                block_attr = sel_block_attr;
                highlight_attr = sel_highlight_attrs;
            }

            switch (display_style) {
                case LINE_DISPLAY_SHORT_ARROW:
                    swin_wattron(sview->win, arrow_attr);
                    swin_waddch(sview->win, '>');
                    swin_wattroff(sview->win, arrow_attr);
                    break;
                case LINE_DISPLAY_LONG_ARROW:
                    swin_wattron(sview->win, arrow_attr);
                    column_offset = get_line_leading_ws_count(
                        sline->line, sline->len);
                    column_offset -= (sview->cur->sel_col + 1);
                    if (column_offset < 0)
                        column_offset = 0;

                    /* Now actually draw the arrow */
                    for (int j = 0; j < column_offset; j++)
                        swin_waddch(sview->win, SWIN_SYM_HLINE);

                    swin_waddch(sview->win, '>');
                    swin_wattroff(sview->win, arrow_attr);

                    break;
                case LINE_DISPLAY_HIGHLIGHT:
                    swin_waddch(sview->win, ' ');
                    printline_attrs = highlight_attr;
                    break;
                case LINE_DISPLAY_BLOCK:
                    column_offset = get_line_leading_ws_count(
                        sline->line, sline->len);
                    column_offset -= (sview->cur->sel_col + 1);
                    if (column_offset < 0)
                        column_offset = 0;

                    /* Now actually draw the space to the block */
                    for (int j = 0; j < column_offset; j++)
                        swin_waddch(sview->win, ' ');

                    /* Draw the block */
                    swin_wattron(sview->win, block_attr);
                    swin_waddch(sview->win, ' ');
                    swin_wattroff(sview->win, block_attr);
                    break;
            }
        } else {
            swin_waddch(sview->win, ' ');
        }


        /* Print the text */
        if (line < 0 || line >= count) {
            for (int j = 2 + lwidth; j < width; j++)
                swin_waddch(sview->win, ' ');
        } else {
            int x, y;
            y = swin_getcury(sview->win);
            x = swin_getcurx(sview->win);

            hl_printline(sview->win, sline->line, sline->len,
                printline_attrs, -1, -1, sview->cur->sel_col + column_offset,
                width - lwidth - 2);

            if (hlsearch && sview->last_hlregex) {
                struct hl_line_attr *attrs = 
                    hl_regex_highlight(&sview->last_hlregex, sline->line,
                    search_attr);
                if (sbcount(attrs)) {
                    hl_printline_highlight(sview->win, sline->line, sline->len,
                        attrs, x, y, sview->cur->sel_col + column_offset,
                        width - lwidth - 2);
                    sbfree(attrs);
                }
            }

            if (is_sel_line && sview->hlregex) {
                struct hl_line_attr *attrs = 
                    hl_regex_highlight(&sview->hlregex, sline->line,
                    inc_search_attr);
                if (sbcount(attrs)) {
                    hl_printline_highlight(sview->win, sline->line, sline->len,
                        attrs, x, y, sview->cur->sel_col + column_offset,
                        width - lwidth - 2);
                    sbfree(attrs);
                }
            }
        }
    }

    switch(dorefresh) {
        case WIN_NO_REFRESH:
            swin_wnoutrefresh(sview->win);
            break;
        case WIN_REFRESH:
            swin_wrefresh(sview->win);
            break;
    }

    sbfree(sel_highlight_attrs);
    sbfree(exe_highlight_attrs);

    return 0;
}

void source_move(struct sviewer *sview, SWINDOW *win)
{
    swin_delwin(sview->win);
    sview->win = win;
}

static int clamp_line(struct sviewer *sview, int line)
{
    if (line < 0)
        line = 0;
    if (line >= sbcount(sview->cur->file_buf.lines))
        line = sbcount(sview->cur->file_buf.lines) - 1;

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
        height = swin_getmaxy(sview->win);
        width = swin_getmaxx(sview->win);

        lwidth = log10_uint(sbcount(sview->cur->file_buf.lines)) + 1;
        max_width = sview->cur->file_buf.max_width - width + lwidth + 6;

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
            sview->cur->sel_line = sbcount(sview->cur->file_buf.lines) - 1;
        } else {
            /* Set line (note correction for 0-based line counting) */
            sview->cur->sel_line = clamp_line(sview, line - 1);
        }

        sview->cur->sel_rline = sview->cur->sel_line;
    }
}

static struct list_node *source_get_asmnode(struct sviewer *sview,
    uint64_t addr, int *line)
{
    struct list_node *node = NULL;

    if (addr)
    {
        /* Search for a node which contains this address */
        for (node = sview->list_head; node; node = node->next)
        {
            if (addr >= node->addr_start && addr <= node->addr_end)
                break;
        }
    }

    if (node && line)
    {
        int i;

        for (i = 0; i < sbcount(node->file_buf.addrs); i++)
        {
            if (node->file_buf.addrs[i] == addr)
            {
                *line = i;
                break;
            }
        }
    }

    return node;
}

int source_set_exec_line(struct sviewer *sview, const char *path, int sel_line, int exe_line)
{
    if (path) {
        /* If they passed us a path, try to locate that node */
        sview->cur = source_get_node(sview, path);

        /* Not found.... */
        if (!sview->cur) {
            /* Check that the file exists */
            if (!fs_verify_file_exists(path))
                return 5;

            /* Add a new node for this file */
            sview->cur = source_add(sview, path);
        }
    }

    /* Buffer the file if it's not already */
    if (load_file(sview->cur))
        return 4;

    /* Update line, if set */
    if (sel_line > 0)
        sview->cur->sel_line = clamp_line(sview, sel_line - 1);

    /* Set executing line if passed a valid value */
    if (exe_line == -1) {
        sview->cur->exe_line = -1;
    } else if (exe_line > 0) {
        sview->cur->exe_line = clamp_line(sview, exe_line - 1);
    }

    return 0;
}

int source_set_exec_addr(struct sviewer *sview, uint64_t addr)
{
    int line = -1;

    if (!addr)
        addr = sview->addr_frame;

    /* Search for a node which contains this address */
    sview->cur = source_get_asmnode(sview, addr, &line);
    if (!sview->cur)
        return -1;

    sview->cur->sel_line = clamp_line(sview, line);
    sview->cur->exe_line = clamp_line(sview, line);
    return 0;
}

void source_free(struct sviewer *sview)
{
    /* Free all file buffers */
    while (sview->list_head)
        source_del(sview, sview->list_head->path);

    hl_regex_free(&sview->hlregex);
    sview->hlregex = NULL;
    hl_regex_free(&sview->last_hlregex);
    sview->last_hlregex = NULL;

    swin_delwin(sview->win);
    sview->win = NULL;

    free(sview);
}

void source_search_regex_init(struct sviewer *sview)
{
    if (!sview || !sview->cur)
        return;

    /* Start searching at the beginning of the selected line */
    sview->cur->sel_rline = sview->cur->sel_line;
}

static int wrap_line(struct list_node *node, int line)
{
    int count = sbcount(node->file_buf.lines);

    if (line < 0)
        line = count - 1;
    else if (line >= count)
        line = 0;

    return line;
}

int source_search_regex(struct sviewer *sview,
        const char *regex, int opt, int direction, int icase)
{
    struct list_node *node = sview ? sview->cur : NULL;

    if (!node)
        return -1;

    if (regex && *regex) {
        int line;
        int line_end;
        int line_inc = direction ? +1 : -1;
        int line_start = node->sel_rline;

        line = wrap_line(node, line_start + line_inc);

        if (cgdbrc_get_int(CGDBRC_WRAPSCAN))
        {
            // Wrapping is on so stop at the line we started on.
            line_end = line_start;
        }
        else
        {
            // No wrapping. Stop at line 0 if searching down and last line
            // if searching up.
            line_end = direction ? 0 : sbcount(node->file_buf.lines) - 1;
        }

        for(;;) {
            int ret;
            int start, end;
            char *line_str = node->file_buf.lines[line].line;

            ret = hl_regex_search(&sview->hlregex, line_str, regex, icase, &start, &end);
            if (ret > 0) {
                /* Got a match */
                node->sel_line = line;

                /* Finalized match - move to this location */
                if (opt == 2) {
                    node->sel_rline = line;

                    hl_regex_free(&sview->last_hlregex);
                    sview->last_hlregex = sview->hlregex;
                    sview->hlregex = 0;
                }
                return 1;
            }

            line = wrap_line(node, line + line_inc);
            if (line == line_end)
                break;
        }
    }

    /* Nothing found - go back to original line */
    node->sel_line = node->sel_rline;
    return 0;
}

static void source_clear_breaks(struct sviewer *sview)
{
    struct list_node *node;

    for (node = sview->list_head; node != NULL; node = node->next)
    {
        int i;
        for (i = 0; i < sbcount(node->lflags); i++)
            node->lflags[i].breakpt = 0;
    }
}

void source_set_breakpoints(struct sviewer *sview,
        struct tgdb_breakpoint *breakpoints)
{
    int i;
    struct list_node *node;

    source_clear_breaks(sview);

    for (i = 0; i < sbcount(breakpoints); i++) {
        if (breakpoints[i].path) {
            node = source_get_node(sview, breakpoints[i].path);
            if (!load_file(node)) {
                int line = breakpoints[i].line;
                int enabled = breakpoints[i].enabled;
                if (line > 0 && line <= sbcount(node->lflags)) {
                    node->lflags[line - 1].breakpt = enabled ? 1 : 2;
                }
            }
        } else if (breakpoints[i].addr) {
            int line = 0;
            node = source_get_asmnode(sview, breakpoints[i].addr, &line);
            if (node) {
                node->lflags[line].breakpt = breakpoints[i].enabled ? 1 : 2;
            }
        }
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

    /* If the file timestamp or tab size changed, reload the file */
    int dirty = cur->last_modification < timestamp;
    dirty |= cgdbrc_get_int(CGDBRC_TABSTOP) != cur->file_buf.tabstop;

    if ((auto_source_reload || force) && dirty) {

        if (release_file_memory(sview->cur) == -1)
            return -1;

        if (load_file(cur))
            return -1;
    }

    return 0;
}
