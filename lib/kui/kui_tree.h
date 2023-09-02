#ifndef __KUI_TREE_H__
#define __KUI_TREE_H__

#include <memory>

/* Doxygen headers {{{ */
/*!
 * \file
 * kui_tree.h
 *
 * \brief
 * This interface is the algorithm behind libkui's fast macro finding
 * abilities.  It is a statefull data structure, which can be told to start
 * matching, to receive input, and then to stop matching. When it is told to
 * stop, it knows exactly which macro's have been completed, and how much data
 * is extra.
 */
/* }}} */

class kui_map;
class kui_tree_node;
using node_ptr_type = std::shared_ptr<kui_tree_node>;

/* struct kui_tree {{{ */

/**
 * This data structure is capable of storing a set of maps internally in such a
 * way that it is easy to see what maps are active if a char at a time is fed
 * to this structure.
 *
 * Also, it can determine what mapping was reached if one was found.
 */
class kui_tree
{
public:

    /**
     * Create a kui tree. This data structure is designed specifically for the
     * Key User Interface. It should be optimized for speed, since determining
     * what key the user wants to be processed next should be fast.
     *
     * @return
     * A new instance on success, or throw std::bad_alloc on error.
     */
    kui_tree();

    /**
     * Destroy a kui tree.
     */
    ~kui_tree() = default;

    /*************************************************************************/
    /**
     * @name Inserting and Deleteing from a kui_tree
     * These are the basic functions of adding to and removing from a kui_tree
     */
    /*************************************************************************/

    /*@{*/

    /**
     * Adding a key to the kui tree.
     *
     * \param klist
     * The data to add into the tree.
     * It is a null terminated list.
     *
     * \param data
     * If this is the "value" part of the map being inserted.
     */
    void insert(const int *klist, kui_map *data);

    /**
     * Removing a key from the kui tree.
     *
     * \param klist
     * The key to remove from the tree
     * It is a null terminated list.
     */
    void erase(const int *klist);

    /*@}*/

    /*************************************************************************/
    /**
     * @name Setting and Querying the state of a kui_tree
     * A kui_tree is a stateful ADT. This interface documents how to set/get
     * the state of a particular kui tree.
     */
    /*************************************************************************/

    /*@{*/

    enum class kui_tree_state
    {
        FOUND = 0,
        MATCHING,
        NOT_FOUND,
        ERROR
    };

    /**
     * Get's the state of the current tree.
     *
     * \return
     * The state of the tree
     */
    kui_tree_state get_state() const;

    /**
     * To start searching for a macro, reset the state of the tree.
     */
    void reset_state();

    /**
     * To finish searching for a macro, finalize the state of the tree.
     * This allows the tree to do any book keeping it must do, and allows it
     * to find out exaclty which macro should was called, and how many extra
     * key strokes have been received.
     */
    void finalize_state();

    /**
     * Get's the data if the state is FOUND.
     *
     * \return
     * The data put in this macro on success, or nullptr on error.
     */
    kui_map *get_data() const;

    /*@}*/

    /*************************************************************************/
    /**
     * @name General operations on a kui tree.
     * These function's are the basic functions used to operate on a kui tree
     */
    /*************************************************************************/

    /*@{*/

    /**
     * Advance the state of the tree by one key.
     *
     * \param key
     * The data.
     *
     * \param map_found
     * returns as 1 if a map was found with this char push, otherwise 0
     *
     * @return
     * true on success, or false 1 on error.
     */
    bool push_key(int key, int *map_found);

    /*@}*/

private:
    void insert(node_ptr_type node, const int *klist, kui_map *data);
    void erase (node_ptr_type node, const int *klist);

    /* The root of the tree */
    node_ptr_type root;

    /* The current position pointing into the tree (while looking for a map) */
    node_ptr_type cur;

    /* The last node found while looking for a map. */
    /* This happens because maps can be subsets of other maps. */
    node_ptr_type found_node;

    /* The internal state of the tree (still looking, map found, not found) */
    kui_tree_state state;

    /* If a map was found, this is set to 1 while looking, otherwise 0. */
    int found;
};

#endif
