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

int fs_util_is_valid ( const char *dir ) {
    char actual_dir[PATH_MAX];
#ifdef HAVE_CYGWIN
    extern void cygwin_conv_to_full_win32_path(const char *path, char *win32_path);
    char cygwin_actual_dir[PATH_MAX];
#endif

    if(dir == NULL) {
        err_msg("%s:%d -> $HOME is not set", __FILE__, __LINE__);
        return 0;
    }

   /* Get the directory to check */
#ifdef HAVE_CYGWIN
    cygwin_conv_to_full_win32_path(dir, cygwin_actual_dir);
    strncpy( actual_dir, cygwin_actual_dir, strlen(cygwin_actual_dir) + 1);
#else 
    strncpy( actual_dir, dir, strlen(dir) + 1);
#endif

    /* Check if directory dir is readable and writeable */
    if ( access ( actual_dir, R_OK | W_OK ) == -1 ) {
        if ( errno == ENOENT ) {
            err_msg("%s:%d directory '%s' is not set", __FILE__, __LINE__, dir);
            return 0; 
        }

        err_msg("%s:%d directory '%s' does not have read/write permissions", 
                __FILE__, __LINE__, dir);
        return 0;
   }

    return 1;
}


int fs_util_create_dir ( const char *dir ) {
    char actual_dir[PATH_MAX];
    struct stat st;

#ifdef HAVE_CYGWIN
    extern void cygwin_conv_to_full_win32_path(const char *path, char *win32_path);
    char cygwin_actual_dir[PATH_MAX];
#endif

    if(dir == NULL) {
        err_msg("%s:%d dir is NULL", __FILE__, __LINE__);
        return 0;
    }

   /* Get the directory to check */
#ifdef HAVE_CYGWIN
    cygwin_conv_to_full_win32_path(dir, cygwin_actual_dir);
    strncpy( actual_dir, cygwin_actual_dir, strlen(cygwin_actual_dir) + 1);
#else 
    strncpy( actual_dir, dir, strlen(dir) + 1);
#endif

    /* Check to see if already exists, if does not exist continue */
    if ( !stat( actual_dir, &st ) ) {
        /* The file exists, see if it is a directory */
        if ( S_ISDIR ( st.st_mode ) )
            return 1;
        else {
            err_msg("%s:%d file %d is not a directory", __FILE__, __LINE__, actual_dir);
            return 0;
        }
    } else {
        /* The file does not exist, create it */
        if ( errno == ENOENT ) {
            if ( mkdir( actual_dir, 0755 ) == -1 ) {
                err_msg("%s:%d directory %s could not be made", __FILE__, __LINE__, actual_dir);
                return 0;
            } else
                return 1;
        }

        /* Error */
        return 0;
    }

    return 1;
}

int fs_util_create_dir_in_base ( const char *base, const char *dirname ) {
    char dir[PATH_MAX];

    /* Make surr the directory is valid */
    if ( !fs_util_is_valid ( base ) ) {
        err_msg("%s:%d fs_util_is_valid error", __FILE__, __LINE__);
        return -1;
    }

    sprintf( dir, "%s/%s", base, dirname );
    return fs_util_create_dir ( dir );
}

void fs_util_get_path ( const char *base, const char *name, char *path ) {
    char dir[PATH_MAX];

    sprintf( dir, "%s/%s", base, name );

#ifdef HAVE_CYGWIN
    cygwin_conv_to_full_win32_path(dir, path);
#else 
    strncpy( path, dir, strlen(dir) + 1);
#endif
}
