/* GLIB - Library of useful routines for C programming
 * Copyright (C) 1995-1997  Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __STD_LIST_H__
#define __STD_LIST_H__

/**
 * The following functions are used to create, manage, and destroy standard 
 * doubly linked lists. Each element in the list contains a piece of data, 
 * together with pointers which link to the previous and next elements in the 
 * list. This enables easy movement in either direction through the list. The 
 * data item is of type "gpointer", which means the data can be a pointer to 
 * your real data or (through casting) a numeric value (but do not assume 
 * that int and gpointer have the same size!). These routines internally 
 * allocate list elements in blocks, which is more efficient than allocating 
 * elements individually.
 *
 * There is no function to specifically create a list. Instead, simply create 
 * a variable of type list* and set its value to NULL; NULL is considered to 
 * be the empty list.
 *
 * To add elements to a list, use the std_list_append(), std_list_prepend(), 
 * std_list_insert(), or std_list_insert_sorted() routines. In all cases they 
 * accept a pointer to the beginning of the list, and return the (possibly 
 * changed) pointer to the beginning of the list. Thus, for all of the 
 * operations that add or remove elements, be sure to save the returned 
 * value!
 */

#include <stdlib.h>
#include "std_types.h"

/**
 * The doubly linked list context.
 */
struct std_list;

/**
 * Free a list.
 *
 * If list elements contain dynamically-allocated memory, 
 * they should be freed first.
 *
 * \param list
 * The list to free.
 *
 * @return 0 on success, or -1 on error.
 */
int
std_list_free (struct std_list *list);

/**
 * Frees one list element. It is usually used after std_list_remove_link().
 *
 * \param list
 * The list item to delete
 */
void std_list_free_1 ( struct std_list *list );

/**
 * Adds a new element on to the end of the list.
 *
 * \param list
 * The list to append to
 *
 * \param data
 * The data to put in the list.
 *
 * @return
 * The new list.
 */
struct std_list* std_list_append (
	struct std_list *list,
	void *data);

/**
 * Adds a new element on to the start of the list.
 *
 * \param list
 * The list to prepend to
 *
 * \param data
 * The data to put in the list.
 *
 * @return
 * The new list.
 */
struct std_list* std_list_prepend (
	struct std_list *list,
	void *data);

/**
 * This inserts a new element (with value data) into the list at the 
 * given position. If position is 0, this is just like std_list_prepend(); 
 * if position is less than 0, this is just like std_list_append().
 *
 * \param list
 * The list to insert into.
 *
 * \param data
 * The data to put in the list.
 *
 * \param position
 * The position in the list to insert into.
 *
 * @return
 * return the new list.
 */
struct std_list* std_list_insert (
	struct std_list *list,
	void *data,
	int position );

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
 * The new list
 */
struct std_list* std_list_insert_sorted (
	struct std_list *list,
	void *data,
	STDCompareFunc func);

/**
 * Inserts a new element into the list before the sibling.
 *
 * \param list
 * The list to insert into
 *
 * \param sibling
 * the list element before which the new element is inserted 
 * or NULL to insert at the end of the list.
 *
 * \param data
 * The data to insert
 *
 * @return
 * The new list.
 */
struct std_list* std_list_insert_before (
	struct std_list *list,
	struct std_list *sibling,
	void *data);

/**
 * Concatinate the two lists.
 *
 * \param list1
 * The first list
 *
 * \param list2
 * The second list
 *
 * @return
 * The new list.
 */
struct std_list* std_list_concat ( 
	struct std_list *list1,
	struct std_list *list2);

/**
 * Removes an element from a list. If two elements contain the same data, 
 * only the first is removed. If none of the elements contain the data, 
 * the list is unchanged.
 *
 * \param list
 * The list to remove an item from.
 *
 * \param data
 * The data to remove
 *
 * @return
 * The new list
 */
struct std_list* std_list_remove (
	struct std_list *list,
	const void *data );

/**
 * Remove all the elements in the list
 */
struct std_list* std_list_remove_all ( 
	struct std_list *list,
	const void *data );

/**
 * Removes an element from a list, without freeing the element. 
 * The removed element's prev and next links are set to NULL, so 
 * that it becomes a self-contained list with one element.
 *
 * \param list
 * The list to operate on.
 *
 * \param llink
 * An element in the list.
 *
 * @return
 * The new start of the list, without the element.
 */
struct std_list* std_list_remove_link (
	struct std_list *list,
	struct std_list *llink );

/**
 * Deletes the node link_ from list.
 *
 * \param list
 * The list to delete the node from.
 *
 * \param link_
 * The link to remove.
 *
 * @return
 * The start of the new list
 */
