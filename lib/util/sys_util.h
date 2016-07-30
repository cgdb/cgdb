#ifndef __SYS_UTIL_H__
#define __SYS_UTIL_H__

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif /* HAVE_STDLIB_H */

/* These are wrappers for the memory management functions 
 * If a memory allocation fails cgdb will exit
 * They act identical to the POSIX calls
 */
void *cgdb_calloc(size_t nmemb, size_t size);
void *cgdb_malloc(size_t size);
void *cgdb_realloc(void *ptr, size_t size);
char *cgdb_strdup(const char *s);
int cgdb_close(int fd);

/**
 * Convert a string to an integer.
 *
 * @param str
 * The string to convert.
 *
 * @param num
 * The integer result on success.
 * On failure, the value passed in will remain unchanged.
 *
 * @return
 * 0 on success or -1 on error.
 */
int cgdb_string_to_int(char *str, int *num);

/**
 * Check to see if cgdb supports debugger attachment detection.
 *
 * @return
 * 1 if it does support detecting debugger attachment, otherwise 0.
 */
int cgdb_supports_debugger_attach_detection();

/* Check if debugger is attached to cgdb.
 * Return 0 for no, 1 for yes, -1 for error.
 */
int cgdb_is_debugger_attached();

#endif
