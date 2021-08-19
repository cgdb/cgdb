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

#define MAXLINE 4096

/* Clean cgdb up (when exiting) */
void cgdb_cleanup_and_exit(int val);

/*
 * See documentation in cgdb.c.
 */
int run_shell_command(const char *command);

#endif
