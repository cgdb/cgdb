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
#include "tgdb_list.h"

static void tgdb_print_item(void *item) {
    struct tgdb_command *com = (struct tgdb_command *)item;
    FILE *fd = stderr;

    if ( !com ) {
        err_msg("%s:%d ERROR: ITEM IS NULL", __FILE__, __LINE__);
        return;
    }

    switch ( com->header ) {
        case TGDB_UPDATE_BREAKPOINTS:
		{
			struct tgdb_list *list = (struct tgdb_list *)com->data;
			tgdb_list_iterator *iterator;
			struct tgdb_breakpoint *tb;

			fprintf ( fd, "Breakpoint start\n" );

			iterator = tgdb_list_get_first ( list );

			while ( iterator ) {
				tb = (struct tgdb_breakpoint *)tgdb_list_get_item ( iterator );

				if ( tb == NULL )
					err_msg("%s:%d breakpoint is NULL", __FILE__, __LINE__);

				fprintf ( fd, 
					"\tFILE(%s) FUNCNAME(%s) LINE(%d) ENABLED(%d)\n",
					string_get ( tb->file ), 
					string_get ( tb->funcname ), 
					tb->line, 
					tb->enabled );
				
				iterator = tgdb_list_next ( iterator );
			}

			fprintf ( fd, "Breakpoint end\n" );
           	break;
		}
        case TGDB_UPDATE_FILE_POSITION:
           {
				struct tgdb_file_position *tfp = (struct tgdb_file_position *)com->data;

                fprintf ( stderr, 
                  "TGDB_UPDATE_FILE_POSITION ABSOLUTE(%s)RELATIVE(%s)LINE(%d)\n",
                  string_get ( tfp->absolute_path ), 
                  string_get ( tfp->relative_path ), 
                  tfp->line_number);
           }
           break;
        case TGDB_UPDATE_SOURCE_FILES:
            {
				struct tgdb_list *list = ( struct tgdb_list *) com->data;
				tgdb_list_iterator *i;
                char *s;

				i = tgdb_list_get_first ( list );

                while ( i ) {
					s = (char*)tgdb_list_get_item ( i );
                    fprintf ( stderr, "TGDB_SOURCE_FILE (%s)\n", s );
					i = tgdb_list_next ( i );
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

static int tgdb_breakpoint_free ( void *data ) {
	struct tgdb_breakpoint *tb;
	tb = (struct tgdb_breakpoint *)data;

	/* Free the structure */
	string_free ( tb->file );
	tb->file = NULL;
	string_free ( tb->funcname );
	tb->funcname = NULL;
	free ( tb );
	tb = NULL;
	return 0;
}

static int tgdb_source_files_free ( void *data ) {
	char *s = (char*)data;
	free ( s );
	s = NULL;
	return 0;
}

void tgdb_delete_command(void *item){
    struct tgdb_command *com = (struct tgdb_command*) item;

    if ( !com ) {
        return;
    }

    switch ( com->header ) {
        case TGDB_UPDATE_BREAKPOINTS:
		{
			struct tgdb_list *list = (struct tgdb_list *)com->data;

			tgdb_list_free ( list, tgdb_breakpoint_free );
           	break;
		}
        case TGDB_UPDATE_FILE_POSITION:
		{
			struct tgdb_file_position *tfp = (struct tgdb_file_position *)com->data;

			string_free ( tfp->absolute_path ), 
			tfp->absolute_path = NULL;
			string_free ( tfp->relative_path ), 
			tfp->relative_path = NULL;

			free ( tfp );
			tfp = NULL;

			break;
		}
        case TGDB_UPDATE_SOURCE_FILES:
            {
				struct tgdb_list *list = ( struct tgdb_list *) com->data;
				tgdb_list_free ( list, tgdb_source_files_free );
            }
        case TGDB_SOURCES_DENIED:
        case TGDB_ABSOLUTE_SOURCE_ACCEPTED:
        case TGDB_ABSOLUTE_SOURCE_DENIED:
        case TGDB_DISPLAY_UPDATE:
        case TGDB_QUIT_ABNORMAL:
		case TGDB_QUIT_NORMAL:
        default:
            break;
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
    struct tgdb_command *item = (struct tgdb_command *)
			xmalloc(sizeof(struct tgdb_command));
    item->header = new_header;
    item->data = ndata;
    queue_append(q, item);
    /*err_msg("UPDATE(%s)", command);*/
}

void tgdb_traverse_command_queue(struct queue *q) {
    queue_traverse_list(q, tgdb_print_item);
}
