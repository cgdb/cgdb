#include "queue.h"
#include "sys_util.h"

struct node {
    void *data;
    struct node *next;
};

struct queue {
    int size;
    struct node *head;
};

struct queue *queue_init(void)
{
    struct queue *q = (struct queue *) cgdb_malloc(sizeof (struct queue));

    q->size = 0;
    q->head = NULL;
    return q;
}

void queue_append(struct queue *q, void *item)
{
    struct node *cur = q->head, *node;

    /* Allocate the new node */
    node = (struct node *) cgdb_malloc(sizeof (struct node));
    node->data = item;
    node->next = NULL;

    if (q->head == NULL)
        q->head = node;
    else {
        while (cur->next != NULL)
            cur = cur->next;

        cur->next = node;
    }
    q->size = q->size + 1;
}

void *queue_pop(struct queue *q)
{
    struct node *cur;
    void *d;

    if (!q)
        return (void *) NULL;

    cur = q->head;
    if (cur == NULL)
        return (void *) NULL;

    /* Move the head of the queue forward */
    q->head = q->head->next;

    /* Assertion: cur is the leftover node that needs to be freed */
    d = cur->data;
    cur->next = NULL;
    free(cur);
    cur = NULL;

    q->size = q->size - 1;

    return d;
}

void queue_free_list(struct queue *q, item_func func)
{
    struct node *prev, *cur = q->head;

    if (cur == NULL || (!func))
        return;

    while (cur != NULL) {
        prev = cur;
        cur = cur->next;
        /* Remove the previous node */
        func(prev->data);
        free(prev);
        prev = NULL;
    }

    q->size = 0;
    q->head = NULL;
}

void queue_traverse_list(struct queue *q, item_func func)
{
    struct node *cur = q->head;

    if (!func)
        return;

    while (cur != NULL) {
        func(cur->data);
        cur = cur->next;
    }
}

int queue_size(struct queue *q)
{
    /* This list is empty */
    if (q->head == NULL)
        return 0;
    else
        return q->size;
}
