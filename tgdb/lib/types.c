/* From configure */
#include "config.h"

/* Library includes */
#include <string.h>
#include <stdlib.h>

/* Local includes */
#include "types.h"
#include "error.h"
#include "util.h"

static void tgdb_print_item(void *item) {
    size_t j;
    struct Command *com = (struct Command *)item;
    FILE *fd = stderr;

    if ( !com ) {
        err_msg("%s:%d ERROR: ITEM IS NULL", __FILE__, __LINE__);
        return;
    }

    switch(com->header) {
        case BREAKPOINTS_BEGIN:
            fprintf(fd, "TGDB_BREAKPOINTS_BEGIN(%s)\n", com->data);         break;
        case BREAKPOINT:
            fprintf(fd, "TGDB_BREAKPOINT(%s)\n", com->data);                break;
        case BREAKPOINTS_END:
            fprintf(fd, "TGDB_BREAKPOINT_END(%s)\n", com->data);            break;
        case SOURCE_FILE_UPDATE:
            fprintf(fd, "TGDB_SOURCE_FILE_UPDATE(%s)\n", com->data);        break;
        case LINE_NUMBER_UPDATE:
            fprintf(fd, "TGDB_LINE_NUMBER_UPDATE(%s)\n", com->data);        break;
        case SOURCES_START:
            fprintf(fd, "TGDB_SOURCES_START(%s)\n", com->data);             break;
        case SOURCE_FILE:
            fprintf(fd, "TGDB_SOURCE_FILE(%s)\n", com->data);               break;
        case SOURCES_END:
            fprintf(fd, "TGDB_SOURCES_END(%s)\n", com->data);               break;
        case SOURCES_DENIED:
            fprintf(fd, "TGDB_SOURCES_DENIED(%s)\n", com->data);            break;
        case ABSOLUTE_SOURCE_ACCEPTED:
            fprintf(fd, "TGDB_ABSOLUTE_SOURCE_ACCEPTED(%s)\n", com->data);  break;
        case ABSOLUTE_SOURCE_DENIED:
            fprintf(fd, "TGDB_ABSOLUTE_SOURCE_DENIED(%s)\n", com->data);    break;
        case QUIT:
            fprintf(fd, "TGDB_QUIT(%s)\n", com->data);                      break;
        default:
            fprintf(fd, "%s:%d ERROR TGDB_UNKNOWN\n", __FILE__, __LINE__);  break;
    }
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

int tgdb_append_command(struct queue *q, 
                        enum INTERFACE_COMMANDS new_header, 
                        char *buf, char *buf2, char *buf3){
    char command[MAXLINE];
    struct Command *item = (struct Command *)xmalloc(sizeof(struct Command));
    item->data[0] = '\0';

    if(buf != NULL)
        strcat(item->data, buf);

    if(buf2 != NULL){
        strcat(item->data, " ");
        strcat(item->data, buf2);
    }

    if(buf3 != NULL){
        strcat(item->data, " ");
        strcat(item->data, buf3);
    }
   
    item->header = new_header;

    queue_append(q, item);
    /*err_msg("UPDATE(%s)", command);*/

    return 0;
}

void tgdb_traverse_command(struct queue *q) {
    queue_traverse_list(q, tgdb_print_item);
}

