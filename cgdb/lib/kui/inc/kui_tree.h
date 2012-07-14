#ifndef __KUI_TREE_H__
#define __KUI_TREE_H__

/* Includes {{{*/

#include "std_list.h"

/* }}}*/

/* Doxygen headers {{{ */
/*! 
 * \file
 * kui_tree.h
 *
 * \brief
 * This interface is the algorithm behind libkui's fast macro finding abilities.
 * It is a statefull data structure, which can be told to start matching, to
 * recieve input, and then to stop matching. When it is told to stop, it knows
 * exactly which macro's have been completed, and how much data is extra.
 */
/* }}} */

/* struct kui_map {{{ */
/******************************************************************************/
/**
 * @name Creating and Destroying a kui_map.
 * These functions are for createing and destroying a kui map.
 *
 * A kui map is basically a key value pair. As far as the map is concerned both
 * the key and the value are of type 'char*'. This is because, the user needs to
 * type the map in at the keyboard. This may seem obvious at first, but things 
 * like ESCAPE and HOME have to be typed in also.
 */
/******************************************************************************/

/*@{*/

struct kui_tree;

/**
 * Create a kui tree. This data structure is designed specifically for 
 * the Key User Interface. It should be optimized for speed, since determining
 * what key the user wants to be processed next should be fast.
 *
 * @return
 * A new instance on success, or NULL on error. 
 */
struct kui_tree *kui_tree_create(void);

/**
 * Destroy a kui map.
 *
 * \param ktree
 * The kui tree to destroy
 *
 * @return 
 * 0 on success, or -1 on error.
 */
int kui_tree_destroy(struct kui_tree *ktree);

/*@}*/

/******************************************************************************/
/**
 * @name Inserting and Deleteing from a kui_tree
 * These are the basic functions of adding to and removing from a kui_tree
 */
/******************************************************************************/

/*@{*/

/**
 * Adding a key to the kui tree.
 *
 * \param ktree
 * The tree to add a new key too.
 *
 * \param klist
 * The data to add into the tree.
 * It is a null terminated list.
 *
 * \param data
 * If this is the "value" part of the map being inserted.
 *
 * @return
 * 0 on success, or -1 on error.
 */
int kui_tree_insert(struct kui_tree *ktree, int *klist, void *data);

/**
 * Adding a key from the kui tree.
 *
 * \param ktree
 * The tree to delete a key from.
 *
 * \param klist
 * The key to remove from the tree
 * It is a null terminated list.
 *
 * @return
 * 0 on success, or -1 on error.
 */
int kui_tree_delete(struct kui_tree *ktree, int *klist);

/*@}*/

/******************************************************************************/
/**
 * @name Setting and Querying the state of a kui_tree
 * A kui_tree is a stateful ADT. This interface documents how to set/get
 * the state of a particular kui tree.
 */
/******************************************************************************/

/*@{*/

enum kui_tree_state {
    KUI_TREE_FOUND = 0,
    KUI_TREE_MATCHING,
    KUI_TREE_NOT_FOUND,
    KUI_TREE_ERROR
};

/**
 * To start searching for a macro, reset the state of the tree.
 *
 * \param ktree
 * The tree to start searching for a macro in.
 *
 * @return
 * 0 on success, or -1 on error.
 */
int kui_tree_reset_state(struct kui_tree *ktree);

/**
 * To finish searching for a macro, finalize the state of the tree.
 * This allows the tree to do any book keeping it must do, and allows it
 * to find out exaclty which macro should was called, and how many extra
 * key strokes have been recieved.
 *
 * \param ktree
 * The tree to finalize the state of.
 *
 * @return
 * 0 on success, or -1 on error.
 */
int kui_tree_finalize_state(struct kui_tree *ktree);

/**
 * Get's the state of the current tree.
 *
 * \param ktree
 * The tree to get the state of.
 *
 * \param state
 * The state of the tree
 *
 * @return
 * 0 on success, or -1 on error.
 */
int kui_tree_get_state(struct kui_tree *ktree, enum kui_tree_state *state);

/**
 * Get's the data if the state is KUI_TREE_FOUND.
 *
 * \param ktree
 * The tree to get the data from
 *
 * \param data
 * The data put in for this macro
 *
 * \return
 * 0 on success, or -1 on error.
 */
int kui_tree_get_data(struct kui_tree *ktree, void *data);

/*@}*/

/******************************************************************************/
/**
 * @name General operations on a kui tree.
 * These function's are the basic functions used to operate on a kui tree
 */
/******************************************************************************/

/*@{*/

/**
 * Advance the state of the tree by one key.
 *
 * \param ktree
 * The tree to change the state of.
 *
 * \param key
 * The data.
 *
 * \param map_found
 * returns as 1 if a map was found with this char push, otherwise 0
 *
 * @return
 * 0 on success, or -1 on error.
 */
int kui_tree_push_key(struct kui_tree *ktree, int key, int *map_found);

/*@}*/

/* }}} */

#endif
