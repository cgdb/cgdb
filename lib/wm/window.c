/* window.c:
 * ---------
 *
 * Window code for CGDB.  See header for full documentation.
 */

/* Standard Includes */
#include <assert.h>
#include <stdlib.h>

/* Local Includes */
#include "window.h"

int
wm_window_init(wm_window *window, WINDOW *cwindow, wm_window *parent)
{
    window->cwindow = cwindow;
    window->parent = parent;
    window->is_splitter = 0;
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
wm_window_resize(wm_window *window)
{
    getbegyx(window->cwindow, window->top, window->left);
    getmaxyx(window->cwindow, window->height, window->width);
    if (window->show_status_bar) {
        window->height--;
    }
    if (window->resize) {
        window->resize(window);
    }

    return 0;
}

void
wm_window_show_status_bar(wm_window *window, int value)
{
    window->show_status_bar = value;
    getbegyx(window->cwindow, window->top, window->left);
    getmaxyx(window->cwindow, window->height, window->width);
    if (value) {
        window->height--;
    }
}
