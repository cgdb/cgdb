#include "ibuf.h"
#include "sys_util.h"

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

struct ibuf {
    char *buf;
    unsigned long cur_buf_pos;
    unsigned long cur_block_size; 
    unsigned long BLOCK_SIZE;
};

struct ibuf *ibuf_init(void) {
    struct ibuf *s = (struct ibuf *)xmalloc(sizeof(struct ibuf));

    s->BLOCK_SIZE       = 4096;
    s->cur_block_size   = 1;
    s->cur_buf_pos      = 0;
    s->buf = (char *)xmalloc(sizeof(char)*(s->BLOCK_SIZE));
    s->buf[s->cur_buf_pos] = '\0';

    return s;
}

void ibuf_free(struct ibuf *s) {
    if ( !s )
        return;

    free(s->buf);
    free(s);
    s = NULL;
}

void ibuf_clear(struct ibuf *s) {
    if ( !s )
        return;

    s->cur_block_size = 1;
    s->cur_buf_pos    = 0;
    s->buf[s->cur_buf_pos] = '\0';
}

void ibuf_addchar(struct ibuf *s, char c) {
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

void ibuf_add ( struct ibuf *s, const char *d ) {
    int length = strlen ( d ), i;

    for ( i = 0; i < length; i++ )
        ibuf_addchar ( s, d[i] );
}

void ibuf_delchar(struct ibuf *s) {
    if ( !s )
        return; 

    if ( s->cur_buf_pos > 0 ) {
        --(s->cur_buf_pos);
        s->buf[(s->cur_buf_pos)] = '\0';
    }
}

char *ibuf_get(struct ibuf *s) {
    if ( !s )
        return NULL; 

    return s->buf;
}

unsigned long ibuf_length(struct ibuf *s) {
    if ( !s )
        return 0; 
    return s->cur_buf_pos;
}

struct ibuf *ibuf_dup ( struct ibuf *s ) {
	struct ibuf *ns = ibuf_init ();
	ibuf_add ( ns, ibuf_get ( s ) );
	return ns;	
}
