#include "ibuf.h"
#include "sys_util.h"

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#if HAVE_CTYPE_H
#include <ctype.h>
#endif /* HAVE_CTYPE_H */

struct ibuf {
    char *buf;
    unsigned long cur_buf_pos;
    unsigned long cur_block_size;
    unsigned long BLOCK_SIZE;
};

struct ibuf *ibuf_init(void)
{
    struct ibuf *s = (struct ibuf *) cgdb_malloc(sizeof (struct ibuf));

    s->BLOCK_SIZE = 4096;
    s->cur_block_size = 1;
    s->cur_buf_pos = 0;
    s->buf = (char *) cgdb_malloc(sizeof (char) * (s->BLOCK_SIZE));
    s->buf[s->cur_buf_pos] = '\0';

    return s;
}

void ibuf_free(struct ibuf *s)
{
    if (!s)
        return;

    free(s->buf);
    free(s);
    s = NULL;
}

void ibuf_clear(struct ibuf *s)
{
    if (!s)
        return;

    s->cur_block_size = 1;
    s->cur_buf_pos = 0;
    s->buf[s->cur_buf_pos] = '\0';
}

void ibuf_addchar(struct ibuf *s, char c)
{
    if (!s)
        return;

    /* the '+1' is for the null-terminated char */
    if (s->cur_buf_pos + 1 == ((s->cur_block_size) * s->BLOCK_SIZE)) {
        /* NOTE: s->cur_block_size *= 2 would double the buffer size 
         *  however, I didn't think it was necesary since it should rarly go
         *  above 1 block.
         */
        s->cur_block_size += 1;
        s->buf = (char *) realloc(s->buf, s->cur_block_size * s->BLOCK_SIZE);
    }

    /* Add the new char and null terminate */
    s->buf[(s->cur_buf_pos)++] = c;
    s->buf[(s->cur_buf_pos)] = '\0';
}

void ibuf_add(struct ibuf *s, const char *d)
{
    int length = strlen(d), i;

    for (i = 0; i < length; i++)
        ibuf_addchar(s, d[i]);
}

void ibuf_delchar(struct ibuf *s)
{
    if (!s)
        return;

    if (s->cur_buf_pos > 0) {
        --(s->cur_buf_pos);
        s->buf[(s->cur_buf_pos)] = '\0';
    }
}

char *ibuf_get(struct ibuf *s)
{
    if (!s)
        return NULL;

    return s->buf;
}

unsigned long ibuf_length(struct ibuf *s)
{
    if (!s)
        return 0;
    return s->cur_buf_pos;
}

struct ibuf *ibuf_dup(struct ibuf *s)
{
    struct ibuf *ns = ibuf_init();

    ibuf_add(ns, ibuf_get(s));
    return ns;
}

void ibuf_trim(struct ibuf *s)
{

    int i, j = 0;

    if (!s)
        return;

    /* Find the first non-space character */
    for (i = 0; i < s->cur_buf_pos && isspace(s->buf[i]); i++);

    /* If spaces were found, shift string leftward to overwrite them */
    if (i > 0) {
        for (j = 0; i < s->cur_buf_pos; j++, i++) {
            s->buf[j] = s->buf[i];
        }

        /* Update buf pos and null terminate */
        s->cur_buf_pos = j;
        s->buf[s->cur_buf_pos] = '\0';
        if (s->cur_buf_pos == 0) {
            return;
        }
    }

    /* Loop from the end, looking for spaces */
    for (i = s->cur_buf_pos - 1; i > 0 && isspace(s->buf[i]); i--);

    s->cur_buf_pos = i + 1;
    s->buf[s->cur_buf_pos] = '\0';
}
