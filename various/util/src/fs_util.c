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

#if HAVE_UNISTD_H
#include <unistd.h>
#endif  /* HAVE_UNISTD_H */

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#if HAVE_LIMITS_H
#include <limits.h>
#endif /* HAVE_LIMITS_H */

#if HAVE_SIGNAL_H
#include <signal.h>
#endif /* HAVE_SIGNAL_H */

#if HAVE_ERRNO_H
#include <errno.h>
#endif /* HAVE_ERRNO_H */

#include "fs_util.h"
#include "error.h"

#define MAXLINE 4096


static char global_config_dir[PATH_MAX];

char *tgdb_util_get_config_dir(void){
    return global_config_dir; 
}

char *tgdb_util_get_config_gdbinit_file(void){
    static char filename[PATH_MAX];
    strncpy(filename, global_config_dir, strlen(global_config_dir) + 1);
#ifdef HAVE_CYGWIN
    strcat( filename, "\\gdb_init");
#else
    strcat( filename, "/gdb_init");
#endif
    return filename;
}

char *tgdb_util_get_config_gdb_debug_file(void){
    static char filename[PATH_MAX];
    strncpy(filename, global_config_dir, strlen(global_config_dir) + 1);
#ifdef HAVE_CYGWIN
    strcat( filename, "\\tgdb_debug");
#else
    strcat( filename, "/tgdb_debug");
#endif
    return filename;
}

int tgdb_util_set_home_dir(void) {
    char tgdb_config_dir_unix_path[MAXLINE];
    char homeDir[MAXLINE];
    char *env = getenv("HOME");
    struct stat st;

#ifdef HAVE_CYGWIN
   char tgdb_config_dir_win_path[MAXLINE];
   char win32_homedir[MAXLINE];
   extern void cygwin_conv_to_full_win32_path(const char *path, char *win32_path);
#endif

   if(env == NULL)
      err_quit("%s:%d -> $HOME is not set", __FILE__, __LINE__);

   sprintf( tgdb_config_dir_unix_path, "%s/.tgdb", env );

#ifdef HAVE_CYGWIN
   cygwin_conv_to_full_win32_path(tgdb_config_dir_unix_path, tgdb_config_dir_win_path);
   strncpy( tgdb_config_dir_unix_path, tgdb_config_dir_win_path, strlen(tgdb_config_dir_win_path) + 1);
   cygwin_conv_to_full_win32_path(env, win32_homedir);
   strncpy( homeDir, win32_homedir, strlen(win32_homedir) + 1);
#else 
   strncpy( homeDir, env, strlen(env) + 1);
#endif

   /* Check to see if already exists, if does not exist continue */
   if ( stat( tgdb_config_dir_unix_path, &st ) == -1 && errno == ENOENT ) {
       /* Create home config directory if does not exist */
       if ( access( env, R_OK | W_OK ) == -1 )
           return -1;

       if ( mkdir( tgdb_config_dir_unix_path, 0755 ) == -1 )
           return -1;
   }

#ifdef HAVE_CYGWIN
   sprintf( global_config_dir, "%s\\", tgdb_config_dir_unix_path );
#else
   sprintf( global_config_dir, "%s/", tgdb_config_dir_unix_path );
#endif

   return 0;
}
