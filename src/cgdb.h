/* cgdb.h:
 * -------
 *
 * Contains macros for any of the sources here to use.
 */

#ifndef _CGDB_H_
#define _CGDB_H_

/* ----------- */
/* Definitions */
/* ----------- */

#define GDB_MAXBUF 4096               /* GDB input buffer size */

/* Debug utility macro, use it like printf: */
#ifdef DEBUG
#define debug(args...) fprintf(stderr, args)
#else
#define debug(args...)
#endif

/* Colors */
#define CGDB_COLOR_GREEN            1
#define CGDB_COLOR_RED              2
#define CGDB_COLOR_CYAN             3
#define CGDB_COLOR_WHITE            4
#define CGDB_COLOR_MAGENTA          5
#define CGDB_COLOR_BLUE             6
#define CGDB_COLOR_YELLOW           7

#define CGDB_COLOR_INVERSE_GREEN    8
#define CGDB_COLOR_INVERSE_RED      9
#define CGDB_COLOR_INVERSE_CYAN     10
#define CGDB_COLOR_INVERSE_WHITE    11
#define CGDB_COLOR_INVERSE_MAGENTA  12
#define CGDB_COLOR_INVERSE_BLUE     13
#define CGDB_COLOR_INVERSE_YELLOW   14
#define CGDB_COLOR_STATUS_BAR       15

/* Special char to use for vertical line 
 * CYGWIN does not support this character 
 */
#ifdef HAVE_CYGWIN
    #define VERT_LINE ':'
#else
    #define VERT_LINE ACS_VLINE
#endif

/* Keys */
#define CGDB_BACKSPACE_KEY(c) (c == 8 || c == 127 || c == KEY_BACKSPACE)
#define CGDB_ESC 27

/* Control chars used */
#define CONTROL_K 11
#define CONTROL_T 20
#define CONTROL_U 21

#define CGDB_KEY_RESIZE KEY_MAX

#endif
