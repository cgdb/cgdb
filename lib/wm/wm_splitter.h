#ifndef __CGDB_WM_SPLITTER_H__
#define __CGDB_WM_SPLITTER_H__

#include "wm_window.h"
#include "types.h"

/**
 * Splitter structure.
 */
typedef struct {
    wm_window window;
    wm_direction orientation;
    wm_window **children;
    size_t num_children; /* Number of children */
    size_t array_length; /* Length of children array */
} wm_splitter;

/**
 * Create a new splitter window (derived from wm_window).  Be sure to call its
 * init function before using.
 *
 * @param orientation
 * The orientation of the split.
 *
 * @return
 * Returns a newly allocated wm_splitter.
 */
wm_splitter *wm_splitter_create(wm_direction orientation);

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

/**
 * Resize the given window in the given direction.  Other windows will be
 * pushed as necessary to accomodate the new space.
 *
 * The screen will be redrawn after the window is resized.
 *
 * @param splitter
 * The splitter.
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
int wm_splitter_resize_window(wm_splitter *splitter, wm_window *window,
                              wm_direction dir, int size);

/**
 * Split a child of this splitter in the opposite orientation (creates a new
 * splitter).
 *
 * @param splitter
 * The splitter that owns the child.
 *
 * @param window
 * The child window to be split, or NULL to place the window at the end.
 *
 * @param new_window
 * The new window to become a peer of window.
 *
 * @param orientation
 * The orientation of the new split.
 *
 * @return
 * Zero on success, non-zero on failure.
 */
int wm_splitter_split(wm_splitter *splitter, wm_window *window,
                      wm_window *new_window, wm_direction orientation);

#endif /* __CGDB_WM_SPLITTER_H__ */
