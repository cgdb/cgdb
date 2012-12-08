/* wm.h:
 * -----
 */

#ifndef __CGDB_WM_H__
#define __CGDB_WM_H__

#include "wm_window.h"
#include "types.h"

/**
 * @file
 * wm.h
 *
 * @brief
 * This is the interface to the window management library.
 */

/**
 * @name Window Management Overview
 * Libwm is the window manager library for CGDB.  This abstracts the
 * management of "windows" in the terminal, so that the calling program can
 * create, arrange and delete windows in the terminal space.  This management
 * is transparent to the confined window, which can do input and drawing
 * without knowledge of actual screen coordinates or whether it is seen by the
 * user at all.
 */

/**
 * The window manager object, one is required for any window management
 * operations.
 */
struct window_manager_s;
typedef struct window_manager_s window_manager;

/**
 * Creates a new window manager.  When done with this object, call wm_destroy
 * to free it.
 *
 * @param main_window
 * The initial window, which will be the top level window until any kind of
 * splitting occurs.  The window manager now owns this pointer.
 *
 * @param cli
 * The command line widget that will live at the bottom of the screen.  The
 * window manager is currently hard-coded to expect this to exist, so you must
 * always provide one.  The window manager now owns this pointer.
 *
 * @return
 * A new window manager is returned, or NULL on error.
 */
window_manager *wm_create(wm_window *main_window, wm_window *cli);

/**
 * Free the window manager, recursively destroying all associated windows.
 *
 * @param wm
 * The window manager to destroy.
 *
 * @return
 * Zero on success, non-zero on failure.
 */
int wm_destroy(window_manager *wm);

/**
 * Send keyboard input into the window manager.  It will be passed to
 * whichever window is currently focused.
 *
 * @param wm
 * The window manager.
 *
 * @param data
 * The sequence of keyboard data received from the user (as integers).
 *
 * @param len
 * The number of keystrokes contained in data.
 */
int wm_input(window_manager *wm, int *data, size_t len);

/**
 * Redraw all visible windows (because the display was damaged for some reason,
 * or maybe the user hit C-l).
 *
 * @param wm
 * The window manager.
 *
 * @return
 * Zero on success, non-zero on failure.
 */
int wm_redraw(window_manager *wm);

/**
 * Resize the given window in the given direction.  Other windows will be
 * pushed as necessary to accomodate the new space.
 *
 * The screen will be redrawn after the window is resized.
 *
 * @param wm
 * The window manager.
 *
 * @param window
 * The window to resize.
 *
 * @param dir
 * The direction being resized (vertical -> wider/narrower, horizontal ->
 * taller/shorter). Counterintuitive perhaps, but this is consistent with the
 * split orientation.  Think of it as moving a splitter.
 *
 * @param size
 * The new size of the window.  If the size is too large, the window will be
 * sized to fit.
 *
 * @return
 * Zero on success, non-zero on failure.
 */
int wm_resize(window_manager *wm, wm_window *window, wm_orientation dir,
              unsigned size);

/**
 * Split the current window, creating a new window which will divide the space
 * occupied by the original window.
 *
 * @param wm
 * The window manager.
 *
 * @param window
 * The window object to place in the newly created space. The window manager
 * now owns this pointer.
 *
 * @param orientation
 * Orientation of the split (HORIZONTAL or VERTICAL).
 *
 * @return
 * Zero on success, non-zero on failure.
 */
int wm_split(window_manager *wm, wm_window *window, wm_orientation orientation);

/**
 * Close the given window.  Remaining windows will be shuffled to fill in
 * empty screen real estate.
 *
 * @param wm
 * The window manager.
 *
 * @param window
 * The window to close.
 *
 * @return
 * Zero on success, non-zero on failure.
 */
int wm_close(window_manager *wm, wm_window *window);

/**
 * Focus the given window.
 *
 * @param wm
 * The window manager.
 *
 * @param window
 * The window to focus.
 */
void wm_focus(window_manager *wm, wm_window *window);

