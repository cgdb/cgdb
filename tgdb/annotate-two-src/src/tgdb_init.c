#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_STDLIB_H 
#include <stdlib.h>
#endif  /* HAVE_STDLIB_H */

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#if HAVE_STDIO_H
#include <stdio.h> 
#endif /* HAVE_STDIO_H */

#if HAVE_ERRNO_H
#include <errno.h>
#endif /* HAVE_ERRNO_H */

#if HAVE_UNISTD_H
#include <unistd.h>
#endif  /* HAVE_UNISTD_H */

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */

#if HAVE_LIMITS_H
#include <limits.h>
#endif /* HAVE_LIMITS_H */

#if HAVE_SIGNAL_H
#include <signal.h>
#endif /* HAVE_SIGNAL_H */

#include "tgdb_init.h"
#include "error.h"
#include "pseudo.h"
#include "types.h"
#include "io.h"
#include "globals.h"
#include "terminal.h"
#include "config.h"
#include "sys_util.h"
#include "fs_util.h"

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
   char *gdb_init_file;

   if ( tgdb_util_set_home_dir() == -1 )
      return -1;

   gdb_init_file = tgdb_util_get_config_gdbinit_file();

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
