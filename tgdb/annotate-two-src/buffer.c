#include "buffer.h"
#include "types.h"
#include "error.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>

void buffer_free_command( void *item ) {
   struct command *com;
   if ( item == NULL ) 
      return;
   
   com = (struct command *) item;
   free ( com->data );
   com->data = NULL;
   free ( com );
   com = NULL;
}

void buffer_print ( struct node *head ) {
    struct node *cur = head;
    struct command *com;

    fprintf(stderr, "BUFFER_SIZE(%d)\n", queue_size(head));
    while ( cur != NULL ) {
        com = (struct command *) cur->data;
        fprintf(stderr, "ITEM:\n");
        fprintf(stderr, "\tDATA(%s)\n", com->data);

        switch(com->com_type) {
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

        switch(com->out_type) {
            case COMMANDS_SHOW_USER_OUTPUT:
                fprintf(stderr, "\tOUT_TYPE: COMMANDS_SHOW_USER_OUTPUT\n"); break;
            case COMMANDS_HIDE_OUTPUT:
                fprintf(stderr, "\tOUT_TYPE: COMMANDS_HIDE_OUTPUT\n");      break;
            default:
                fprintf(stderr, "\tOUT_TYPE: ERROR\n");                     break;
        }

        switch(com->com_to_run) {
            case COMMANDS_INFO_SOURCES:
                fprintf(stderr, "\tCOM_TYPE: COMMANDS_INFO_SOURCES\n");             break;
            case COMMANDS_INFO_LIST:
                fprintf(stderr, "\tCOM_TYPE: COMMANDS_INFO_LIST\n");                break;
            case COMMANDS_INFO_SOURCE:
                fprintf(stderr, "\tCOM_TYPE: COMMANDS_INFO_SOURCE\n");              break;
            case COMMANDS_INFO_BREAKPOINTS:
                fprintf(stderr, "\tCOM_TYPE: COMMANDS_INFO_BREAKPOINTS\n");         break;
            case COMMANDS_TTY:
                fprintf(stderr, "\tCOM_TYPE: COMMANDS_INFO_TTY\n");                 break;
            case COMMANDS_VOID:
                fprintf(stderr, "\tCOM_TYPE: COMMANDS_INFO_VOID\n");                break;

            default:
                fprintf(stderr, "\tCOM_TYPE: ERROR\n");                             break;
        }

        cur = cur->next;
    }

    fprintf(stderr, "NULL\n");
}
