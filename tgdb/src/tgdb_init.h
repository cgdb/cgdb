#ifndef __TGDB_INIT_H__
#define __TGDB_INIT_H__

#ifdef __cplusplus
extern "C" {
#endif 

/* forkAndExecPty: Starts GDB as a child process and puts a pseudo-terminal 
 * ------------      between them.
 *   debugger  - The path to the debugger to use
 *   argc      - Number of arguments in the argument vector
 *   argv      - List of arguments to pass to GDB (as passed to tgdb)
 *   slavename - (output parameter --)
 *   masterfd  - Output parameter: File descriptor of pseudo-terminal
 *   need_nl_mapping - determines if tgdb has to set the flag in the pty
 *                to not map '\n' to "\r\n". If set to TRUE then the 
 *                mapping will be turned off. 
 *
 * Returns: PID of GDB process on success, -1 on error
 */
int tgdb_init_forkAndExecPty(char *debugger, int argc, char *argv[],
                   char *slavename, int *masterfd,
                   int need_nl_mapping);

/* tgdb_init_does_gdb_need_mapping: This function is Evil!
 *     
 *    This function fork's and exec's gdb putting a pty 
 *    between the two. The command just determine's if gdb uses
 *    either '\r\n' or '\n' to be a new line. Typically it uses '\n' if gdb was
 *    compiled on UNIX. Otherwise if gdb was compiled on windows a new line is 
 *    translated to '\r\n' through printf.
 *
 *    debugger  - The path to the debugger to use
 *
 *    Returns: -1 on error.
 *             FALSE if gdb only uses '\n' as a new line.
 *             TRUE  if gdb uses '\r\n' as a new line.
 */
int tgdb_init_does_gdb_need_mapping(char *debugger);

/* tgdb_init_setup_config_file:
 *    This finds the users home directory and saves it. By doing so
 *    this routine saves tgdb init directory ( $HOME/.tgdb ).
 *    Also, the .gdb_init file is created that gdb reads in on start up.
 * 
 * Returns: -1 on error, 0 on success.
 */
int tgdb_init_setup_config_file(void);

/* tgdb_init_new_tty: 
 * This will return a masterfd, slavefd and slavename of a new pseudo terminal.
 * It should be used to tell gdb what tty to redirect the child ( program being
 * debugged) input/output to.
 *
 * The sname char pointer should be SLAVE_SIZE long.
 * 
 * Return: -1 on error, 0 on success.
 */
int tgdb_init_new_tty(int *masterfd, int *slavefd, char *sname);
#ifdef __cplusplus
}
#endif

#endif
