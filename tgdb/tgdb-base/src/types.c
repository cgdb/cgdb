#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#if HAVE_STDLIB_H 
#include <stdlib.h>
#endif  /* HAVE_STDLIB_H */

/* Local includes */
#include "types.h"
#include "error.h"
#include "sys_util.h"
#include "ibuf.h"

static void tgdb_print_item(void *item) {
    struct Command *com = (struct Command *)item;
//    FILE *fd = stderr;

    if ( !com ) {
        err_msg("%s:%d ERROR: ITEM IS NULL", __FILE__, __LINE__);
        return;
    }

    switch ( com->header ) {
        case TGDB_UPDATE_BREAKPOINTS:
           break;
        case TGDB_UPDATE_FILE_POSITION:
           {
                struct queue *q = ( struct queue * ) com->data;
                struct string *absolute_name;
                struct string *relative_name;
                struct string *line_number;

                absolute_name = queue_pop ( q );
                relative_name = queue_pop ( q );
                line_number   = queue_pop ( q );

                fprintf ( stderr, 
                  "TGDB_UPDATE_FILE_POSITION ABSOLUTE(%s)RELATIVE(%s)LINE(%s)\n",
                  string_get ( absolute_name ), 
                  string_get ( relative_name ), 
                  string_get ( line_number ));
                                    

                string_clear ( absolute_name );
                string_clear ( relative_name );
                string_clear ( line_number );
                break;
 
           }
           break;
        case TGDB_UPDATE_SOURCE_FILES:
            {
                struct queue *q = (struct queue *) com->data;
                char *s;

                while ( queue_size ( q ) > 0 ) {
                    s = queue_pop( q );
                    fprintf ( stderr, "TGDB_SOURCE_FILE (%s)\n", s );
                    free ( s );
                    s = NULL;
                }
            }

            break;
        case TGDB_SOURCES_DENIED:
			fprintf ( stderr, "TGDB_SOURCES_DENIED\n" );
           break;
        case TGDB_ABSOLUTE_SOURCE_ACCEPTED:
           break;
        case TGDB_ABSOLUTE_SOURCE_DENIED:
           break;
        case TGDB_DISPLAY_UPDATE:
           break;
        case TGDB_QUIT_ABNORMAL:
		case TGDB_QUIT_NORMAL:
           break;

        default:

            break;
    }

//    switch(com->header) {
//        case BREAKPOINTS_BEGIN:
//            fprintf(fd, "TGDB_BREAKPOINTS_BEGIN(%s)\n", com->data);         break;
//        case BREAKPOINT:
//            fprintf(fd, "TGDB_BREAKPOINT(%s)\n", com->data);                break;
//        case BREAKPOINTS_END:
//            fprintf(fd, "TGDB_BREAKPOINT_END(%s)\n", com->data);            break;
//        case SOURCE_FILE_UPDATE:
//            fprintf(fd, "TGDB_SOURCE_FILE_UPDATE(%s)\n", com->data);        break;
//        case CURRENT_FILE_UPDATE:
//            fprintf(fd, "TGDB_CURRENT_FILE_UPDATE(%s)\n", com->data);       break;
//        case LINE_NUMBER_UPDATE:
//            fprintf(fd, "TGDB_LINE_NUMBER_UPDATE(%s)\n", com->data);        break;
//        case SOURCES_START:
//            fprintf(fd, "TGDB_SOURCES_START(%s)\n", com->data);             break;
//        case SOURCE_FILE:
//            fprintf(fd, "TGDB_SOURCE_FILE(%s)\n", com->data);               break;
//        case SOURCES_END:
//            fprintf(fd, "TGDB_SOURCES_END(%s)\n", com->data);               break;
//        case SOURCES_DENIED:
//            fprintf(fd, "TGDB_SOURCES_DENIED(%s)\n", com->data);            break;
//        case ABSOLUTE_SOURCE_ACCEPTED:
//            fprintf(fd, "TGDB_ABSOLUTE_SOURCE_ACCEPTED(%s)\n", com->data);  break;
//        case ABSOLUTE_SOURCE_DENIED:
//            fprintf(fd, "TGDB_ABSOLUTE_SOURCE_DENIED(%s)\n", com->data);    break;
//        case QUIT:
//            fprintf(fd, "TGDB_QUIT(%s)\n", com->data);                      break;
//        default:
//            fprintf(fd, "%s:%d ERROR TGDB_UNKNOWN\n", __FILE__, __LINE__);  break;
//    }
}

void tgdb_delete_command(void *item){
    struct Command *com = (struct Command*) item;

    if ( !com ) {
        return;
    }
  
    free(com);
    com = NULL;
}

void tgdb_delete_commands(struct queue *q) {
    queue_free_list(q, tgdb_delete_command);
}

void tgdb_append_command(
            struct queue *q, 
            enum INTERFACE_COMMANDS new_header, 
            void *ndata){
    struct Command *item = (struct Command *)xmalloc(sizeof(struct Command));
    item->header = new_header;
    item->data = ndata;
    queue_append(q, item);
    /*err_msg("UPDATE(%s)", command);*/
}

void tgdb_traverse_command(struct queue *q) {
    queue_traverse_list(q, tgdb_print_item);
}
