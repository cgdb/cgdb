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

/* Local Includes */
#include "wm.h"

/* --------------- */
/* Data Structures */
/* --------------- */

/**
 * A row in the grid that is the window layout.  There are no accessor methods
 * for this type, use the members directly.
 *
 * \see wmctx
 */
typedef struct wm_row {
   
   /**
    * Pointers to the windows in this row, from left to right.  These are
    * a subset of the windows found in the wmctx->windows list.
    *
    * Element type: wm_window
    */
   std_list windows;

   /**
    * The height of this row 
    */
   int height;

} *wm_row;

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
     * Grid stores the arrangement of the windows on the screen.  It is a list
     * of rows, each containing a list of windows that span the row. 
     *
     * Element type: wm_row
     */
    std_list grid;

};

/* ------------------------- */
/* Local Function Prototypes */
/* ------------------------- */

/**
 * Creates a wm_row object.  Window list is initialized to an empty list,
 * height default to zero.
 *
 * @return Newly allocated row, or NULL on failure.
 */
static wm_row wm_row_create();

/**
 * Destroys a wm_row object.
 *
 * @param row  The row to destroy, if NULL no action is performed.
 *
 * @return Zero on success, non-zero on failure.  This method never fails.
 */
static int wm_row_destroy(wm_row row);

/* --------- */
/* Functions */
/* --------- */

/* wm_create:
 */
wmctx wm_create(wm_widget widget)
{
    /* Parameter bounds check */
    assert(widget != NULL);

    /* Allocate a new context and initial window */
    wmctx context    = (wmctx)     malloc(sizeof(struct wmctx));
    wm_window window = window_create(widget);
    wm_row    row    = wm_row_create();
    /* Be extra paranoid */
    if (window == NULL || row == NULL) {
        window_destroy(window);
        free(context);
        context = NULL;
    }

    if (context != NULL) {

        /* TODO: Verify that this is the safest way to destroy our lists. */
        context->windows = std_list_create((STDDestroyNotify) window_destroy);
        context->grid    = std_list_create((STDDestroyNotify) wm_row_destroy);

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

static wm_row wm_row_create()
{
    wm_row row = malloc(sizeof(struct wm_row));

    /* Be extra paranoid */
    if (row){

        /* No destroy function passed in because we don't want to destroy
         * windows in this list.  As mentioned above, this list is only used
         * for layout information.  The primary window list is in the wm_ctx.
         */
        row->windows = std_list_create(NULL);
        row->height  = 0;

    }

    return row;
}

static int wm_row_destroy(wm_row row)
{
    if (row != NULL) {
        std_list_destroy(row->windows);
        free(row);
    }

    return 0;
}

#endif

