#include <stdlib.h>
#include <string.h>
#include "kui_tree.h"

/* Internal Documentation {{{*/
/*
 * This documentation is intended to be a brief description behind how kui_tree
 * works internally.
 *
 */

/* }}}*/

/* struct kui_tree_node {{{ */

/**
 * This is a node in the ku_tree.
 * It represents a single key. Many nodes could make up a single map.
 */
struct kui_tree_node {
	/**
	 * The key this node represents.
	 */
	int key;

	/**
	 * If non-null, this node represents a macro and the data is 
	 * the value of the macro.
	 *
	 * Otherwise, this node is not the end of a macro.
	 */
	void *macro_value;

	/**
	 * A doubly linked list of all the children.
	 * The list is sorted by the key so that a faster lookup can happen.
	 *
	 * If this is a bottleneck, it can be changed into a hash, or something
	 * more efficient.
	 */
	std_list children;
};

/**
 * Destroys a kui tree node.
 *
 * \param ktnode
 * The tree node to destroy
 *
 * @return
 * 0 on success, or -1 on error.
 */
int kui_tree_node_destroy ( struct kui_tree_node *ktnode ) {
	int retval = 0;

	if ( !ktnode )
		return -1;

	if ( ktnode->children ) {
		if ( std_list_destroy ( ktnode->children ) == -1 )
			retval = -1;
		ktnode->children = NULL;
	}

	free ( ktnode );
	ktnode = NULL;

	return retval;
}

static int destroy_kui_tree_node_callback ( void *data ) {
	struct kui_tree_node *ktnode = (struct kui_tree_node *)data;

	if ( !ktnode )
		return -1;

	return kui_tree_node_destroy ( ktnode );
}

/**
 * Creates a new kui tree node.
 *
 * @return
 * A new instance on success, or NULL on error. 
 */
struct kui_tree_node *kui_tree_node_create ( void ) {
	struct kui_tree_node *ktnode;

	ktnode = (struct kui_tree_node*) malloc ( sizeof ( struct kui_tree_node ) );

	if (!ktnode )
		return NULL;

	ktnode->key = 0;
	ktnode->macro_value = NULL;

	/* Allocate children */
	ktnode->children = std_list_create (destroy_kui_tree_node_callback);

	if ( ! ktnode->children ) {
		kui_tree_node_destroy ( ktnode );
		return NULL;
	}

	return ktnode;
}

/**
 * Finds a pointer to the child node.
 *
 * \param key
 * The key to look for.
 *
 * \param children
 * The children to look in
 *
 * \param found 
 * If the key was found, this will return as true, otherwise false.
 *
 * \param ktnode
 * If found, this will be the node that contains the item.
 *
 * \return
 * 0 on success, or -1 on error.
 */
static int kui_tree_find ( 
		int key,
		std_list children,
		int *found,
		struct kui_tree_node **ktnode ) {
	struct kui_tree_node *node;
	std_list_iterator iter;
	*found = 0;
	*ktnode = NULL;

	/* Look for the key in the children */
	for ( iter = std_list_begin ( children );
		  iter != std_list_end ( children );
		  iter = std_list_next ( iter ) ) {
		void *data;
		if ( std_list_get_data ( iter, &data ) == -1 )
			return -1;

		node = ( struct kui_tree_node *)data;

		if ( node->key == key ) {
			*found = 1;
			*ktnode = node;
			break;
		} else if ( node->key > key )
			break;
	}

	return 0;
}

static int kui_tree_node_compare_callback ( 
		const void *a,
		const void *b ) {
	struct kui_tree_node *one = (struct kui_tree_node *)a;
	struct kui_tree_node *two = (struct kui_tree_node *)b;

	if ( one->key == two->key )
		return 0;
	else if ( one->key < two->key )
		return -1;
	else
		return 1;
}

static int kui_tree_node_key_compare_callback ( 
		const void *a,
		const void *b ) {
	struct kui_tree_node *one = (struct kui_tree_node *)a;
	int two = *(int*)b;

	if ( one->key == two )
		return 0;
	else if ( one->key < two )
		return -1;
	else
		return 1;
}

static int kui_tree_node_insert ( 
		struct kui_tree_node *ktnode, 
		int *klist, 
		void *data ) {
	struct kui_tree_node *node;
	int found;

	if ( !ktnode )
		return -1;

	if ( klist[0] == 0 ) {
		ktnode->macro_value = data;
		return 0;
	}

	if ( kui_tree_find ( klist[0], ktnode->children, &found, &node ) == -1 )
		return -1;

	/* If the node was found, use it, otherwise create a new one */
	if ( !found ) {
		node = kui_tree_node_create ();

		if ( !node )
			return -1;

		node->key = klist[0];

		if ( std_list_insert_sorted ( ktnode->children, node, kui_tree_node_compare_callback ) == -1 )
			return -1;
	}

	return kui_tree_node_insert ( node, &klist[1], data );
}