/**
 * Check if the given window has input focus.
 *
 * @param wm
 * The window manager.
 *
 * @param window
 * The window to check
 *
 * @return
 * 1 if the window is focused, 0 otherwise.
 */
int wm_is_focused(window_manager *wm, wm_window *window);

/**
 * Move the focus in the given direction.  Includes the current cursor
 * position (if available) to help focus the correct window.  You can simply
 * provide (0, 0) if you don't wish to specify a cursor.
 *
 * @param wm
 * The window manager.
 *
 * @param dir
 * The direction to move focus.
 *
 * @param cursor_pos
 * The current position of the cursor relative to the focused window..
 *
 * @return
 * Zero on success, non-zero on failure.
 */
int wm_move_focus(window_manager *wm, wm_direction dir, wm_position cursor_pos);

/**
 * Replace the main window.  The main window will be destroyed and replaced
 * with the given window.  It will also receive focus.
 *
 * @param wm
 * The window manager.
 *
 * @param window
 * The new window to be main.
 *
 * @return
 * Zero on success, non-zero on failure.
 */
int wm_new_main(window_manager *wm, wm_window *window);

/**
 * Put the window manager into (or take out of) command-line mode.  All input
 * will be passed to the cli until this mode is exited.
 *
 * @param wm
 * The window manager.
 *
 * @param value
 * True (non-zero) to enable, false to disable.
 */
void wm_cli_mode(window_manager *wm, int value);

/**
 * @name WM Options
 * The data types and operations used to set and retrieve options for the
 * window manager.  The options are used to control the behavior of the
 * windows manager in a way that emulates Vim's window system.
 */

/**
 * Set of options that affect window manager behavior.
 */
typedef enum {

    /**
     * Option "cmdheight" (shorthand: "ch") is of type: integer
     */
    CMDHEIGHT,

    /**
     * Option "eadirection" (shorthand: "ead") is of type: wm_orientation
     */
    EADIRECTION,

    /**
     * Option "equalalways" (shorthand: "ea") is of type: boolean
     */
    EQUALALWAYS,

    /**
     * Option "splitbelow" (shorthand: "sb") is of type: boolean
     */
    SPLITBELOW,

    /**
     * Option "splitright" (shorthand: "spr") is of type: boolean
     */
    SPLITRIGHT,

    /**
     * Option "winfixheight" (shorthand: "wfh") is of type: boolean
     */
    WINFIXHEIGHT,

    /**
     * Option "winminheight" (shorthand: "wmh") is of type: integer
     */
    WINMINHEIGHT,

    /**
     * Option "winminwidth" (shorthand: "wmw") is of type: integer
     */
    WINMINWIDTH,

    /**
     * Option "winheight" (shorthand: "wh") is of type: integer
     */
    WINHEIGHT,

    /**
     * Option "winwidth" (shorthand: "wiw") is of type: integer
     */
    WINWIDTH
} wm_option;

/**
 * Option value types
 */
typedef enum {
    WM_INTEGER,
    WM_BOOLEAN,
    WM_EADIR,
    WM_UNKNOWN
} wm_opttype;

/**
 * An option setting
 */
typedef struct {

    /**
     * Type of option this structure describes
     */
    wm_opttype type;

    /**
     * Actual value of option is stored in this variant type
     */
    union {
        /** if (type == WM_INTEGER) */
        int int_val;

        /** if (type == WM_BOOLEAN) */
        char bool_val;

        /** if (type == WM_EADIR) */
        wm_orientation ead_val;
    } variant;

} wm_optval;

/**
 * Get the value of the specified option.
 *
 * @param   option  The option value to retrieve.
 *
 * @return  The value of the specified option.  The type will be set
 *          to WM_UNKNOWN if an unknown option is specified, in which case
 *          the variant is undefined.
 */
wm_optval wm_option_get(wm_option option);

/**
 * Set the value of the specified option.
 *
 * @param   option  The option to set.
 * @param   value   The value to set for the given option.
 *
 * @return  Zero on success, non-zero on failure.
 */
int wm_option_set(wm_option option, wm_optval value);

void wm_dump(window_manager *wm, const char *path);

#endif /* __CGDB_WM_H__ */
