#include "tgdb_list.h"
#include "sys_util.h"
#include "error.h"

struct tgdb_list_node {
	struct tgdb_list_node *next;
	struct tgdb_list_node *prev;
	void *data;
};

struct tgdb_list {
	int size;
	struct tgdb_list_node *head;
	struct tgdb_list_node *tail;
};

struct tgdb_list_iterator {
	struct tgdb_list_node *node;
};

struct tgdb_list_iterator *tgdb_list_iterator_init ( void ) {
	struct tgdb_list_iterator *i = (struct tgdb_list_iterator *)
		xmalloc ( sizeof ( struct tgdb_list_iterator ) );

	i->node = NULL;

	return i;
}

void tgdb_list_iterator_free ( struct tgdb_list_iterator *i ) {
	i->node = NULL;
	free ( i );
	i = NULL;
}

struct tgdb_list *tgdb_list_init ( void ) {
	struct tgdb_list *list;

	list = (struct tgdb_list *)xmalloc ( sizeof ( struct tgdb_list ) );

	/* Initialize all data members */
	list->size = 0;
	list->head = NULL;
	list->tail = NULL;

	return list;
}

/*
 * Can insert any node between 2 other nodes.
 *
 * list
 *    This is the current list we are inserting into.
 *
 * before
 *    The node we want to insert after.
 *
 * after
 *    The node to insert before.
 *
 * new_node
 *    The node to insert into the list.
 */
static void tgdb_list_insert ( 
	struct tgdb_list *list,
	struct tgdb_list_node *before,
	struct tgdb_list_node *after,
	struct tgdb_list_node *new_node ) {

	/* Do nothing if node or list is NULL */
	if ( !new_node || !list)
		return;

	/* Inserting first item into list */
	if ( before == NULL && after == NULL ) {
		list->head = new_node;
		new_node->next = NULL;
		new_node->prev = NULL;

	/* Insert at beggining of list */
	} else if ( before == NULL ) {
		new_node->next = list->head;
		new_node->prev = NULL;

		list->head->prev = new_node;

		list->head = new_node;

	/* Special case, insert into tail with list size 1 */
	} else if ( after == NULL && list->size == 1 ) {
		new_node->next = NULL;
		new_node->prev = list->head;

		list->head->next = new_node;

		list->tail = new_node;

	/* Insert at end of list */
	} else if ( after == NULL ) {

		new_node->next = NULL;
		new_node->prev = list->tail;

		list->tail->next = new_node;

		list->tail = new_node;

	/* Insert into middle of list */
	} else {
		new_node->next = before->next;
		new_node->prev = after->prev;

		before->next = new_node;
		before->prev = new_node;
	}

	list->size++;
}

/*
 * Can delete any node out of the tree
 *
 * list
 *    This is the current list we are deleting from
 *
 * node
 *    The node to delete
 */
static void tgdb_list_delete (
		struct tgdb_list *list,
		struct tgdb_list_node *node ) {

	/* Do nothing if node or list is NULL */
	if ( !node || !list)
		return;
	
	/* Deleting from an empty list */
	if ( tgdb_list_size ( list ) == 0) {

		free ( node->data );
		node->next = NULL;
		node->prev = NULL;
		node       = NULL;

		list->head = NULL;
		list->tail = NULL;

	/* Deleting last element in the list */
	} else if ( tgdb_list_size ( list ) == 1 ) {
		/* Only the head is populated, free it */
		free ( node->data );
		node->next = NULL;
		node->prev = NULL;
		node       = NULL;

		list->head = NULL;
		list->tail = NULL;

	/* Deleting from beggining of list */
	} else if ( node->prev == NULL ) {
		node->next->prev = NULL;
		list->head = node->next;

		free ( node->data );
		node->next = NULL;
		node->prev = NULL;
		node       = NULL;

		/* If the list is size 2, remove the tail and set only the head */
		if ( tgdb_list_size ( list ) == 2 )
			list->tail = NULL;

	/* Delete from end of list */
	} else if ( node->next == NULL ) {

		node->prev->next = NULL;
		list->tail = node->prev;

		free ( node->data );
		node->next = NULL;
		node->prev = NULL;
		node       = NULL;

	/* Delete from middle of list */
	} else {
		node->prev->next = node->next;
		node->next->prev = node->prev;

		free ( node->data );
		node->next = NULL;
		node->prev = NULL;
		node       = NULL;
	}

	list->size--;
}

