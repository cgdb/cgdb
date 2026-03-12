/* Local Includes */
#include "sys_util.h"
#include "sys_win.h"
#include "highlight_groups.h"
#include "locals.h"

extern hl_groups_ptr hl_groups_instance;

struct lviewer *locals_new(SWINDOW *win)
{
    struct lviewer *viewer = new lviewer();
    viewer->win = win;
    return viewer;
}

void locals_free(struct lviewer *viewer)
{
    if (!viewer)
        return;
    if (viewer->win) {
        swin_delwin(viewer->win);
        viewer->win = NULL;
    }
    delete viewer;
}

void locals_set(struct lviewer *viewer,
                const std::list<tgdb_local_variable> &locals)
{
    viewer->locals = locals;
}

void locals_refresh(struct lviewer *viewer, int focus,
                    enum win_refresh dorefresh)
{
    int width = swin_getmaxx(viewer->win);

    swin_werase(viewer->win);

    /* Header row */
    int header_attr = hl_groups_get_attr(hl_groups_instance, HLG_STATUS_BAR);
    swin_wattron(viewer->win, header_attr);
    const char *header = "Local Variables";
    swin_mvwprintw(viewer->win, 0, 0, "%-*s", width, header);
    swin_wattroff(viewer->win, header_attr);

    /* One variable per row */
    int row = 1;
    for (const tgdb_local_variable &var : viewer->locals) {
        const char *prefix = var.is_arg ? "[arg] " : "      ";
        const std::string &val = var.value.empty() ? std::string("<complex>") : var.value;

        /* Truncate to window width so ncurses doesn't wrap */
        char buf[4096];
        snprintf(buf, sizeof(buf), "%s%s = %s",
                 prefix, var.name.c_str(), val.c_str());
        swin_mvwprintw(viewer->win, row, 0, "%-*.*s", width, width, buf);
        row++;
    }

    switch (dorefresh) {
        case WIN_NO_REFRESH:
            swin_wnoutrefresh(viewer->win);
            break;
        case WIN_REFRESH:
            swin_wrefresh(viewer->win);
            break;
    }
}

void locals_move(struct lviewer *viewer, SWINDOW *win)
{
    if (viewer->win)
        swin_delwin(viewer->win);
    viewer->win = win;
}
