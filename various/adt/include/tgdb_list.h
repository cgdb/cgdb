#ifndef __TGDB_LIST_H__
#define __TGDB_LIST_H__

struct tgdb_list;
struct tgdb_list_node;
typedef struct tgdb_list_node tgdb_list_iterator;

typedef int (*tgdb_list_func) (void *item);

/* 
 * Initializes a new empty list.
 * 
 * Returns
 *    The new list, or NULL on error.
 */
struct tgdb_list *tgdb_list_init(void);

/*
 * Destory a list
 *
 * \param list
 * The list to destroy
 *
 * \return
 * 0 on success or -1 on error.
 */
int tgdb_list_destroy(struct tgdb_list *list);

/* 
 * Appends item to the end of the list.
 * 
 * tlist
 *    The list to append an item to.
 *
 * item
 *    The item to add to the list
 *
 * returns
 *    0 on success, or -1 on error
 */
int tgdb_list_append(struct tgdb_list *tlist, void *item);

/* 
 * Prepends item to the beggining of the list.
 * 
 * tlist
 *    The list to prepend an item to.
 *
 * item
 *    The item to add to the list
 *
 * returns
 *    0 on success, or -1 on error
 */
int tgdb_list_prepend(struct tgdb_list *tlist, void *item);

/* 
 * appends the item after the position of the iterator.
 *
 * tlist
 *    The list to add an item to
 * 
 * i
 *    The iterator to insert after.
 *
 * item
 *    The item to add to the list
 *
 * returns
 *    0 on success, or -1 on error
 */
int tgdb_list_insert_after(struct tgdb_list *tlist,
        tgdb_list_iterator * i, void *item);

/* 
 * prepands the item before the position of the iterator.
 * 
 * tlist
 *    The list to add an item to
 * 
 * i
 *    The iterator to insert before.
 *
 * item
 *    The item to add to the list
 *
 * returns
 *    0 on success, or -1 on error
 */
int tgdb_list_insert_before(struct tgdb_list *tlist,
        tgdb_list_iterator * i, void *item);

/* Traversing the tree */

/* 
 * Traverses each item in the list, passing each element to func
 *
 * tlist
 *    The list to traverse
 *
 * func
 *    The function to call on each item
 *
 * returns
 *    -1 on error, 0 on success. If func returns -1, then the list will stop
 *    traversing and return -1.
 */
int tgdb_list_foreach(struct tgdb_list *tlist, tgdb_list_func func);

/* Freeing the tree */

/* 
 * Free's list item by calling func on each element
 *
 * After this function is called, the list is empty.
 *
 * tlist
 *    The list to free
 *
 * func
 *    The function to free an item
 *
 * returns
 *    -1 on error, 0 on success. If func returns -1, then the list will stop
 *    traversing and return -1.
 */
int tgdb_list_free(struct tgdb_list *tlist, tgdb_list_func func);

/*
 * This will clear a list.
 *
 * tlist
 * 	  The list to free
 *
 * returns
 *    -1 on error, 0 on success.
 */
int tgdb_list_clear(struct tgdb_list *tlist);

/* 
 * Gets the size of the list.
 *
 * tlist
 *    The list context to get the size of.
 *
 * Returns
 * 	  The size of the list, 0 if empty, -1 on error.
 */
int tgdb_list_size(struct tgdb_list *tlist);

/* Moving through the list */

/*
 * Gets a hold of the first iterator in the list
 *
 * tlist
 *    The list to get the beggining of
 *
 * i
 *    The iterator to assign at the first position of the list
 *
 * Returns
 * 	  the iterator on success, or NULL on error
 */
tgdb_list_iterator *tgdb_list_get_first(struct tgdb_list *tlist);

/*
 * Gets a hold of the last iterator in the list
 *
 * tlist
 *    The list to get the last iterator.
 *
 * i
 *    The iterator to assign at the last position of the list
 *    
 * Returns
 * 	  the iterator on success, or NULL on error
 */
tgdb_list_iterator *tgdb_list_get_last(struct tgdb_list *tlist);

/* 
 * Moves the iterator one step forward through the list
 *
 * i
 *    The iterator to advance.
 *    
 * Returns
 * 	  the iterator on success, or NULL on error
 */
tgdb_list_iterator *tgdb_list_next(tgdb_list_iterator * i);

/* 
 * Moves the iterator one step backwards through the list
 *
 * i
 *    The iterator to move backwards.
 *
 * Returns
 * 	  the iterator on success, or NULL on error
 */
tgdb_list_iterator *tgdb_list_previous(tgdb_list_iterator * i);

/*
 * Gets the data at the iterator's position.
 *
 * i
 *    The iterator to get the item at.
 *
 * return
 *    The item at the iterator, or NULL on error
 */
void *tgdb_list_get_item(tgdb_list_iterator * i);

#endif /* __TGDB_LIST_H__ */
