#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_STDLIB_H 
#include <stdlib.h>
#endif  /* HAVE_STDLIB_H */

#include "gdbmi_parser.h"
#include "logger.h"

int gdbmi_parse (void);

/* flex */
typedef struct yy_buffer_state* YY_BUFFER_STATE;
extern YY_BUFFER_STATE gdbmi__scan_string( const char *yy_str );
extern void gdbmi__delete_buffer( YY_BUFFER_STATE state );
extern FILE *gdbmi_in;
extern int gdbmi_lex ( void );
extern char *gdbmi_text;
extern int gdbmi_lineno;
extern gdbmi_output_ptr tree;

struct gdbmi_parser {
	char *last_error;
};

gdbmi_parser_ptr gdbmi_parser_create ( void ) {
	gdbmi_parser_ptr parser;
	
	parser = (gdbmi_parser_ptr)malloc ( sizeof ( struct gdbmi_parser ) );

	if ( !parser ) {
		logger_write_pos ( logger, __FILE__, __LINE__, "malloc failed" );
		return NULL;
	}

	return parser;
}

int gdbmi_parser_destroy ( gdbmi_parser_ptr parser ) {
	
	if ( !parser )
		return -1;

	if ( parser->last_error ) {
		free ( parser->last_error );
		parser->last_error = NULL;
	}

	free ( parser );
	parser = NULL;
	return 0;
}

int gdbmi_parser_parse_string ( 
	gdbmi_parser_ptr parser, 
	const char *mi_command,
    gdbmi_output_ptr *pt ) {

	if ( !parser )
		return -1;

	if ( !mi_command )
		return -1;

	/* Create a new input buffer for flex. */
	YY_BUFFER_STATE state = gdbmi__scan_string( (char*)mi_command );

	/* Create a new input buffer for flex. */
	gdbmi_parse ();

	*pt = tree;

	gdbmi__delete_buffer ( state );
	
	return -1;
}

int gdbmi_parser_parse_file ( 
	gdbmi_parser_ptr parser, 
	const char *mi_command_file,
    gdbmi_output_ptr *pt ) {

	if ( !parser )
		return -1;

	if ( !mi_command_file )
		return -1;

	/* Initialize data */
	gdbmi_in = fopen ( mi_command_file, "r" );

	if ( !gdbmi_in ) {
		logger_write_pos ( logger, __FILE__, __LINE__, "gdbmi_lexer error" );
		return -1;
	}

	gdbmi_parse ();

	*pt = tree;

	fclose ( gdbmi_in );

	return 0;
}

int gdbmi_parser_error ( gdbmi_parser_ptr parser, char **parser_error ) {
	return -1;
}

#if 0
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
		else  {
			printf ( "ERROR(%s)\n", gdbmi_text );
			return -1;
		}
	} while ( gdbmi_text );

	return 0;
}
#endif

