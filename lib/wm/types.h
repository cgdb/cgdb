#ifndef __CGDB_WM_TYPES_H__
#define __CGDB_WM_TYPES_H__

/**
 * Orientation values.
 */
typedef enum {
    WM_HORIZONTAL,
    WM_VERTICAL,
    WM_BOTH,
} wm_orientation;

/**
 * Direction values.
 */
typedef enum {
    WM_UP,
    WM_DOWN,
    WM_LEFT,
    WM_RIGHT,
} wm_direction;

/**
 * Represents a screen position.
 */
typedef struct {
    int top;
    int left;
} wm_position;

#endif /* __CGDB_WM_TYPES_H__ */
