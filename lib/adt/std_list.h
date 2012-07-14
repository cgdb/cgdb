#ifndef __STD_LIST_H__
#define __STD_LIST_H__

#include <stdlib.h>
#include "std_types.h"

/*! 
 * \file
 * std_list.h
 *
 * \brief
 * An STL style doubly linked list.
 */

/**
 * The doubly linked list context.
 */
struct std_list;

/**
 * A typedef for an std_list.
 * This is what the programmer should declare.
 */
typedef struct std_list *std_list;

/**
 * A forward declaration, only around to satisfy the iterator typedef.
 */
struct std_list_node;

/**
 * A std_list_iterator. This is used to iterator over a std_list.
 */
typedef struct std_list_node *std_list_iterator;

/**
 * Create a list.
 *
 * \param destroy_func
 * DestroyFunc will be called on the data, before it is removed.
 *
 * @return
 * A new list on success, or NULL on error.
 */
struct std_list *std_list_create(STDDestroyNotify destroy_func);

/**
 * Destroy a list.
 *
 * Do not attempt to use this list after it is destroyed.
 *
 * If list elements contain dynamically-allocated memory, 
 * they should be freed first.
 *
 * @return
 * 0 on success, or -1 on error.
 */
int std_list_destroy(struct std_list *list);

/**
 * Append the item to the end of the list.
 *
 * \param list
 * The list to append to.
 *
 * \param data.
 * The new data.
 *
 * @return
 * 0 on success, or -1 on error.
 */
int std_list_append(struct std_list *list, void *data);

/**
 * Prepend the item to the end of the list.
 *
 * \param list
 * The list to prepend to.
 *
 * \param data.
 * The new data.
 *
 * @return
 * 0 on success, or -1 on error.
 */
int std_list_prepend(struct std_list *list, void *data);

/**
 * Inserts a new element into the list before the iterator.
 *
 * \param list
 * The list to insert into
 *
 * \param iter
 * The iterator to insert before.
 *
 * \param data
 * The data to insert
 *
 * @return
 * 0 on success, or -1 on error.
 */
int std_list_insert(struct std_list *list,
        const std_list_iterator iter, void *data);

/**
 * Inserts a new element into the list, using the given comparison 
 * function to determine its position.
 *
 * \param list
 * the list to insert into
 *
 * \param data
 * The data to insert
 *
 * \param func
 * The function used to sort the data
 * It should return a number > 0 if the first parameter comes after 
 * the second parameter in the sort order.
 *
 * @return
 * 0 on success, or -1 on error.
 */
int std_list_insert_sorted(struct std_list *list,
        void *data, const STDCompareFunc func);

/**
 * Like std_list_sort(), but the comparison function accepts a user data argument.
 *
 * \param list
 * The list to sort
 *
 * \param compare_func
 * comparison function
 *
 * \param user_data
 * user data to pass to comparison function.
 *
 * @return
 * 0 on success, or -1 on error.
 */
/*
 * int std_list_sort (
 * struct std_list *list,
 * STDCompareDataFunc compare_func,
 * void *user_data );
 */

/**
 * Removes an element from a list. If two elements contain the same data, 
 * only the first is removed. If none of the elements contain the data, 
 * the list is unchanged.
 *
 * \param list
 * The list to remove an item from.
 *
 * \param iter
 * The iterator to remove the data at. The iterator will be moved
 * to the next position in the list.
 *
 * @return
 * The iterator after the one being removed, or NULL on error.
 */
std_list_iterator std_list_remove(struct std_list *list,
        std_list_iterator iter);

/**
 * Removes all the elements from a list. 
 *
 * \param list
 * The list to remove an item from.
 *
 * @return
 * 0 on success, or -1 on error.
 */
int std_list_remove_all(struct std_list *list);

/**
 * Finds an element in a list, using a supplied function to find the 
 * desired element. It iterates over the list, calling the given function 
 * which should return 0 when the desired element is found. The function 
 * takes two void* arguments, the list element's data and the 
 * given user data.
 *
 * \param list
 * The list to search.
 *
 * \param data
 * user data passed to the function.
 *
 * \param func
 * The function to call for each element. 
 * It should return 0 when the desired element is found.
 *
 * @return
 * the found iterator on success,
 * std_list_end if not found,
 * NULL on error
 */
std_list_iterator std_list_find(const struct std_list *list,
        const void *data, const STDCompareFunc func);

/**
 * Gets the first element in a list.
 *
 * \param list
 * The list to get the element from.
 *
 * @return
 * The first iterator, or NULL on error.
 */
std_list_iterator std_list_begin(const struct std_list *list);

/**
 * Gets the position after the last element.
 *
 * \param list
 * The list to get the element from.
 *
 * @return
 * The last iterator, or NULL on error.
 */
std_list_iterator std_list_end(const struct std_list *list);

/**
 * Gets the next iterator.
 *
 * \param iter
 * The iterator to get the next position of.
 *
 * @return
 * The next iterator, or NULL on error.
 */
std_list_iterator std_list_next(const std_list_iterator iter);

/**
 * Gets the previous iterator.
 *
 * \param iter
 * The iterator to get the previous position of.
 *
 * @return
 * The previous iterator, or NULL on error.
 */
std_list_iterator std_list_previous(const std_list_iterator iter);

/**
 * Gets the number of elements in a list.
 *
 * \param list
 * The list to get the length of.
 *
 * @return
 * The number of elements in the list, or -1 on error.
 */
int std_list_length(struct std_list *list);

/**
 * Calls a function for each element of a list.
 *
 * \param list
 * The list to traverse
 *
 * \param func
 * The function to call with each element's data.
 * If func returns 0, the traversing will stop.
 *
 * \param user_data
 * User data to pass to the function
 *
 * @return
 * 0 on success, or -1 on error.
 */
int std_list_foreach(const struct std_list *list,
        const STDFunc func, void *user_data);

/**
 * Get's the data at the specified list position.
 *
 * \param iter
 * The iterator to get the data from.
 *
 * \param data
 * Pass in the address of a pointer, and the pointer will be set to point at
 * the data for this element.  Will be set to NULL on error.
 * 
 * @return
 * 0 on success, or -1 on error.
 */
int std_list_get_data(std_list_iterator iter, void *data);

#endif /* __STD_LIST_H__ */
