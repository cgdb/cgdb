#include "util.h"
#include <string.h>
#include <unistd.h>
#include <errno.h>

void *xcalloc(size_t nmemb, size_t size) {
    void *t = calloc(nmemb, size);
    if (t)
        return t;

    exit(-1);
}

void *xmalloc(size_t size) {
    void *t = malloc(size);
    if (t)
        return t;

    exit(-1);
}

void *xrealloc(void *ptr, size_t size) {
    void *t = realloc(ptr, size);
    if (t)
        return t;

    exit(-1);
}

char *xstrdup(const char *s) {
    char *t = strdup(s);
    if (t)
        return t;

    exit(-1);
}

int xclose(int fd) {
    int ret;
xclose_start:
    if ( ( ret = close(fd)) == -1 && errno == EINTR)
        goto xclose_start;
    else if ( ret == -1 )
        exit(-1);
    
    return 0;
}
