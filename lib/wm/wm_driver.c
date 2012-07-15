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
    int lines;
};
typedef struct test_widget test_widget;

/* ---------------- */
/* Widget Functions */
/* ---------------- */

/* test_widget: C++ style cast */
test_widget *get_test_widget(wm_window *window)
{
    return (test_widget *) window;
}

/* test_init: Constructor function. */
int test_init(wm_window *window)
{
    test_widget *widget = get_test_widget(window);
    widget->lines = 0;
    mvwprintw(window->cwindow, widget->lines, 0,
              "%d: Constructor function called",
              widget->lines);
    get_test_widget(window)->lines++;
    wrefresh(window->cwindow);
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
    int i;

    /* Display data we received */
    wprintw(window->cwindow, "%d: ", get_test_widget(window)->lines++);
    for (i = 0; i < len; i++) {
        wprintw(window->cwindow, "%c", (char) data[i]);
    }
    wprintw(window->cwindow, "\r\n");
    wrefresh(window->cwindow);

    return 0;
}

/* test_input: Resize function. */
int test_resize(wm_window *window)
{
    int height, width;

    /* Output our new size */
    getmaxyx(window->cwindow, height, width);
    wprintw(window->cwindow, "%d: ", get_test_widget(window)->lines++);
    wprintw(window->cwindow, "New Size: H: %d, W: %d\r\n", height, width);
    wrefresh(window->cwindow);

    return 0;
}

/* ------------- */
/* Main Function */
/* ------------- */

int main(int argc, char *argv[])
{
    WINDOW *mainwin = NULL;
    test_widget mywidget;
    
    if ((mainwin = initscr()) == NULL ) {
	    fprintf(stderr, "Error initialising ncurses.\n");
	    exit(1);
    }

    mywidget.window.init = test_init;
    mywidget.window.destroy = test_destroy;
    mywidget.window.input = test_input;
    mywidget.window.resize = test_resize;

    /* Create a Window Manager context */
    window_manager *wm = wm_create((wm_window *) &mywidget);
    sleep(3);

    /* TODO: Send some input, split, etc. */

    /* Destroy the Window Manager context */
    wm_destroy(wm);

    delwin(mainwin);
    endwin();
    refresh();

    return 0;
}

#endif /* DOXYGEN */
