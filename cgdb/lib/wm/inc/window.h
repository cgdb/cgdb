/* window.h:
 * ---------
 */

#ifndef _WINDOW_H_
#define _WINDOW_H_

/* Standard Includes */
/* ----------------- */

/* Autoconf header */
#include <config.h>

/* Curses library */
#if HAVE_CURSES_H
#include <curses.h>
#endif /* HAVE_CURSES_H */

/**
 * @file     window.h
 * @author   Mike Mueller <mmueller@cs.uri.edu>
 * @brief    Please Read This: wm_widget
 */

/**
 * @name Windows
 * Window behavior is contained in this file.  This is very straightforward,
 * if you are reading this you most likely need to read the wm_widget
 * documentation.  The key to using the window manager properly is creating
 * valid widgets.
 */

//@{

/**
 * Window identifier type (comparable to a file descriptor)
 */
typedef int wid_t;

/**
 * Window object 
 */
typedef struct wm_window *wm_window;

/* Forward declarations */
struct wm_widget;
typedef struct wm_widget *wm_widget;

/**
 * Widget: This is the object that client code will have to implement.  This is
 * the key to properly using the window manager.  The UI developer needs to
 * create a widget which implements the following interface, using C-style
 * "inheritance".  By this, I mean the widget type will be a struct whose first
 * element is a wm_widget, and therefore "is a" wm_widget (with some minor
 * casting here and there).
 *
 * Example:
 * <pre> struct file_dialog {
 *     wm_widget widget;
 *     char *file_list[];
 *     int index;
 * };</pre>
 * 
 * Widget data can now be accessed in the following ways:
 * <pre> struct file_dialog *dlg = malloc(sizeof(struct file_dialog));
 * dlg->widget = malloc(sizeof(struct wm_widget));
 * dlg->widget->create     = dialog_constructor;
 * (wm_widget) dlg->create = dialog_constructor;</pre>
 *
 * See field documentation to identify which fields need to be set when
 * creating a new widget.
 */
struct wm_widget {
    
    /**
     * Constructor function, called when the window is initially created.
     * Although the widget is already created by the time the window is created,
     * odds are there is some initial setup that needs to occur based on the
     * window being created.  For example, the size of the widget is not known,
     * and it cannot do any drawing until a window exists.  This is basically
     * a notification event.  Set this field to NULL if no constructor is
     * needed.
     *
     * @param  widget  The widget being created
     * @return Zero on success, non-zero on failure.
     */
    int (*create)(wm_widget widget);

    /**
     * Destructor function, called when window is destroyed.  This function
     * should deallocate anything that the widget references, and finally
     * the widget itself.  None of the "Internal Use Only" members should be
     * touched, however!  The windowing code will clean that up automatically,
     * along with the curses 'win' member. Set this field to NULL if no 
     * destructor is needed.
     *
     * @param  widget  The widget to destroy
     * @return Zero on success, non-zero on failure.
     */
    int (*destroy)(wm_widget widget);

    /**
     * Input function, called when keyboard input is received for this widget.
     * Passed in the form of an integer array (of chars or CGDB_KEY_xxx types),
     * and an integer specifying the size of the input array.  Set this field
     * to NULL if no input handling is needed.
     *
     * @param  widget  The widget to receive the input
     * @param  data    The keyboard data read from the user
     * @param  len     The size of the data array
     * @return Zero on success, non-zero on failure.
     */
    int (*input)(wm_widget widget, int *data, int len); 

    /**
     * Resize function, called when the window containing this widget is
     * resized.  The dimensions are not passed here because the widget can
     * simply use the data of the 'win' member.  Set this field to NULL if no
     * resize event handling is needed.
     *
     * @param  widget  The widget receiving the resize event
     * @return Zero on success, non-zero on failure.
     */
    int (*resize)(wm_widget widget, int height, int width);

    /**
     * The curses window assigned to the widget.  The widget should do all
     * drawing to this window.  If information about the size of the window
     * is needed, the widget can retrieve the data via this member.  Remember,
     * this will be destroyed by the windowing code, so do not attempt to 
     * destroy it in the widget destructor.
     */
    WINDOW *win;

};

/**
 * Creates a new window with the specified widget.
 *
 * @param  widget  Widget to which the window is permanently bound
 *
 * @return The window identifier of the new window on success, or -1 on failure
 */
wid_t window_create(wm_widget widget);
 
/**
 * Destroys the specified window.  Calls the destroy function of the associated
 * widget before deallocating.
 *
 * @param  win_id  Identifier of the window to destroy.
 *
 * @return Zero on success, non-zero on failure.
 */
int window_destroy(wid_t win_id);

//@}

#endif

