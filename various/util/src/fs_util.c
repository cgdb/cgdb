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

#if HAVE_SIGNAL_H
#include <signal.h>
#endif /* HAVE_SIGNAL_H */

#if HAVE_ERRNO_H
#include <errno.h>
#endif /* HAVE_ERRNO_H */

#if HAVE_CYGWIN
#include <sys/cygwin.h>
#endif /* HAVE_CYGWIN */

#include "fs_util.h"
#include "logger.h"

#define MAXLINE 4096

/* TODO: The cgywin_conv functions used here are deprecated. */

int fs_util_is_valid ( const char *dir ) {
    char actual_dir[FSUTIL_PATH_MAX];
#ifdef HAVE_CYGWIN
    char cygwin_actual_dir[FSUTIL_PATH_MAX];
#endif

    if(dir == NULL) {
        logger_write_pos ( logger, __FILE__, __LINE__, "$HOME is not set");
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
            logger_write_pos ( logger, __FILE__, __LINE__, "directory '%s' is not set", dir);
            return 0; 
        }

        logger_write_pos ( logger, __FILE__, __LINE__, "directory '%s' does not have read/write permissions", dir);
        return 0;
   }

    return 1;
}


int fs_util_create_dir ( const char *dir ) {
    char actual_dir[FSUTIL_PATH_MAX];
    struct stat st;

#ifdef HAVE_CYGWIN
    char cygwin_actual_dir[FSUTIL_PATH_MAX];
#endif

    if(dir == NULL) {
        logger_write_pos ( logger, __FILE__, __LINE__, "dir is NULL");
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
            logger_write_pos ( logger, __FILE__, __LINE__, "file %d is not a directory", actual_dir);
            return 0;
        }
    } else {
        /* The file does not exist, create it */
        if ( errno == ENOENT ) {
            if ( mkdir( actual_dir, 0755 ) == -1 ) {
                logger_write_pos ( logger, __FILE__, __LINE__, "directory %s could not be made", actual_dir);
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
    char dir[FSUTIL_PATH_MAX];

    /* Make surr the directory is valid */
    if ( !fs_util_is_valid ( base ) ) {
        logger_write_pos ( logger, __FILE__, __LINE__, "fs_util_is_valid error");
        return -1;
    }

    sprintf( dir, "%s/%s", base, dirname );
    return fs_util_create_dir ( dir );
}

void fs_util_get_path ( const char *base, const char *name, char *path ) {
    char dir[FSUTIL_PATH_MAX];

    sprintf( dir, "%s/%s", base, name );

#ifdef HAVE_CYGWIN
    cygwin_conv_to_full_win32_path(dir, path);
#else 
    strncpy( path, dir, strlen(dir) + 1);
#endif
}
