/* Standard Includes */
#include <string.h>

/* Local Includes */
#include "std_list.h"

/**
 * A node in the linked list.
 */
struct std_list_node {
	/**
	 * The data stored in the node.
	 */ 
	void *data;

	/**
	 * The next node in the list, or NULL.
	 */
    struct std_list_node *next;

	/**
	 * The previous node in the list, or NULL.
	 */
    struct std_list_node *prev;
};

/**
 * The doubly linked list context.
 *
 * This stores the begin and end (one past last node) of the list,
 * and can store any state data associated with a list context.
 */
struct std_list {
	/**
	 * A pointer to the first node in the list.
	 *
	 * If the list is empty, this points to end.
	 */
    struct std_list_node *begin;

	/**
	 * A pointer to the node after the last node in the list.
	 * If the list is empty, this points to an empty node.
	 */
    struct std_list_node *end;
	
	/**   
	 * The size of the list.
	 */
	int length;

	/**
	 * A destroy function used to free the user's data before a list
	 * item is free'd.
	 */
	STDDestroyNotify destroy_func;
};

/* Functions */
/* --------- */

/**
 * Allocate a single std_list_node.
 *
 * @return
 * A new list on success, or NULL on error
 */
static struct std_list_node* std_list_node_create (void)
{
  struct std_list_node *list_node;
  
  list_node = malloc ( sizeof ( struct std_list_node ));

  if ( !list_node )
	  return NULL;

  list_node->data = NULL;
  list_node->next = NULL;
  list_node->prev = NULL;
  
  return list_node;
}

/**
 * Destroy a single std_list_node.
 *
 * @return
 * 0 on success, or -1 on error.
 */
static int std_list_node_destroy ( 
		struct std_list_node *list_node,
        STDDestroyNotify destroy_func ) {

	if ( !list_node )
		return -1;

	if ( destroy_func )
	    destroy_func ( list_node->data );

	list_node->data = NULL;
	list_node->next = NULL;
	list_node->prev = NULL;
	free ( list_node );
	list_node = NULL;
	
	return 0;
}

/**
 * Allocate for a new list.
 *
 * @return
 * A new list on success, or NULL on error
 */
struct std_list* std_list_create ( STDDestroyNotify destroy_func )
{
  struct std_list *list;
  
  list = malloc ( sizeof ( struct std_list ));

  if ( !list )
	  return NULL;

  list->begin = std_list_node_create ();

  if ( !list->begin )
	  return NULL;

  list->end = list->begin;
  list->length = 0;
  list->destroy_func = destroy_func;
  
  return list;
}

int std_list_remove_all ( struct std_list *list ) {
	std_list_iterator iter;

	/* Traverse the list and free the data members */
	for ( iter = std_list_begin ( list ); 
		  iter != std_list_end ( list ); ){ 
		iter = std_list_remove ( list, iter );

	    if (!iter )
			return -1;
	}
	
	return 0;
}

int std_list_destroy ( struct std_list *list ) {

	if ( !list )
		return -1;


	if ( std_list_remove_all ( list ) == -1 )
		return -1;

    /* Free the dummy node (remove won't do this) */
    std_list_node_destroy(list->end, list->destroy_func);   
    
    /* Free the list structure */
    free(list); 
	
	return 0;
}

int std_list_append ( 
    struct std_list *list,
    void *data ) {

    if ( !list )
        return -1;

	if ( std_list_insert ( list, list->end, data ) == -1 )
		return -1;

    return 0;
}

int std_list_prepend ( 
    struct std_list *list,
    void *data ) {

    if ( !list )
        return -1;

	if ( std_list_insert ( list, list->begin, data ) == -1 )
		return -1;

    return 0;
}

int std_list_insert (
	struct std_list *list,
	const std_list_iterator iter,
	void *data) {

	std_list_iterator new_node;

	if ( !list )
		return -1;

	if ( !iter )
		return -1;

	new_node = std_list_node_create ();

	if ( !new_node )
		return -1;

	new_node->data = data;

	if ( iter == list->begin ) {
		new_node->next = iter;
		iter->prev = new_node;
		list->begin = new_node;
	} else { 
		std_list_iterator before;
		before = iter->prev;

		new_node->next = iter;
		before->next = new_node;

		new_node->prev = before;
		iter->prev = new_node;
	}

	list->length = list->length + 1;

	return 0;
}

