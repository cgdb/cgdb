/* window.h:
 * ---------
 *
 */

#ifndef _WINDOW_H_
#define _WINDOW_H_

/* ----- */
/* Types */
/* ----- */

/* Window ID type */
typedef int wid_t;

/* Window object (implementation in src) */
struct wm_window;

/* Widget type */
typedef struct {
    
    /* Curses window for the widget to work in */
    WINDOW *win;
    
    /* Private data: Not touched by windowing code, meant for widget-specific
     * information. */
    void *data;

} wm_widget;

/* ------------------- */
/* Function Prototypes */
/* ------------------- */


#endif

