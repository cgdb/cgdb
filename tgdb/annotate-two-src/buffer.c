#include "buffer.h"
#include "types.h"
#include "error.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>

void buffer_free_command( void *item ) {
   struct command *com;
   if ( item == NULL ) 
      return;
   
   com = (struct command *) item;
   free ( com->data );
   com->data = NULL;
   free ( com );
   com = NULL;
}

struct node *buffer_append(struct node *head, struct node *new_node) {
   struct node *prev = NULL;
   struct node *cur = head;

   if ( cur == NULL ) {
      cur = new_node;
      cur->next = NULL;
      return cur;
   }

   while ( cur != NULL ) {
      prev = cur;
      cur = cur -> next;
   }

   cur = new_node;
   prev->next = cur;

   return head;
}

struct node *buffer_remove_and_increment(struct node *head, buffer_free_func func) {
   struct node *prev = NULL;
   struct node *cur = head;
   
   if ( cur == NULL ) 
      return NULL;

   if ( cur -> next != NULL ) {
      prev = cur;
      cur = cur -> next;
      func ( prev->data );
      free ( prev );
      prev = NULL;
      return cur;
   } else {
      free ( cur );
      cur = NULL;
      return NULL;
   }
}

void buffer_free_list(struct node *item, buffer_free_func func) {
   struct node *prev, *cur = item;
   
   if ( cur == NULL )
      return;

   while ( cur != NULL ) {
      prev = cur;
      cur = cur->next;
      func ( prev->data );
      free ( prev );
      prev = NULL; 
   }

   func ( cur->data );
   free ( cur );
   prev = NULL; 
}

struct node *buffer_write_command_and_append ( struct node *n, struct command *com ) {
   struct node *new_node = ( struct node * ) xmalloc ( sizeof ( struct node ) );

   if ( new_node == NULL )
      return NULL;;

   new_node->data = ( void * ) com;
   new_node->next = NULL;

   return buffer_append ( n, new_node );
}

static char user_command[MAXLINE];
static int user_command_length = 0;

int buffer_is_empty(void) {
   if ( user_command_length == 0 )
      return TRUE;
   else
      return FALSE;
}

struct node *buffer_write_line ( struct node *com, const char *c ) {

  struct node *temp = NULL;
  struct command *ncom = ( struct command *) xmalloc (sizeof(struct command));
  int length = strlen(c);
  ncom->data = xmalloc ( sizeof( char) * ( length + 1) );
  strncpy ( ncom->data, c, length + 1);
  ncom->com_type = BUFFER_USER_COMMAND;
  ncom->out_type = COMMANDS_SHOW_USER_OUTPUT;
  ncom->com_to_run  = COMMANDS_VOID;
  temp = buffer_write_command_and_append( com,  ncom);

  return temp;
}

void buffer_clear_string ( void ) {
   memset ( user_command, '\0', MAXLINE );
   user_command_length = 0;
}

void buffer_print ( struct node *head ) {
   struct node *cur = head;
   struct command *com;

   while ( cur != NULL ) {
      com = (struct command *) cur->data;
      err_msg("DATA(%s), BUF_COM_TYPE(%d), BUF_OUTPUT_TYPE(%d), COM_TO_RUN(%d)\n", 
               com->data, com->com_type, com->out_type, com->com_to_run);
      cur = cur->next;
   }

   err_msg("NULL\n");
}
