#include "ibuf.h"
#include "sys_util.h"

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

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

void string_add ( struct string *s, const char *d ) {
    int length = strlen ( d ), i;

    for ( i = 0; i < length; i++ )
        string_addchar ( s, d[i] );
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

struct string *string_dup ( struct string *s ) {
	struct string *ns = string_init ();
	string_add ( ns, string_get ( s ) );
	return ns;	
}
