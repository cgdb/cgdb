#if 0
/* wm.c:
 * -----
 *
 * Window manager code for CGDB.  See header for full documentation.
 */

/* Standard Includes */
#include <assert.h>

/* Project Includes */
#include <std_list.h>
#include <std_tree.h>

/* Local Includes */
#include "wm.h"

/* --------------- */
/* Data Structures */
/* --------------- */

/** 
 * The Window Manager context structure.  This is passed to every method in
 * the interface, and tracks all the state information associated with 
 * windows management.
 */
struct wmctx {

    /** 
     * Windows is the list of windows in memory.  Not all windows are on the
     * screen at a given time, so the locations of the windows are stored in
     * the grid field. 
     *
     * Element type: wm_window
     */
    std_list windows;

    /**
     * Layout stores the arrangement of the windows on the screen.  It is a
     * binary tree, with the following rationale:  Windows in vi are created
     * via "split" functions.  A single window is split into two windows.
     * This lends itself to the binary tree structure as the node can just
     * create two children -- the original window, and the new window.
     *
     * This leads to certain conditions.  Every node has either zero or two
     * children, never one.  If a node has no children, it represents a window
     * on the screen.  If a node has 2 children, it is not a displayed window,
     * but rather a screen region which is shared by its two child windows.
     * (Of course, either or both of these "windows" could be split as well,
     * the parent does not care.)
     *   
     * Element type: tree_node
     *
     * \see tree_node
     */
    std_tree layout;

};

/**
 * The tree_node structure which represents a window or a parent screen region
 * in the 'layout' member of wmctx.
 *
 * \see wmctx
 */
typedef struct tree_node {

    /**
     * Window is a pointer to the window which this node represents.  If it
     * is NULL, then this is a parent node which has two child windows.
     * Note that this structure is not responsible for window allocation
     * and deallocation, it simply refers to existing windows.  Window
     * creation and destruction should be done through the wmctx->windows.
     */
    wm_window window;

} *tree_node;

/* ------------------------- */
/* Local Function Prototypes */
/* ------------------------- */

/**
 * Creates a tree_node object.  Window is initialized to NULL.
 * height default to zero.
 *
 * @return Newly allocated node, or NULL on failure.
 */
static tree_node tree_node_create();

/**
 * Destroys a tree_node object.
 *
 * @param node  The node to destroy, if NULL no action is performed.
 *
 * @return Zero on success, non-zero on failure.  This method never fails.
 */
static int tree_node_destroy(tree_node node);

/* --------- */
/* Functions */
/* --------- */

/* wm_create:
 */
wmctx wm_create(wm_widget widget)
{
    /* Parameter bounds check */
    assert(widget != NULL)

            /* Allocate a new context and initial window */
    wmctx context = (wmctx) malloc(sizeof (struct wmctx));
    wm_window window = window_create(widget);

    /* Be extra paranoid */
    if (window == NULL) {
        free(context);
        context = NULL;
    }

    if (context != NULL) {

        /* TODO: Verify that this is the safest way to destroy our lists. */
        context->windows = std_list_create((STDDestroyNotify) window_destroy);
        context->grid = std_list_create((STDDestroyNotify) wm_row_destroy);

        /* Populate the window list and grid with an initial window */
        window->id = 0;
        std_list_append(context->windows, window);
        std_list_append(row->windows, window);

        /* Slap that initial row into the grid */
        std_list_append(context->grid, row);
    }

    return context;
}

/* wm_destroy:
 */
int wm_destroy(wmctx context)
{
    if (context != NULL) {
        std_list_destroy(context->windows);
        std_list_destroy(context->grid);
        free(context);
    }

    return 0;
}

/* wm_redraw:
 */
int wm_redraw(wmctx context)
{
    return 0;
}

/* wm_hsplit:
 */
wid_t wm_hsplit(wm_widget widget, int size)
{
    /* Not implemented */
    return -1;
}

/* wm_vsplit:
 */
wid_t wm_vsplit(wm_widget widget, int size)
{
    /* Not implemented */
    return -1;
}

/* wm_close:
 */
int wm_close(wid_t win_id)
{
    return 0;
}

/* wm_option_get:
 */
wm_optval wm_option_get(wm_option option)
{
    wm_optval rv;

    rv.type = WM_UNKNOWN;

    return rv;
}

/* wm_option_set:
 */
int wm_option_set(wm_option option, wm_optval value)
{
    return 0;
}

/* ------------------------------ */
/* Local Function Implementations */
/* ------------------------------ */

static tree_node tree_node_create()
{
    tree_node node = malloc(sizeof (struct tree_node));

    /* Be extra paranoid */
    if (node) {
        node->window = NULL;
    }

    return node;
}

static int tree_node_destroy(tree_node node)
{
    if (node != NULL) {
        free(row);
    }

    return 0;
}

#endif
