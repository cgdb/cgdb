#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <sys/types.h>

struct node {
   void *data;
   struct node *next;
};

typedef void (*item_free_func)(void *item);

/* queue_append: Appends new_node to the queue.
 *      head        - The head of the queue.
 *      item        - The item to add to the queue
 *      returns     - The new head of the queue
 *
 * Return: The head of the list. This call can not fail.
 */
struct node *queue_append(struct node *head, void *item);

/* queue_pop: Returns the first element and removes it from the list.
 *      head        - The head of the queue.
 *      item        - The item requested.
 *      returns     - The new head of the queue
 *
 *      NOTE: This function only returns the item, it will not free it.
 *      If the caller does not free it, it is a memory leak.
 */
struct node *queue_pop(struct node *head, void **item);

/* queue_free_list: Free's list item by calling func on each element
 *      head        - The head of the queue.
 *      func        - The function to free an item
 */
void queue_free_list(struct node *head, item_free_func func);

/* buffers_size: Gets the size of the queue.
 *
 * Returns          - The size of the list, 0 if head is NULL
 */
int queue_size(struct node *head);

#endif /* __QUEUE_H__ */
