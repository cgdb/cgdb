#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gdbmi_pt.h"
#include "gdbmi_parser.h"

static void usage ( char *progname ) {

	printf ( "%s <file>\n", progname );
	exit ( -1 );
}

int main ( int argc, char **argv ) {
	gdbmi_parser_ptr parser_ptr;
	gdbmi_output_ptr output_ptr;
	int result, parse_failed;

	if ( argc != 2 )
		usage( argv[0] );	

	parser_ptr = gdbmi_parser_create ();

	result = gdbmi_parser_parse_file ( 
			parser_ptr, 
			argv[1], 
			&output_ptr,
		    &parse_failed );

	if ( result == -1 ) {
		fprintf ( stderr, "%s:%d", __FILE__, __LINE__ );
		return -1;
	}

	if ( parse_failed ) {
		if ( result == -1 ) {
			fprintf ( stderr, "%s:%d", __FILE__, __LINE__ );
			return -1;
		}
	} else {
		print_gdbmi_output ( output_ptr );
	}

	if ( parse_failed ) {
		output_ptr = NULL;
	} else {
		if ( destroy_gdbmi_output ( output_ptr ) == -1 ) {
			fprintf ( stderr, "%s:%d", __FILE__, __LINE__ );
			return -1;
		}
	}

        gdbmi_parser_destroy (parser_ptr);

	return 0;
}
