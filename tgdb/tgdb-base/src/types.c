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
					tb->file, tb->funcname, tb->line, tb->enabled );
				
				iterator = tgdb_list_next ( iterator );
			}

			fprintf ( fd, "Breakpoint end\n" );
           	break;
		}
        case TGDB_UPDATE_FILE_POSITION:
           {
				struct tgdb_file_position *tfp = (struct tgdb_file_position *)com->data;

                fprintf ( fd, 
                  "TGDB_UPDATE_FILE_POSITION ABSOLUTE(%s)RELATIVE(%s)LINE(%d)\n",
                  tfp->absolute_path, 
                  tfp->relative_path, 
                  tfp->line_number);
           		break;
           }
        case TGDB_UPDATE_SOURCE_FILES:
            {
				struct tgdb_list *list = ( struct tgdb_list *) com->data;
				tgdb_list_iterator *i;
                char *s;

				fprintf ( fd, "Inferior source files start\n" );
				i = tgdb_list_get_first ( list );

                while ( i ) {
					s = (char*)tgdb_list_get_item ( i );
                    fprintf ( fd, "TGDB_SOURCE_FILE (%s)\n", s );
					i = tgdb_list_next ( i );
                }
				fprintf ( fd, "Inferior source files end\n" );
            	break;
            }
        case TGDB_SOURCES_DENIED:
			fprintf ( fd, "TGDB_SOURCES_DENIED\n" );
           	break;
        case TGDB_ABSOLUTE_SOURCE_ACCEPTED:
			{
				struct tgdb_source_file *file = 
						(struct tgdb_source_file *) com->data;
				fprintf( fd, "TGDB_ABSOLUTE_SOURCE_ACCEPTED(%s)\n", 
						file->absolute_path );
				break;
			}
        case TGDB_ABSOLUTE_SOURCE_DENIED:
			{
				struct tgdb_source_file *file = 
						(struct tgdb_source_file *) com->data;
				fprintf( fd, "TGDB_ABSOLUTE_SOURCE_DENIED(%s)\n", 
						file->absolute_path );
				break;
			}
        case TGDB_DISPLAY_UPDATE:
			fprintf ( fd, "TGDB_DISPLAY_UPDATE\n" );
           	break;
        case TGDB_QUIT_ABNORMAL:
			fprintf ( fd, "TGDB_QUIT_ABNORMAL\n" );
			break;
		case TGDB_QUIT_NORMAL:
			fprintf ( fd, "TGDB_QUIT_NORMAL\n" );
           	break;
    }
}

static int tgdb_breakpoint_free ( void *data ) {
	struct tgdb_breakpoint *tb;
	tb = (struct tgdb_breakpoint *)data;

	/* Free the structure */
	free ( (char*)tb->file );
	tb->file = NULL;
	free ( (char*)tb->funcname );
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
			struct tgdb_file_position *tfp = 
					(struct tgdb_file_position *)com->data;

			free ( tfp->absolute_path ), 
			tfp->absolute_path = NULL;
			free ( tfp->relative_path ), 
			tfp->relative_path = NULL;

			free ( tfp );
			tfp = NULL;
			break;
		}
        case TGDB_UPDATE_SOURCE_FILES:
		{
			struct tgdb_list *list = ( struct tgdb_list *) com->data;
			tgdb_list_free ( list, tgdb_source_files_free );
			break;
		}
        case TGDB_SOURCES_DENIED:
			/* Nothing to do */
			break;
        case TGDB_ABSOLUTE_SOURCE_ACCEPTED:
        case TGDB_ABSOLUTE_SOURCE_DENIED:
		{
			struct tgdb_source_file *file = 
					(struct tgdb_source_file *) com->data;
			free ( file->absolute_path );
			file->absolute_path = NULL;
			free ( file );
			file = NULL;
		   	break;
		}
        case TGDB_DISPLAY_UPDATE:
			/* Nothing to do */
			break;
        case TGDB_QUIT_ABNORMAL:
			/* Nothing to do */
			break;
		case TGDB_QUIT_NORMAL:
			/* Nothing to do */
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
