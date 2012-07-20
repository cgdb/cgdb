#include "std_list.h"
#include "wm_splitter.h"

/* Forward declarations */
static int wm_splitter_init(wm_window *window);
static int wm_splitter_destroy(wm_window *window);
static int wm_splitter_input(wm_window *window, int *data, int len);
static int wm_splitter_redraw(wm_window *window);
static int wm_splitter_resize(wm_window *window);
static int wm_splitter_compare(const void *window1, const void *window2);

wm_splitter *
wm_splitter_create(wm_direction orientation)
{
    wm_splitter *splitter = (wm_splitter *) malloc(sizeof(wm_splitter));

    splitter->window.init = wm_splitter_init;
    splitter->window.destroy = wm_splitter_destroy;
    splitter->window.input = wm_splitter_input;
    splitter->window.redraw = wm_splitter_redraw;
    splitter->window.resize = wm_splitter_resize;
    splitter->orientation = orientation;
    splitter->children = std_list_create(NULL);

    return splitter;
}

int
wm_splitter_remove(wm_splitter *splitter, wm_window *window)
{
    wm_window *child = NULL;
    std_list_iterator i = std_list_find(splitter->children, window,
                                        wm_splitter_compare);

    std_list_get_data(i, &child);
    if (!window) {
        return -1;
    }

    std_list_remove(splitter->children, i);
    wm_window_destroy(window);
    return wm_splitter_resize((wm_window *) splitter);
}

int
wm_splitter_split(wm_splitter *splitter, wm_window *window,
                  wm_window *new_window, wm_direction orientation)
{
    std_list_iterator iter;

    /* Find first window for splitting/insertion */
    if (window) {
        iter = std_list_find(splitter->children, window, wm_splitter_compare);
        if (!iter || iter == std_list_end(splitter->children)) {
            return 1;
        }
    } else if (orientation != splitter->orientation) {
        /* Tried to append a window and got the orientation wrong. */
        return 1;
    }

    /* TODO: Handle splitright, splitbelow options. */

    if (orientation == splitter->orientation) {
        WINDOW *cwindow = derwin(splitter->window.cwindow, 1, 1, 0, 0);
        if (cwindow == NULL) {
            abort();
        }
        /* TODO: This may be the 2nd (or 3rd, ...) time that init was called
         * on this widget.  Probably should tighten this up if possible. */
        wm_window_init(new_window, cwindow, (wm_window *) splitter);
        if (window) {
            iter = std_list_next(iter);
            std_list_insert(splitter->children, iter, new_window);
        } else {
            std_list_append(splitter->children, new_window);
        }
    } else {
        wm_splitter *new_splitter = wm_splitter_create(orientation);
        wm_window_init((wm_window *) new_splitter, window->cwindow,
                       (wm_window *) splitter);
        iter = std_list_remove(splitter->children, iter);
        wm_splitter_split(new_splitter, NULL, window, orientation);
        wm_splitter_split(new_splitter, window, new_window, orientation);
        std_list_insert(splitter->children, iter, new_splitter);
    }

    return wm_splitter_resize((wm_window *) splitter);
}

/* Window method implementations */

static int
wm_splitter_init(wm_window *window)
{
    window->is_splitter = 1;
    wm_window_show_status_bar(window, 0);
    return 0;
}

static int
wm_splitter_destroy(wm_window *window)
{
    wm_splitter *splitter = (wm_splitter *) window;
    std_list_iterator i = std_list_begin(splitter->children);

    for (; i != std_list_end(splitter->children); i = std_list_next(i)) {
        wm_window *window;
        std_list_get_data(i, &window);
        wm_window_destroy(window);
    }

    return 0;
}

static int
wm_splitter_input(wm_window *window, int *data, int len)
{
    return 0;
}

static int
wm_splitter_redraw(wm_window *window)
{
    wm_splitter *splitter = (wm_splitter *) window;
    std_list_iterator i = std_list_begin(splitter->children);

    for (; i != std_list_end(splitter->children); i = std_list_next(i)) {
        wm_window *child = NULL;
        std_list_get_data(i, &child);
        wm_window_redraw(child);
        wrefresh(window->cwindow);
    }

    return 0;
}

static int
wm_splitter_resize(wm_window *window)
{
    /* TODO: Handle window size options (splitting evenly right now) */
    wm_splitter *splitter = (wm_splitter *) window;
    std_list_iterator i = std_list_begin(splitter->children);
    int num_children = std_list_length(splitter->children);
    int new_dimension, remainder;
    int position;

    if (splitter->orientation == WM_HORIZONTAL) {
        new_dimension = window->height / num_children;
        remainder = window->height % num_children;
        position = window->top;
    } else {
        new_dimension = window->width / num_children;
        remainder = window->width % num_children;
        position = window->left;
    }

    for (; i != std_list_end(splitter->children); i = std_list_next(i)) {
        int my_dimension = new_dimension;
        wm_window *child = NULL;
        std_list_get_data(i, &child);
        if (remainder) {
            my_dimension++;
            remainder--;
        }
        /* Resize and relocate the window */
        if (splitter->orientation == WM_HORIZONTAL) {
            mvwin(child->cwindow, position, window->left);
            wresize(child->cwindow, my_dimension, window->width);
        } else {
            mvwin(child->cwindow, window->top, position);
            wresize(child->cwindow, window->height, my_dimension);
        }
        position += my_dimension;
        /* Notify */
        wm_window_resize(child);
    }

    return wm_splitter_redraw(window);
}

static int
wm_splitter_compare(const void *window1, const void *window2)
{
    /* Naive pointer identity. */
    if (window1 < window2) {
        return -1;
    } else if (window1 == window2) {
        return 0;
    } else {
        return 1;
    }
}
