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

char *string_literal_bugfix = "\\";
char *string_literal_bugfix2 = "\"";
char *string_literal_bugfix2_a = "\"\"";
char *string_literal_bugfix2_b = "\"abc\"";
char *string_literal_bugfix2_c = "\"abc\"\\\"";
char *string_literal_bugfix2_d = "\"abc\"\\\n\"";

#include <stdio.h>

#define TEST 1

int main ( int argc, char **argv ) {

   if ( argc == 1 )
      return -1;
   else
      return 0;

	
	return 0;
}
