#include "queue.h"
#include "types.h"
#include "error.h"

struct node *queue_append(struct node *head, void *item) {
    struct node *cur = head, *node;

    node = (struct node *)xmalloc(sizeof(struct node));
    node->data = item;
    node->next = NULL;

    if ( cur == NULL )
        return node;

    while ( cur -> next != NULL )
        cur = cur -> next;
    
    cur->next = node;
    return head;
}

struct node *queue_pop(struct node *head, void **item) {
    struct node *cur = head;

    if ( head == NULL ) 
        return NULL;

    head = head->next;
    
    /* Assertion: cur is the leftover node that needs to be freed */
    *item = cur->data;
    free(cur);
    cur = NULL;

    return head;
}

void queue_free_list(struct node *item, item_free_func func) {
   struct node *prev, *cur = item;
   
   if ( cur == NULL )
      return;

   while ( cur != NULL ) {
      prev = cur;
      cur = cur->next;
      if ( func )
	 func ( prev->data );
      free ( prev );
      prev = NULL; 
   }

   if ( func )
     func ( cur->data );
   free ( cur );
   prev = NULL; 
}

int queue_size(struct node *head) {
   struct node *cur = head;
   int size = 0;

   /* This list is empty */
   if ( cur == NULL )
       return 0;

   while ( cur != NULL ) {
      cur = cur -> next;
      size += 1; 
   }

   return size;
}
