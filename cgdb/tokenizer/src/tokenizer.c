#include "tokenizer.h"
#include "sys_util.h"
#include <stdio.h>
#include <string.h>

/* Some default file extensions */
char *c_extensions[] = {
    ".c", ".C", ".cc", ".cpp", ".cxx", ".h", ".hpp", ".hxx" };
char *ada_extensions[] = { ".adb", ".ads", ".ada",".ADB", ".ADS", ".ADA" };

extern int c_lex ( void );
extern FILE *c_in;
extern char *c_text;

extern int ada_lex ( void );
extern FILE *ada_in;
extern char *ada_text;

struct tokenizer {
	enum tokenizer_language_support lang;
	int (*tokenizer_lex)(void);
	FILE **tokenizer_in;
	char **tokenizer_text;

    enum tokenizer_type tpacket;
    struct ibuf *i;
};

struct tokenizer *tokenizer_init ( void ) {
	struct tokenizer *n = ( struct tokenizer * ) cgdb_malloc ( sizeof ( struct tokenizer ) );
	n->i = ibuf_init ();
	n->lang = TOKENIZER_LANGUAGE_UNKNOWN;
	n->tokenizer_lex = NULL;
	n->tokenizer_in = NULL;
	n->tokenizer_text = NULL;
	return n;
}

int tokenizer_set_file ( struct tokenizer *t, const char *file, enum tokenizer_language_support l ) {

	if ( l < TOKENIZER_ENUM_START_POS || l >= TOKENIZER_LANGUAGE_UNKNOWN )
        return 0;
	
	t->lang=l;

	if ( l == TOKENIZER_LANGUAGE_C ) {
		t->tokenizer_lex 	= c_lex;
		t->tokenizer_in  	= &c_in;
		t->tokenizer_text 	= &c_text;
	} else {
		t->tokenizer_lex 	= ada_lex;
		t->tokenizer_in  	= &ada_in;
		t->tokenizer_text 	= &ada_text;
	}
	
    *(t->tokenizer_in) = fopen ( file, "r" );

    if ( !(*(t->tokenizer_in)) ) {
        fprintf ( stderr, "%s:%d tokizer_set_file error", __FILE__, __LINE__ );
        return -1;
    }

    return 0;
}

int tokenizer_get_token ( struct tokenizer *t ) {
	if ( t == NULL || t->tokenizer_lex == NULL )
		return 0;

	t->tpacket = (t->tokenizer_lex)();
	ibuf_clear ( t->i );
	ibuf_add ( t->i, ( const char * ) *(t->tokenizer_text) );

	if ( !(t->tpacket) ) {
		fclose ( *(t->tokenizer_in) );
		return 0;
	}
	
    return 1;
}

enum tokenizer_type tokenizer_get_packet_type ( struct tokenizer *t ) {
	return t->tpacket;
}

const char *tokenizer_get_printable_enum ( enum tokenizer_type e ) {
	const char *enum_array[] = {
		"TOKENIZER_KEYWORD",
		"TOKENIZER_TYPE",
		"TOKENIZER_LITERAL",
		"TOKENIZER_NUMBER",
		"TOKENIZER_COMMENT",
		"TOKENIZER_DIRECTIVE",
		"TOKENIZER_TEXT",
		"TOKENIZER_NEWLINE"
	};

	if ( e >= TOKENIZER_KEYWORD && e < TOKENIZER_ERROR )
		return enum_array[e - TOKENIZER_ENUM_START_POS];
	else if ( e == TOKENIZER_ERROR )
		return "TOKEN ERROR!";

	return NULL;
}

char *tokenizer_get_data ( struct tokenizer *t ) {
	char *string = ibuf_get ( t->i );
	return string;
}

enum tokenizer_language_support tokenizer_get_default_file_type ( 
			const char *file_extension ) {
	enum tokenizer_language_support l = TOKENIZER_LANGUAGE_UNKNOWN;
	int i;

	if ( !file_extension )
        return TOKENIZER_LANGUAGE_UNKNOWN;

    for (i = 0; i < sizeof(c_extensions) / sizeof(char *); i++)
        if (strcmp(file_extension, c_extensions[i]) == 0)
            l = TOKENIZER_LANGUAGE_C;

    for (i = 0; i < sizeof(ada_extensions) / sizeof(char *); i++)
        if (strcmp(file_extension, ada_extensions[i]) == 0)
            l = TOKENIZER_LANGUAGE_ADA;

    return l;
}
