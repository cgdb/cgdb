#ifndef __UTIL_H__
#define __UTIL_H__

#include <stdlib.h>

/* These are wrappers for the memory management functions 
 * If a memory allocation fails cgdb will exit
 */
void *xcalloc(size_t nmemb, size_t size);
void *xmalloc(size_t size);
void *xrealloc(void *ptr, size_t size);
char *xstrdup(const char *s);
int xclose(int fd);

#endif
