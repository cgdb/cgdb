#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#if HAVE_UNISTD_H
#include <unistd.h>
#endif  /* HAVE_UNISTD_H */

#if HAVE_ERRNO_H
#include <errno.h>
#endif /* HAVE_ERRNO_H */

#include "sys_util.h"

void *cgdb_calloc(size_t nmemb, size_t size) {
    void *t = calloc(nmemb, size);
    if (t)
        return t;

    exit(-1);
}

void *cgdb_malloc(size_t size) {
    void *t = malloc(size);
    if (t)
        return t;

    exit(-1);
}

void *cgdb_realloc(void *ptr, size_t size) {
    void *t = realloc(ptr, size);
    if (t)
        return t;

    exit(-1);
}

char *cgdb_strdup(const char *s) {
    char *t = strdup(s);
    if (t)
        return t;

    exit(-1);
}

int cgdb_close(int fd) {
    int ret;
cgdb_close_start:
    if ( ( ret = close(fd)) == -1 && errno == EINTR)
        goto cgdb_close_start;
    else if ( ret == -1 )
        exit(-1);
    
    return 0;
}
