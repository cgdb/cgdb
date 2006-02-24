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

#define CGDB_KEY_RESIZE KEY_MAX

#define MAXLINE 4096

int cgdb_set_esc_sequence_timeout ( int msec );

/* TODO: Remove the below 3 lines. This is a reorganization effort to allow 
 * TGDB to understand the new request/response mechanism that TGDB supports.
 */
struct tgdb;
struct tgdb_request;
int handle_request (struct tgdb *tgdb, struct tgdb_request *request);

void cleanup();

#endif
