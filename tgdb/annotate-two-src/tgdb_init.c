#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>
#include <signal.h>

#include "tgdb_init.h"
#include "error.h"
#include "pseudo.h"
#include "types.h"
#include "io.h"
#include "globals.h"
#include "terminal.h"
#include "config.h"
#include "util.h"
#include "tgdb_util.h"

#if 0
        char cur = '\0', prev = '\0', prev2 = '\0';
        int status;

        /* Parse output */
        do {
            /*io_display_char(stderr, cur);*/
            if ( cur == '\n' ) {
                if ( prev == '\r' && prev2 == '\r' ) {
                    local_need_mapping = TRUE;
                    break;
                } else if ( prev == '\r' ){
                    local_need_mapping = FALSE;
                    break;
                } else {
                    err_msg("%s:%d -> Unexpected result", __FILE__, __LINE__);
                    local_need_mapping = -1;
                    break;
                }
            }
         
            prev2 = prev;
            prev  = cur;
        } while ( io_read_byte(&cur, masterfd) != -1);


        io_writen(masterfd, "quit\n", 5); /* Tell gdb to quit */
        
        /* Read rest of data */
        while( io_read_byte(&cur, masterfd) != -1);

        if (waitpid(pid, &status, 0) == -1) {
            err_msg("(%s:%d) waitpid failed", __FILE__, __LINE__);
            free_memory(slavename, masterfd, local_argc, local_argv);
            return -1;
        }
#endif

/* tgdb_make_config_file: makes a config file for the user.
 *    Return: -1 on error or 0 on success.
 */
int tgdb_init_setup_config_file(void){
   char gdb_init_file[MAXLINE];
   char path[PATH_MAX];

   if ( tgdb_util_set_home_dir(path) == -1 )
      return -1;

   global_set_config_dir(path);
   global_get_config_gdb_init_file(gdb_init_file);

   if ( access( gdb_init_file, R_OK || W_OK ) == -1 ) {
      FILE *fp = fopen( gdb_init_file, "w" );
      if ( fp ) {
         fprintf( fp, 
               "set annotate 2\n"
               "set height 0\n"
               "set prompt (tgdb) \n" 
               );
         fclose( fp );
      } else {
         err_msg("%s:%d -> Could not open (%s)", __FILE__, __LINE__, gdb_init_file);
         return -1;
      }
   } else {
      err_msg("%s:%d -> Could not access (%s)", __FILE__, __LINE__, gdb_init_file);
      return -1;
   }

   return 0;
}
