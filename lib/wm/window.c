/* window.c:
 * ---------
 *
 * Window code for CGDB.  See header for full documentation.
 */

/* Standard Includes */
#include <assert.h>
#include <stdlib.h>

/* Local Includes */
#include "std_list.h"
#include "window.h"
#include "wm_splitter.h"

int
wm_window_init(wm_window *window, WINDOW *cwindow, wm_window *parent)
{
    window->cwindow = cwindow;
    window->parent = parent;
    window->is_splitter = 0;
    getmaxyx(window->cwindow, window->real_height, window->real_width);
    wm_window_show_status_bar(window, 1);

    if (window->init) {
        window->init(window);
    }

    return 0;
}

int
wm_window_destroy(wm_window *window)
{
    if (window != NULL) {
        delwin(window->cwindow);
        if (window->destroy) {
            window->destroy(window);
        }
        free(window);
    }

    return 0;
}

int
wm_window_redraw(wm_window *window)
{
    if (window->show_status_bar) {
        mvwprintw(window->cwindow, window->height, 0, "-- Status bar --");
        wclrtoeol(window->cwindow);
    }
    if (window->redraw) {
        window->redraw(window);
    }

    return 0;
}

int
wm_window_resize_event(wm_window *window)
{
    getbegyx(window->cwindow, window->top, window->left);
    getmaxyx(window->cwindow, window->height, window->width);
    window->real_height = window->height;
    window->real_width = window->width;
    if (window->show_status_bar) {
        window->height--;
    }
    if (window->resize) {
        window->resize(window);
    }

    return 0;
}

int
wm_window_place(wm_window *window, int top, int left, int height, int width)
{
    /* Note: Assumes resizes are always smaller or bigger, not (smaller height
     * larger width). Curses window operations will fail if the window tries
     * to grow or move into a space that is out of bounds, so order is
     * important. */
    wresize(window->cwindow, height, width);
    mvwin(window->cwindow, top, left);
    wresize(window->cwindow, height, width);
    mvwin(window->cwindow, top, left);

    wm_window_resize_event(window);
}

void
wm_window_show_status_bar(wm_window *window, int value)
{
    window->show_status_bar = value;
    getbegyx(window->cwindow, window->top, window->left);
    getmaxyx(window->cwindow, window->height, window->width);
    window->real_height = window->height;
    window->real_width = window->width;
    if (value) {
        window->height--;
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
