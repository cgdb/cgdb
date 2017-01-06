#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif /* HAVE_STDLIB_H */

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#include "fs_util.h"

#include "sys_util.h"
#include "sys_win.h"
#include "filedlg.h"
#include "cgdb.h"
#include "cgdbrc.h"
#include "highlight.h"
#include "sources.h"
#include "kui_term.h"
#include "highlight_groups.h"

struct file_buffer {
    char **files;               /* Array containing file */
    int max_width;              /* Width of longest line in file */

    int sel_line;               /* Current line selected in file dialog */
    int sel_col;                /* Current column selected in file dialog */
    int sel_rline;              /* Current line used by regex */
};

struct filedlg {
    struct file_buffer *buf;    /* All of the widget's data ( files ) */
    struct hl_regex_info *last_hlregex;
    struct hl_regex_info *hlregex;
    SWINDOW *win;               /* Curses window */
    struct ibuf *G_line_number; /* Line number user wants to 'G' to */
};

static char regex_line[MAX_LINE];   /* The regex the user enters */
static int regex_line_pos;      /* The index into the current regex */
static int regex_search;        /* Currently searching text ? */
static int regex_direction;     /* Direction to search */

/* print_in_middle: Prints the message 'string' centered at line in win 
 * ----------------
 *
 *  win:    Curses window
 *  line:   The line to print the message at
 *  width:  The width of the window
 *  string: The message to print
 */
static void print_in_middle(SWINDOW *win, int line, int width, const char *string)
{
    int x, y;
    int j;
    int length = strlen(string);

    y = swin_getcury(win);
    x = swin_getcurx(win);

    x = (int) ((width - length) / 2);

    swin_wmove(win, line, 0);
    for (j = 0; j < x; j++)
        swin_waddch(win, ' ');

    swin_mvwprintw(win, line, x, "%s", string);

    for (j = x + length; j < width; j++)
        swin_waddch(win, ' ');
}

struct filedlg *filedlg_new(int pos_r, int pos_c, int height, int width)
{
    struct filedlg *fd;

    /* Allocate a new structure */
    fd = (struct filedlg *)cgdb_malloc(sizeof(struct filedlg));

    /* Initialize the structure */
    fd->win = swin_newwin(height, width, pos_r, pos_c);

    /* Initialize the buffer */
    fd->buf = (struct file_buffer *)cgdb_malloc(sizeof(struct file_buffer));

    fd->G_line_number = ibuf_init();
    fd->last_hlregex = NULL;
    fd->hlregex = NULL;
    fd->buf->files = NULL;
    fd->buf->max_width = 0;
    fd->buf->sel_line = 0;
    fd->buf->sel_col = 0;
    fd->buf->sel_rline = 0;

    return fd;
}

void filedlg_free(struct filedlg *fdlg)
{
    filedlg_clear(fdlg);

    ibuf_free(fdlg->G_line_number);

    hl_regex_free(&fdlg->last_hlregex);
    fdlg->last_hlregex = NULL;

    hl_regex_free(&fdlg->hlregex);
    fdlg->hlregex = NULL;

    swin_delwin(fdlg->win);
    fdlg->win = NULL;

    free(fdlg->buf);
    fdlg->buf = NULL;

    free(fdlg);
}

