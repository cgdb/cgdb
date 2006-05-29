#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_STDLIB_H 
#include <stdlib.h>
#endif  /* HAVE_STDLIB_H */

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#if HAVE_STDIO_H
#include <stdio.h>
#endif

#include "tgdb_types.h"
#include "tgdb_command.h"
#include "tgdb_client_command.h"
#include "sys_util.h"

struct tgdb_command *tgdb_command_create (    
		const char *tgdb_command_data,
        enum tgdb_command_choice command_choice, 
		struct tgdb_client_command *tcc,
       int is_buffered_console_command) {

	struct tgdb_command *tc;

	tc = (struct tgdb_command *)xmalloc ( sizeof ( struct tgdb_command ) );

	if ( tgdb_command_data )
		tc->tgdb_command_data = strdup ( tgdb_command_data );
	else
		tc->tgdb_command_data = NULL;

	tc->command_choice = command_choice;
	tc->client_command = tcc;
	tc->is_buffered_console_command = is_buffered_console_command;

	return tc;
}
        
void tgdb_command_destroy ( void *item) {
	struct tgdb_command *tc = (struct tgdb_command *) item;
	
	tgdb_client_command_destroy ( tc->client_command );
	tc->client_command = NULL;

	free ( tc );
	tc = NULL;
}

void tgdb_command_print ( void *item ) {
	fprintf ( stderr, "unimplemented\n" );
}
