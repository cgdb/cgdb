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

/* buffer_get_incomplete_command:
 * This will return what the user is currently typing for a command. 
 * It will copy into buf what is being typed.
 * Return: 
 *    It will return a pointer to buf.
 *    or NULL if nothing has been typed.
 */
char *buffer_get_incomplete_command( char *buf );

/* buffer_is_empty: 
 * Determines if the internal buffer is empty.
 * Returns: TRUE if empty, otherwise FALSE.
 */
int buffer_is_empty(void);

/* buffer_write_char:
 *    This keeps track of all of the char's it gets until a new line
 *    is reached. At that point, a new command is added to the list.
 *    Only a '\b' will remove one char from what is in the buffer.
 *    A ^W may someday be supported.
 */
struct node *buffer_write_char ( struct node *com, const char c );

/* Set's the internal string to '\0' */
void buffer_clear_string ( void );

/* Prints out the buffer */
void buffer_print ( struct node *head );

#endif /* __BUFFER_H__ */
