#ifndef __CGDB_WM_SPLITTER_H__
#define __CGDB_WM_SPLITTER_H__

#include "wm_window.h"
#include "types.h"

/**
 * Splitter structure.
 */
typedef struct {
    wm_window window;
    wm_orientation orientation;
    wm_window **children;
    size_t num_children; /* Number of children */
    size_t array_length; /* Length of children array */
} wm_splitter;

/**
 * Create a new splitter window (derived from wm_window).
 *
 * @param orientation
 * The orientation of the split.
 *
 * @return
 * Returns a newly allocated wm_splitter.
 */
wm_splitter *wm_splitter_create(wm_orientation orientation);

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
                              wm_orientation dir, int size);

/**
 * Split a child of this splitter in the opposite orientation (creates a new
 * splitter).
 *
 * This triggers a splitter layout event, you do not need to manually call
 * layout or redraw after this.
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
                      wm_window *new_window, wm_orientation orientation);

/**
 * Find the window neighboring the given window, in the given direction.  The
 * cursor position is used to make this more precise, but can also be set to
 * (0, 0) if you aren't that picky.
 *
 * If the position falls outside of the bounds of this splitter, it will find
 * the nearest match within this splitter.  For example, if the splitter
 * starts at column 20 and the position is 10, the result will be the first
 * child (at column 20).
 *
 * @param splitter
 * The splitter object.
 *
 * @param window
 * The window whose neighbor we'd like to find.
 *
 * @param dir
 * The direction to look.
 *
 * @param cursor_pos
 * The current absolute (relative to screen) cursor position.
 *
 * @return
 * The neighboring window if found, otherwise NULL.
 */
wm_window *wm_splitter_get_neighbor(wm_splitter *splitter, wm_window *window,
                                    wm_direction dir, wm_position cursor_pos);

#endif /* __CGDB_WM_SPLITTER_H__ */
