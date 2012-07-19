/* wm_driver.c:
 * ------------
 *
 * Test program for window manager.
 */

#ifndef DOXYGEN

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_CURSES_H
#include <curses.h>
#elif HAVE_NCURSES_CURSES_H
#include <ncurses/curses.h>
#endif /* HAVE_CURSES_H */

#include <stdlib.h>
#include <unistd.h>

/* Local Includes */
#include "wm.h"
#include "window.h"

/* An example widget */
struct test_widget {
    wm_window window;
    int color;
};
typedef struct test_widget test_widget;

/* Internal widget functions */
static test_widget *test_create();
int test_redraw(wm_window *window);

/* test_widget: C++ style cast */
test_widget *get_test_widget(wm_window *window)
{
    return (test_widget *) window;
}

/* test_init: Constructor function. */
int test_init(wm_window *window)
{
    test_redraw(window);
    return 0;
}

/* test_destroy: Destructor function. */
int test_destroy(wm_window *window)
{
    /* Nothing needs to be deallocated */
    return 0;
}

/* test_input: Input function. */
int test_input(wm_window *window, int *data, int len)
{
    return 0;
}

/* test_input: Resize function. */
int test_resize(wm_window *window)
{
    test_redraw(window);
    return 0;
}

int test_redraw(wm_window *window)
{
    test_widget *widget = (test_widget *) window;
    int i, j;

    wattron(window->cwindow, COLOR_PAIR(widget->color));
    for (i = 0; i < window->height; ++i) {
        for (j = 0; j < window->width; ++j) {
            mvwprintw(window->cwindow, i, j, "%d", (i+j) % 10);
        }
    }
    wattroff(window->cwindow, COLOR_PAIR(widget->color));
    wrefresh(window->cwindow);
}

/* Create a new test widget */
static test_widget *test_create(int color)
{
    test_widget *widget = (test_widget *) malloc(sizeof(test_widget));

    widget->window.init = test_init;
    widget->window.destroy = test_destroy;
    widget->window.input = test_input;
    widget->window.redraw = test_redraw;
    widget->window.resize = test_resize;
    widget->color = color;

    return widget;
}

/* ------------- */
/* Main Function */
/* ------------- */

int main(int argc, char *argv[])
{
    WINDOW *mainwin = NULL;
    test_widget *widget1, *widget2, *widget3;
    window_manager *wm = NULL;
    int i;

    if ((mainwin = initscr()) == NULL ) {
	    fprintf(stderr, "Error initialising ncurses.\n");
	    exit(1);
    }

    if (has_colors()) {
        start_color();
#ifdef NCURSES_VERSION
        use_default_colors();
#else
        bkgdset(0);
        bkgd(COLOR_WHITE);
#endif
        init_pair(1, COLOR_RED, COLOR_BLACK);
        init_pair(2, COLOR_BLUE, COLOR_BLACK);
        init_pair(3, COLOR_GREEN, COLOR_BLACK);
        init_pair(4, COLOR_CYAN, COLOR_BLACK);
        init_pair(5, COLOR_YELLOW, COLOR_BLACK);
    }

    for (i = 1; i <= 4; i++) {
        test_widget *widget = test_create(i);
        if (!wm) {
            wm = wm_create((wm_window *) widget);
        } else {
            wm_split(wm, (wm_window *) widget, WM_HORIZONTAL);
        }
        sleep(2);
    }

    /* Destroy the Window Manager context */
    wm_destroy(wm);

    endwin();

    return 0;
}

#endif /* DOXYGEN */
