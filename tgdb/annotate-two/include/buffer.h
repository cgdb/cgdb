#ifndef __BUFFER_H__
#define __BUFFER_H__

#include "types.h"
#include "queue.h"

/* buffer_new_item: Creates a new item and initializes it */
struct command *buffer_new_item(    
        char *ndata, 
        enum buffer_command_type    ncom_type, 
        enum buffer_output_type     nout_type,
        enum buffer_command_to_run  ncom_to_run);

/* buffer_free_item: Free's item (struct command *) */
void buffer_free_item ( void *item );

/* buffer_print_item: Print an item (struct command *) */
void buffer_print_item ( void *item );

#endif /* __BUFFER_H__ */
