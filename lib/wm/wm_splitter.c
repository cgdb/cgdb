#include <stdlib.h>

#include "wm_splitter.h"

/* Forward declarations */
static int wm_splitter_init(wm_window *window);
static int wm_splitter_destroy(wm_window *window);
static int wm_splitter_input(wm_window *window, int *data, int len);
static int wm_splitter_redraw(wm_window *window);
static int wm_splitter_resize(wm_window *window);
static int wm_splitter_find_child(wm_splitter *splitter, wm_window *window);
static int wm_splitter_array_remove(wm_splitter *splitter, wm_window *window);

const int DEFAULT_ARRAY_LENGTH = 4;

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
    splitter->children = malloc(sizeof(wm_window *) * DEFAULT_ARRAY_LENGTH);
    splitter->num_children = 0;
    splitter->array_length = DEFAULT_ARRAY_LENGTH;

    return splitter;
}

int
wm_splitter_remove(wm_splitter *splitter, wm_window *window)
{
    /* TODO: Remove this splitter if number of children == 1 */
    if (wm_splitter_array_remove(splitter, window)) {
        return -1;
    }
    wm_window_destroy(window);
    return wm_splitter_resize((wm_window *) splitter);
}

int wm_splitter_resize_window(wm_splitter *splitter, wm_window *window,
                              wm_direction dir, int size)
{
    int i;
    int desired_change;

    /* See TODO in wm_splitter_remove(). */
    if (splitter->num_children == 1) {
        return -1;
    }
    if (dir != splitter->orientation) {
        return -1;
    }

    switch (dir) {
        case WM_HORIZONTAL:
            desired_change = size - window->height;
            break;
        case WM_VERTICAL:
            desired_change = size - window->width;
            break;
        default:
            return -1;
    }

    /* Find successors of window to trade space with */
    i = wm_splitter_find_child(splitter, window);
    if (i < 0) {
        return -1;
    }

    if (desired_change < 0) {
        wm_window *next = NULL;
        int wrapped = 0;
        if (i+1 == splitter->num_children) {
            next = splitter->children[i-1];
            wrapped = 1;
        } else {
            next = splitter->children[i+1];
        }
        if (dir == WM_HORIZONTAL) {
            if (window->real_height + desired_change < 2) {
                desired_change = -(window->real_height - 2);
            }
            next->real_height -= desired_change;
            window->real_height += desired_change;
            if (wrapped) {
                window->top -= desired_change;
            } else {
                next->top += desired_change;
            }
        } else {
            if (window->real_width + desired_change < 2) {
                desired_change = -(window->real_width - 2);
            }
            next->real_width -= desired_change;
            window->real_width += desired_change;
            if (wrapped) {
                window->left -= desired_change;
            } else {
                next->left += desired_change;
            }
        }
        wm_window_place(window, window->top, window->left,
                        window->real_height, window->real_width);
        wm_window_place(next, next->top, next->left,
                        next->real_height, next->real_width);
    } else if (desired_change > 0) {

    }

    return 0;
}

int
wm_splitter_split(wm_splitter *splitter, wm_window *window,
                  wm_window *new_window, wm_direction orientation)
{
    wm_window *obj = new_window;
    int pos = splitter->num_children;
    int i;

    /* Find first window for splitting/insertion */
    if (window) {
        i = wm_splitter_find_child(splitter, window);
        if (i < 0) {
            return -1;
        }
        pos = i + 1;
    } else if (orientation != splitter->orientation) {
        /* Tried to append a window and got the orientation wrong. */
        return -1;
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
    } else {
        wm_splitter *new_splitter = wm_splitter_create(orientation);
        wm_window_init((wm_window *) new_splitter, window->cwindow,
                       (wm_window *) splitter);
        wm_splitter_array_remove(splitter, window);
        pos--;
        wm_splitter_split(new_splitter, NULL, window, orientation);
        wm_splitter_split(new_splitter, window, new_window, orientation);
        obj = (wm_window *) new_splitter;
    }

    if (splitter->num_children == splitter->array_length) {
        splitter->array_length *= 2;
        splitter->children = realloc(splitter->children,
            sizeof(wm_window *) * splitter->array_length);
    }
    for (i = pos; i < splitter->num_children; ++i) {
        splitter->children[i+1] = splitter->children[i];
    }
    splitter->children[pos] = obj;
    splitter->num_children++;

    return wm_window_resize_event((wm_window *) splitter);
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
    int i;

    for (i = 0; i < splitter->num_children; ++i) {
        wm_window_destroy(splitter->children[i]);
    }
    free(splitter->children);

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
    int i;

    for (i = 0; i < splitter->num_children; ++i) {
        wm_window_redraw(splitter->children[i]);
        wrefresh(window->cwindow);
    }

    return 0;
}

