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
     * do all their drawing to this curses window.  Do not hang onto this,
     * because the window can be given a new cwindow at any time.
     */
    WINDOW *cwindow;

    /** Flag to indicate that a status bar should be drawn. (Default true). */
    int show_status_bar;

    /** The number of rows available in this window. */
    int height;

    /** The number of columns available in this window. */
    int width;

    /** True if this window is an instance of a splitter. */
    int is_splitter;

    /**
     * Initialization hook for child implementations.  When the window is
     * passed to the window manager, a curses window will be allocated for it
     * and then passed to this init method.  Before this point, the curses
     * window is unavailable.
     *
     * The child object will probably want to use this time to draw the
     * contents on the screen.
     *
     * @param window
     * The window being initialized.
     *
     * @return
     * Zero on success, non-zero on failure.
     */
    int (*init)(wm_window *window);

    /**
     * Destructor function, called when window is destroyed.  Use this to
     * clean up any member data, but do not delete the derived window object
     * itself.
     *
     * @param window
     * The window to clean up.
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
     * Redraw the widget.  This method will be called whenever the window
     * needs to be redrawn, your implementation should assume the entire
     * contents need to be re-rendered.
     *
     * @param window
     * The window receiving the redraw request.
     *
     * @return
     * Zero on success, non-zero on failure.
     */
    int (*redraw)(wm_window *window);

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
 *
 * @param parent
 * The window that contains and owns this window.
 *
 * @return
 * Zero on success, non-zero on failure.
 */
int wm_window_init(wm_window *window, WINDOW *cwindow, wm_window *parent);

/**
 * Destroys the specified window. Calls the destroy function of the associated
 * widget before deallocating.
 *
 * @param window
 * The window to destroy.
 *
 * @return
 * Zero on success, non-zero on failure.
 */
int wm_window_destroy(wm_window *window);

/**
 * Notify the specified window that it should be re-rendered.
 *
 * @param window
 * The window to redraw.
 *
 * @return
 * Zero on success, non-zero on failure.
 */
int wm_window_redraw(wm_window *window);

/**
 * Notify the specified window that it has been resized.
 *
 * @param window
 * The window that was resized.
 *
 * @return
 * Zero on success, non-zero on failure.
 */
int wm_window_resize(wm_window *window);

/**
 * Tell this window whether it should display a status bar or not.  This
 * affects the available height for the child widget, so you might want to
 * call resize after calling this. (Normally happens during init, so no resize
 * needed.)
 *
 * @param window
 * The window to modify.
 *
 * @param value
 * True to display, false to hide.
 */
void wm_window_show_status_bar(wm_window *window, int value);

#endif
