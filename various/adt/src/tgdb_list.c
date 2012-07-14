#include "tgdb_list.h"
#include "sys_util.h"

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

struct tgdb_list *tgdb_list_init(void)
{
    struct tgdb_list *list;

    list = (struct tgdb_list *) cgdb_malloc(sizeof (struct tgdb_list));

    /* Initialize all data members */
    list->size = 0;
    list->head = NULL;
    list->tail = NULL;

    return list;
}

int tgdb_list_destroy(struct tgdb_list *list)
{
    if (!list)
        return -1;

    free(list);
    list = NULL;

    return 0;
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
static void tgdb_list_insert(struct tgdb_list *list,
        struct tgdb_list_node *before,
        struct tgdb_list_node *after, struct tgdb_list_node *new_node)
{

    /* Do nothing if node or list is NULL */
    if (!new_node || !list)
        return;

    /* Special case, insert into tail with list size 1 */
    if (before == NULL && after == NULL && list->size == 1) {
        new_node->next = NULL;
        new_node->prev = list->head;

        list->head->next = new_node;

        list->tail = new_node;

        /* Inserting first item into list */
    } else if (before == NULL && after == NULL) {
        list->head = new_node;
        new_node->next = NULL;
        new_node->prev = NULL;

        /* Insert at beggining of list */
    } else if (before == NULL) {
        new_node->next = list->head;
        new_node->prev = NULL;

        list->head->prev = new_node;

        list->head = new_node;

        /* Insert at end of list */
    } else if (after == NULL) {

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
static void tgdb_list_delete(struct tgdb_list *list,
        struct tgdb_list_node *node)
{

    /* Do nothing if node or list is NULL */
    if (!node || !list)
        return;

    /* Deleting from an empty list */
    if (tgdb_list_size(list) == 0) {

        node->next = NULL;
        node->prev = NULL;
        node = NULL;

        list->head = NULL;
        list->tail = NULL;

        /* Deleting last element in the list */
    } else if (tgdb_list_size(list) == 1) {
        /* Only the head is populated, free it */
        node->next = NULL;
        node->prev = NULL;
        node = NULL;

        list->head = NULL;
        list->tail = NULL;

        /* Deleting from beggining of list */
    } else if (node->prev == NULL) {
        node->next->prev = NULL;
        list->head = node->next;

        node->next = NULL;
        node->prev = NULL;
        node = NULL;

        /* If the list is size 2, remove the tail and set only the head */
        if (tgdb_list_size(list) == 2)
            list->tail = NULL;

        /* Delete from end of list */
    } else if (node->next == NULL) {

        node->prev->next = NULL;
        list->tail = node->prev;

        node->next = NULL;
        node->prev = NULL;
        node = NULL;

        /* Delete from middle of list */
    } else {
        node->prev->next = node->next;
        node->next->prev = node->prev;

        node->next = NULL;
        node->prev = NULL;
        node = NULL;
    }

    list->size--;
}

static struct tgdb_list_node *tgdb_list_new_node(void)
{
    struct tgdb_list_node *node;

    node = (struct tgdb_list_node *) cgdb_malloc(sizeof (struct
                    tgdb_list_node));

    node->data = (void *) NULL;
    node->next = NULL;
    node->prev = NULL;

    return node;
}

int tgdb_list_append(struct tgdb_list *tlist, void *item)
{
    struct tgdb_list_node *node;

    if (!tlist)
        return -1;

    if ((node = tgdb_list_new_node()) == NULL)
        return -1;

    node->data = item;

    tgdb_list_insert(tlist, tlist->tail, NULL, node);

    return 0;
}

int tgdb_list_prepend(struct tgdb_list *tlist, void *item)
{
    struct tgdb_list_node *node;

    if (!tlist)
        return -1;

    if ((node = tgdb_list_new_node()) == NULL)
        return -1;

    node->data = item;

    tgdb_list_insert(tlist, NULL, tlist->head, node);

    return 0;
}

int tgdb_list_insert_after(struct tgdb_list *tlist,
        tgdb_list_iterator * i, void *item)
{
    struct tgdb_list_node *node;

    if (!tlist)
        return -1;

    if ((node = tgdb_list_new_node()) == NULL)
        return -1;

    node->data = item;

    tgdb_list_insert(tlist, i, i->next, node);

    return 0;
}

int tgdb_list_insert_before(struct tgdb_list *tlist,
        tgdb_list_iterator * i, void *item)
{

    struct tgdb_list_node *node;

    if (!tlist)
        return -1;

    if ((node = tgdb_list_new_node()) == NULL)
        return -1;

    node->data = item;

    tgdb_list_insert(tlist, i->prev, i, node);

    return 0;
}

int tgdb_list_foreach(struct tgdb_list *tlist, tgdb_list_func func)
{
    tgdb_list_iterator *i = tgdb_list_get_first(tlist);

    while (i) {
        if (func(i->data) == -1)
            return -1;
        i = tgdb_list_next(i);
    }

    return 0;
}

int tgdb_list_free(struct tgdb_list *tlist, tgdb_list_func func)
{
    tgdb_list_iterator *i = tgdb_list_get_first(tlist);

    while (i) {
        if (func(i->data) == -1)
            return -1;
        tgdb_list_delete(tlist, i);
        /* Can't call tgdb_list_next () here on the iterator.
         * This is because the iterator was just deleted.
         */
        i = tgdb_list_get_first(tlist);
    }

    return 0;
}

int tgdb_list_clear(struct tgdb_list *tlist)
{
    tgdb_list_iterator *i = tgdb_list_get_first(tlist);

    while (i) {
        tgdb_list_delete(tlist, i);
        /* Can't call tgdb_list_next () here on the iterator.
         * This is because the iterator was just deleted.
         */
        i = tgdb_list_get_first(tlist);
    }

    return 0;
}

int tgdb_list_size(struct tgdb_list *tlist)
{
    if (tlist)
        return tlist->size;

    return 0;
}

tgdb_list_iterator *tgdb_list_get_first(struct tgdb_list * tlist)
{
    if (tlist) {
        return tlist->head;
    }

    return NULL;
}

tgdb_list_iterator *tgdb_list_get_last(struct tgdb_list * tlist)
{
    if (tlist) {
        if (tlist->size == 1)
            return tlist->head;
        else
            return tlist->tail;
    }

    return NULL;
}

tgdb_list_iterator *tgdb_list_next(tgdb_list_iterator * i)
{
    if (i) {
        return i->next;
    }

    return NULL;
}

tgdb_list_iterator *tgdb_list_previous(tgdb_list_iterator * i)
{
    if (i) {
        return i->prev;
    }

    return NULL;
}

void *tgdb_list_get_item(tgdb_list_iterator * i)
{
    if (i)
        return i->data;

    return (void *) NULL;
}
