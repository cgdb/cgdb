#include "gdbmi_il.h"
#include <stdio.h>

int print_result_class_kind ( enum result_class_kind *param ) {
	switch ( *param ) {
		case GDBMI_DONE:
			printf ( "GDBMI_DONE\n" );
			break;
		case GDBMI_RUNNING:
			printf ( "GDBMI_RUNNING\n" );
			break;
		case GDBMI_CONNECTED:
			printf ( "GDBMI_CONNECTED\n" );
			break;
		case GDBMI_ERROR:
			printf ( "GDBMI_ERROR\n" );
			break;
		case GDBMI_EXIT:
			printf ( "EXIT\n" );
			break;
		default:
			return -1;
	};
	
	return 0;
}

int print_oob_record ( oob_record_ptr param );

int print_output ( struct output *param ) {
	int result;

	result = print_oob_record ( param->oob_record_var );
	if ( result == -1 )
		return -1;
	
	result = print_result_record ( param->result_record_var );
	if ( result == -1 )
		return -1;

	return 0;
}

int print_output_list ( struct output *param ) {
	struct output_ptr cur = param;
	int result;

	while ( cur ) {
		result = print_output ( cur );
		if ( result == -1 )
			return result;

		cur++;
	}

	return 0;
}

int print_result_record ( struct result_record *param ) {
	int result;

	result = print_token ( param->token_var );
	if ( result == -1 )
		return -1;
	
	result = print_result_class_kind ( param->result_class_var );
	if ( result == -1 )
		return -1;
	
	result = print_result ( param->result_var );
	if ( result == -1 )
		return -1;

	return 0;
}

int print_result_record_ptr ( struct result_record *param ) {
	struct result_record_ptr cur = param;
	int result;

	while ( cur ) {
		result = print_result_record ( cur );
		if ( result == -1 )
			return result;

		cur++;
	}

	return 0;
}

int print_record_kind ( enum record_kind *param ) {
	switch ( *param ) {
		case GDBMI_ASYNC:
			printf ( "GDBMI_ASYNC\n" );
			break;
		case GDBMI_STREAM:
			printf ( "GDBMI_STREAM\n" );
			break;
		default:
			return -1;
	};
	
	return 0;
}

int print_async_record ( struct async_record_ptr param );
int print_stream_record ( struct stream_record_ptr param );

int print_oob_record ( struct oob_record *param ) {
	int result;

	result = print_record_kind ( param->record_var );
	if ( result == -1 )
		return -1;

	if ( param->record_var == GDBMI_ASYNC ) {
		result = print_async_record ( param->variant.async_record_var );
		if ( result == -1 )
			return -1;
	} else if ( param->record_var == GDBMI_STREAM ) {
		result = print_stream_record ( param->variant.stream_record_var );
		if ( result == -1 )
			return -1;
	} else
		return -1;

	return 0;
}

int print_oob_record_list ( struct oob_record *param ) {
	struct oob_record_ptr cur = param;
	int result;

	while ( cur ) {
		result = print_oob_record ( cur );
		if ( result == -1 )
			return result;

		cur++;
	}

	return 0;
}
