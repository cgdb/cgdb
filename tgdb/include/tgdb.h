#ifndef __TGDB_H__
#define __TGDB_H__

#ifdef __cplusplus
extern "C" {
#endif 

#include <config.h>

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif 

#include "types.h"

/* tgdb_init: Allows libtgdb to configure itself.
 * Returns: 0 on success or -1 on error.
 * If this function returns -1 then it can not correctly be used 
 * to interface with gdb.
 */
int tgdb_init(void);

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
int (*tgdb_start)(char *debugger, int argc, char **argv, 
                    int *gdb, int *child, int *readline);

/* tgdb_send: Sends a character to the debugger that the user typed.
 *
 *    line     - the user typed command to send
 *
 * RETURNS: NULL on error else a pointer to a NULL-terminated string
 *          that should be displayed by the GUI in the GDB pane.
 *          This string is statically allocated, so do not free it.
 */
char *(*tgdb_send)(char *line);

/* tgdb_send_input: GUI should send every char the user is typing to gdb
 *
 * c        - Every character typed by the user to gdb.
 * RETURNS: 0 on success or -1 on error
 */
int (*tgdb_send_input)(char c);

/* tgdb_recv_input: All data returned from this should be displayed.
 *
 * buf        - This buf should be displayed
 * RETURNS: 0 on success or -1 on error
 */
int (*tgdb_recv_input)(char *buf);

/* tgdb_tty_send: Sends a character to the program being debugged by gdb.
 *
 *    c     - the character to send
 *
 * RETURNS: NULL on error else a pointer to a NULL-terminated string
 *          that should be displayed by the GUI in the GDB pane.
 *          This string is statically allocated, so do not free it.
 */
char *(*tgdb_tty_send)(char c);

/* tgdb_recv: Gets output from the debugger and from the library.
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
size_t (*tgdb_recv)(char *buf, size_t n, struct Command ***com); 

/* tgdb_tty_recv: 
 * 
 * This is called when the child file descriptor is active.
 *    buf   -  The output from the child that should be displayed to the user.
 *    n     -  The size of buf.
 *
 * RETURNS: the amount of bytes in buf on success or -1 on error
 */
size_t (*tgdb_tty_recv)(char *buf, size_t n);

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
int (*tgdb_new_tty)(void);

/* tgdb_tty_name:
 *
 * This returns the name of the pseudo terminal device that tgdb is using
 * to communicate with gdb in regards to the program being debugged.
 * It returns a string that is at most SLAVE_SIZE characters long.
 */
char *(*tgdb_tty_name)(void);

/* tgdb_run_command: Runs a command.
 * Returns: 0 on success, otherwise -1.
 */
int (*tgdb_run_command)(char *com);

/* tgdb_get_sources: Gets a list of source files that make up the program
 * being debugged.
 *
 *    This function triggers tgdb to discover all of the source files that is 
 *    in the program being debugged. It returns these files by way of the com
 *    structure in tgdb_recv. The first header will be SOURCES_START. 
 *    Every struct with a header SOURCE_FILE will be a valid source file.
 *    That will continue until a SOURCES_END is returned.
 */
int (*tgdb_get_sources)(void);

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

/* tgdb_get_prompt: Returns a pointer to the current prompt */
char* (*tgdb_get_prompt)(void);

/*  tgdb_shutdown: Terminates tgdb's library support.
 *
 * RETURNS: 0 on success or -1 on error
 */
int (*tgdb_shutdown)(void);

#ifdef __cplusplus
}
#endif

#endif /* __TGDB_H__ */
