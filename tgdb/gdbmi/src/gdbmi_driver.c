#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gdbmi_grammar.h"

int gdbmi_parse (void);

static void usage ( char *progname ) {

	printf ( "%s <file>\n", progname );
	exit ( -1 );
}

extern FILE *gdbmi_in;
extern int gdbmi_lex ( void );
extern char *gdbmi_text;
int gdbmi_get_lineno  (void);

int instrument_lexer ( void ) {
	//enum gdbmi_lexer_pattern pattern;
	enum yytokentype pattern;

	do {
		pattern = gdbmi_lex ();

		if ( pattern == 0 ) {
			printf ( "END_OF_FILE\n" );
			break;
		} else if ( pattern == OPEN_BRACE )
			printf ( "OPEN_BRACE\n" );
		else if ( pattern == CLOSED_BRACE )
			printf ( "CLOSED_BRACE\n" );
		else if ( pattern == OPEN_BRACKET )
			printf ( "OPEN_BRACKET\n" );
		else if ( pattern == CLOSED_BRACKET )
			printf ( "CLOSED_BRACKET\n" );
		else if ( pattern == ADD_OP )
			printf ( "ADD_OP\n" );
		else if ( pattern == MULT_OP )
			printf ( "MULT_OP\n" );
		else if ( pattern == EQUAL_SIGN )
			printf ( "EQUAL_SIGN\n" );
		else if ( pattern == TILDA )
			printf ( "TILDA\n" );
		else if ( pattern == AT_SYMBOL )
			printf ( "AT_SYMBOL\n" );
		else if ( pattern == AMPERSAND )
			printf ( "AMPERSAND\n" );
		else if ( pattern == NEWLINE )
			printf ( "NEWLINE\n" );
		else if ( pattern == INTEGER_LITERAL )
			printf ( "INTEGER_LITERAL\n" );
		else if ( pattern == STRING_LITERAL )
			printf ( "STRING_LITERAL(%s)\n", gdbmi_text );
		else if ( pattern == CSTRING )
			printf ( "CSTRING(%s)\n", gdbmi_text );
		else if ( pattern == COMMA )
			printf ( "COMMA\n" );
		else if ( pattern == CARROT )
			printf ( "CARROT\n" );
		else if ( pattern == GDB )
			printf ( "GDB\n" );
		else if ( pattern == DONE )
			printf ( "DONE\n" );
		else  {
			printf ( "ERROR(%s)\n", gdbmi_text );
			return -1;
		}
	} while ( gdbmi_text );

	return 0;
}


int main ( int argc, char **argv ) {
	if ( argc != 2 )
		usage( argv[0] );	

	/* Initialize data */
	gdbmi_in = fopen ( argv[1], "r" );

	if ( !gdbmi_in ) {
        fprintf ( stderr, "%s:%d gdbmi_lexer error", __FILE__, __LINE__ );
        return -1;
    }

//	if ( instrument_lexer () == -1 )
//		return -1;

	gdbmi_parse ();

	return 0;
}

/* Some stub functions */

void gdbmi_error (const char *s) { 
	printf ( "Error: %s ", s );
	if ( strcmp ( gdbmi_text, "\n" ) == 0 )
		printf ( "at end of line %d\n", gdbmi_get_lineno() );
	else {
		printf ( "at token(%s), line (%d)\n", gdbmi_text, gdbmi_get_lineno() );
		gdbmi_lex();
		printf ( "before (%s)\n", gdbmi_text );
	}
}

void* append_to_list ( void *list, void*item ) {
	void *cur = list;
	void *prev = NULL;
	int size = 0;

	if (!item)
		return NULL;

	while ( cur ) {
		prev = cur;
		cur++;
		size++;
	}

	/* size is now the number of elements in the list 
	 * Add one for the new item, also add one for the null terminating value
	 */
	list = realloc ( list, sizeof ( void*)*(size+2) );

	/* list was empty */
	if ( prev == NULL ) {
		list = item;
		list++;
		list = NULL;
	} else {
		prev++; /* Get to the new item*/
		prev = item;

		prev++;
		prev = NULL;
	}

	return list;
}
