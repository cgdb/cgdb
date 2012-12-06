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

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Local Includes */
#include "wm.h"
#include "wm_window.h"

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

/* test_layout: Resize event. */
int test_layout(wm_window *window)
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
            mvwprintw(window->cwindow, i, j, "%d", (i+j) % 9);
        }
    }
    wattroff(window->cwindow, COLOR_PAIR(widget->color));
    wrefresh(window->cwindow);
}

char *test_status_text(wm_window *window, size_t max_length)
{
    return strdup("This is my status");
}

/* Create a new test widget */
static test_widget *test_create(int color)
{
    test_widget *widget = (test_widget *) malloc(sizeof(test_widget));
    wm_window_init((wm_window *) widget);

    /* Window data */
    widget->window.layout = test_layout;
    widget->window.redraw = test_redraw;
    widget->window.status_text = test_status_text;

    /* My widget data */
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

    test_widget *widgets[6];
    unsigned delay = 400000;
    for (i = 0; i < 5; i++) {
        widgets[i] = test_create(i+1);
        switch (i) {
            case 0:
                wm = wm_create((wm_window *) widgets[i]);
                break;
            case 1:
            case 2:
            case 4:
                assert(!wm_split(wm, (wm_window *) widgets[i], WM_HORIZONTAL));
                break;
            case 3:
                assert(!wm_split(wm, (wm_window *) widgets[i], WM_VERTICAL));
                break;
        }
        usleep(delay);
    }

    for (i = 1; i <= 4; ++i) {
        assert(!wm_resize(wm, (wm_window *) widgets[3], WM_HORIZONTAL,
                widgets[3]->window.real_height - i));
        usleep(delay);
    }

    for (i = 1; i <= 4; ++i) {
        assert(!wm_resize(wm, (wm_window *) widgets[3], WM_HORIZONTAL,
                widgets[3]->window.real_height + i));
        usleep(delay);
    }
    for (i = 1; i <= 3; ++i) {
        assert(!wm_resize(wm, (wm_window *) widgets[2], WM_VERTICAL,
                widgets[2]->window.real_width - 1));
        usleep(delay);
    }
    for (i = 1; i <= 3; ++i) {
        assert(!wm_resize(wm, (wm_window *) widgets[2], WM_VERTICAL,
                widgets[2]->window.real_width + 2));
        usleep(delay);
    }
    for (i = 1; i <= 3; ++i) {
        assert(!wm_resize(wm, (wm_window *) widgets[2], WM_HORIZONTAL,
                widgets[2]->window.real_height - i));
        usleep(delay);
    }
    for (i = 1; i <= 60; ++i) {
        assert(!wm_resize(wm, (wm_window *) widgets[0], WM_HORIZONTAL,
                widgets[0]->window.real_height + 1));
        usleep(delay/10);
    }
    for (i = 1; i <= 20; ++i) {
        assert(!wm_resize(wm, (wm_window *) widgets[1], WM_HORIZONTAL,
                widgets[1]->window.real_height + 1));
        usleep(delay/10);
    }
    for (i = 1; i <= 60; ++i) {
        assert(!wm_resize(wm, (wm_window *) widgets[2], WM_HORIZONTAL,
                widgets[2]->window.real_height + 1));
        usleep(delay/10);
    }
    wm_dump(wm, "wm.out");
    for (i = 4; i >= 1; --i) {
        assert(!wm_close(wm, (wm_window *) widgets[i]));
        usleep(delay);
    }
    wm_dump(wm, "wm.out");

    /* Destroy the Window Manager context */
    wm_destroy(wm);

    endwin();

    return 0;
}

#endif /* DOXYGEN */
