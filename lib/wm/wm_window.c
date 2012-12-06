/* window.c:
 * ---------
 *
 * Window code for CGDB.  See header for full documentation.
 */

/* Standard Includes */
#include <assert.h>
#include <stdlib.h>
#include <string.h>

/* Local Includes */
#include "std_list.h"
#include "wm_splitter.h"
#include "wm_window.h"
#include "wm.h"

/* Default hook implementations - these may be replaced inside the window
 * structure by child widget hook functions. */
static int hook_destroy(wm_window *window);
static int hook_layout(wm_window *window);
static int hook_input(wm_window *window, int *data, int len);
static int hook_redraw(wm_window *window);
static char *hook_status_text(wm_window *window, size_t max_length);
static void hook_minimum_size(wm_window *window, int *height, int *width);

int
wm_window_init(wm_window *window)
{
    memset(window, 0, sizeof(wm_window));
    window->destroy = hook_destroy;
    window->layout = hook_layout;
    window->input = hook_input;
    window->redraw = hook_redraw;
    window->status_text = hook_status_text;
    window->minimum_size = hook_minimum_size;
    wm_window_show_status_bar(window, 1);

    return 0;
}

int
wm_window_destroy(wm_window *window)
{
    if (window != NULL) {
        delwin(window->cwindow);
        window->destroy(window);
        free(window);
    }

    return 0;
}

int
wm_window_set_context(wm_window *window, window_manager *wm,
                      wm_window *parent, WINDOW *cwindow)
{
    if (wm) {
        window->wm = wm;
    }
    if (parent) {
        window->parent = parent;
    }
    assert(cwindow != NULL);
    window->cwindow = cwindow;
}

int
wm_window_layout_event(wm_window *window)
{
    getbegyx(window->cwindow, window->top, window->left);
    getmaxyx(window->cwindow, window->height, window->width);
    window->real_height = window->height;
    window->real_width = window->width;
    if (window->show_status_bar) {
        window->height--;
    }
    window->layout(window);

    return 0;
}

int
wm_window_redraw(wm_window *window)
{
    int i;
    if (window->show_status_bar) {
        char *status = window->status_text(window, window->width);
        size_t status_len = status ? strlen(status) : 0;
        char fill = wm_is_focused(window->wm, window) ? '^' : ' ';
        char text[2] = { 0 };
        wattron(window->cwindow, WA_REVERSE);
        for (i = 0; i < window->width; ++i) {
            /* TODO: Allow spaces in status that don't get filled */
            if (i < status_len && status[i] != ' ') {
                text[0] = status[i];
            } else {
                text[0] = fill;
            }
            mvwprintw(window->cwindow, window->height, i, text);
        }
        wattroff(window->cwindow, WA_REVERSE);
        free(status);
    }
    window->redraw(window);

    return 0;
}

void
wm_window_show_status_bar(wm_window *window, int value)
{
    if (value != window->show_status_bar) {
        window->show_status_bar = value;
        if (window->cwindow) {
            wm_window_layout_event(window);
        }
    }
}

void
wm_window_dump(wm_window *window, FILE *out, int indent)
{
    int i;
    int ctop, cleft, cheight, cwidth;
    getbegyx(window->cwindow, ctop, cleft);
    getmaxyx(window->cwindow, cheight, cwidth);

    if (window->is_splitter) {
        wm_splitter *splitter = (wm_splitter *) window;
        for (i = 0; i < indent; ++i) {
            fprintf(out, " ");
        }
        fprintf(out, "+ Split: %s ", splitter->orientation ==
                WM_HORIZONTAL ? "Horizontal" : "Vertical", splitter);
    } else {
        for (i = 0; i < indent; ++i) {
            fprintf(out, " ");
        }
        fprintf(out, "- Window ", window);
    }
    fprintf(out, "(us: +%d+%d %dx%d, curses: +%d+%d %dx%d)\n",
            window->top, window->left, window->real_height, window->real_width,
            ctop, cleft, cheight, cwidth);
    if (window->is_splitter) {
        wm_splitter *splitter = (wm_splitter *) window;
        int i;
        for (i = 0; i < splitter->num_children; ++i) {
            wm_window_dump(splitter->children[i], out, indent + 2);
        }
    }
}

/* Default hook implementations */

static int
hook_destroy(wm_window *window)
{
    return 0;
}

static int
hook_layout(wm_window *window)
{
    return 0;
}

static int
hook_input(wm_window *window, int *data, int len)
{
    return 0;
}

static int
hook_redraw(wm_window *window)
{
    return 0;
}

static char *
hook_status_text(wm_window *window, size_t max_length)
{
    return NULL;
}

static void
hook_minimum_size(wm_window *window, int *height, int *width)
{
    assert(height != NULL);
    assert(width != NULL);
    *height = 1;
    *width = 1;
    if (window->show_status_bar) {
        *height += 1;
    }
}
