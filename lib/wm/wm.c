/* wm.c:
 * -----
 *
 * Window manager code for CGDB.  See header for full documentation.
 */

/* Standard Includes */
#include <assert.h>

/* Project Includes */
#include <std_list.h>
#include <std_btree.h>

/* Local Includes */
#include "wm.h"

/**
 * The window manager structure.  This is passed to every method in the
 * interface, and tracks all the state information associated with windows
 * management.
 */
struct window_manager_s {

    wm_window *main_window;

    wm_window *focused_window;

    /* TODO: Command line widget, options, ... */

};

window_manager *wm_create(wm_window *main_window)
{
    WINDOW *cwindow = NULL;
    window_manager *wm = NULL;
    int y, x;

    /* Allocate a new window manager and initial window */
    wm = (window_manager *) malloc(sizeof(window_manager));
    if (!wm) {
        return NULL;
    }

    wm->main_window = main_window;
    wm->focused_window = main_window;

    /* TODO: Factor command height into the new window's height. */
    getmaxyx(stdscr, y, x);
    cwindow = subwin(stdscr, y-2, x, 0, 0);

    window_init(main_window, cwindow);

    return wm;
}

int wm_destroy(window_manager *wm)
{
    if (wm) {
        window_destroy(wm->main_window);
        free(wm);
    }

    return 0;
}

int wm_redraw(window_manager *wm)
{
    /* Not implemented */
    return -1;
}

int wm_split(wm_window *window, wm_direction orientation)
{
    /* Not implemented */
    return -1;
}

int wm_close(wm_window *window)
{
    /* Not implemented */
    return -1;
}

wm_optval wm_option_get(wm_option option)
{
    wm_optval rv;

    /* Not implemented */
    rv.type = WM_UNKNOWN;

    return rv;
}

int wm_option_set(wm_option option, wm_optval value)
{
    /* Not implemented */
    return -1;
}
