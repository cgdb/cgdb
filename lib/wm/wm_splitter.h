#ifndef __CGDB_WM_SPLITTER_H__
#define __CGDB_WM_SPLITTER_H__

#include "window.h"
#include "types.h"

/**
 * Splitter structure.
 */
typedef struct {
    wm_window window;
    wm_direction orientation;
    std_list children; /* List of (wm_window *) */
} wm_splitter;

/**
 * Create a new splitter window (derived from wm_window).  Be sure to call its
 * init function before using.
 *
 * @return
 * Returns a newly allocated wm_splitter.
 */
wm_splitter *wm_splitter_create(wm_direction orientation);

/**
 * Add a window to a splitter.
 *
 * @param splitter
 * The splitter to add to.
 *
 * @param window
 * The window to add.  The splitter now owns this pointer.
 *
 * @return
 * Zero on success, non-zero on failure.
 */
int wm_splitter_add(wm_splitter *splitter, wm_window *window);

/**
 * Remove a window from a splitter.
 *
 * @param splitter
 * The splitter to remove from.
 *
 * @param window
 * The window to remove, which will be destroyed.
 *
 * @return
 * Zero on success, non-zero on failure.
 */
int wm_splitter_remove(wm_splitter *splitter, wm_window *window);

#endif /* __CGDB_WM_SPLITTER_H__ */
