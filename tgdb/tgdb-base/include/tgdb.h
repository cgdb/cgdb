#ifndef __TGDB_H__
#define __TGDB_H__

/* tgdb:
 * -----
 *
 *  This is the interface to the gui.
 *  The gui can call any functions in this interface.
 */

#ifdef __cplusplus
extern "C" {
#endif 

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */

#include "types.h"

/* tgdb_start: This starts up gdb and returns a fd to gdb's output and to
 *            the childs output. Both fd's should only be used by the library. 
 *            They are only returned so that the gui programmer can do some 
 *            sort of I/O multiplexing with them. This is the first call in the 
 *            API that the gui needs to call.
 *            
 *            When gdb is active, tgdb_recv needs to be called. Otherwise, when
 *            child is active, tgdb_child_recv needs to be called.
 *
 *            debugger: The path to the desired debugger to use.
 *                      If this is NULL then, just "gdb" is used.
 *            argc:     The number of arguments to pass to debugger.
 *            argv:     The arguments to pass to the debugger.
 *            gdb:      The descriptor to the debugger's I/O.
 *            child:    The descriptor to the debugged program.
 *            readline: The descriptor that returns readline's output
 *
 * RETURNS: 0 on success or -1 on error
 */
int tgdb_init(
            char *debugger, 
            int argc, char **argv, 
            int *gdb, int *child, int *readline);

/* tgdb_send: 
 * ----------
 *
 *  Sends a command to the debugger
 *
 *  command  - The user typed command to send
 *             If line is null, then the '\n' command is assummed.
 *             The command should not have a '\n' at the end.
 *  out_type - if 1, the command will be shown before the output.
 *             if 2, the command will not be shown, just output.
 *
 * RETURNS: NULL on error else a pointer to a NULL-terminated string
 *          that should be displayed to the user.
 *          This string is statically allocated, so do not free it.
 */
char *tgdb_send(char *command, int out_type);

/* tgdb_send_input: 
 * ----------------
 *
 *  All input to the debugger should be passed here.
 *
 *  c - The character to pass to the debugger.
 *
 *  Returns: 0 on success or -1 on error
 */
int tgdb_send_input(char c);

/* tgdb_recv_input: 
 * ----------------
 *
 *  The output of the debugger can be recieved here.
 *
 *  buf - The output of the debugger will be retured in this buffer.
 *
 *  Returns: 0 on success or -1 on error
 */
int tgdb_recv_input(char *buf);

/* tgdb_tty_send: 
 * --------------
 *
 *  Sends a character to the inferior program.
 *
 *    c     - the character to send
 *
 * RETURNS: NULL on error else a pointer to a NULL-terminated string
 *          that should be displayed by the GUI in the GDB pane.
 *          This string is statically allocated, so do not free it.
 */
char *tgdb_tty_send(char c);

/* tgdb_recv: 
 * ----------
 *
 *  Gets output from the debugger and from the library.
 *      
 *    buf   -  The output from gdb that should be displayed to the user.
 *    n     -  The size of buf.
 *    com   -  Commands from the library.
 *
 * RETURNS: the amount of bytes in buf on success or -1 on error
 * 
 *    NOTE: The caller must call tgdb_delete_command on com
 *          before the calling this function again. The first time
 *          the user can call this function without worrying.
 */
size_t tgdb_recv(char *buf, size_t n, struct queue *q); 

/* tgdb_tty_recv: 
 * 
 * This is called when the child file descriptor is active.
 *    buf   -  The output from the child that should be displayed to the user.
 *    n     -  The size of buf.
 *
 * RETURNS: the amount of bytes in buf on success or -1 on error
 */
size_t tgdb_tty_recv(char *buf, size_t n);

/* tgdb_new_tty:
 *
 * This allocates a new tty and tells gdb to use it.
 * Basically that means that whatever I/O was left in the buffer is blasted
 * and the user can start fresh.
 *
 * RETURNS: 
 *      -1 on error or 0 on success
 *      The return value only indicates whether the tty was allocated properly,
 *      not whether gdb accepted the tty, since this can only be determined 
 *      when gdb responds, not when the command is given.
 */
int tgdb_new_tty(void);

/* tgdb_tty_name:
 *
 * This returns the name of the pseudo terminal device that tgdb is using
 * to communicate with gdb in regards to the program being debugged.
 * It returns a string that is at most SLAVE_SIZE characters long.
 */
char *tgdb_tty_name(void);

/* tgdb_get_sources: 
 * -----------------
 *
 *  Gets a list of source files that make up the program being debugged.
 *  If the function succeeds the gui will get back TGDB_UPDATE_SOURCE_FILES
 *  containing a list of all the source files. Otherwise the gui will get
 *  back TGDB_SOURCES_DENIED.
 */
int (*tgdb_get_sources)(void);

/* tgdb_get_source_absolute_filename:
 * ----------------------------------
 *
 * This function will return the absolute path to the file file.
 * file must be a valid relative path. 
 *
 * If this functions succeeds TGDB_ABSOLUTE_SOURCE_ACCEPTED will be returned.
 * Otherwise, TGDB_ABSOLUTE_SOURCE_DENIED gets returned.
 */
int (*tgdb_get_source_absolute_filename)(char *file);

/* tgdb_err_msg: Returns an error message from the last library call that
 *               generated an error message.
 *
 * Return: A pointer to the buffer that contains the error message is returned.
 * The buffer returned is static and can be invalid after the next library 
 * call. If the client desires to keep it they should copy it.
 * If there is no error message, NULL is returned.
 */
char* (*tgdb_err_msg)(void);

/*  tgdb_shutdown: Terminates tgdb's library support.
 *
 * RETURNS: 0 on success or -1 on error
 */
int tgdb_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif /* __TGDB_H__ */
