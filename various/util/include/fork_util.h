#ifndef __FORK_UTIL_H__
#define __FORK_UTIL_H__

/* invoke_process: Forks and execs a process
 *      filename: The name of the program to invoke.
 *      argc: The number of parameters to the path.
 *      argv:  an array of pointers to null-terminated strings that represent 
 *             the argument list  available  to  the new  program.
 *             The  first argument, by convention, should point to the file 
 *             name associated with the file being executed. The array of 
 *             pointers must be terminated by a NULL pointer.
 *      in:    Writing to this fd, will write to the STDIN of new program.
 *      out:   Reading from fd, will read from the STDOUT-STDERR of new program.
 *
 *      Return: -1 on error, pid of child on success
 */
int invoke_process(
    char *filename,
    int argc, char *argv[], 
    int *in, int *out,
    int c1, int c2);

/* invoke_pty_process: Starts a child process and puts a pty 
 * ------------        between them.
 *   name       - The path to the program to exec
 *   argc       - Number of arguments in the argv vector
 *   argv       - List of args to pass to new process
 *   slavename  - Output param: The name of the pty device used.
 *   masterfd   - Output parameter: File descriptor of pty
 *   extra_input- This is an extra pipe into the process created.
 *
 * Returns: PID of GDB process on success, -1 on error
 */ 

int invoke_pty_process(
    char *name, 
    int argc, char *argv[], 
    char *slavename, int *masterfd,
    int *extra_input);

/* invoke_pty_process: Starts a child process and puts a pty 
 * ------------        between them.
 *   slavename  - Output param: The name of the pty device used.
 *   masterfd   - Output parameter: File descriptor of pty
 *   extra_input- This is an extra pipe into the process created.
 *   entry      - This is the function called instead of exec.
 *                It is passed the read end of extra_input.
 *
 * Returns: PID of GDB process on success, -1 on error
 */ 

int invoke_pty_process_function( 
    char *slavename, int *masterfd, 
    int *extra_input,
    int (*entry)(int));

int pty_free_process(int *masterfd, char *sname);

/* tgdb_util_new_tty: 
 * This will return a masterfd, slavefd and slavename of a new pseudo terminal.
 * It should be used to tell gdb what tty to redirect the child ( program being
 * debugged) input/output to.
 *
 * The sname char pointer should be SLAVE_SIZE long.
 * 
 * Return: -1 on error, 0 on success.
 */
int util_new_tty(int *masterfd, int *slavefd, char *sname);

/* tgdb_util_free_tty: Free's a tty session.
 * Return: -1 on error, 0 on success.
 */
int util_free_tty(int *masterfd, int *slavefd, char *sname);

/* free_memory: utility function that frees up memory.
 *
 * argc:    The number of items in argv
 * argv:    free is called on each of these
 */
void free_memory(int argc, char *argv[]);

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
        char *path, 
        int argc, char *argv[], 
        int *in, int *out, 
        int choice, char *filename);

#endif
