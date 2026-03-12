#ifndef _LOCALS_H_
#define _LOCALS_H_

#include <list>
#include "sys_win.h"
#include "tgdb.h"

/**
 * The locals viewer window — displays local variables and function arguments
 * for the current stack frame.
 */
struct lviewer {
    /* The current local variables / arguments to display. */
    std::list<tgdb_local_variable> locals;
    /* The curses window used for rendering. */
    SWINDOW *win;
};

/** Allocate a new lviewer attached to the given window. */
struct lviewer *locals_new(SWINDOW *win);

/** Free all resources owned by the viewer (including the window). */
void locals_free(struct lviewer *viewer);

/**
 * Replace the displayed locals list.
 *
 * @param viewer  The lviewer to update.
 * @param locals  The new list of local variables and arguments.
 */
void locals_set(struct lviewer *viewer,
                const std::list<tgdb_local_variable> &locals);

/** Redraw the locals pane. */
void locals_refresh(struct lviewer *viewer, int focus,
                    enum win_refresh dorefresh);

/** Move the viewer to a new curses window (old window is deleted). */
void locals_move(struct lviewer *viewer, SWINDOW *win);

#endif /* _LOCALS_H_ */
