/* wm.h:
 * -----
 * 
 * Window manager library for CGDB.  This abstracts the management of "windows"
 * in the terminal, so that the calling program can create, arrange and delete
 * windows in the terminal space.
 *
 * This management is transparent to the confined widget, which can do input
 * and drawing without knowledge of actual screen coordinates or whether it
 * is seen by the user at all.
 *
 * A window is bound to a single widget at the time of creation, and they are
 * inextricably bound until the window is closed.  (The opposite is not true,
 * though, as multiple windows may be bound to the same widget.)
 *
 * The widget is expected to implement a draw() function, which will be called
 * when the window needs to be refreshed.
 *
 * Refer to this document for Window Manager function documentation.  The
 * operations on windows can be found in window.h.
 */

#ifndef _WM_H_
#define _WM_H

/* Local Includes */
#include "window.h"

/* ------- */
/* Options */
/* ------- */

/* List of options */
typedef enum {
    CMDHEIGHT,          /* ch:  Integer */
    EADIRECTION,        /* ead: wm_eadir */
    EQUALALWAYS,        /* ea:  Boolean */
    SPLITBELOW,         /* sb:  Boolean */
    SPLITRIGHT,         /* spr: Boolean */
    WINFIXHEIGHT,       /* wfh: Boolean */
    WINMINHEIGHT,       /* wmh: Integer */
    WINMINWIDTH,        /* wmw: Integer */
    WINHEIGHT,          /* wh:  Integer */
    WINWIDTH            /* wiw: Integer */
} wm_option;

/* Equal always direction */
typedef enum {
    HORIZONTAL,
    VERTICAL,
    BOTH
} wm_eadir;

/* Option value */
typedef enum {
    WM_INTEGER,
    WM_BOOLEAN,
    WM_EADIR,
    WM_UNKNOWN
} wm_opttype;

typedef struct {
    /* Type of option this structure describes */
    wm_opttype type;

    union {
        /* if (type == WM_INTEGER) */
        int int_val;

        /* if (type == WM_BOOLEAN) */
        char bool_val;

        /* if (type == WM_EADIR) */
        wm_eadir ead_val;
    } variant;
} wm_optval;

/* ------------------ */
/* Function Protoypes */
/* ------------------ */

/* wm_option_get: Get the value of the specified option.
 * --------------
 *
 *   option: The option value to retrieve
 *
 * Return Value: The value of the specified option.  The type will be set
 *               to WM_UNKNOWN if an unknown option is specified.
 */
wm_optval wm_option_get(wm_option option);

/* wm_option_set: Set the value of the specified option.
 * --------------
 *
 *   option: The option to set
 *   value:  The value to set for the given option
 *
 * Return Value: Zero on success, non-zero on failure.
 */
int wm_option_set(wm_option option, wm_optval value);

/* wm_hsplit: Split the current window horizontally.
 * ----------
 *
 *   widget: The widget which will be bound to the new window 
 *   size:   Size of the new window (zero means even split)
 *
 * Return Value: The window ID of the new window.
 */
wid_t wm_hsplit(wm_widget widget, int size);

/* wm_vsplit: Split the current window vertically.
 * ----------
 *
 *   widget: The widget to which the new window will be bound
 *   size:   Size of the new window (zero means even split)
 *
 * Return Value: The window ID of the new window.
 */
wid_t wm_vsplit(wm_widget widget, int size);

/* wm_close: Close the specified window.
 * ---------
 *
 *   win_id: The window ID of the window to close
 *
 * Return Value: Zero on success, non-zero on failure.
 */
int wm_close(wid_t win_id);

#endif
