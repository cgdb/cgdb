#ifndef __FORK_UTIL_H__
#define __FORK_UTIL_H__

/**
 * This code probably belongs in a separate file, or maybe in pseudo.h. However,
 * I don't want to change that in the rl BRANCH.
 */

struct pty_pair;
typedef struct pty_pair *pty_pair_ptr;

/**
 * Create a new pty pair context.
 *
 * \return
 * A new pty_pair on success, NULL on error.
 */
pty_pair_ptr pty_pair_create (void);

/**
 * Destroy the pty pair context.
 *
 * \param pty_pair
 * The pty_pair context to destroy
 *
 * \return
 * 0 on success or -1 on error
 */
int pty_pair_destroy (pty_pair_ptr pty_pair);

/**
 * Get the masterfd from the pty pair.
 *
 * \param pty_pair
 * The pair to get the masterfd from.
 *
 * \return
 * The master fd on success or -1 on error.
 */
int pty_pair_get_masterfd (pty_pair_ptr pty_pair);

/**
 * Get the slavefd from the pty pair.
 *
 * \param pty_pair
 * The pair to get the slavefd from.
 *
 * \return
 * The slave fd on success or -1 on error.
 */
int pty_pair_get_slavefd (pty_pair_ptr pty_pair);

/**
 * Get the slave device name from the pty pair.
 *
 * \param pty_pair
 * The pair to get the slave device name from.
 *
 * \return
 * The slave device name or NULL on error.
 * The return data should not be modified.
 */
const char *pty_pair_get_slavename (pty_pair_ptr pty_pair);

/* invoke_debugger: Forks and execs the path.
 *      path: The path to the path.
 *      argc: The number of parameters to the path.
 *      argv:  an array of pointers to null-terminated strings that represent 
 *             the argument list  available  to  the new  program.
 *             The  first argument, by convention, should point to the file 
 *             name associated with the file being executed. The array of 
 *             pointers must be terminated by a NULL pointer.
 *      in:    Writing to this fd, will write to the STDIN of new program.
 *      out:   Reading from fd, will read from the STDOUT-STDERR of new program.
 *      choice: 0 for annotate 2, 1 for gdbmi
 *      filename: The name of the init file for annotate 2
 *
 *      Return: -1 on error, pid of child on success
 */
int invoke_debugger(
        const char *path, 
        int argc, char *argv[], 
        int *in, int *out, 
        int choice, char *filename);

#endif
