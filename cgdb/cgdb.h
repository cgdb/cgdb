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

/* From the ncurses doupdate() man page:
 *
 * The routine wrefresh works by first calling wnoutrefresh, which copies the
 * named window to the virtual screen, and then calling doupdate, which compares
 * the virtual screen to the physical screen and does the actual update. If the
 * programmer wishes to output several windows at once, a series of calls to
 * wrefresh results in alternating calls to wnoutrefresh and doupdate, causing
 * several bursts of output to the screen. By first calling wnoutrefresh for each
 * window, it is then possible to call doupdate once, resulting in only one burst
 * of output, with fewer total characters transmitted and less CPU time used.
 *
 * So we use the win_refresh option to tell the routine whether to use
 * wrefresh() or wnoutrefresh(). This eliminates quite a bit of flashing as well.
 */
enum win_refresh { WIN_NO_REFRESH, WIN_REFRESH };

/* TODO: Remove the below 3 lines. This is a reorganization effort to allow 
 * TGDB to understand the new request/response mechanism that TGDB supports.
 */
struct tgdb;
struct tgdb_request;
int handle_request(struct tgdb *tgdb, struct tgdb_request *request);

void cleanup();

/*
 * See documentation in cgdb.c.
 */
int run_shell_command(const char *command);

#endif
