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

int fs_util_is_valid(const std::string &dir)
{
    /* Check if directory dir is readable and writeable */
    if (access(dir.c_str(), R_OK | W_OK) == -1) {
        clog_error(CLOG_CGDB,
                "directory '%s' does not have read/write permissions",
                dir.c_str());
        return 0;
    }

    return 1;
}

int fs_util_create_dir(const std::string &dir)
{
    struct stat st;

    /* Check to see if already exists, if does not exist continue */
    if (!stat(dir.c_str(), &st)) {
        /* The file exists, see if it is a directory */
        if (S_ISDIR(st.st_mode))
            return 1;
        else {
            clog_error(CLOG_CGDB, "file %s is not a directory", dir.c_str());
            return 0;
        }
    } else {
        /* The file does not exist, create it */
        if (errno == ENOENT) {
            if (mkdir(dir.c_str(), 0755) == -1) {
                clog_error(CLOG_CGDB,
                        "directory %s could not be made", dir.c_str());
                return 0;
            } else
                return 1;
        }

        /* Error */
        return 0;
    }

    return 1;
}

int fs_util_create_dir_in_base(const std::string &base,
        const std::string &dirname)
{
    std::string dir = base + "/" + dirname;
    return fs_util_create_dir(dir.c_str());
}

std::string fs_util_get_path(const std::string &base, const std::string &name)
{
    return base + "/" + name;
}

int fs_util_file_exists_in_path(const std::string &filePath)
{
    struct stat buff;
    char * tok, *local_pathStr;
    std::string testPath;
    int result = -1;
    char *pathStr = getenv("PATH");
    local_pathStr = (char *)malloc(strlen(pathStr) + 1);
    strcpy(local_pathStr, pathStr);
    
    if (stat(filePath.c_str(), &buff) >= 0)
    {
        free(local_pathStr);
        return 0;
    }
    /* Check all directories in path*/
    tok = strtok(local_pathStr, ":");
    while (tok != NULL)
    {
        testPath = fs_util_get_path(tok, filePath);
        if (stat(testPath.c_str(), &buff) >= 0) {
            result = 0;
            break;
        }
        tok = strtok(NULL, ":");
    }
    free(local_pathStr);
    return result;
}

int fs_verify_file_exists(const std::string &path)
{
    struct stat st;

    /* Check for read permission of file, already exists */
    if (stat(path.c_str(), &st) == -1)
        return 0;

    return 1;
}
