#ifndef __GDBMI_INIT_H__
#define __GDBMI_INIT_H__

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
 *
 *      Return: -1 on error, pid of child on success
 */
int invoke_debugger(char *path, int argc, char *argv[], int *in, int *out);

#endif /* __GDBMI_INIT_H__ */