static int
wm_splitter_resize(wm_window *window)
{
    /* TODO: Handle window size options (splitting evenly right now) */
    wm_splitter *splitter = (wm_splitter *) window;
    int sum = 0, remainder = 0;
    int position;
    float *proportions = malloc(sizeof(float) * splitter->num_children);
    int prev_dimension = 0;
    int *new_sizes = malloc(sizeof(int) * splitter->num_children);
    int redistribute = 0;
    int i;

    for (i = 0; i < splitter->num_children; ++i) {
        wm_window *child = splitter->children[i];
        if (splitter->orientation == WM_HORIZONTAL) {
            prev_dimension += child->real_height;
            proportions[i] = child->real_height;
        } else {
            prev_dimension += child->real_width;
            proportions[i] = child->real_width;
        }
        /* Note: Special case: initially created windows are 1 x 1. */
        if (child->real_height == 1 && child->real_width == 1) {
            redistribute = 1;
        }
    }
    if (redistribute) {
        /* Distribute windows equally */
        if (splitter->orientation == WM_HORIZONTAL) {
            for (i = 0; i < splitter->num_children; i++) {
                new_sizes[i] = window->real_height / splitter->num_children;
            }
            sum = window->real_height;
        } else {
            for (i = 0; i < splitter->num_children; i++) {
                new_sizes[i] = window->real_width / splitter->num_children;
            }
            sum = window->real_width;
        }
    } else {
        /* Distribute windows according to previous proportions. */
        sum = 0;
        for (i = 0; i < splitter->num_children; ++i) {
            proportions[i] /= prev_dimension;
            if (splitter->orientation == WM_HORIZONTAL) {
                new_sizes[i] = proportions[i] * window->real_height;
            } else {
                new_sizes[i] = proportions[i] * window->real_width;
            }
            sum += new_sizes[i];
        }
    }

    if (splitter->orientation == WM_HORIZONTAL) {
        remainder = window->height - sum;
        position = window->top;
    } else {
        remainder = window->width - sum;
        position = window->left;
    }

    for (i = 0; i < splitter->num_children; ++i) {
        int my_dimension = new_sizes[i];
        wm_window *child = splitter->children[i];
        if (remainder) {
            my_dimension++;
            remainder--;
        }
        /* Resize and relocate the window */
        if (splitter->orientation == WM_HORIZONTAL) {
            wm_window_place(child, position, window->left,
                            my_dimension, window->width);
        } else {
            wm_window_place(child, window->top, position,
                            window->height, my_dimension);
        }
        position += my_dimension;
        /* Notify */
        wm_window_resize_event(child);
    }

    free(new_sizes);
    free(proportions);
    return wm_splitter_redraw(window);
}

static int
wm_splitter_find_child(wm_splitter *splitter, wm_window *window)
{
    int i = 0;
    for (i = 0; i < splitter->num_children; ++i) {
        if (splitter->children[i] == window) {
            return i;
        }
    }
    return -1;
}

static int
wm_splitter_array_remove(wm_splitter *splitter, wm_window *window)
{
    int i = wm_splitter_find_child(splitter, window);
    int j;
    if (i < 0) {
        return -1;
    }

    splitter->num_children--;
    for (j = i; j < splitter->num_children; ++j) {
        splitter->children[j] = splitter->children[j+1];
    }

    return 0;
}
