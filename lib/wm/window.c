/* window.c:
 * ---------
 *
 * Window code for CGDB.  See header for full documentation.
 */

/* Standard Includes */
#include <assert.h>
#include <stdlib.h>

/* Local Includes */
#include "window.h"

void window_init(wm_window *window, WINDOW *cwindow)
{
    window->cwindow = cwindow;
    window->show_status_bar = 1;
    if (window->init) {
        window->init(window);
    }
}

void window_destroy(wm_window *window)
{
    if (window != NULL) {
        delwin(window->cwindow);
        if (window->destroy) {
            window->destroy(window);
        }
    }
}
