/* From configure */
#include "config.h"

/* Library includes */
#include <string.h>
#include <stdlib.h>

/* Local includes */
#include "types.h"
#include "error.h"
#include "util.h"

static const int COMSIZE = 16;
static size_t command_size = 0;
static size_t command_cur_size = 0;

void tgdb_traverse_command(FILE *fd, struct Command ***com){
   size_t j;

   for(j = 0; com != NULL && (*com) != NULL && (*com)[j] != NULL; ++j){
      if((*com)[j]->header == BREAKPOINTS_BEGIN)
         fprintf(fd, "TGDB_BREAKPOINTS_BEGIN(%s)\n", (*com)[j]->data);
      else if((*com)[j]->header == BREAKPOINT)
         fprintf(fd, "TGDB_BREAKPOINT(%s)\n", (*com)[j]->data);
      else if((*com)[j]->header == BREAKPOINTS_END)
         fprintf(fd, "TGDB_BREAKPOINT_END(%s)\n", (*com)[j]->data);
      else if((*com)[j]->header == SOURCE_FILE_UPDATE)
         fprintf(fd, "TGDB_SOURCE_FILE_UPDATE(%s)\n", (*com)[j]->data);
      else if((*com)[j]->header == LINE_NUMBER_UPDATE)
         fprintf(fd, "TGDB_LINE_NUMBER_UPDATE(%s)\n", (*com)[j]->data);
      else if((*com)[j]->header == SOURCES_START)
         fprintf(fd, "TGDB_SOURCES_START(%s)\n", (*com)[j]->data);
      else if((*com)[j]->header == SOURCE_FILE)
         fprintf(fd, "TGDB_SOURCE_FILE(%s)\n", (*com)[j]->data);
      else if((*com)[j]->header == SOURCES_END)
         fprintf(fd, "TGDB_SOURCES_END(%s)\n", (*com)[j]->data);
      else if((*com)[j]->header == SOURCES_DENIED)
         fprintf(fd, "TGDB_SOURCES_DENIED(%s)\n", (*com)[j]->data);
      else if((*com)[j]->header == ABSOLUTE_SOURCE_ACCEPTED)
         fprintf(fd, "TGDB_ABSOLUTE_SOURCE_ACCEPTED(%s)\n", (*com)[j]->data);
      else if((*com)[j]->header == ABSOLUTE_SOURCE_DENIED)
         fprintf(fd, "TGDB_ABSOLUTE_SOURCE_DENIED(%s)\n", (*com)[j]->data);
      else if((*com)[j]->header == QUIT)
         fprintf(fd, "TGDB_QUIT(%s)\n", (*com)[j]->data);
      else
         fprintf(fd, "TGDB_UNKNOWN %s\n", (*com)[j] -> data);
   }
}

void tgdb_delete_command(struct Command ***com){
   int i;
  
   for(i = 0; i < command_cur_size && (com != NULL) && (*com != NULL) && (*com)[i] != NULL; ++i)
      free((*com)[i]);

   free(*com);
   com = NULL;

   command_size = 0;
   command_cur_size = 0;
}

int tgdb_append_command(struct Command ***com, enum INTERFACE_COMMANDS new_header, 
                        char *buf, char *buf2, char *buf3){
   char command[MAXLINE];

   while(command_cur_size >= command_size)
      *com = realloc(*com, (command_size += COMSIZE)*(sizeof(struct Command *)));

   command[0] = '\0';
   
   if(buf != NULL)
      strcat(command, buf);

   if(buf2 != NULL){
      strcat(command, " ");
      strcat(command, buf2);
   }

   if(buf3 != NULL){
      strcat(command, " ");
      strcat(command, buf3);
   }

   (*com)[command_cur_size] = xmalloc(sizeof(struct Command));

   (*com)[command_cur_size]->header = new_header;
   strcpy(((*com)[command_cur_size++])->data, command);

   /*err_msg("UPDATE(%s)", command);*/
   
   return 0;
}

int tgdb_end_command(struct Command ***com){

   while(command_cur_size >= command_size)
      *com = realloc(*com, (command_size += COMSIZE)*(sizeof(struct Command *)));

   if(com != NULL && ((*com) != NULL))
      (*com)[command_cur_size] = NULL;

   /*tgdb_traverse_command(com);*/
      
   return 0;
}
