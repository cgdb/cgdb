#ifndef __QUEUE_H__
#define __QUEUE_H__

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */

struct queue;

typedef void (*item_func)(void *item);

/* queue_init: Initializes a new empty queue.
 *      returns     - The new head of the queue
 */
struct queue *queue_init(void);

/* queue_append: Appends new_node to the queue.
 *      q           - The queue to modify
 *      item        - The item to add to the queue
 *
 * Return: The new queue.
 */
void queue_append(struct queue *q, void *item);

/* queue_pop: Returns the first element and removes it from the list.
 *      q           - The queue to modify
 *      returns     - The item requested, Null if no items in queue
 *
 *      NOTE: This function only returns the item, it will not free it.
 *      If the caller does not free it, it is a memory leak.
 */
void *queue_pop(struct queue *q);

/* queue_free_list: Free's list item by calling func on each element
 *      q           - The queue to modify
 *      func        - The function to free an item
 */
void queue_free_list(struct queue *q, item_func func);

/* queue_traverse_list: Traverses list passing each element to func
 *      q           - The queue to traverse
 *      func        - The function to call on each item
 */
void queue_traverse_list(struct queue *q, item_func func);

/* buffers_size: Gets the size of the queue.
 *      q           - The queue to modify
 * Returns          - The size of the list, 0 if head is NULL
 */
int queue_size(struct queue *q);

#endif /* __QUEUE_H__ */
