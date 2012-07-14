/* wm_driver.c:
 * ------------
 *
 * Test program for window manager.  Allows user to create windows to play
 * with the features of the wm library.
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

/* Local Includes */
#include "wm.h"
#include "window.h"

/* An example widget */
struct test_widget {
    wm_widget widget;
    int lines;
};
typedef struct test_widget test_widget;

/* ---------------- */
/* Widget Functions */
/* ---------------- */

/* test_widget: C++ style cast */
test_widget *get_test_widget(wm_widget widget)
{
    return (test_widget *) widget;
}

/* test_create: Constructor function. */
int test_create(wm_widget widget)
{
    wprintw(widget->win, "%d: Constructor function called\r\n",
            get_test_widget(widget)->lines = 1);
    get_test_widget(widget)->lines++;
    return 0;
}

/* test_destroy: Destructor function. */
int test_destroy(wm_widget widget)
{
    /* Nothing needs to be deallocated */
    return 0;
}

/* test_input: Input function. */
int test_input(wm_widget widget, int *data, int len)
{
    int i;

    /* Display data we received */
    wprintw(widget->win, "%d: ", get_test_widget(widget)->lines++);
    for (i = 0; i < len; i++) {
        wprintw(widget->win, "%c", (char) data[i]);
    }
    wprintw(widget->win, "\r\n");

    return 0;
}

/* test_input: Resize function. */
int test_resize(wm_widget widget)
{
    int height, width;

    /* Output our new size */
    getmaxyx(widget->win, height, width);
    wprintw(widget->win, "%d: ", get_test_widget(widget)->lines++);
    wprintw(widget->win, "New Size: H: %d, W: %d\r\n", height, width);

    return 0;
}

/* ------------- */
/* Main Function */
/* ------------- */

int main(int argc, char *argv[])
{
#if 0
    test_widget mywidget;

    mywidget.widget = (wm_widget) malloc(sizeof (struct wm_widget));
    mywidget.widget->create = test_create;
    mywidget.widget->destroy = test_destroy;
    mywidget.widget->input = test_input;
    mywidget.widget->resize = test_resize;

    /* Create a Window Manager context */
    wmctx wm_ctx = wm_create((wm_widget) & mywidget);

    /* Destroy the Window Manager context */
    wm_destroy(wm_ctx);
#endif

    return 0;
}

#endif /* DOXYGEN */
