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
#include "error.h"
#include "sys_util.h"

struct tgdb_queue_command *tgdb_command_create (    
		const char *tgdb_command_data,
        enum tgdb_command_choice command_choice, 
        enum tgdb_command_action_choice action_choice,
		struct tgdb_client_command *tcc ) {

	struct tgdb_queue_command *tc;

	tc = (struct tgdb_queue_command *)xmalloc ( sizeof ( struct tgdb_queue_command ) );

	if ( tgdb_command_data )
		tc->tgdb_command_data = strdup ( tgdb_command_data );
	else
		tc->tgdb_command_data = NULL;

	tc->command_choice = command_choice;
	tc->action_choice  = action_choice;
	tc->client_command = tcc;

	return tc;
}
        
void tgdb_command_destroy ( void *item) {
	struct tgdb_queue_command *tc = (struct tgdb_queue_command *) item;
	
	tgdb_client_command_destroy ( tc->client_command );
	tc->client_command = NULL;

	free ( tc );
	tc = NULL;
}

void tgdb_command_print ( void *item ) {
	fprintf ( stderr, "unimplemented\n" );
}