static int kui_tree_node_delete ( 
		struct kui_tree_node *ktnode, 
		struct kui_tree_node **from,
		int *klist ) {
	
	struct kui_tree_node *node;
	int found;

	if ( !ktnode )
		return -1;

	/* base case */
	if ( klist[0] == 0 ) 
		return 0;

	if ( kui_tree_find ( klist[0], ktnode->children, &found, &node ) == -1 )
		return -1;

	if ( !found ) /* The key must be found */
		return -1;


	if ( kui_tree_node_delete ( node, from, &klist[1] ) == -1 )
		return -1;

	/* No need to mess with the bottom node on the way out */
	if ( klist[1] == 0 ) {
		*from = node;
		return 0;
	}

	/* If "from" has no children, remove it from the current node's children
	 * and then delete it. 
	 * If it has children, leave it alone.
	 */
	if ( std_list_length ( (*from)->children ) == 0 ) {
		std_list_iterator iter;
		iter = std_list_find ( node->children, &(*from)->key, kui_tree_node_key_compare_callback );

		if ( !iter )
			return -1;

		/* From is freed by std_list_remove callback */
		if ( std_list_remove ( node->children, iter ) == NULL )
			return -1;
	}

	*from = node;

	return 0;
}

/* }}} */

/* struct kui_tree {{{ */

/**
 * This data structure is capable of storing a set of maps internally in such a
 * way that it is easy to see what maps are active if a char at a time is fed
 * to this structure.
 *
 * Also, it can determine what mapping was reached if one was found.
 */
struct kui_tree {
	/* The root of the tree*/
	struct kui_tree_node *root;
	/* The current position pointing into the tree ( while looking for a map )*/
	struct kui_tree_node *cur;
	/* The last node found while looking for a map.*/
	/* This happens because maps can be subsets of other maps.*/
	struct kui_tree_node *found_node;
	/* The internal state of the tree ( still looking, map found, not found )*/
	enum kui_tree_state state;
	/* If a map was found at all, this is set to 1 while looking, otherwise 0.*/
	int found;
};

int kui_tree_destroy ( struct kui_tree *ktree ) {
	int retval = 0;

	if ( !ktree )
		return -1;

	if ( kui_tree_node_destroy ( ktree->root ) == -1 )
		retval = -1;

	free ( ktree );
	ktree = NULL;

	return retval;
}

struct kui_tree *kui_tree_create ( void ) {
	struct kui_tree *ktree;

	ktree = (struct kui_tree *)malloc ( sizeof ( struct kui_tree ) );

	if ( !ktree )
		return NULL;

	ktree->root = kui_tree_node_create ();

	if ( !ktree->root ) {
		kui_tree_destroy ( ktree );
		return NULL;
	}	

	return ktree;
}

int kui_tree_insert ( struct kui_tree *ktree, int *klist, void *data ) {
	return kui_tree_node_insert ( ktree->root, klist, data );
}

int kui_tree_delete ( struct kui_tree *ktree, int *klist ) {
	struct kui_tree_node *from;
	if ( kui_tree_node_delete ( ktree->root, &from, klist ) == -1 )
		return -1;

	if ( std_list_length ( from->children ) == 0 ) {
		std_list_iterator iter;
		iter = std_list_find ( ktree->root->children, &from->key, kui_tree_node_key_compare_callback );

		if ( !iter )
			return -1;

		/* From is freed by std_list_remove callback */
		if ( std_list_remove ( ktree->root->children, iter ) == NULL )
			return -1;
	}

	return 0;
}

int kui_tree_reset_state ( struct kui_tree *ktree ) {
	if ( !ktree )
		return -1;

	ktree->cur = ktree->root;
	ktree->state = KUI_TREE_MATCHING;
	ktree->found = 0;
	ktree->found_node = NULL;

	return 0;
}

int kui_tree_finalize_state ( struct kui_tree *ktree ) {
	if ( !ktree )
		return -1;

	if ( ktree->found )
		ktree->state = KUI_TREE_FOUND;

	return 0;
}

int kui_tree_get_state ( struct kui_tree *ktree, enum kui_tree_state *state ) {
	if ( !ktree )
		return -1;

	*state = ktree->state;

	return 0;
}

int kui_tree_get_data ( struct kui_tree *ktree, void *data ) {
	if ( !ktree )
		return -1;

	if ( !ktree->found )
		return -1;

    memcpy(data, &ktree->found_node->macro_value, sizeof(void *));

	return 0;
}

int kui_tree_push_key ( 
		struct kui_tree *ktree, 
		int key,
	    int *map_found ) {
	int found;
	struct kui_tree_node *ktnode;

	*map_found = 0;

	if ( ktree->state != KUI_TREE_MATCHING )
		return -1;

	/* Check to see if this key matches */
	if ( kui_tree_find ( key, ktree->cur->children, &found, &ktnode ) == -1 )
		return -1;

	/* Not found */
	if ( !found ) {
		ktree->state = KUI_TREE_NOT_FOUND;
		ktree->cur = NULL;
		return 0;
	}

	if ( found ) {
		ktree->cur = ktnode;

		if ( std_list_length ( ktnode->children ) == 0 ) 
			ktree->state = KUI_TREE_FOUND;


		if ( ktnode->macro_value ) {
			ktree->found = 1;
			ktree->found_node = ktnode;
			*map_found = 1;
		}
	}

	return 0;
}

/* }}} */
