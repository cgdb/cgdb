#ifndef __TGDB_UTIL_H__
#define __TGDB_UTIL_H__


/* tgdb_util_new_tty: 
 * This will return a masterfd, slavefd and slavename of a new pseudo terminal.
 * It should be used to tell gdb what tty to redirect the child ( program being
 * debugged) input/output to.
 *
 * The sname char pointer should be SLAVE_SIZE long.
 * 
 * Return: -1 on error, 0 on success.
 */
int tgdb_util_new_tty(int *masterfd, int *slavefd, char *sname);

/* tgdb_util_set_home_dir: Ceate a config dir in user's home dir.
 *
 * config_dir: Returns the path to the config dir created.
 *             The buffer config_dir should probably at least be
 *             PATH_MAX long.
 */
int tgdb_util_set_home_dir(char *config_dir);

#endif
