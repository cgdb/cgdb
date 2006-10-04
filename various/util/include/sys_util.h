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

#endif
