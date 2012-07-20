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
#include "wm_splitter.h"

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

    getmaxyx(stdscr, y, x);
    cwindow = derwin(stdscr, y-1, x, 0, 0);

    wm_window_init(main_window, cwindow, NULL);
    wm_redraw(wm);

    return wm;
}

int wm_destroy(window_manager *wm)
{
    if (wm) {
        wm_window_destroy(wm->main_window);
        free(wm);
    }

    return 0;
}

int wm_redraw(window_manager *wm)
{
    wm_window_redraw(wm->main_window);
    refresh();
    return 0;
}

int wm_split(window_manager *wm, wm_window *window, wm_direction orientation)
{
    wm_splitter *splitter = NULL;
    wm_window *orig = wm->focused_window;

    wm->focused_window = window;

    /* If already inside a splitter, hand off to wm_splitter_split(). */
    if (orig->parent && orig->parent->is_splitter) {
        splitter = (wm_splitter *) orig->parent;
        wm_splitter_split(splitter, orig, window, orientation);
        return wm_window_redraw((wm_window *) splitter);
    }

    /* This is the top-level window and it's not a splitter yet. */
    splitter = wm_splitter_create(orientation);
    wm_window_init((wm_window *) splitter, orig->cwindow, NULL);
    wm_splitter_split(splitter, NULL, orig, orientation);
    wm_splitter_split(splitter, orig, window, orientation);
    wm->main_window = (wm_window *) splitter;

    return wm_window_redraw((wm_window *) splitter);
}

int wm_close()
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
