#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_STDLIB_H 
#include <stdlib.h>
#endif  /* HAVE_STDLIB_H */

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#include "tgdb_interface.h"
#include "types.h"
#include "queue.h"
#include "error.h"
#include "sys_util.h"

void tgdb_interface_free_command( void* item) {
   struct command *com;
   if ( item == NULL ) 
      return;
   
   com = (struct command *) item;
   free ( com->data );
   com->data = NULL;
   free ( com );
   com = NULL;
}

struct command *tgdb_interface_new_command(    
        const char *ndata, 
        enum buffer_command_type    ncom_type, 
        enum buffer_output_type     nout_type,
        enum buffer_command_to_run  ncom_to_run,
        void *client_data) {

    struct command *command = ( struct command * ) xmalloc ( sizeof (struct command) );
    if ( ndata != NULL ) {
        int length = strlen(ndata);
        command->data = ( char * ) xmalloc ( sizeof (char *) * ( length + 1 ));
        strncpy( command->data, ndata, length + 1);
    } else 
        command->data = NULL;

    command->com_type       = ncom_type;
    command->out_type       = nout_type;
    command->com_to_run     = ncom_to_run;
    command->client_data    = client_data;
      
    return command;
}


void tgdb_interface_print_command ( struct command *item ) {
    struct command *i = item;
    fprintf(stderr, "ITEM:\n");
    fprintf(stderr, "\tDATA(%s)\n", i->data);

    switch(i->com_type) {
        case BUFFER_VOID:
            fprintf(stderr, "\tBUF_TYPE: BUFFER_VOID\n");           break;
        case BUFFER_GUI_COMMAND:
            fprintf(stderr, "\tBUF_TYPE: BUFFER_GUI_COMMAND\n");    break;
        case BUFFER_TGDB_COMMAND:
            fprintf(stderr, "\tBUF_TYPE: BUFFER_TGDB_COMMAND\n");   break;
        case BUFFER_USER_COMMAND:
            fprintf(stderr, "\tBUF_TYPE: BUFFER_USER_COMMAND\n");   break;
        default:
            fprintf(stderr, "\tBUF_TYPE: ERROR\n");                 break;
    }

    switch(i->out_type) {
        case COMMANDS_SHOW_USER_OUTPUT:
            fprintf(stderr, "\tOUT_TYPE: COMMANDS_SHOW_USER_OUTPUT\n"); break;
        case COMMANDS_HIDE_OUTPUT:
            fprintf(stderr, "\tOUT_TYPE: COMMANDS_HIDE_OUTPUT\n");      break;
        default:
            fprintf(stderr, "\tOUT_TYPE: ERROR\n");                     break;
    }

    switch(i->com_to_run) {
//        case ANNOTATE2_COMMANDS_INFO_SOURCES:
//            fprintf(stderr, "\tCOM_TYPE: ANNOTATE2_COMMANDS_INFO_SOURCES\n");             break;
//        case ANNOTATE2_COMMANDS_INFO_LIST:
//            fprintf(stderr, "\tCOM_TYPE: ANNOTATE2_COMMANDS_INFO_LIST\n");                break;
//        case ANNOTATE2_COMMANDS_INFO_SOURCE_RELATIVE:
//            fprintf(stderr, "\tCOM_TYPE: ANNOTATE2_COMMANDS_INFO_SOURCE_RELATIVE\n");     break;
//        case ANNOTATE2_COMMANDS_INFO_SOURCE_ABSOLUTE:
//            fprintf(stderr, "\tCOM_TYPE: ANNOTATE2_COMMANDS_INFO_SOURCE_ABSOLUTE\n");     break;
//        case ANNOTATE2_COMMANDS_INFO_BREAKPOINTS:
//            fprintf(stderr, "\tCOM_TYPE: ANNOTATE2_COMMANDS_INFO_BREAKPOINTS\n");         break;
//        case ANNOTATE2_COMMANDS_TTY:
//            fprintf(stderr, "\tCOM_TYPE: ANNOTATE2_COMMANDS_INFO_TTY\n");                 break;
//        case ANNOTATE2_COMMANDS_VOID:
//            fprintf(stderr, "\tCOM_TYPE: ANNOTATE2_COMMANDS_INFO_VOID\n");                break;

        default:
            fprintf(stderr, "\tCOM_TYPE: ERROR\n");                             break;
    }
}
