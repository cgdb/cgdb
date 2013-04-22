#ifndef __FS_UTIL_H__
#define __FS_UTIL_H__

/*******************************************************************************
 *
 * This is the file system unit. All attempts to access the file system should
 * be directed through this unit.
 *
 * All char pointers passed to functions in this unit should be FSUTIL_PATH_MAX
 * in length at a minumum. Unfortunatly, the POSIX PATH_MAX can't be used here
 * since it isn't defined with the HURD OS.
 *
 * Anyways, I think in the long run the static buffer will not be the best
 * option and it should be replaced with a dynamic data structure. However,
 * for the sake of time, it is done this way.
 ******************************************************************************/

#define FSUTIL_PATH_MAX 1024

/* fs_util_is_valid:
 * -----------------
 *
 *  Checks to see if the directory dir exists and has read/write permissions.
 *
 *  dir - The directory to check.
 *
 *  Returns 1 on succes and 0 on failure
 */

int fs_util_is_valid(const char *dir);

/* fs_util_create_dir:
 * -------------------
 *
 * Creates the directory dir
 *
 *  dir - The directory to create.
 *
 *  Returns 
 *      1 on succes or if dir already exists.
 *      0 on failure.
 */
int fs_util_create_dir(const char *dir);

/* fs_util_create_dir_in_base:
 * ---------------------------
 *
 * Creates the directory dirname in directory base
 * First calls fs_util_is_valid for base before trying to create directory.
 *
 * 
 *  base    - The directory to put the new directory dirname
 *  dirname - Then name of the directory to create in directory base
 *
 *  Returns 
 *      1 on succes or if dir already exists.
 *      0 on failure.
 */
int fs_util_create_dir_in_base(const char *base, const char *dirname);

/* fs_util_get_path:
 * -----------------
 *
 *  Returns the path of the directory/file name in directory base
 *  ex. base=/usr/local, dirname=bin => path=/usr/local/bin
 *
 *  base    - The directory to put the new directory dirname
 *  name    - Then name of the name to create in directory base
 */
void fs_util_get_path(const char *base, const char *name, char *path);

/* fs_util_file_exists_in path:
 * ----------------------------
 *
 * Checks if the file exists in any known location (absolute path
 * and paths stored in $PATH.
 */
int fs_util_file_exists_in_path(char * filePath);
#endif
