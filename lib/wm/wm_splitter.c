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
    /* TODO: Remove this splitter if number of children == 1 */
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

int wm_splitter_resize_window(wm_splitter *splitter, wm_window *window,
                              wm_direction dir, int size)
{
    std_list_iterator iter;
    int desired_change;

    /* See TODO in wm_splitter_remove(). */
    if (std_list_length(splitter->children) == 1) {
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
    iter = std_list_find(splitter->children, window, wm_splitter_compare);
    if (!iter || iter == std_list_end(splitter->children)) {
        return -1;
    }

    if (desired_change < 0) {
        wm_window *next = NULL;
        int wrapped = 0;
        iter = std_list_next(iter);
        if (iter == std_list_end(splitter->children)) {
            std_list_iterator iter2 = std_list_begin(splitter->children);
            iter2 = std_list_next(iter2);
            iter = std_list_begin(splitter->children);
            while (!next && iter2 != std_list_end(splitter->children)) {
                wm_window *tmp;
                std_list_get_data(iter2, &tmp);
                if (tmp == window) {
                    std_list_get_data(iter, &next);
                } else {
                    iter = iter2;
                    iter2 = std_list_next(iter2);
                }
            }
            /* Something went really wrong */
            if (!next) {
                return -1;
            }
            wrapped = 1;
        }
        std_list_get_data(iter, &next);
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
    std_list_iterator i;
    int num_children = std_list_length(splitter->children);
    int sum = 0, remainder = 0;
    int position;
    float *proportions = malloc(sizeof(float) * num_children);
    int prev_dimension = 0;
    int *new_sizes = malloc(sizeof(int) * num_children);
    int j;
    int redistribute = 0;

    i = std_list_begin(splitter->children);
    for (j = 0; i != std_list_end(splitter->children);
         i = std_list_next(i), ++j)
    {
        wm_window *child;
        std_list_get_data(i, &child);
        if (splitter->orientation == WM_HORIZONTAL) {
            prev_dimension += child->real_height;
            proportions[j] = child->real_height;
        } else {
            prev_dimension += child->real_width;
            proportions[j] = child->real_width;
        }
        /* Note: Special case: initially created windows are 1 x 1. */
        if (child->real_height == 1 && child->real_width == 1) {
            redistribute = 1;
        }
    }
    if (redistribute) {
        /* Distribute windows equally */
        if (splitter->orientation == WM_HORIZONTAL) {
            for (j = 0; j < num_children; j++) {
                new_sizes[j] = window->real_height / num_children;
            }
            sum = window->real_height;
        } else {
            for (j = 0; j < num_children; j++) {
                new_sizes[j] = window->real_width / num_children;
            }
            sum = window->real_width;
        }
    } else {
        /* Distribute windows according to previous proportions. */
        sum = 0;
        for (j = 0; j < num_children; ++j) {
            proportions[j] /= prev_dimension;
            if (splitter->orientation == WM_HORIZONTAL) {
                new_sizes[j] = proportions[j] * window->real_height;
            } else {
                new_sizes[j] = proportions[j] * window->real_width;
            }
            sum += new_sizes[j];
        }
    }

    if (splitter->orientation == WM_HORIZONTAL) {
        remainder = window->height - sum;
        position = window->top;
    } else {
        remainder = window->width - sum;
        position = window->left;
    }

    i = std_list_begin(splitter->children);
    for (j = 0; i != std_list_end(splitter->children);
         i = std_list_next(i), ++j)
    {
        int my_dimension = new_sizes[j];
        wm_window *child = NULL;
        std_list_get_data(i, &child);
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
