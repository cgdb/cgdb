/* wm.c:
 * -----
 *
 * Window manager code for CGDB.  See header for full documentation.
 */

/* Project Includes */
#include <std_list.h>

/* Local Includes */
#include "wm.h"

/* --------------- */
/* Data Structures */
/* --------------- */

/**
 * A row in the grid that is the window layout.
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
 * Creates a wm_row object.
 *
 * @return Newly allocated row, or NULL on failure.
 */
static wm_row wm_row_create();

/**
 * Destroys a wm_row object.
 *
 * @return Zero on success, non-zero on failure.
 */
static int wm_row_destroy(wm_row row);

/* --------- */
/* Functions */
/* --------- */

/* wm_create:
 */
wmctx wm_create(wm_widget widget)
{
    wmctx rv = malloc(sizeof(struct wmctx));

    return rv;
}

/* wm_destroy:
 */
int wm_destroy(wmctx context)
{
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
    wm_row *rv = malloc(sizeof(struct wm_row));

    if (rv){

    }

    return rv;
}

static int wm_row_destroy(wm_row row)
{
    return -1;
}

