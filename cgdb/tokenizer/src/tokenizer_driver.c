#include <stdio.h>
#include <string.h>

#include "tokenizer.h"

static void usage ( void ) {

	printf ( "tokenizer_driver <file> <c|ada>\n" );
	exit ( -1 );
}

int main ( int argc, char **argv ) {
	struct tokenizer *t = tokenizer_init ();
	int ret;
	enum tokenizer_language_support l = TOKENIZER_LANGUAGE_UNKNOWN;
	
	if ( argc != 3 )
		usage();	

	if ( strcmp ( argv[2], "c" ) == 0 ) 
		l = TOKENIZER_LANGUAGE_C;
	else if ( strcmp ( argv[2], "ada" ) == 0 ) 
		l = TOKENIZER_LANGUAGE_ADA;
	else
		usage();

	
	if ( tokenizer_set_file ( t, argv[1], l ) == -1 ) {
		printf ( "%s:%d tokenizer_set_file error\n", __FILE__, __LINE__ );
		return -1;
	}

	while ( ( ret = tokenizer_get_token ( t )) > 0 ) {
		enum tokenizer_type e = tokenizer_get_packet_type ( t );
		printf ( "Token:\n" );
		printf ( "\tNumber: %d\n", e );
		printf ( "\tType: %s\n", tokenizer_get_printable_enum ( e ) );
		printf ( "\tData: %s\n", tokenizer_get_data ( t ) );
	}

	if ( ret == 0 )
		printf ( "finished!\n" );
	else if ( ret == -1 )
		printf ( "Error!\n" );
	
	return 0;
}
