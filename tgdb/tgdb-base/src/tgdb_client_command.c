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

#include "tgdb_client_command.h"
#include "tgdb_types.h"
#include "queue.h"
#include "sys_util.h"

int tgdb_client_command_destroy ( void* item) {
   struct tgdb_client_command *com;
   if ( item == NULL ) 
      return -1;
   
   com = (struct tgdb_client_command *) item;
   free ( com->tgdb_client_command_data );
   com->tgdb_client_command_data = NULL;
   free ( com );
   com = NULL;
   return 0;
}

struct tgdb_client_command *tgdb_client_command_create (    
        const char *ndata, 
        enum tgdb_client_command_choice command_choice, 
        enum tgdb_client_command_action_choice action_choice,
        void *client_data) {

    struct tgdb_client_command *command = ( struct tgdb_client_command * ) xmalloc ( sizeof (struct tgdb_client_command) );
    if ( ndata != NULL ) {
        int length = strlen(ndata);
        command->tgdb_client_command_data = ( char * ) xmalloc ( sizeof (char *) * ( length + 1 ));
        strncpy( command->tgdb_client_command_data, ndata, length + 1);
    } else 
        command->tgdb_client_command_data = NULL;

    command->command_choice       		= command_choice;
    command->action_choice     			= action_choice;
    command->tgdb_client_private_data   = client_data;
      
    return command;
}


void tgdb_client_command_print ( void *item ) {
	fprintf ( stderr, "unimplemented\n" );
}
