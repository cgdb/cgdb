#ifndef __A2_TGDB_H__
#define __A2_TGDB_H__

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */

#include "types.h"
#include "tgdb_interface.h"


/*  The only commands used
    ----------------------------
    info breakpoints    -> Get a list of breakpoints
    tty                 -> Tell gdb where to send inferior's output
    complete            -> Complete the current console line
    info sources        -> Show all the sources inferior is made of
    info source         -> Shows information on the current source file
    list                -> displays the contents of current source file
    complete arg        -> completes the argument arg
*/

enum annotate_commands {
    ANNOTATE_VOID = 0,
    ANNOTATE_INFO_BREAKPOINTS,
    ANNOTATE_TTY,
    ANNOTATE_COMPLETE,
    ANNOTATE_INFO_SOURCES,
    ANNOTATE_INFO_SOURCE_RELATIVE,
    ANNOTATE_INFO_SOURCE_ABSOLUTE,
    ANNOTATE_INFO_SOURCE,
    ANNOTATE_LIST,
    ANNOTATE_SET_PROMPT
};

/* a2_find_valid_debugger:
 * -----------------------
 *  Determines if there is a gdb that supports annotate = 2.
 *
 *  debugger:   - The path to the desired debugger to use.  
 *                If this is NULL then just "gdb" is used.
 *  argc:       - The number of arguments passed to the debugger.
 *  argv:       - The arguments passed to the debugger.
 *
 *  debugger_stdin: - writing writes to debugger's stdin
 *  gdb:        - The debugger's descriptor to select on.
 *  readline:   - Readline's descriptor to select on.
 *  config_dir  - This is the directory to write configuration files to
 *
 *  Returns 
 *      1 if there is, 
 *      0 if there isn't
 *      -1 on error
 */
int a2_find_valid_debugger ( 
            char *debugger,
            int argc, char **argv,
            int *debugger_stdin, int *gdb,
            char *config_dir);

/* a2_tgdb_init:
 * -------------
 *
 * This initializes libannotate.
 *
 * inferior_tty_name: The name of the tty that the inferior should use.
 *
 * Returns: -1 on error, 0 on success
 */
int a2_tgdb_init( const char *inferior_tty_name );

/* a2_tgdb_shutdown:
 * -----------------
 *
 *  This properly shuts down the annotate two subsystem.
 */
int a2_tgdb_shutdown(void);

/* a2_tgdb_get_source_absolute_filename:
 * -------------------------------------
 *
 * file: The relative name of the filename to get.
 */
int a2_tgdb_get_source_absolute_filename(char *file);

/* a2_tgdb_get_sources:
 * --------------------
 *
 */
int a2_tgdb_get_sources(void);

/* a2_set_inferior_tty:
 * --------------------
 *
 *  Sets the inferior's new tty name.
 *
 *  inferior_tty_name: The name of the tty that the inferior should use.
 *
 *  Returns: -1 on error, 0 on success
 */
int a2_set_inferior_tty ( const char *inferior_tty_name );

/* a2_tgdb_err_msg:
 * ----------------
 *
 */
char *a2_tgdb_err_msg(void);

/* These are called from within annotate-two-src for now 
 * A better approach will be given in the future.
 */
int a2_tgdb_change_prompt(char *prompt);

/* a2_tgdb_command_callback: 
 * -------------------------
 *
 * This is called when readline determines a command has been typed. 
 *
 *      line - The command the user typed without the '\n'.
 */
int a2_tgdb_command_callback(const char *line);

/* a2_tgdb_completion_callback: 
 * ----------------------------
 *
 *  This is called when readline determines a command needs to be completed.
 *
 *      line - The command to be completed
 */
int a2_tgdb_completion_callback(const char *line);

/* Returns 1 if ready to recieve data, otherwise 0 */
int a2_is_ready ( void );

/* a2_command_typed_at_prompt:
 *      This functions recieves the characters the user has typed.
 *      It only gets called if it is not busy ( the function above )
 */
void a2_command_typed_at_prompt ( int i );

/* a2_tgdb_is_debugger_ready:
 * --------------------------
 *
 * This determines if the annotate two subsystem is ready to have gdb recieve
 * another command.
 *
 * Returns: 1 if it is ready, 0 if it is not.
 */
int a2_tgdb_is_debugger_ready(void);

#endif /* __A2_TGDB_H__ */
