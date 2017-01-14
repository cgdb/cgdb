#include "sys_util.h"
#include "ibuf.h"

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

struct ibuf {
    char *buf;
    unsigned long pos;
    unsigned long size;
};

struct ibuf *ibuf_init(void)
{
    struct ibuf *s = (struct ibuf *) cgdb_malloc(sizeof (struct ibuf));

    s->size = 8192;
    s->buf = (char *) cgdb_malloc(s->size);

    s->pos = 0;
    s->buf[0] = 0;
    return s;
}

void ibuf_free(struct ibuf *s)
{
    if (s) {
        free(s->buf);
        free(s);
    }
}

void ibuf_clear(struct ibuf *s)
{
    if (s) {
        s->pos = 0;
        s->buf[0] = '\0';
    }
}

static void ibuf_ensuresize(struct ibuf *s, unsigned long size)
{
    if (size > s->size) {
        while (s->size < size)
            s->size *= 2;

        s->buf = (char *) cgdb_realloc(s->buf, s->size);
    }
}

void ibuf_addchar(struct ibuf *s, char c)
{
    if (s) {
        ibuf_ensuresize(s, s->pos + 2);

        s->buf[s->pos++] = c;
        s->buf[s->pos] = 0;
    }
}

int ibuf_add(struct ibuf *s, const char *d)
{
    int length = strlen(d);

    ibuf_ensuresize(s, s->pos + length + 2);

    strcpy(s->buf + s->pos, d);
    s->pos += length;
    return length;
}

void ibuf_delchar(struct ibuf *s)
{
    if (s && (s->pos > 0))
        s->buf[--(s->pos)] = 0;
}

char *ibuf_get(struct ibuf *s)
{
    return s ? s->buf : NULL;
}

unsigned long ibuf_length(struct ibuf *s)
{
    return s ? s->pos : 0;
}

struct ibuf *ibuf_dup(struct ibuf *s)
{
    struct ibuf *ns = ibuf_init();

    ibuf_add(ns, ibuf_get(s));
    return ns;
}
