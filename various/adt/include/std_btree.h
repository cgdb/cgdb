/*
 * std_btree: A basic binary tree implementation.
 *
 * Copyright (c) 2004, Mike Mueller & Bob Rossi
 * Subject to the terms of the GNU General Public License
 */

#ifndef __STD_BTREE_H__
#define __STD_BTREE_H__

#include "std_types.h"

/**
 * @file 
 * std_btree.h
 *
 * @brief 
 * An STL-style binary tree data type.
 */

/**
 * @name Standard Binary Tree
 *
 * This is a basic binary tree implementation.  This tree is not automatically
 * balanced, the user can add and remove nodes from any part of the tree at
 * will.
 *
 * Properties of this tree:
 *   Every node has a parent pointer
 *   The root node has no parent
 *   It is not sorted in any particular manner
 */

/**
 * The standard binary tree type.  Declare an instance of std_btree in your
 * code and initialize it with std_btree_create.
 */
typedef struct std_btree *std_btree;

/**
 * A reference to a node in the binary tree.
 */
typedef struct std_btree_node *std_btree_node, *std_btree_iterator;

/**
 * Enumeration used to identify left/right children in accessor methods.
 */
enum std_btree_child { STD_BTREE_LEFT, STD_BTREE_RIGHT };

/**
 * Standard binary tree constructor.
 *
 * @param destroy_func
 * This method will be called on the data when the tree is destroyed.  Set to
 * NULL if no data destructor is needed.
 *
 * @return
 * A new (empty) binary tree on success, NULL on failure.
 */
std_btree std_btree_create(const STDDestroyNotify destroy_func);

/**
 * Standard binary tree destructor.  Releases all memory associated with the
 * given binary tree.  The data destructor method, if specified,  will be 
 * called on every element.
 * 
 * @param tree
 * The tree to destroy.
 *
 * @return
 * Zero on success, non-zero on failure.
 */
int std_btree_destroy(std_btree tree);

/**
 * Get an iterator for this tree.
 *
 * @param tree
 * The tree to access.
 *
 * @return
 * An iterator pointing to the root node of this tree, NULL if tree is empty.
 */
std_btree_iterator std_btree_root(const std_btree tree);

/**
 * Get the data for the node referenced by the specified iterator.
 *
 * @param iter
 * Iterator pointing to the desired node.
 *
 * @param data
 * The address of a pointer which will be set to point to the data.
 * (This is an "in out" parameter.)  Your data can be accessed after this call
 * by: * (my_type *) data
 *
 * @return
 * Zero on success, non-zero on failure.
 */
int std_btree_get_data(std_btree_iterator iter, void *data);

/**
 * Access the child of the current node.
 *
 * @param iter
 * Iterator to use as a starting point.
 *
 * @param child
 * One of STD_BTREE_LEFT or STD_BTREE_RIGHT, specifies which child iterator
 * to return.
 *
 * @return
 * An iterator pointing to the specified child node, NULL if no child exists.
 */
std_btree_iterator std_btree_child(
    const std_btree_iterator iter,
    enum std_btree_child child);

/**
 * Access the parent of the current node.
 *
 * @param iter
 * Iterator to use as a starting point.
 *
 * @return
 * An iterator pointing to the parent of this node, or NULL for the root node.
 */
std_btree_iterator std_btree_parent(const std_btree_iterator iter);

/**
 * Adds a node to the tree.
 *
 * @param tree
 * The tree to add to.
 *
 * @param iter
 * Iterator to use as a starting point.  Use NULL if the tree is currently
 * empty, add will create the root node.
 *
 * @param child
 * One of STD_BTREE_LEFT or STD_BTREE_RIGHT, specifies which child to add to
 * the current node.  If specified child already exists, add will fail.
 *
 * @param data
 * The data to store at the specified location.
 *
 * @return
 * Zero on success, non-zero on failure.
 */
int std_btree_add(
    std_btree tree,
    std_btree_iterator iter,
    enum std_btree_child child,
    void *data);                             

/**
 * Removes a subtree from the tree.
 *
 * @param iter
 * Iterator pointing to the node to destroy.  The tree will be pruned at this
 * point, including all children.  The destroy_func, if specified, will be
 * called on any destroyed nodes.
 *
 * @return
 * Zero on success, non-zero on failure (always succeeds).
 */
int std_btree_remove(std_btree_iterator iter);

/**
 * Replaces the data at the specified node of the tree with the new data
 * provided.  The data previously occupying this location will be destroyed
 * with destroy_func, if it was specified.
 *
 * @param iter
 * Iterator pointing to the node to replace.
 *
 * @param data
 * The data to store at the specified location.
 *
 * @return
 * Zero on success, non-zero on failure.
 */
int std_btree_replace(std_btree_iterator iter, void *data);

/*
 *  Utility methods
 */

/**
 * Identifies whether the given node is the root node.
 *
 * @param iter
 * Iterator pointing to the desired node.
 *
 * @return
 * Non-zero if node is root, zero otherwise.
 */
int std_btree_isroot(std_btree_iterator iter);

/**
 * Identifies the given node as a leaf.
 *
 * @param iter
 * Iterator pointing to the desired node.
 *
 * @return
 * Non-zero for a leaf node, zero for a non-leaf node.
 */
int std_btree_isleaf(std_btree_iterator iter);

#endif
