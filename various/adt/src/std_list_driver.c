#include <stdio.h>
#include "std_list.h"

int printType ( void *data, void *user_data ) {
	int i = *(int*)data;
	printf ( "FUNCTION:%d\n", i );
	return 1;
}

void destroy (void *data) {
	int *a = (int*)data;
	free ( a );
	a = NULL;
}


int main(int argc, char **argv){

	std_list *l;
	std_list_iterator iter;
	int i;

	l = std_list_create ( destroy );

	if ( !l ) {
		printf ( "creation failed\n" );
		return -1;
	}

	// Add 1000 items
	for ( i = 0; i < 1000; ++i ) {
		int *data = malloc ( sizeof ( int ) );
		*data = i;
		if ( std_list_append ( l, data ) == -1 )  {
			printf ( "append failed\n" );
			return -1;
		}
	}
			

	// traverse them
	for ( 
		iter = std_list_begin ( l ); 
		iter != std_list_end ( l ); 
		iter = std_list_next ( iter ) ){ 

		int *data;
		if ( std_list_get_data ( iter, (void**)&data ) == -1 ) {
			printf ( "get data failed\n" );
			return -1;
		}

		printf ( "ITEM(%d)\n", *data );
	}

	if ( std_list_foreach ( l, printType, NULL ) == -1 ) {
		printf ( "foreach failed\n" );
		return -1;
	}

	// destroy list
	if ( std_list_destroy ( l ) == -1 ) {
		printf ( "deletion failed\n" );
		return -1;

	}

	return 0;
}