int filedlg_add_file_choice(struct filedlg *fd, const char *file_choice)
{
    int length;
    int index, i;
    int equal = 1;              /* Not set to 0, because 0 *is* equal */

    if (file_choice == NULL || *file_choice == '\0')
        return -1;

    /* Make sure file exists. If temp files are used to create an
     * executable, and the temp files are deleted, they pollute the
     * file open dialog with files you can't actually open.
     *
     * The downside to not showing them all is that a user might
     * not understand why certain files aren't showing up. O well.
     */
    if (file_choice[0] != '*') {
        if (fs_verify_file_exists(file_choice) == 0)
            return -4;
    }

    /* find index to insert by comparing:
     * Absolute paths go to the end
     * Relative paths go before the absolute paths 
     */
    for (i = 0; i < sbcount(fd->buf->files); i++) {
        /* Don't add duplicate entry's ... gdb outputs duplicates */
        if ((equal = strcmp(fd->buf->files[i], file_choice)) == 0)
            return -3;
        else if (equal < 0) {
            /* Inserting filename, stop before relative path */
            if ((file_choice[0] != '.' && file_choice[0] != '/')
                    && fd->buf->files[i][0] == '.')
                break;

            /* Inserting filename, stop before absolute path */
            if (file_choice[0] != '/' && fd->buf->files[i][0] == '/')
                break;

        } else if (equal > 0) { /* Found ( file_choice is greater ) */
            /* Inserting Absolute path, it goes to the end */
            if (file_choice[0] == '/' && fd->buf->files[i][0] != '/')
                continue;

            /* Inserting relative path, continue until before absolute or relative path */
            if (file_choice[0] == '.' && (fd->buf->files[i][0] != '.'
                            && fd->buf->files[i][0] != '/'))
                continue;

            break;
        }
    }

    index = i;

    sbpush(fd->buf->files, NULL);

    /* shift everything down and then insert into index */
    for (i = sbcount(fd->buf->files) - 1; i > index; i--)
        fd->buf->files[i] = fd->buf->files[i - 1];

    fd->buf->files[index] = cgdb_strdup(file_choice);

    if ((length = strlen(file_choice)) > fd->buf->max_width)
        fd->buf->max_width = length;

    return 0;
}

void filedlg_clear(struct filedlg *fd)
{
    int i;

    ibuf_clear(fd->G_line_number);

    for (i = 0; i < sbcount(fd->buf->files); i++)
        free(fd->buf->files[i]);

    sbfree(fd->buf->files);
    fd->buf->files = NULL;

    fd->buf->max_width = 0;
    fd->buf->sel_line = 0;
    fd->buf->sel_col = 0;
    fd->buf->sel_rline = 0;
}

static int clamp_line(struct filedlg *fd, int line)
{
    if (line < 0)
        line = 0;
    if (line >= sbcount(fd->buf->files))
        line = sbcount(fd->buf->files) - 1;

    return line;
}

static void filedlg_vscroll(struct filedlg *fd, int offset)
{
    if (fd->buf)
        fd->buf->sel_line = clamp_line(fd, fd->buf->sel_line + offset);
}

static void filedlg_hscroll(struct filedlg *fd, int offset)
{
    int lwidth;
    int max_width;
    int width;

    if (fd->buf) {
        width = swin_getmaxx(fd->win);

        lwidth = log10_uint(sbcount(fd->buf->files)) + 1;
        max_width = fd->buf->max_width - width + lwidth + 6;

        fd->buf->sel_col += offset;
        if (fd->buf->sel_col > max_width)
            fd->buf->sel_col = max_width;
        if (fd->buf->sel_col < 0)
            fd->buf->sel_col = 0;
    }
}

static void filedlg_set_sel_line(struct filedlg *fd, int line)
{
    if (fd->buf)
        fd->buf->sel_line = clamp_line(fd, line);
}

static void filedlg_search_regex_init(struct filedlg *fd)
{
    if (!fd || !fd->buf)
        return;

    /* Start searching at the beginning of the selected line */
    fd->buf->sel_rline = fd->buf->sel_line;
}

static int wrap_line(struct file_buffer *buffer, int line)
{
    int count = sbcount(buffer->files);

    if (line < 0)
        line = count - 1;
    else if (line >= count)
        line = 0;

    return line;
}