static struct tgdb_list_node* tgdb_list_new_node ( void ) {
	struct tgdb_list_node *node;
	node = (struct tgdb_list_node *)xmalloc ( sizeof ( struct tgdb_list_node ));

	node->data = (void*) NULL;
	node->next = NULL;
	node->prev = NULL;

	return node;
}

void tgdb_list_append ( struct tgdb_list *tlist, void *item ) {
	struct tgdb_list_node *node = tgdb_list_new_node ();

	node->data = item;

	tgdb_list_insert ( tlist, tlist->tail, NULL, node );	
}

void tgdb_list_prepend ( struct tgdb_list *tlist, void *item ) {
	struct tgdb_list_node *node = tgdb_list_new_node ();

	node->data = item;

 	tgdb_list_insert ( tlist, NULL, tlist->head, node );	
}

void tgdb_list_insert_after ( 
		struct tgdb_list *tlist, 
		struct tgdb_list_iterator *i, 
		void *item ) {
	
	struct tgdb_list_node *node = tgdb_list_new_node ();

	node->data = item;

	tgdb_list_insert ( tlist, i->node, i->node->next, node );
}

void tgdb_list_insert_before ( 
		struct tgdb_list *tlist,
		struct tgdb_list_iterator *tlisti, 
		void *item ) {

	struct tgdb_list_node *node = tgdb_list_new_node ();

	node->data = item;

	tgdb_list_insert ( tlist, tlisti->node->prev, tlisti->node, node );
}

void tgdb_list_foreach ( struct tgdb_list *tlist, tgdb_list_func func ) {
	struct tgdb_list_iterator *i = tgdb_list_iterator_init ();
	int val;

    val = tgdb_list_get_first ( tlist, i );

	while ( val != 0 ) {
		func ( i->node->data );
		val = tgdb_list_next ( i );
	}

	tgdb_list_iterator_free ( i );
	i = NULL;
}



void tgdb_list_free ( struct tgdb_list *tlist, tgdb_list_func func ) {
	struct tgdb_list_iterator *i = tgdb_list_iterator_init ();
	int val;

	val = tgdb_list_get_first ( tlist, i );

	while ( val != 0 ) {
		func ( i->node->data );
		tgdb_list_delete ( tlist, i->node->data );
		val = tgdb_list_next ( i );
	}

	tgdb_list_iterator_free ( i );
	i = NULL;
}

int tgdb_list_size ( struct tgdb_list *tlist ) {
	if ( tlist ) 
		return tlist->size;

	return 0;
}

int tgdb_list_get_first ( struct tgdb_list *tlist, struct tgdb_list_iterator *i ) {
	if ( tlist ) {
		i->node=tlist->head;
		return 1;
	}

	return 0;
}

int tgdb_list_get_last ( struct tgdb_list *tlist, struct tgdb_list_iterator *i ) {
	if ( tlist ) {
		if ( tlist->size == 1 )
			i->node = tlist->head;
		else
			i->node = tlist->tail;

		return 1;
	}

	return 0;
}

int tgdb_list_next ( struct tgdb_list_iterator *i ) {
	if ( i ) {
		i->node = i->node->next;
		return 1;
	}

	return 0;
}

int tgdb_list_previous ( struct tgdb_list_iterator *i ) {
	if ( i ) {
		i->node = i->node->prev;
		return 1;
	}

	return 0;
}

void *tgdb_list_get_item ( struct tgdb_list_iterator *i ) {
	if ( i )
		return i->node->data;

	return (void*)NULL;
}
