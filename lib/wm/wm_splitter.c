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
wm_splitter_add(wm_splitter *splitter, wm_window *window)
{
    /* TODO: Handle splitbelow, splitright options. */

    /* Temporary window, will be resized. */
    WINDOW *cwindow = subwin(splitter->window.cwindow, 1, 1, 0, 0);
    /* TODO: Calling init twice perhaps, kind of bad. */
    wm_window_init(window, cwindow, (wm_window *) splitter);
    std_list_append(splitter->children, window);
    return wm_splitter_resize((wm_window *) splitter);
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

/* Window method implementations */

static int
wm_splitter_init(wm_window *window)
{
    wm_splitter *splitter = (wm_splitter *) window;
    window->is_splitter = 1;
    if (splitter->orientation == WM_HORIZONTAL) {
        wm_window_show_status_bar(window, 0);
    }
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
    /* TODO: Handle orientation. */
    /* TODO: For now, this splits the space evenly. */
    wm_splitter *splitter = (wm_splitter *) window;
    std_list_iterator i = std_list_begin(splitter->children);
    int num_children = std_list_length(splitter->children);
    int new_height = window->height / num_children;
    int remainder = window->height % num_children;
    int row = 0;

    for (; i != std_list_end(splitter->children); i = std_list_next(i)) {
        int my_height = new_height;
        wm_window *child = NULL;
        std_list_get_data(i, &child);
        if (remainder) {
            my_height++;
            remainder--;
        }
        /* Resize and relocate the window */
        mvwin(child->cwindow, row, 0);
        wresize(child->cwindow, my_height, window->width);
        row += my_height;
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
