/* wm.h:
 * -----
 */

#ifndef _WM_H_
#define _WM_H_

/* Local Includes */
#include "window.h"

/** 
 * @file
 * wm.h
 *
 * @brief
 * This is the interface to the window management library.
 */

/**
 * @name Window Management Overview
 * Libwm is the window manager library for CGDB.  This abstracts the management
 * of "windows"
 * in the terminal, so that the calling program can create, arrange and delete
 * windows in the terminal space.  This management is transparent to the 
 * confined widget, which can do input and drawing without knowledge of actual
 * screen coordinates or whether it is seen by the user at all.
 *
 * A window is bound to a single widget at the time of creation, and they are
 * inextricably bound until the window is closed.  (The opposite is not true,
 * though, as multiple windows may be bound to the same widget.)  The widget is 
 * expected to implement a draw() function, which will be called
 * when the window needs to be refreshed.
 *
 * Refer to this document for Window Manager function documentation.  The
 * operations on windows can be found in window.h.
 */

//@{

/**
 * A window manager context, one is required for any window management
 * operations.
 */
typedef struct wmctx *wmctx;

/**
 * Creates a new window management context.  This should be called before
 * attempting any other window management operations.  When done, call
 * wm_destroy() to deallocate the context.
 *
 * @param  widget  The initial widget, which will be bound to the first window
 *                 that is automatically created.  This cannot be NULL.
 *
 * @return A new context is returned, or NULL on error.
 */
wmctx wm_create(wm_widget widget);

/**
 * Deallocates the context, destroying all associated windows.
 *
 * @param  context  The context to destroy.
 *
 * @return Zero on success, non-zero on failure.
 */
int wm_destroy(wmctx context);

/**
 * Redraw all visible windows (because the display was damaged for some reason,
 * or maybe the user hit C-l).
 *
 * @param  context  The context to redraw.
 *
 * @return Zero on success, non-zero on failure.
 */
int wm_redraw(wmctx context);

/** 
 * Split the current window horizontally, creating a new window which will 
 * evenly share the space occupied by the original window.
 *
 * @param  widget  The widget which will be bound to the new window.  If NULL
 *                 then the new window will be assigned the same widget as
 *                 the current window.
 * @param  size    Size of the new window (zero means even split).
 *
 * @return The window ID (win_id >= 0) of the new window, or -1 on error.
 */
wid_t wm_hsplit(wm_widget widget, int size);

/** 
 * Split the current window vertically, creating a new window which will evenly
 * share the space occupied by the original window.
 *
 * @param  widget  The widget which will be bound to the new window.  If NULL
 *                 then the new window will be assigned the same widget as
 *                 the current window.
 * @param  size    Size of the new window (zero means even split).
 *
 * @return The window ID (win_id >= 0) of the new window, or -1 on error.
 */
wid_t wm_vsplit(wm_widget widget, int size);

/** 
 * Close the specified window.  Remaining windows will be shuffled to fill in
 * empty screen real estate.
 *
 * @param  win_id  The window ID of the window to close.
 *
 * @return Zero on success, non-zero on failure.
 */
int wm_close(wid_t win_id);

//@}

/**
 * @name WM Options
 * The data types and operations used to set and retrieve options for the
 * window manager.  The options are used to control the behavior of the
 * windows manager in a way that emulates Vim's window system.
 */

//@{

/**
 * Set of options that affect window manager behavior.
 */
typedef enum {

    /**
     * Option "cmdheight" (shorthand: "ch") is of type: integer
     */
    CMDHEIGHT,

    /**
     * Option "eadirection" (shorthand: "ead") is of type: wm_eadir
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
 * Equal always direction 
 */
typedef enum {
    HORIZONTAL,
    VERTICAL,
    BOTH
} wm_eadir;

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
        wm_eadir ead_val;
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

//@}

#endif
