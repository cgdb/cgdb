#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif /* HAVE_STDLIB_H */

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#if HAVE_STDIO_H
#include <stdio.h>
#endif /* HAVE_STDIO_H */

#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#if HAVE_SIGNAL_H
#include <signal.h>
#endif /* HAVE_SIGNAL_H */

#if HAVE_ERRNO_H
#include <errno.h>
#endif /* HAVE_ERRNO_H */

#include "sys_util.h"
#include "fs_util.h"

int fs_util_is_valid(const char *dir)
{
    char actual_dir[FSUTIL_PATH_MAX];

    if (dir == NULL) {
        clog_error(CLOG_CGDB, "$HOME is not set");
        return 0;
    }

    /* Get the directory to check */
    strncpy(actual_dir, dir, strlen(dir) + 1);

    /* Check if directory dir is readable and writeable */
    if (access(actual_dir, R_OK | W_OK) == -1) {
        if (errno == ENOENT) {
            clog_error(CLOG_CGDB,
                    "directory '%s' is not set", dir);
            return 0;
        }

        clog_error(CLOG_CGDB,
                "directory '%s' does not have read/write permissions", dir);
        return 0;
    }

    return 1;
}

int fs_util_create_dir(const char *dir)
{
    char actual_dir[FSUTIL_PATH_MAX];
    struct stat st;

    if (dir == NULL) {
        clog_error(CLOG_CGDB, "dir is NULL");
        return 0;
    }

    /* Get the directory to check */
    strncpy(actual_dir, dir, strlen(dir) + 1);

    /* Check to see if already exists, if does not exist continue */
    if (!stat(actual_dir, &st)) {
        /* The file exists, see if it is a directory */
        if (S_ISDIR(st.st_mode))
            return 1;
        else {
            clog_error(CLOG_CGDB, "file %s is not a directory", actual_dir);
            return 0;
        }
    } else {
        /* The file does not exist, create it */
        if (errno == ENOENT) {
            if (mkdir(actual_dir, 0755) == -1) {
                clog_error(CLOG_CGDB,
                        "directory %s could not be made", actual_dir);
                return 0;
            } else
                return 1;
        }

        /* Error */
        return 0;
    }

    return 1;
}

int fs_util_create_dir_in_base(const char *base, const char *dirname)
{
    char dir[FSUTIL_PATH_MAX];

    snprintf(dir, sizeof(dir), "%s/%s", base, dirname);
    return fs_util_create_dir(dir);
}

void fs_util_get_path(const char *base, const char *name, char *path)
{
    snprintf(path, FSUTIL_PATH_MAX, "%s/%s", base, name);
}

int fs_util_file_exists_in_path(char * filePath)
{
    struct stat buff;
    char * tok, *local_pathStr;
    char testPath[1024];
    int result = -1;
    char *pathStr = getenv("PATH");
    local_pathStr = (char *)malloc(strlen(pathStr) + 1);
    strcpy(local_pathStr, pathStr);
    
    if (stat(filePath, &buff) >= 0)
    {
        free(local_pathStr);
        return 0;
    }
    /* Check all directories in path*/
    tok = strtok(local_pathStr, ":");
    while (tok != NULL)
    {
        snprintf(testPath, sizeof(testPath), "%s/%s", tok, filePath);
        if (stat(testPath, &buff) >= 0) {
            result = 0;
            break;
        }
        tok = strtok(NULL, ":");
    }
    free(local_pathStr);
    return result;
}

int fs_verify_file_exists(const char *path)
{
    struct stat st;

    /* Check for read permission of file, already exists */
    if (stat(path, &st) == -1)
        return 0;

    return 1;
}
