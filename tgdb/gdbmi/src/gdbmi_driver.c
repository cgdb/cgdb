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
	int result, parse_failed;
	char *parse_error;

	if ( argc != 2 )
		usage( argv[0] );	

	logger = logger_create ();

	if ( logger_set_fd( logger, stderr ) == -1 ) {
		fprintf ( stderr, "%s:%d logger_set_fd error", __FILE__, __LINE__ );
		return -1;
	}

	parser_ptr = gdbmi_parser_create ();

	result = gdbmi_parser_parse_file ( 
			parser_ptr, 
			argv[1], 
			&output_ptr,
		    &parse_failed );

	if ( result == -1 ) {
		logger_write_pos ( logger, __FILE__, __LINE__, "gdbmi_parser_parse_file error");
		return -1;
	}

	if ( parse_failed ) {
		result = gdbmi_parser_get_error ( parser_ptr, &parse_error );

		if ( result == -1 ) {
			logger_write_pos ( logger, __FILE__, __LINE__, "gdbmi_parser_get_error error");
			return -1;
		}

		logger_write_pos ( logger, __FILE__, __LINE__, "%s", parse_error );
	} else {
		print_gdbmi_output ( output_ptr );
	}

	if ( destroy_gdbmi_output ( output_ptr ) == -1 ) {
		logger_write_pos ( logger, __FILE__, __LINE__, "free failed" );
		return -1;
	}

	logger_destroy ( logger );

	return 0;
}
