#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gdbmi_pt.h"
#include "logger.h"
#include "gdbmi_parser.h"

static void usage ( char *progname ) {

	printf ( "%s <file>\n", progname );
	exit ( -1 );
}

int main ( int argc, char **argv ) {
	gdbmi_parser_ptr parser_ptr;
	gdbmi_output_ptr output_ptr;
	int result;

	if ( argc != 2 )
		usage( argv[0] );	

	logger = logger_create ();

	if ( logger_set_fd( logger, stderr ) == -1 ) {
		fprintf ( stderr, "%s:%d logger_set_fd error", __FILE__, __LINE__ );
		return -1;
	}

	parser_ptr = gdbmi_parser_create ();

	result = gdbmi_parser_parse_file ( parser_ptr, argv[1], &output_ptr );

	print_gdbmi_output ( output_ptr );

	if ( destroy_gdbmi_output ( output_ptr ) == -1 ) {
		fprintf ( stderr, "%s:%d free failed", __FILE__, __LINE__ );
		return -1;
	}

	return 0;
}
