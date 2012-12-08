/* window.h:
 * ---------
 */

#ifndef _WM_WINDOW_H_
#define _WM_WINDOW_H_

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
struct window_manager_s;
typedef struct window_manager_s window_manager;

/**
 * Window structure.  This structure and associated functions provide the
 * generic window functionality that every window will need.  Specific
 * instances of window objects (e.g. a file viewer) will extend this structure
 * with their own data fields, as well as set the function pointers to
 * implement the API declared here.  Event functions that aren't needed can be
 * left at their default values (wm_window implements default behaviors).
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
 * Your widget_create(...) method should always call wm_window_init(self) as
 * the first step.  This initializes the window structure fields (in most
 * cases, to zero - hook function pointers included).  Then your create method
 * can override whichever values and function pointers are needed.
 *
 * Your constructor cannot do any drawing or manipulation of the curses
 * window.  A curses window is not available until the first layout() call
 * occurs.
 *
 * To summarize, window creation steps:
 *
 * 1. Your widget constructor call.
 *    a. Calls wm_window_init()
 *    b. Populates hooks and other internal variables for first time setup.
 * 2. Window manager calls wm_window_set_context when a curses context is
 *    ready for your widget.
 *    a. wm, parent, and cwindow variables are set.
 * 3. Window manager calls wm_window_layout_event occurs, calling your
 *    'layout' hook.
 *    a. Window dimension variables are set (top, left, height, width).
 *    b. Your widget should update its state as necessary.
 *    c. Your widget should redraw() itself.
 */
struct wm_window_s {

    /**
     * Back-pointer to the window manager that owns this window. This pointer
     * is NULL until the first layout() call occurs.
     */
    window_manager *wm;

    /**
     * The window that contains this window, or NULL for the top level. This
     * pointer is always NULL until the first layout() call occurs. */
    wm_window *parent;

    /**
     * The curses window assigned to the window.  Child implementations should
     * do all their drawing to this curses window.  Do not hang onto this,
     * because the window can be given a new cwindow at any time.
     *
     * This pointer is NULL until the first layout() call occurs.
     */
    WINDOW *cwindow;

    /** The number of rows available in this window. */
    int height;

    /** The number of columns available in this window. */
    int width;

    /** The top row of the window (absolute). (You shouldn't need this.) */
    int top;

    /** The left column of the window (absolute). (You shouldn't need this.) */
    int left;

    /** The actual height of the window including any decorations. */
    int real_height;

    /** The actual width of the window including any decorations. */
    int real_width;

    /* Flags */

    /** True if this window is an instance of a splitter. */
    int is_splitter;

    /** Flag to indicate that a status bar should be drawn. (Default true). */
    int show_status_bar;

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
     * Layout event handler, called when the widget needs to handle changes to
     * its layout.  This will occur when a widget is placed in a new curses
     * window (including the first widget init), when the widget needs to be
     * resized, etc.
     *
     * @param window
     * The window receiving the layout event.
     *
     * @return
     * Zero on success, non-zero on failure.
     */
    int (*layout)(wm_window *window);

    /**
     * Input function, called when keyboard input is received for this window.
     * Passed in the form of an integer array (of chars or CGDB_KEY_xxx types),
     * and an integer specifying the size of the input array.
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
    int (*input)(wm_window *window, int *data, size_t len);

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
     * Get status bar text for the widget.  This will be drawn in the status
     * bar of the window, if one is visible, and truncated to fit in the
     * available space.
     *
     * @param max_length
     * The available space for status text, if you care to try to compress or
     * expand it yourself.
     *
     * @return
     * A newly-allocated string containing status text for the window, or NULL
     * if no status text is applicable.  The caller owns the returned pointer.
     */
    char *(*status_text)(wm_window *window, size_t max_length);

    /**
     * Get the minimum dimensions for this widget.  The window manager will
     * do its best to prevent the widget from being sized smaller than this.
     * This should include any status bars or other decorations.
     *
     * The default implementation of this function will return 1 x 1 (or 2 x 1
     * for windows with status bars).
     *
     * @param height
     * Output parameter, set to the minimum height for this widget.
     *
     * @param width
     * Output parameter, set to the minimum width for this widget.
     */
    void (*minimum_size)(wm_window *window, int *height, int *width);
};

/**
 * Initialize the given window object.  This is not a _create function because
 * window implementations will embed a wm_window in their structs.
 *
 * Sets all window data to a well-defined state (typically zero or NULL).
 *
 * @param window
 * The window to initialize.
 *
 * @return
 * Zero on success, non-zero on failure.
 */
int wm_window_init(wm_window *window);

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
 * Give the window a new UI context.  This should be called the first time a
 * window is placed into the user interface, and any time a new curses window
 * is allocated for the window (e.g. a new split occurs).
 *
 * This function assumes you may need to perform some manipulation of the
 * curses window after passing in a context.  After you are done, you should
 * call wm_window_layout_event() to allow the window to render itself.
 *
 * @param window
 * The window to update.
 *
 * @param wm
 * The window manager that owns this window (or NULL to leave unchanged).
 *
 * @param parent
 * The new parent window of this window (or NULL to leave unchanged).
 *
 * @param cwindow
 * The curses window that this window should use (never NULL).
 *
 * @return
 * Zero on success, non-zero on failure.
 */
int wm_window_set_context(wm_window *window, window_manager *wm,
                          wm_window *parent, WINDOW *cwindow);

/**
 * Notify the specified window that a layout change has occurred.  This can be
 * a resize, a new split (and association with a new cwindow), etc.  This
 * gives the widget a chance to update itself to handle the new environment it
 * finds itself in.
 *
 * @param window
 * The window that was touched.
 *
 * @return
 * Zero on success, non-zero on failure.
 */
int wm_window_layout_event(wm_window *window);

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

void
wm_window_dump(wm_window *window, FILE *out, int indent);

#endif /* _WM_WINDOW_H_ */
