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

#ifndef __STD_BBTREE_H__
#define __STD_BBTREE_H__

#include "std_types.h"

/**
 * The balanced binary tree context.
 */
struct std_bbtree;

typedef int (*STDTraverseFunc) (
	void* key,
        void* value,
        void* data);

/**
 * Create a new tree.
 *
 * \param key_compare_func
 * The function used to order the nodes in the list. 
 * It should return values similar to the standard strcmp() function - 
 *   0 if the two arguments are equal, 
 * 	 a negative value if the first argument comes before the second, 
 * 	 or a positive value if the first argument comes after the second.
 * 
 * @return
 * A new tree on success, or NULL on error.
 */
struct std_bbtree* std_bbtree_new ( STDCompareFunc key_compare_func );

/**
 * Creates a new std_bbtree with a comparison function that accepts user data.
 * See std_bbtree_new() for more details.
 *
 * \param key_compare_func
 * Same as above.
 *
 * \param key_compare_data
 * This data should be passed along as the last argument to key_compare_func 
 *
 * @return
 * A new tree on success, or NULL on error.
 */
struct std_bbtree* std_bbtree_new_with_data (
	STDCompareDataFunc key_compare_func,
	void * key_compare_data );

/**
 * Creates a new std_bbtree like std_bbtree_new() and allows to specify functions 
 * to free the memory allocated for the key and value that get called when 
 * removing the entry from the std_bbtree.
 *
 * \param key_compare_data
 * Same as above.
 *
 * \param key_destroy_func
 * A function to free the memory allocated for the key used when removing the 
 * entry from the std_bbtree or NULL if you don't want to supply such a function.
 *
 * \param value_destroy_func
 * A function to free the memory allocated for the value used when removing the 
 * entry from the std_bbtree or NULL if you don't want to supply such a function.
 *
 * @return
 * A new tree on success, or NULL on error.
 */
struct std_bbtree* std_bbtree_new_full (
	STDCompareDataFunc key_compare_func,
    void *key_compare_data,
	STDDestroyNotify key_destroy_func,
	STDDestroyNotify value_destroy_func );

/**
 * Destroys the tree. If keys and/or values are dynamically allocated, you 
 * should either free them first or create the tree using std_bbtree_new_full().
 * In the latter case the destroy functions you supplied will be called on 
 * all keys and values before destroying the tree.
 *
 * \param tree
 * The tree to destroy.
 *
 * @return
 * 0 on success, or -1 on error.
 */
int std_bbtree_destroy ( struct std_bbtree *tree );

/**
 * Inserts a key/value pair into a std_bbtree. If the given key already exists 
 * in the std_bbtree its corresponding value is set to the new value. If you 
 * supplied a value_destroy_func when creating the std_bbtree, the old value is 
 * freed using that function. If you supplied a key_destroy_func when 
 * creating the std_bbtree, the passed key is freed using that function.
 *
 * The tree is automatically 'balanced' as new key/value pairs are added,
 * so that the distance from the root to every leaf is as small as possible.
 *
 * \param tree
 * The tree to insert into.
 *
 * \param key
 * The key to insert.
 *
 * \param value
 * The value to insert.
 *
 * @return
 * 0 on success, or -1 on error.
 */
int std_bbtree_insert (
	struct std_bbtree *tree,
	void *key,
	void *value );

/**
 * Inserts a new key and value into a std_bbtree similar to std_bbtree_insert(). 
 * The difference is that if the key already exists in the std_bbtree, it gets 
 * replaced by the new key. If you supplied a value_destroy_func when 
 * creating the struct std_bbtree, the old value is freed using that function. If you 
 * supplied a key_destroy_func when creating the #struct std_bbtree, the old key is 
 * freed using that function. 
 *
 * The tree is automatically 'balanced' as new key/value pairs are added,
 * so that the distance from the root to every leaf is as small as possible.
 *
 * \param tree
 * The tree to insert into.
 *
 * \param key
 * The key to insert.
 *
 * \param value
 * The value to insert.
 *
 * @return
 * 0 on success, or -1 on error.
 */
int std_bbtree_replace (
	struct std_bbtree *tree,
    void *key,
    void *value );