int std_list_insert_sorted (
	struct std_list *list,
	void *data,
	const STDCompareFunc func) {

    std_list_iterator current;
    void *current_data;

	if ( !list )
		return -1;

	if ( !func )
		return -1;

    /* Seek to the corrent position to insert new item */
    for (current  = std_list_begin(list); 
         current != std_list_end(list);
         current  = std_list_next(current))
    {
        if (std_list_get_data(current, &current_data) != 0){
            return -2;
        }

        /* If the new data is <= data at this node, stop looping and
         * insert the data into the list before the current. */
        if (func(data, current_data) <= 0){
            break;
        }
    }

    /* Insert the item before the current iterator now. */
    return std_list_insert(list, current, data);
}

std_list_iterator std_list_remove (
	struct std_list *list,
	std_list_iterator iter ) {

	std_list_iterator after;

	if ( !list )
		return NULL;

	if ( !iter )
		return NULL;

	/* remove the iterator */
	if ( iter == list->end )
		return NULL;

	if ( iter == list->begin ) {
		after = iter->next;
		list->begin = after;
		list->begin->prev = NULL;
	} else {
		std_list_iterator before;	

		before = iter->prev;
		after = iter->next;

		before->next = after;
		after->prev = before;
	}

	if ( std_list_node_destroy ( iter, list->destroy_func ) == -1 )
		return NULL;

	list->length -= 1;

	return after;
}

std_list_iterator std_list_find (
	const struct std_list *list,
	const void *data,
	const STDCompareFunc func ) {
	std_list_iterator iter;
	std_list_iterator found;

	if ( !list )
		return NULL;

	if ( !func )
		return NULL;

	found = std_list_end ( list );

	/* Find the item */
	for ( iter = std_list_begin ( list ); 
		  iter != std_list_end ( list ); 
		  iter = std_list_next ( iter ) ){ 
		void *iter_data;

		if ( std_list_get_data ( iter, &iter_data ) == -1 )
			return NULL;
	
		if ( func ( iter_data, data ) == 0 ) {
			found = iter;
			break;
		}
	}

	return iter;
}

std_list_iterator std_list_begin ( const struct std_list *list ) {

	if ( !list ) 
		return NULL;

	return list->begin;
}

std_list_iterator std_list_end ( const struct std_list *list ) {

	if ( !list ) 
		return NULL;

	return list->end;
}

std_list_iterator std_list_next ( const std_list_iterator iter ){
	if ( !iter ) 
		return NULL;

	return iter->next;
}

std_list_iterator std_list_previous ( const std_list_iterator iter ){
	if ( !iter ) 
		return NULL;

	return iter->prev;
}

int std_list_length ( struct std_list *list ) {

	if ( !list )
		return -1;

	return list->length;
}

int std_list_foreach ( 
	const struct std_list *list,
	const STDFunc func,
	void *user_data ) {
	std_list_iterator iter;

	if ( !list )
		return -1;

	if ( !func )
		return -1;

	/* do the foreach */
	for ( iter = std_list_begin ( list ); 
		  iter != std_list_end ( list ); 
		  iter = std_list_next ( iter ) ){ 
		void *data;

		if ( std_list_get_data ( iter, &data ) == -1 )
			return -1;
		
		if ( func ( data, user_data ) == 0 )
			break;
	}

	return 0;
}

int std_list_sort (
	struct std_list *list,
	STDCompareFunc compare_func ) {

	if ( !list )
		return -1;

	if ( !compare_func )
		return -1;

	/* sort the list */

	return 0;
}

int std_list_sort_with_data (
	struct std_list *list,
	STDCompareDataFunc compare_func,
	void *user_data ) {
	if ( !list )
		return -1;

	if ( !compare_func )
		return -1;

	/* sort the list */

	return 0;
}

int std_list_get_data ( 
    std_list_iterator iter,
    void *data	) {

	if ( !iter )
		return -1;

    /* Copy the pointer */
    memcpy(data, &iter->data, sizeof(void *));

	return 0;
}
