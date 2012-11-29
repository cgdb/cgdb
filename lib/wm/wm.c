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

    /* TODO: CLI */
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

int wm_resize(window_manager *wm, wm_window *window, wm_direction dir,
              unsigned size)
{
    wm_splitter *splitter;

    /* Can't resize the top-level window */
    if (window->parent == NULL) {
        return -1;
    }

    if (window->is_splitter) {
        splitter = (wm_splitter *) window;
    } else {
        splitter = (wm_splitter *) window->parent;
    }

    /* If resize is along same direction as split, we need to size the parent */
    if (dir == splitter->orientation) {
        wm_splitter_resize_window((wm_splitter *) window->parent,
                                  window, dir, size);
    } else {
        wm_splitter_resize_window((wm_splitter *) splitter->window.parent,
                                  (wm_window *) splitter, dir, size);
    }

    return wm_redraw(wm);
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

int wm_close(window_manager *wm, wm_window *window)
{
    wm_splitter *splitter;

    /* Only support closing real windows */
    if (window->is_splitter) {
        return -1;
    }
    if (window->parent == NULL) {
        return -1;
    }
    splitter = (wm_splitter *) window->parent;
    if (wm_splitter_remove(splitter, window)) {
        return -1;
    }

    return wm_redraw(wm);
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

void wm_dump(window_manager *wm, const char *path)
{
    FILE *out = fopen(path, "a");
    fprintf(out, "Window Manager Dump:\n");
    wm_window_dump(wm->main_window, out, 2);
    fclose(out);
}
