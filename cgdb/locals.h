#ifndef _LOCALS_H_
#define _LOCALS_H_

#include "sys_win.h"
#include "cgdb.h"
#include "../lib/tgdb/tgdb_types.h"


struct lviewer {
    struct tgdb_stack_variable *locals;
    SWINDOW *win; /* the curses window for rendering */
};

struct lviewer *locals_new(SWINDOW *win);
void locals_free(struct lviewer *viewer);
void locals_refresh(struct lviewer *viewer, int focus, enum win_refresh dorefresh);
void locals_move(struct lviewer *viewer, SWINDOW *win);

#endif /* ifndef _LOCALS_H_ */
