/* window.h:
 * ---------
 */

#ifndef _WINDOW_H_
#define _WINDOW_H_

#include <config.h>

#if HAVE_CURSES_H
#include <curses.h>
#elif HAVE_NCURSES_CURSES_H
#include <ncurses/curses.h>
#endif /* HAVE_CURSES_H */

/**
 * @file     window.h
 * @author   Mike Mueller <mmueller@cs.uri.edu>
 */

/* Forward declarations. */
struct wm_window_s;
typedef struct wm_window_s wm_window;

/**
 * Window structure.  This structure and associated functions provide the
 * generic window functionality that every window will need.  Specific
 * instances of window objects (e.g. a file viewer) will extend this structure
 * with their own data fields, as well as set the function pointers to
 * implement the API declared here.  Event functions that aren't needed can be
 * left NULL.
 *
 * Example:
 * <pre> struct file_dialog {
 *     wm_window window;
 *     char *file_list[];
 *     int index;
 * };</pre>
 *
 * Implemented this way, a file_dialog can be passed anywhere a wm_window is
 * required.  (Poor man's inheritance.)
 *
 * See field documentation to identify which fields need to be set when
 * creating a new window.
 */
struct wm_window_s {

    /** The window that contains this window, or NULL for the top level. */
    wm_window *parent;

    /**
     * The curses window assigned to the window.  Child implementations should
     * do all their drawing to this curses window.
     */
    WINDOW *cwindow;

    /** Flag to indicate that a status bar should be drawn. (Default true). */
    int show_status_bar;

    /**
     * Initialization hook for child implementations.  This will be called
     * after the wm_window has been initialized, so a curses window has been
     * created.  The child object will probably want to use this time to draw
     * the contents on the screen.
     *
     * @param window
     * The window being initialized.
     *
     * @return
     * Zero on success, non-zero on failure.
     */
    int (*init)(wm_window *window);

    /**
     * Destructor function, called when window is destroyed.  Implement if
     * your object needs to do any destruction work.  (The window_destroy
     * function will clean up wm_window data, child objects do not need to.)
     *
     * @param window
     * The window to destroy.
     *
     * @return
     * Zero on success, non-zero on failure.
     */
    int (*destroy)(wm_window *window);

    /**
     * Input function, called when keyboard input is received for this window.
     * Passed in the form of an integer array (of chars or CGDB_KEY_xxx types),
     * and an integer specifying the size of the input array.  Set this field
     * to NULL if no input handling is needed.
     *
     * @param window
     * The window to receive the input.
     *
     * @param data
     * The keyboard data read from the user.
     *
     * @param len
     * The size of the data array.
     *
     * @return
     * Zero on success, non-zero on failure.
     */
    int (*input)(wm_window *window, int *data, int len);

    /**
     * Resize event handler.  Implement this to handle resize events
     * (redrawing contents if needed).
     *
     * @param window
     * The window receiving the resize event.
     *
     * @return
     * Zero on success, non-zero on failure.
     */
    int (*resize)(wm_window *window);

};

/**
 * Initialize the given window.  This is not a _create function because window
 * implementations will embed a wm_window in their structs.
 *
 * @param window
 * The window to initialize.
 *
 * @param cwindow
 * The curses window that the window should use.
 */
void window_init(wm_window *window, WINDOW *cwindow);

/**
 * Destroys the specified window. Calls the destroy function of the associated
 * widget before deallocating.
 *
 * @param window
 * The window to destroy.
 */
void window_destroy(wm_window *window);

#endif
