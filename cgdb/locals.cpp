/* Local Includes */
#include "sys_util.h"
#include "stretchy.h"
#include "sys_win.h"
#include "cgdb.h"
#include "cgdbrc.h"
#include "highlight_groups.h"
#include "locals.h"
#include "highlight.h"

struct lviewer *locals_new(SWINDOW *win) {
    struct lviewer *viewer;
    viewer = (lviewer *)cgdb_malloc(sizeof(lviewer));
    viewer->win = win;
    return viewer;
}

void locals_refresh(struct lviewer *viewer, int focus, enum win_refresh dorefresh)
{
    swin_werase(viewer->win);

    const char *str = "Local Variables\n";
    hl_printline(viewer->win, str, strlen(str), NULL, -1, -1, 0, strlen(str));

    tgdb_stack_variable *vars = viewer->locals;

    while(vars != NULL) {
        char *str = (char *) calloc(strlen(vars->name) + strlen(vars->value) + 2, sizeof(char));
        sprintf(str, "%s: %s\n", vars->name, vars->value);
        hl_printline(viewer->win, str, strlen(str), NULL, -1, -1, 0, strlen(str));
        vars = vars->next;
    }

    switch(dorefresh) {
        case WIN_NO_REFRESH:
            swin_wnoutrefresh(viewer->win);
            break;
        case WIN_REFRESH:
            swin_wrefresh(viewer->win);
            break;
    }
}

static void free_tgdb_stack_variables(tgdb_stack_variable *variables)
{
    if (variables == NULL) return;
    free_tgdb_stack_variables(variables->next);
    free(variables->name);
    free(variables->value);
    free(variables);
}

void locals_free(struct lviewer *viewer) {
    swin_delwin(viewer->win);
    viewer->win = NULL;

    free_tgdb_stack_variables(viewer->locals);

    /* Release the viewer object */
    free(viewer);
}

void locals_move(struct lviewer *viewer, SWINDOW *win) {
    swin_delwin(viewer->win);
    viewer->win = win;
}
