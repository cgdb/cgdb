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

    wm_window_set_context(main_window, wm, NULL, cwindow);
    wm_window_layout_event(main_window);
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

int wm_resize(window_manager *wm, wm_window *window, wm_orientation dir,
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

int wm_split(window_manager *wm, wm_window *window, wm_orientation orientation)
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
    wm_window_set_context((wm_window *) splitter, wm, NULL, orig->cwindow);
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

int wm_new_main(window_manager *wm, wm_window *window)
{
    wm_window_destroy(wm->main_window);
    wm->main_window = window;
    if (window->cwindow) {
        delwin(window->cwindow);
    }
    wm_window_set_context(window, wm, NULL, derwin(stdscr, 0, 0, 0, 0));
    wm_focus(wm, window);
    wm_window_layout_event(window);
    wm_redraw(wm);
}

void wm_focus(window_manager *wm, wm_window *window)
{
    /* Consider checking that the window is in our hierarchy. */
    wm->focused_window = window;
}

int wm_is_focused(window_manager *wm, wm_window *window)
{
    return wm->focused_window == window;
}

int wm_move_focus(window_manager *wm, wm_direction dir, wm_position cursor_pos)
{
    wm_window *parent = wm->focused_window->parent;
    wm_position abs_cursor_pos;
    wm_window *to_focus = NULL;

    if (!parent) {
        return -1;
    }
    assert(parent->is_splitter);

    /* Get the absolute cursor position. */
    abs_cursor_pos.top = cursor_pos.top + wm->focused_window->top;
    abs_cursor_pos.left = cursor_pos.left + wm->focused_window->left;

    to_focus = wm_splitter_get_neighbor((wm_splitter *) parent,
                                       wm->focused_window, dir, abs_cursor_pos);
    if (to_focus == NULL) {
        return -1;
    }
    wm->focused_window = to_focus;
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
