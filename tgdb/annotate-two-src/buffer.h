#ifndef __BUFFER_H__
#define __BUFFER_H__

#include "types.h"
#include "queue.h"

/* buffer_free_command: Free's item (struct command type) */
void buffer_free_command ( void *item );

/* buffer_print:
 *   head        - The head of the queue.
 */
void buffer_print ( struct node *head );

#endif /* __BUFFER_H__ */
