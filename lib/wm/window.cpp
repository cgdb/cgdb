#if 0
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

/* --------- */
/* Functions */
/* --------- */

wm_window window_create(wm_widget widget)
{
    /* Parameter bounds check */
    assert(widget != NULL);

    /* Allocate a new window */
    wm_window window = (wm_window) malloc(sizeof (struct wm_window));

    /* In such low level code, be extra paranoid */
    if (window != NULL) {
        window->widget = widget;
    }

    return window;
}

int window_destroy(wm_window window)
{
    if (window != NULL) {
        free(window);
    }

    return 0;
}
#endif
