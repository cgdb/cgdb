#include "gdbmi_il.h"
#include <stdio.h>
#include <stdlib.h>


int print_token ( long l ) {
	if ( l == -1 )
		return 0;
	printf ("token(%ld)", l );
	return 0;
}

/* Print result class */
int print_result_class ( enum result_class param ) {
	switch ( param ) {
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

/* Creating, Destroying and printing output */
output_ptr create_output ( void ) {
	return calloc ( 1, sizeof ( struct output ) );
}

int destory_output ( output_ptr param ) {
	free ( param );
	param = NULL;
	return 0;
}

output_ptr append_output ( output_ptr list, output_ptr item ) {
	if ( !item )
		return NULL;

	if ( !list )
		list = item;
	else {
		output_ptr cur = list;
		while ( cur->next )
			cur=cur->next;

		cur->next = item;
	}

	return list;
}

int print_output ( output_ptr param ) {
	output_ptr cur = param;
	int result;

	while ( cur ) {
		result = print_oob_record ( cur->oob_record );
		if ( result == -1 )
			return -1;
		
		result = print_result_record ( cur->result_record );
		if ( result == -1 )
			return -1;

		cur = cur->next;
	}

	return 0;
}

/* Creating, Destroying and printing record */
result_record_ptr create_result_record ( void ) {
	return calloc ( 1, sizeof ( struct result_record ) );
}

int destroy_result_record ( result_record_ptr param ) {
	free ( param );
	param = NULL;
	return 0;
}

int print_result_record ( result_record_ptr param ) {
	int result;

	if ( !param )
		return 0;

	result = print_token ( param->token );
	if ( result == -1 )
		return -1;
	
	result = print_result_class ( param->result_class );
	if ( result == -1 )
		return -1;
	
	result = print_result ( param->result );
	if ( result == -1 )
		return -1;

	return 0;
}

/* Creating, Destroying and printing result */
result_ptr create_result ( void ) {
	return calloc ( 1, sizeof ( struct result_record ) );
}

int destroy_result ( result_ptr param ) {
	free ( param );
	param = NULL;
	return 0;
}

result_ptr append_result ( result_ptr list, result_ptr item ) {
	if ( !item )
		return NULL;

	if ( !list )
		list = item;
	else {
		result_ptr cur = list;
		while ( cur->next )
			cur=cur->next;

		cur->next = item;
	}

	return list;
}

int print_result ( result_ptr param ) {
	result_ptr cur = param;
	int result;

	while ( cur ) {
		printf ( "variable->(%s)\n", cur->variable );
		
		result = print_value ( cur->value );
		if ( result == -1 )
			return -1;

		cur = cur->next;
	}

	return 0;
}

int print_oob_record_kind ( enum oob_record_kind param ) {
	switch ( param ) {
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

/* Creating, Destroying and printing oob_record */
oob_record_ptr create_oob_record ( void ) {
	return calloc ( 1, sizeof ( struct oob_record ) );
}

int destroy_oob_record ( oob_record_ptr param ) {
	free ( param );
	param = NULL;
	return 0;
}

oob_record_ptr append_oob_record ( oob_record_ptr list, oob_record_ptr item ) {
	if ( !item )
		return NULL;

	if ( !list )
		list = item;
	else {
		oob_record_ptr cur = list;
		while ( cur->next )
			cur=cur->next;

		cur->next = item;
	}

	return list;
}

int print_oob_record ( oob_record_ptr param ) {
	oob_record_ptr cur = param;
	int result;

	while ( cur ) {
		result = print_oob_record_kind ( cur->record );
		if ( result == -1 )
			return -1;

		if ( cur->record == GDBMI_ASYNC ) {
			result = print_async_record ( cur->variant.async_record );
			if ( result == -1 )
				return -1;
		} else if ( cur->record == GDBMI_STREAM ) {
			result = print_stream_record ( cur->variant.stream_record );
			if ( result == -1 )
				return -1;
		} else
			return -1;
		
		cur = cur->next;
	}

	return 0;
}

int print_async_record_kind ( enum async_record_kind param ) {
	switch ( param ) {
		case GDBMI_STATUS:
			printf ( "GDBMI_STATUS\n" );
			break;
		case GDBMI_EXEC:
			printf ( "GDBMI_EXEC\n" );
			break;
		case GDBMI_NOTIFY:
			printf ( "GDBMI_NOTIFY\n" );
			break;
		default:
			return -1;
	};
	
	return 0;
}

int print_stream_record_kind ( enum stream_record_kind param ) {
	switch ( param ) {
		case GDBMI_CONSOLE:
			printf ( "GDBMI_CONSOLE\n" );
			break;
		case GDBMI_TARGET:
			printf ( "GDBMI_TARGET\n" );
			break;
		case GDBMI_LOG:
			printf ( "GDBMI_LOG\n" );
			break;
		default:
			return -1;
	};
	
	return 0;
}

/* Creating, Destroying and printing async_record */
async_record_ptr create_async_record ( void ) {
	return calloc ( 1, sizeof ( struct async_record ) );
}

int destroy_async_record ( async_record_ptr param ) {
	free ( param );
	param = NULL;
	return 0;
}

int print_async_record ( async_record_ptr param ) {
	int result;

	if ( !param )
		return 0;

	result = print_token ( param->token );
	if ( result == -1 )
		return -1;

	result = print_async_record_kind ( param->async_record );
	if ( result == -1 )
		return -1;
	
	result = print_async_class ( param->async_class );
	if ( result == -1 )
		return -1;

	result = print_result ( param->result );
	if ( result == -1 )
		return -1;

	return 0;
}

int print_async_class ( enum async_class param ) {
	switch ( param ) {
		case GDBMI_STOPPED:
			printf ( "GDBMI_STOPPED\n" );
			break;
		default:
			return -1;
	};
	
	return 0;
}

int print_value_kind ( enum value_kind param ) {
	switch ( param ) {
		case GDBMI_CSTRING:
			printf ( "GDBMI_CSTRING\n" );
			break;
		case GDBMI_TUPLE:
			printf ( "GDBMI_TUPLE\n" );
			break;
		case GDBMI_LIST:
			printf ( "GDBMI_LIST\n" );
			break;
		default:
			return -1;
	};
	
	return 0;
}

/* Creating, Destroying and printing value */
value_ptr create_value ( void ) {
	return calloc ( 1, sizeof ( struct value ) );
}

int destroy_value ( value_ptr param ) {
	free ( param );
	param = NULL;
	return 0;
}

value_ptr append_value ( value_ptr list, value_ptr item ) {
	if ( !item )
		return NULL;

	if ( !list )
		list = item;
	else {
		value_ptr cur = list;
		while ( cur->next )
			cur=cur->next;

		cur->next = item;
	}

	return list;
}

int print_value ( value_ptr param ) {
	value_ptr cur = param;
	int result;

	while ( cur ) {
		result = print_value_kind ( cur->value_kind );
		if ( result == -1 )
			return -1;

		if ( cur->value_kind == GDBMI_CSTRING ) {
			printf ( "cstring->(%s)\n", cur->variant.cstring );
		} else if ( cur->value_kind == GDBMI_TUPLE ) {
			result = print_tuple ( cur->variant.tuple );
			if ( result == -1 )
				return -1;
		} else if ( cur->value_kind == GDBMI_LIST ) {
			result = print_list ( cur->variant.list );
			if ( result == -1 )
				return -1;
		} else
			return -1;
		
		cur = cur->next;
	}

	return 0;
}

int print_list_kind ( enum list_kind param ) {
	switch ( param ) {
		case GDBMI_VALUE:
			printf ( "GDBMI_VALUE\n" );
			break;
		case GDBMI_RESULT:
			printf ( "GDBMI_RESULT\n" );
			break;
		default:
			return -1;
	};

	return 0;
}

/* Creating, Destroying and printing tuple */
tuple_ptr create_tuple ( void ) {
	return calloc ( 1, sizeof ( struct tuple ) );
}

int destroy_tuple ( tuple_ptr param ) {
	free ( param );
	param = NULL;
	return 0;
}

int print_tuple ( tuple_ptr param ) {
	tuple_ptr cur = param;
	int result;

	while ( cur ) {
		result = print_result ( cur->result );
		if ( result == -1 )
			return -1;

		cur = cur->next;
	}

	return 0;
}

/* Creating, Destroying and printing list */
list_ptr create_list ( void ) {
	return calloc ( 1, sizeof ( struct list ) );
}

int destroy_list ( list_ptr param ) {
	free ( param );
	param = NULL;
	return 0;
}

list_ptr append_list ( list_ptr list, list_ptr item ) {
	if ( !item )
		return NULL;

	if ( !list )
		list = item;
	else {
		list_ptr cur = list;
		while ( cur->next )
			cur=cur->next;

		cur->next = item;
	}

	return list;
}

int print_list ( list_ptr param ) {
	list_ptr cur = param;
	int result;

	while ( cur ) {
		result = print_list_kind ( cur->list_kind );
		if ( result == -1 )
			return -1;

		if ( cur->list_kind == GDBMI_VALUE ) {
			result = print_value ( cur->variant.value );
			if ( result == -1 )
				return -1;
		} else if ( cur->list_kind == GDBMI_RESULT ) {
			result = print_result ( cur->variant.result );
			if ( result == -1 )
				return -1;
		} else
			return -1;
		
		cur = cur->next;
	}

	return 0;
}

/* Creating, Destroying and printing stream_record */
stream_record_ptr create_stream_record ( void ) {
	return calloc ( 1, sizeof ( struct stream_record ) );
}

int destroy_stream_record ( stream_record_ptr param ) {
	free ( param );
	param = NULL;
	return 0;
}

int print_stream_record ( stream_record_ptr param ) {
	int result;

	if ( !param )
		return 0;

	result = print_stream_record_kind ( param->stream_record );
	if ( result == -1 )
		return -1;

	printf ( "cstring->(%s)\n", param->cstring );

	return 0;
}
