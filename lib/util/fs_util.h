#ifndef __FS_UTIL_H__
#define __FS_UTIL_H__

/*******************************************************************************
 *
 * This is the file system unit. All attempts to access the file system should
 * be directed through this unit.
 *
 * All char pointers passed to functions in this unit should be FSUTIL_PATH_MAX
 * in length at a minimum. Unfortunately, the POSIX PATH_MAX can't be used here
 * since it isn't defined with the HURD OS.
 *
 * Anyways, I think in the long run the static buffer will not be the best
 * option and it should be replaced with a dynamic data structure. However,
 * for the sake of time, it is done this way.
 *
 * Functions may log their output using clog; this behavior should change
 * depending on the most recent call to 'fs_util_enable_logging' or
 * 'fs_util_disable_logging'. Keep this in mind when adding new functions in
 * this unit. 
 ******************************************************************************/

#define FSUTIL_PATH_MAX 4096

/* fs_util_enable_logging:
 * ------------------------
 *
 *  Enables error message logging for fs_util functions. 
 *
 *  Returns nothing. 
 */
void fs_util_enable_logging(); 

/* fs_util_disable_logging:
 * ------------------------
 *
 *  Disables error message logging for fs_util functions. 
 *
 *  Returns nothing. 
 */
void fs_util_disable_logging(); 

/* fs_util_is_valid:
 * -----------------
 *
 *  Checks to see if the directory dir exists and has read/write permissions.
 *
 *  dir - The directory to check.
 *
 *  Returns 1 on success and 0 on failure
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
 *      1 on success or if dir already exists.
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
 *      1 on success or if dir already exists.
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

/* fs_verify_file_exists path:
 * ----------------------------
 *
 * Checks if the file exists.
 *
 * @param path
 * The path to determine if it exists or not.
 *
 * @return
 * 1 if the file exists, otherwise 0.
 */
int fs_verify_file_exists(const char *path);

#endif