static int filedlg_search_regex(struct filedlg *fd, const char *regex,
        int opt, int direction, int icase)
{
    if (!fd || !fd->buf)
        return -1;

    if (regex && regex[0]) {
        int line;
        int line_end;
        int line_inc = direction ? +1 : -1;
        int line_start = fd->buf->sel_rline;

        line = wrap_line(fd->buf, line_start + line_inc);

        if (cgdbrc_get_int(CGDBRC_WRAPSCAN))
        {
            // Wrapping is on so stop at the line we started on.
            line_end = line_start;
        }
        else
        {
            // No wrapping. Stop at line 0 if searching down and last line
            // if searching up.
            line_end = direction ? 0 : sbcount(fd->buf->files) - 1;
        }

        for(;;) {
            int ret;
            int start, end;
            char *file = fd->buf->files[line];

            ret = hl_regex_search(&fd->hlregex, file, regex, icase, &start, &end);
            if (ret > 0) {
                /* Got a match */
                fd->buf->sel_line = line;

                /* Finalized match - move to this location */
                if (opt == 2) {
                    fd->buf->sel_rline = line;
                    fd->last_hlregex = fd->hlregex;
                    fd->hlregex = 0;
                }
                return 1;
            }

            line = wrap_line(fd->buf, line + line_inc);
            if (line == line_end)
                break;
        }
    }

    /* Nothing found - go back to original line */
    fd->buf->sel_line = fd->buf->sel_rline;
    return 0;
}

int filedlg_display(struct filedlg *fd)
{
    char fmt[16];
    int width, height;
    int lwidth;
    int file;
    int i;
    int statusbar;
    int arrow_attr;
    int count = sbcount(fd->buf->files);
    int hlsearch = cgdbrc_get_int(CGDBRC_HLSEARCH);
    static const char label[] = "Select a file or press q to cancel.";
    int inc_search_attr = hl_groups_get_attr(hl_groups_instance, HLG_INCSEARCH);
    int search_attr = hl_groups_get_attr(hl_groups_instance, HLG_SEARCH);

    swin_curs_set(0);

    statusbar = hl_groups_get_attr(hl_groups_instance, HLG_STATUS_BAR);
    arrow_attr = hl_groups_get_attr(hl_groups_instance, HLG_SELECTED_LINE_ARROW);

    /* Check that a file is loaded */
    if (fd == NULL || fd->buf == NULL || fd->buf->files == NULL) {
        swin_wrefresh(fd->win);
        return 0;
    }

    /* Initialize variables */
    height = swin_getmaxy(fd->win);
    width = swin_getmaxx(fd->win);

    /* The status bar and display line 
     * Fake the display function to think the height is 2 lines less */
    height -= 2;

    /* Set starting line number (center source file if it's small enough) */
    if (count < height)
        file = (count - height) / 2;
    else {
        file = fd->buf->sel_line - height / 2;
        if (file > count - height)
            file = count - height;
        else if (file < 0)
            file = 0;
    }

    /* Print 'height' lines of the file, starting at 'file' */
    lwidth = log10_uint(count) + 1;
    snprintf(fmt, sizeof(fmt), "%%%dd", lwidth);

    print_in_middle(fd->win, 0, width, label);

    swin_wmove(fd->win, 0, 0);

    for (i = 1; i < height + 1; i++, file++) {
        swin_wmove(fd->win, i, 0);

        /* Outside of filename, just finish drawing the vertical file */
        if (file < 0 || file >= count) {
            int j;

            for (j = 1; j < lwidth; j++)
                swin_waddch(fd->win, ' ');
            swin_waddch(fd->win, '~');

            swin_wattron(fd->win, SWIN_A_BOLD);
            swin_waddch(fd->win, SWIN_SYM_VLINE);
            swin_wattroff(fd->win, SWIN_A_BOLD);

            for (j = 2 + lwidth; j < width; j++)
                swin_waddch(fd->win, ' ');
            continue;
        }

        int x, y;
        char *filename = fd->buf->files[file];

        /* Mark the current file with an arrow */
        if (file == fd->buf->sel_line) {
            swin_wattron(fd->win, SWIN_A_BOLD);
            swin_wprintw(fd->win, fmt, file + 1);
            swin_wattroff(fd->win, SWIN_A_BOLD);

            swin_wattron(fd->win, arrow_attr);
            swin_waddch(fd->win, '-');
            swin_waddch(fd->win, '>');
            swin_wattroff(fd->win, arrow_attr);

        }
        else {
            /* Ordinary file */
            swin_wprintw(fd->win, fmt, file + 1);

            swin_wattron(fd->win, SWIN_A_BOLD);
            swin_waddch(fd->win, SWIN_SYM_VLINE);
            swin_wattroff(fd->win, SWIN_A_BOLD);

            swin_waddch(fd->win, ' ');
        }

        y = swin_getcury(fd->win);
        x = swin_getcurx(fd->win);

        hl_printline(fd->win, filename, strlen(filename),
                     NULL, -1, -1, fd->buf->sel_col, width - lwidth - 2);

        if (hlsearch && fd->last_hlregex) {
            struct hl_line_attr *attrs = hl_regex_highlight(
                    &fd->last_hlregex, filename, search_attr);

            if (sbcount(attrs)) {
                hl_printline_highlight(fd->win, filename, strlen(filename),
                             attrs, x, y, fd->buf->sel_col, width - lwidth - 2);
                sbfree(attrs);
            }
        }

        if (regex_search && file == fd->buf->sel_line) {
            struct hl_line_attr *attrs = hl_regex_highlight(
                    &fd->hlregex, filename, inc_search_attr);

            if (sbcount(attrs)) {
                hl_printline_highlight(fd->win, filename, strlen(filename),
                             attrs, x, y, fd->buf->sel_col, width - lwidth - 2);
                sbfree(attrs);
            }
        }
    }

    /* Add the 2 lines back in so the status bar can be drawn */
    height += 2;

    /* Update status bar */
    swin_wmove(fd->win, height, 0);

    /* Print white background */
    swin_wattron(fd->win, statusbar);

    for (i = 0; i < width; i++)
        swin_mvwprintw(fd->win, height - 1, i, " ");

    if (regex_search && regex_direction)
        swin_mvwprintw(fd->win, height - 1, 0, "Search:%s", regex_line);
    else if (regex_search)
        swin_mvwprintw(fd->win, height - 1, 0, "RSearch:%s", regex_line);

    swin_wattroff(fd->win, statusbar);

    swin_wmove(fd->win, height - (file - fd->buf->sel_line) - 1, lwidth + 2);
    swin_wrefresh(fd->win);

    return 0;
}

