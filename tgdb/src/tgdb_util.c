#include "config.h"

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

#include "tgdb_util.h"
#include "error.h"
#include "pseudo.h"
#include "types.h"
#include "util.h"

int tgdb_util_set_home_dir(char *config_dir) {
   char buffer[MAXLINE];
   char *env = getenv("HOME"); 
   struct stat st;

#ifdef HAVE_CYGWIN
   char win32_path[MAXLINE];
   extern void cygwin_conv_to_full_win32_path(const char *path, char *win32_path);
#endif
   
   if(env == NULL){
      err_msg("%s:%d -> $HOME is not set", __FILE__, __LINE__);
      return -1;
   } 
   
#ifdef HAVE_CYGWIN
   sprintf( buffer, "%s\\.tgdb", env );
#else
   sprintf( buffer, "%s/.tgdb", env );
#endif

#ifdef HAVE_CYGWIN
   cygwin_conv_to_full_win32_path(buffer, win32_path);
   strncpy( buffer, win32_path, strlen(win32_path));
#endif

    /* Check to see if already exists, if does not exist continue */
    if ( stat( buffer, &st ) == -1 && errno == ENOENT) {
        /* Create home config directory if does not exist */
        if ( access( env, R_OK | W_OK ) == -1 )
            return -1;

        if ( mkdir( buffer, 0755 ) == -1 )
            return -1;
   } 

    strncpy(config_dir, buffer, strlen(buffer) + 1);

   return 0;
}

int tgdb_util_new_tty(int *masterfd, int *slavefd, char *sname) {
   static char local_slavename[SLAVE_SIZE];

   if ( pty_open(masterfd, slavefd, local_slavename, SLAVE_SIZE, NULL, NULL) == -1){
      err_msg("%s:%d -> Error: PTY open", __FILE__, __LINE__);
      return -1;   
   }

   strncpy(sname, local_slavename, SLAVE_SIZE);
   return 0;
}
