#ifndef __FS_UTIL_H__
#define __FS_UTIL_H__

/*******************************************************************************
 *
 * This is the file system unit. All attempts to access the file system should
 * be directed through this unit.
 *
 * All char pointers passed to functions in this unit should be PATH_MAX in 
 * length at a minumum.
 *
 ******************************************************************************/

/* fs_util_is_valid:
 * -----------------
 *
 *  Checks to see if the directory dir exists and has read/write permissions.
 *
 *  dir - The directory to check.
 *
 *  Returns 1 on succes and 0 on failure
 */

int fs_util_is_valid ( const char *dir );

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
int fs_util_create_dir ( const char *dir );

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
int fs_util_create_dir_in_base ( const char *base, const char *dirname );

/* fs_util_get_path:
 * -----------------
 *
 *  Returns the path of the directory/file name in directory base
 *  ex. base=/usr/local, dirname=bin => path=/usr/local/bin
 *
 *  base    - The directory to put the new directory dirname
 *  name    - Then name of the name to create in directory base
 */
void fs_util_get_path ( const char *base, const char *name, char *path );

#endif