void filedlg_display_message(struct filedlg *fd, char *message)
{
    int height, width, i;
    int attr;

    attr = hl_groups_get_attr(hl_groups_instance, HLG_STATUS_BAR);

    height = swin_getmaxy(fd->win);
    width = swin_getmaxx(fd->win);

    /* Print white background */
    swin_wattron(fd->win, attr);

    for (i = 0; i < width; i++)
        swin_mvwprintw(fd->win, height - 1, i, " ");

    swin_mvwprintw(fd->win, height - 1, 0, "%s", message);
    swin_wattroff(fd->win, attr);
    swin_wrefresh(fd->win);
}

/* capture_regex: Captures a regular expression from the user.
 * ---------------
 *  Side Effect: 
 *
 *  regex_line: The regex the user has entered.
 *  regex_line_pos: The next available index into regex_line.
 *
 * Return Value: 0 if user gave a regex, otherwise 1.
 */
static int capture_regex(struct filedlg *fd)
{
    int c;
    extern struct kui_manager *kui_ctx;

    /* Initialize the function for finding a regex and tell user */
    regex_search = 1;
    regex_line_pos = 0;
    regex_line[regex_line_pos] = '\0';
    filedlg_display(fd);

    do {
        c = kui_manager_getkey_blocking(kui_ctx);

        if (regex_line_pos == (MAX_LINE - 1) && !(c == CGDB_KEY_ESC || c == 8 || c == 127))
            continue;

        /* Quit the search if the user hit escape */
        if (c == CGDB_KEY_ESC) {
            regex_line_pos = 0;
            regex_line[regex_line_pos] = '\0';
            regex_search = 0;
            filedlg_search_regex(fd, regex_line, 2, regex_direction, 1);
            filedlg_display(fd);
            return 1;
        }

        /* If the user hit enter, then a successful regex has been received */
        if (c == '\r' || c == '\n' || c == CGDB_KEY_CTRL_M) {
            regex_line[regex_line_pos] = '\0';
            break;
        }

        /* If the user hit backspace or delete remove a char */
        if (c == 8 || c == 127) {
            if (regex_line_pos > 0)
                --regex_line_pos;

            regex_line[regex_line_pos] = '\0';
            filedlg_search_regex(fd, regex_line, 1, regex_direction, 1);
            filedlg_display(fd);
            continue;
        }

        /* Add a char, search and draw */
        regex_line[regex_line_pos++] = c;
        regex_line[regex_line_pos] = '\0';
        filedlg_search_regex(fd, regex_line, 1, regex_direction, 1);
        filedlg_display(fd);
    } while (1);

    /* Finished */
    regex_search = 0;
    filedlg_search_regex(fd, regex_line, 2, regex_direction, 1);
    filedlg_display(fd);
    return 0;
}

