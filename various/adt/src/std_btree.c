/*
 * std_btree: A basic binary tree implementation.
 *
 * Copyright (c) 2004, Mike Mueller & Bob Rossi
 * Subject to the terms of the GNU General Public Licence
 */

/* Standard Includes */
#include <assert.h>
#include <stdlib.h>

/* Local Includes */
#include "std_btree.h"
#include "std_types.h"

/**
 * The binary tree structure.
 */
struct std_btree {

    /**
     * A pointer to the root node of the tree.
     */
    std_btree_node root;
    
    /**
     * A pointer to the destructor for data elements in this tree.
     */
    STDDestroyNotify destructor;

};

/**
 * The node structure, the items that make up the binary tree.
 */
struct std_btree_node {

    /**
     * A pointer back to the tree that contains us.  This will be needed
     * if the root changes, for example if someone calls remove() on the
     * root node.
     */
    std_btree tree;
    
    /**
     * The parent of this node.
     */
    std_btree_node parent;

    /**
     * The children of this node.
     */
    std_btree_node children[2];

    /**
     * The data stored in this node.
     */
    void *data;

};

/*
 * Local Function Prototypes
 */

/* std_btree_destroy_data:
 *
 * Destroys the data at the specified node, if a destructor method has
 * been defined.
 */
static void std_btree_destroy_data(std_btree_node node);

/* std_btree_unlink: 
 *
 * Unlinks the given node from the rest of the tree so that it can be
 * destroyed without any dangling references left behind.
 */
static void std_btree_unlink(std_btree_node node);

/*
 * Exported Functions
 */

/* std_btree_create */
std_btree std_btree_create(const STDDestroyNotify destroy_func)
{
    std_btree tree = (std_btree) malloc(sizeof(struct std_btree));

    /* Be extra paranoid */
    if (tree) {
        tree->destructor = destroy_func;
        tree->root = NULL;
    }

    return tree;
}

/* std_btree_destroy */
int std_btree_destroy(std_btree tree)
{
    if (tree) {
        std_btree_remove(tree->root);
        free(tree);
    }

    return 0;
}

/* std_btree_root */
std_btree_iterator std_btree_root(const std_btree tree)
{
    /* Bounds check */
    assert(tree != NULL);

    return tree->root;
}

/* std_btree_get_data */
int std_btree_get_data(std_btree_iterator iter, void *data)
{
    /* Bounds check */
    assert(iter != NULL);
    assert(data != NULL);

    * (void **) data = iter->data;

    return 0;
}

/* std_btree_child */
std_btree_iterator std_btree_child(
    const std_btree_iterator iter,
    enum std_btree_child child)
{
    /* Bounds check */
    assert(iter != NULL);
    assert(child == STD_BTREE_LEFT || child == STD_BTREE_RIGHT);

    return iter->children[child];
}

/* std_btree_parent */
std_btree_iterator std_btree_parent(const std_btree_iterator iter)
{
    /* Bounds check */
    assert(iter != NULL);

    return iter->parent;
}

/* std_btree_add */
int std_btree_add(
    std_btree tree,
    std_btree_iterator iter,
    enum std_btree_child child,
    void *data)
{
    /* Declarations */
    std_btree_node new_node = NULL;

    /* Bounds check */
    assert(tree != NULL);
    assert(child == STD_BTREE_LEFT || child == STD_BTREE_RIGHT);

    /* Verify the child doesn't already exist */
    if (iter && iter->children[child] != NULL) {
        return 1;
    }

    /* Create the new node */
    new_node = (std_btree_node) malloc(sizeof(struct std_btree_node));

    new_node->tree                      = tree;
    new_node->parent                    = iter;
    new_node->children[STD_BTREE_LEFT]  = NULL;
    new_node->children[STD_BTREE_RIGHT] = NULL;
    new_node->data                      = data;

    /* Link it to the parent */
    if (iter == NULL) {
        tree->root = new_node;
    } else {
        iter->children[child] = new_node;
    }

    return 0;    
}

/* std_btree_remove:
 *
 * This method recursively deletes the subtree starting at the given position.
 * It first destroys the left and right subtrees (this is the recursive part),
 * then deletes the given node.
 *
 * The base case is when iter == NULL, in which case the method just returns.
 */
int std_btree_remove(std_btree_iterator iter)
{
    if (iter != NULL) {

        /* Destroy the left and right subtrees */
        std_btree_remove(iter->children[STD_BTREE_LEFT]);
        std_btree_remove(iter->children[STD_BTREE_RIGHT]);

        /* Unlink this node from anyone who might reference it */
        std_btree_unlink(iter);

        /* Destroy the user data */
        std_btree_destroy_data(iter);

        /* Destroy the node */
        free(iter);

    }

    return 0;
}

/* std_btree_replace */
int std_btree_replace(std_btree_iterator iter, void *data)
{
    /* Bounds check */
    assert(iter != NULL);

    /* Destroy original data */
    std_btree_destroy_data(iter);

    /* Replace with new data */
    iter->data = data;

    return 0;
}

/* std_btree_isroot */
int std_btree_isroot(std_btree_iterator iter)
{
    /* Bounds check */
    assert(iter != NULL);

    return iter->parent == NULL;
}

/* std_btree_isleaf */
int std_btree_isleaf(std_btree_iterator iter)
{
    /* Bounds check */
    assert(iter != NULL);

    return (iter->children[STD_BTREE_LEFT] == NULL 
        && iter->children[STD_BTREE_RIGHT] == NULL);
}           

/* 
 * Local Function Implementations
 */

/* std_btree_destroy_data */
static void std_btree_destroy_data(std_btree_node node)
{
    if (node->tree->destructor != NULL) {
        node->tree->destructor(node->data);
    }
}

/* std_btree_unlink: */
static void std_btree_unlink(std_btree_node node)
{
    if (std_btree_isroot(node)) {

        /* This is the root of the tree, this means we're blasting the 
         * entire tree, and the root node should be set back to NULL.
         */
        node->tree->root = NULL;

    } else {

        /* This is not the root node, update the parent to tell it the child
         * has been removed (set appropriate child pointer to NULL).
         */
        int i;
        for (i = STD_BTREE_LEFT; i <= STD_BTREE_RIGHT; i++) {
            if (node->parent->children[i] == node) {
                node->parent->children[i] = NULL;
            }
            break;
        }

    }
}