struct std_list* std_list_delete_link (
	struct std_list *list,
	struct std_list *link_ );

/**
 * Reverses a list. 
 * It simply switches the next and prev pointers of each element.
 *
 * \param list
 * The list to reverse.
 *
 * @return
 * The start of the new list
 */
struct std_list* std_list_reverse ( struct std_list *list);

/**
 * Note that this is a "shallow" copy. 
 * If the list elements consist of pointers to data, the pointers are 
 * copied but the actual data isn't.
 *
 * \param list
 * The list to copy
 *
 * @return
 * The start of the new list
 */
struct std_list* std_list_copy ( struct std_list *list);

/**
 * Gets the element at the given position in a list.
 *
 * \param list
 * The list to get the nth item of.
 *
 * \param n
 * The position of the element, counting from 0.
 *
 * @return
 * The nth element.
 */
struct std_list* std_list_nth (
	struct std_list *list,
	unsigned int n);

/**
 * Gets the element n places before list.
 *
 * \param list
 * The list to start from.
 *
 * \param n
 * The position of the element, counting from 0. 
 *
 * @return
 * The element, or NULL if the position is off the end of the list.
 */
struct std_list* std_list_nth_prev (
	struct std_list *list,
	unsigned int n);

/**
 * Finds the element in a list which contains the given data.
 *
 * \param list
 * The list to search in.
 *
 * \param data
 * The element data to find.
 *
 * @return
 * The found list element, or NULL if it is not found.
 */
struct std_list* std_list_find (
	struct std_list *list,
	const void *data );

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
 * The found list element, or NULL if it is not found.
 */
struct std_list* std_list_find_custom (
	struct std_list *list,
	const void *data,
	STDCompareFunc func );

/**
 * Gets the position of the given element in the list (starting from 0).
 *
 * \param list
 * The list to get the position of.
 *
 * \param llink
 * an element in the list
 *
 * @return
 * The position of the element in the list, or -1 if the element is not found.
 */
int std_list_position ( 
	struct std_list *list,
	struct std_list *llink );

/**
 * Gets the position of the element containing the given data (starting from 0).
 *
 * \param list
 * The list
 *
 * \param data
 * The data to find.
 *
 * @return
 * 	the index of the element containing the data, 
 * 	or -1 if the data is not found.
 */
int std_list_index (
	struct std_list *list,
	const void *data );

/**
 * Gets the last element in a list.
 *
 * \param list
 * The list
 *
 * @return
 * The last element in the list, or NULL if the list has no elements.
 */
struct std_list* std_list_last (struct std_list *list);

/**
 * Gets the first element in a list.
 *
 * \param list
 * The list to get the element from.
 *
 * @return
 * The first element in a list, or NULL if the list has no elements.
 */
struct std_list* std_list_first ( struct std_list *list);

/**
 * Gets the number of elements in a list.
 *
 * \param list
 * The list to get the length of.
 *
 * @return
 * The number of elements in the list.
 */
unsigned int std_list_length ( struct std_list *list );

/**
 * Calls a function for each element of a list.
 *
 * \param list
 * The list to traverse
 *
 * \param func
 * The function to call with each element's data.
 *
 * \param user_data
 * User data to pass to the function
 */
void std_list_foreach ( 
	struct std_list *list,
	STDFunc func,
	void *user_data );

/**
 * Sorts a list using the given comparison function.
 *
 * \param list
 * The list to sort
 *
 * \param compare_func
 * The comparison function used to sort the list. This function is passed 
 * 2 elements of the list and should return 0 if they are equal, a negative 
 * value if the first element comes before the second, or a positive value 
 * if the first element comes after the second.
 *
 * @return
 * The start of the new list
 */
struct std_list* std_list_sort (
	struct std_list *list,
	STDCompareFunc compare_func );

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
 * The start of the new list
 */
struct std_list* std_list_sort_with_data (
	struct std_list *list,
	STDCompareDataFunc compare_func,
	void *user_data );

/**
 * Gets the data of the element at the given position.
 *
 * \param list
 * The list to get the data from
 *
 * \param n
 * The position of the element.
 *
 * @return
 * The element's data, or NULL if the position is off the end of the list. 
 */
void* std_list_nth_data (
	struct std_list *list,
	unsigned int n );

/**
 * Get's the data at the specified list position.
 *
 * \param list
 * The position of the list to get the data.
 *
 * @return
 * The data at the position or NULL on error
 */
void *std_list_get_data ( struct std_list *list );

#endif /* __STD_LIST_H__ */