int filedlg_recv_char(struct filedlg *fd, int key, char *file, int last_key_pressed)
{
    /* Initialize size variables */
    int height = swin_getmaxy(fd->win);

    filedlg_display(fd);

    switch (key) {
        case 'q':
            return -1;
            /* Vertical scrolling */
        case CGDB_KEY_DOWN:
        case 'j':
            filedlg_vscroll(fd, 1);
            break;
        case CGDB_KEY_NPAGE:
        case CGDB_KEY_CTRL_F:  /* VI-style page down */
            filedlg_vscroll(fd, height - 1);
            break;
        case CGDB_KEY_CTRL_D:  /* VI-style 1/2 page down */
            filedlg_vscroll(fd, height / 2);
            break;
        case CGDB_KEY_CTRL_U:  /* VI-style 1/2 page up */
            filedlg_vscroll(fd, -height / 2);
            break;
        case CGDB_KEY_UP:
        case 'k':
            filedlg_vscroll(fd, -1);
            break;
        case CGDB_KEY_PPAGE:
        case CGDB_KEY_CTRL_B:  /* VI-style page up */
            filedlg_vscroll(fd, -(height - 1));
            break;
            /* Horizontal scrolling */
        case CGDB_KEY_RIGHT:
        case 'l':
            filedlg_hscroll(fd, 1);
            break;
        case CGDB_KEY_LEFT:
        case 'h':
            filedlg_hscroll(fd, -1);
            break;
        case '/':
        case '?':
            regex_direction = ('/' == key);

            /* Capturing regular expressions */
            filedlg_search_regex_init(fd);
            capture_regex(fd);
            break;
        case 'n':
            filedlg_search_regex(fd, regex_line, 2, regex_direction, 1);
            break;
        case 'N':
            filedlg_search_regex(fd, regex_line, 2, !regex_direction, 1);
            break;
            /* User selected a file */
        case '\n':
        case '\r':
        case CGDB_KEY_CTRL_M:
            strcpy(file, fd->buf->files[fd->buf->sel_line]);
            return 1;

        case 'g':              /* beginning of file */
            if (last_key_pressed == 'g')
                filedlg_set_sel_line(fd, 0);
            break;
        case 'G': {             /* end of file, or a line number*/
            int lineno = -1, result;
            result = cgdb_string_to_int(ibuf_get(fd->G_line_number), &lineno);
            if (result == 0) {
                filedlg_set_sel_line(fd, lineno -1);
            }
            break;
        }
        default:
            break;
    }

    /* Store digits into G_line_number for 'G' command. */
    if (key >= '0' && key <= '9') {
        ibuf_addchar(fd->G_line_number, key);
    } else {
        ibuf_clear(fd->G_line_number);
    }

    filedlg_display(fd);

    return 0;
}
