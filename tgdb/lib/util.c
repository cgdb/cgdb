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

struct string {
    char *buf;
    unsigned long cur_buf_pos;
    unsigned long cur_block_size; 
    unsigned long BLOCK_SIZE;
};

struct string *string_init(void) {
    struct string *s = (struct string *)xmalloc(sizeof(struct string));

    s->BLOCK_SIZE       = 4096;
    s->cur_block_size   = 1;
    s->cur_buf_pos      = 0;
    s->buf = (char *)xmalloc(sizeof(char)*(s->BLOCK_SIZE));
    s->buf[s->cur_buf_pos] = '\0';

    return s;
}

void string_free(struct string *s) {
    if ( !s )
        return;

    free(s->buf);
    free(s);
    s = NULL;
}

void string_clear(struct string *s) {
    if ( !s )
        return;

    s->cur_block_size = 1;
    s->cur_buf_pos    = 0;
    s->buf[s->cur_buf_pos] = '\0';
}

void string_addchar(struct string *s, char c) {
    if ( !s )
        return; 

    /* the '+1' is for the null-terminated char */
    if(s->cur_buf_pos + 1 == ((s->cur_block_size)*s->BLOCK_SIZE)){
        /* NOTE: s->cur_block_size *= 2 would double the buffer size 
         *  however, I didn't think it was necesary since it should rarly go
         *  above 1 block.
         */
        s->cur_block_size += 1;
        s->buf = (char *)realloc(s->buf, s->cur_block_size*s->BLOCK_SIZE);
    }

    /* Add the new char and null terminate */
    s->buf[(s->cur_buf_pos)++] = c;
    s->buf[(s->cur_buf_pos)] = '\0';
}

void string_delchar(struct string *s) {
    if ( !s )
        return; 

    if ( s->cur_buf_pos > 0 ) {
        --(s->cur_buf_pos);
        s->buf[(s->cur_buf_pos)] = '\0';
    }
}

char *string_get(struct string *s) {
    if ( !s )
        return NULL; 

    return s->buf;
}

unsigned long string_length(struct string *s) {
    if ( !s )
        return 0; 
    return s->cur_buf_pos;
}