/**
 * Removes a key/value pair from a std_bbtree.
 *
 * If the std_bbtree was created using std_bbtree_new_full(), the key and value 
 * are freed using the supplied destroy functions, otherwise you have to 
 * make sure that any dynamically allocated values are freed yourself.
 *
 * \param tree
 * The tree to remove a key from.
 *
 * \param key
 * The key to remove.
 *
 * @return
 * 0 on success, or -1 on error.
 */
int std_bbtree_remove ( 
	struct std_bbtree *tree,
	const void *key );

/**
 * Removes a key and its associated value from a std_bbtree without calling 
 * the key and value destroy functions.
 *
 * \param tree
 * The tree to remove a key from.
 *
 * \param key
 * The key to remove.
 *
 * @return
 * 0 on success, or -1 on error.
 */
int std_bbtree_steal (
	struct std_bbtree *tree,
	const void *key );

/**
 * Gets the value corresponding to the given key. Since a std_bbtree is 
 * automatically balanced as key/value pairs are added, key lookup is very 
 * fast.
 *
 * \param tree
 * The tree to lookup a key from.
 *
 * \param key
 * The key to lookup.
 *
 * @return
 * The value corresponding to the key, or NULL on error.
 */
void* std_bbtree_lookup ( 
	struct std_bbtree *tree,
    const void *key );

/**
 * Looks up a key in the std_bbtree, returning the original key and the
 * associated value and a int which is 1 if the key was found. This 
 * is useful if you need to free the memory allocated for the original key, 
 * for example before calling std_bbtree_remove().
 * 
 * \param tree
 * The tree to lookup a key from.
 * 
 * \parm lookup_key
 * The key to look up.
 *
 * \param orig_key
 * Returns the original key.
 *
 * \param value
 * Returns the value associated with the original key. 
 * 
 * @return
 * 1 if the key was found in the std_bbtree, otherwise 0.
 */
int std_bbtree_lookup_extended (
	struct std_bbtree *tree,
    const void *lookup_key,
    void **orig_key,
    void **value );

/**
 * Calls the given function for each of the key/value pairs in the std_bbtree.
 * The function is passed the key and value of each pair, and the given
 * data parameter. The tree is traversed in sorted order.
 *
 * The tree may not be modified while iterating over it (you can't 
 * add/remove items). To remove all items matching a predicate, you need 
 * to add each item to a list in your STDTraverseFunc as you walk over 
 * the tree, then walk the list and remove each item.
 *
 * \param tree
 * The tree to traverse.
 *
 * \param func
 * The function to call for each node visited. If this function
 * returns 1, the traversal is stopped, otherwise it keeps going.
 *
 * \param user_data
 * The user data to pass to the function.
 *
 * @return
 * 0 on success, or -1 on error.
 */
int std_bbtree_foreach (
	struct std_bbtree *tree,
    STDTraverseFunc func,
    void *user_data );

/**
 * Searches a std_bbtree using search_func.
 *
 * The search_func is called with a pointer to the key of a key/value pair in 
 * the tree, and the passed in user_data. If search_func returns 0 for a 
 * key/value pair, then std_bbtree_search_func() will return the value of that 
 * pair. If search_func returns -1, searching will proceed among the 
 * key/value pairs that have a smaller key; if search_func returns 1, 
 * searching will proceed among the key/value pairs that have a larger key.
 *
 * \param tree
 * The tree to search.
 *
 * \param search_func
 * A function used to search the std_bbtree. 
 *
 * \param user_data
 * The data passed as the second argument to the @search_func function.
 *
 * @return
 * The value corresponding to the found key, or NULL if the key was not found.
 */
void* std_bbtree_search (
	struct std_bbtree *tree,
    STDCompareFunc search_func,
    const void *user_data );

/**
 * Gets the height of a std_bbtree.
 *
 * If the std_bbtree contains no nodes, the height is 0.
 * If the std_bbtree contains only one root node the height is 1.
 * If the root node has children the height is 2, etc.
 *
 * \param tree
 * The tree to get the height of.
 * 
 * @return
 * The height of the std_bbtree.
 */
int std_bbtree_height ( struct std_bbtree *tree );

/**
 * Gets the number of nodes in a std_bbtree.
 *
 * \param tree
 * The tree to get the number of nodes in.
 *
 * @return
 * The number of nodes in the tree.
 */
int std_bbtree_nnodes ( struct std_bbtree *tree );

#endif /* __G_TREE_H__ */

