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

char *buffer_get_incomplete_command( char *buf ) {
   if ( user_command_length == 0 )
      return NULL;
    
    strncpy ( buf, user_command, user_command_length + 1);
    
    memset(user_command, '\0', user_command_length);
    user_command_length = 0;

    return buf;
}
   
struct node *buffer_write_char ( struct node *com, const char c ) {

   if ( c == '\n' ) {
      struct node *temp = NULL;
      if ( user_command_length < MAXLINE - 1 ) {
         struct command *ncom = ( struct command *) xmalloc (sizeof(struct command));
         user_command[user_command_length++] = c;
         user_command[user_command_length++] = '\0';

         ncom->data = xmalloc ( sizeof( char) * ( user_command_length) );
         strncpy ( ncom->data, user_command, user_command_length);
         ncom->com_type = BUFFER_USER_COMMAND;
         ncom->out_type = COMMANDS_SHOW_USER_OUTPUT;
         ncom->com_to_run  = COMMANDS_VOID;
         
         temp = buffer_write_command_and_append( com,  ncom);
         memset( user_command, '\0', user_command_length ); 
         user_command_length = 0;  
      }
      return temp;
   } else if ( c == '\b' ) {
      if ( user_command_length > 0 )
         user_command[--user_command_length] = '\0';
      return com;
   } else {
      if ( user_command_length < MAXLINE ) {
         user_command[user_command_length++] = c;
         user_command[user_command_length] = '\0';
      }

      return com;
   }
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
