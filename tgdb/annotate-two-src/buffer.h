#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <sys/types.h>
#include "types.h"

struct node {
   void *data;
   struct node *next;
};

/* A prototype of a function that knows how to free a data structure.
 * This one is free's a struct node.
 */
typedef void (*buffer_free_func)(void *item);

/* buffer_append: 
 *    Appends new_node to the list head. 
 *    Assumes that new_node is the pointer to be stored. No allocation is done.
 *
 * Return: The head of the list. This call can not fail.
 */
struct node *buffer_append(struct node *head, struct node *new_node);

/* buffer_remove_and_increment:
 *    Removes the first element of the list and free's it. 
 *    
 * Return: The head of the list. This call can not fail.
 */
struct node *buffer_remove_and_increment(struct node *head, buffer_free_func func);

/* buffer_free_list:
 * Free's list item by calling buffer_free_func on all of its elements.
 */
void buffer_free_list(struct node *item, buffer_free_func func);


/******************************************************************************
 * Currently this list is only used for keeping the user's commands. 
 * So it is here. If it needs to go somewhere else. So Be.
 ******************************************************************************/
 
/* buffer_free_command:
 * Free's the item item. Item is a 'struct command' type.
 */
void buffer_free_command ( void *item );

/* buffer_write_command: 
 * This writes a whole command to the list com.
 * Returns: Return the head of the list. Or NULL on error.
 */
struct node *buffer_write_command_and_append ( struct node *n, struct command *com );

/* buffer_is_empty: 
 * Determines if the internal buffer is empty.
 * Returns: TRUE if empty, otherwise FALSE.
 */
int buffer_is_empty(void);

/* buffer_write_line:
 * Writes a  command to the buffer
 */
struct node *buffer_write_line ( struct node *com, const char *c );

/* Set's the internal string to '\0' */
void buffer_clear_string ( void );

/* Prints out the buffer */
void buffer_print ( struct node *head );

#endif /* __BUFFER_H__ */
