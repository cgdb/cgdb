/* From configure */
#include "config.h"

#include "macro.h"
#include "types.h"
#include "error.h"
#include "globals.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

extern char config_dir[MAXLINE];

static FILE *mfd = NULL;

static int macro_debug_on = 0;
static int macro_is_legal = 0;
extern int masterfd;

int macro_start(void){
   char macro_file[MAXLINE];
   char config_dir[4096];

   memset(macro_file, '\0', MAXLINE);
   
   global_get_config_dir(config_dir);
   
   strncpy(macro_file, config_dir, strlen(config_dir));

#ifdef HAVE_CYGWIN
   strcat(macro_file, "\\macro.txt");
#else
   strcat(macro_file, "/macro.txt");
#endif

   if ( access( macro_file, R_OK || W_OK ) == -1 ){
      if( (mfd = fopen( macro_file, "w" )) == NULL){
         err_msg("%s:%d -> Could not open macro file (%s)", __FILE__, __LINE__, macro_file);
         return -1;
      } else {
         macro_debug_on = 1;
         macro_is_legal = 1;
      }
   } else {
      err_msg("%s:%d -> Could not access (%s)", __FILE__, __LINE__, config_dir);
      return -1;
   }

   return 0;
}

int macro_write_char(char c){
   if(macro_debug_on == 1)
      fprintf(mfd, "%c", c);

   return 0;
}

int macro_write_str(const char *str){
   if(macro_debug_on == 1)
      fprintf(mfd, "%s", (char *)str);

   return 0;
}

int macro_turn_macro_on(void){
   if(macro_is_legal)
      macro_debug_on = 1;  
   return 0;
}
int macro_turn_macro_off(void){
   macro_debug_on = 0;
   
   return 0;
}
   
int macro_save(const char *filename){
   char macro_file[MAXLINE];
   FILE *fd = NULL;
   char macro_config_file[MAXLINE];
   char config_dir[4096];
   char c;
   FILE *temp;
   memset(macro_config_file, '\0', MAXLINE);

   global_get_config_dir(config_dir);
   
   strncpy(macro_config_file, config_dir, strlen(config_dir));

#ifdef HAVE_CYGWIN
   strcat(macro_config_file, "\\macro.txt");
#else
   strcat(macro_config_file, "/macro.txt");
#endif

   memset(macro_file, '\0', MAXLINE);

#ifdef HAVE_CYGWIN
   sprintf(macro_file, "%s\\%s", config_dir, filename);
#else
   sprintf(macro_file, "%s/%s", config_dir, filename);
#endif

   if( (temp = fopen(macro_file, "w")) == NULL) {
      err_msg("%s:%d -> could not open debug file", __FILE__, __LINE__);
      return -1;
   }

   fclose(mfd);

   if ( ( fd = fopen(macro_config_file, "r")) == NULL){
      err_msg("%s:%d -> could not open debug file", __FILE__, __LINE__);
      return -1;
   }

   while( (c = fgetc(fd)) != EOF)
      fprintf(temp, "%c", c);

   fclose(fd);
   fclose(temp);

   if ( ( mfd = fopen ( macro_config_file, "a")) == NULL){
      err_msg("%s:%d -> could not append macro file", __FILE__, __LINE__);
      return -1;
   }

   return 0;
}

extern char *tgdb_send(char c);

int macro_load(const char *filename){
   char macro_file[MAXLINE];
   char config_dir[4096];
   FILE *fd;
   char c;
   memset(macro_file, '\0', MAXLINE);
   
   global_get_config_dir(config_dir);
   
#ifdef HAVE_CYGWIN
   sprintf(macro_file, "%s\\%s", config_dir, filename);
#else
   sprintf(macro_file, "%s/%s", config_dir, filename);
#endif

   if( (fd = fopen(macro_file, "r")) == NULL) {
      err_msg("%s:%d -> could not open macro file", __FILE__, __LINE__);
      return -1;
   }

   while( (c = fgetc(fd)) != EOF){
//      if(c == '\r')
//         err_msg("%s", "\\r");
//      else if(c == '\n')
//         err_msg("%s", "\\n\n");
//      else if(c == '\032')
//         err_msg("%s", "\\032");
//      else if(c == '\b')
//         err_msg("%s", "\\b");
//      else {
//         err_msg("(%c)", c);
//      }
         
      if ( tgdb_send(c) == NULL){
         err_msg("%s:%d -> could not write macro file", __FILE__, __LINE__);
         return -1;
      }
   }

   fclose(fd);
   return 0;
}
