// hello
/* Hello World */
/* Hello
 * you
 */
/* a
 * really 
 * realy 
 * reajklfdj
 * rjklfd
 */

#include <stdio.h>

#include "tokenizer.h"

#define TEST 1

int main ( int argc, char **argv ) {

	if ( tokenizer_set_file ( "cfile.c" ) == -1 ) {
		fprintf ( stderr, "%s:%d tokenizer_set_file error\n", __FILE__, __LINE__ );
		return -1;
	}
	
	return 0;
}
