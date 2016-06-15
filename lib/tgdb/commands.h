#ifndef __COMMANDS_H__
#define __COMMANDS_H__

#include "tgdb_types.h"
#include "tgdb_command.h"
#include "a2-tgdb.h"

struct commands;

/* These are the state that an internal command could lead to */
enum COMMAND_STATE {
    /* not a state */
    VOID_COMMAND,
    /* Related to the 'info breakpoints' command */
    INFO_BREAKPOINTS,
    /* Related to the 'info sources' command */
    INFO_SOURCES,
    /* Related to the 'x' command */
    INFO_DISASSEMBLE_PC,
    /* Related to the 'disassemble' command */
    INFO_DISASSEMBLE_FUNC,
    /* Related to the 'info frame' command */
    INFO_FRAME,
    /* Related to the 'server complete' command for tab completion */
    COMMAND_COMPLETE,

    /* Related to the 'info source' command */
    INFO_SOURCE,
};

/* commands_initialize: Initialize the commands unit */
struct commands *commands_initialize(void);
void commands_shutdown(struct commands *c);

/* command_set_state: Sets the state of the command package. This should usually be called
 *                      after an annotation has been read.
 */
void commands_set_state(struct commands *c, enum COMMAND_STATE state);

/* commands_set_field_num: This is used for breakpoint annotations.
 *             field_num is the field that the breakpoint annotation is on.
 */
void commands_set_field_num(struct commands *c, int field_num);

/* command_get_state:   Gets the state of the command package 
 * Returns:          The current state.
 */
enum COMMAND_STATE commands_get_state(struct commands *c);

/* runs a simple command, output goes to user  */
/*int commands_run_command(int fd, struct tgdb_client_command *com);*/

/* commands_issue_command:
 * -----------------------
 *  
 *  This is used by the annotations library to internally issure commands to
 *  the debugger. It sends a command to tgdb-base.
 *
 *  Returns -1 on error, 0 on success
 */
int commands_issue_command(struct commands *c,
        struct tgdb_list *client_command_list,
        enum annotate_commands com, const char *data, int oob);

/* commands_process: This function receives the output from gdb when gdb
 *                   is running a command on behalf of this package.
 *
 *    a     -> the character received from gdb.
 *    com   -> commands to give back to gdb.
 */
void commands_process(struct commands *c, char a,
        struct tgdb_list *response_list, struct tgdb_list *client_command_list);

/* This gives the gui all of the completions that were just read from gdb 
 * through a 'complete' command.
 *
 */
void commands_send_gui_completions(struct commands *c, struct tgdb_list *list);

/* The 3 functions below are for tgdb only.
 * These functions are responsible for keeping tgdb up to date with gdb.
 * If a particular piece of information is needed after each command the user
 * is allowed to give to gdb, then these are the functions that will find out
 * the missing info.
 * These functions should NOT be called for the gui to get work done, and they
 * should not be used for tgdb to get work done. 
 *
 * THEY ARE STRICTLY FOR KEEPING TGDB UP TO DATE WITH GDB
 */

/* commands_prepare_for_command:
 * -----------------------------
 *
 *  Prepare's the client for the command COM to be run.
 *
 *  com:    The command to be run.
 *
 *  Returns: -1 if this command should not be run. 0 otherwise.
 */
int commands_prepare_for_command(struct annotate_two *a2, struct commands *c,
        struct tgdb_command *com);

/* commands_user_ran_command:
 * --------------------------
 *
 * This lets the clients know that the user ran a command.
 * The client can update itself here if it need to.
 *
 * Returns: -1 on error, 0 on success
 */
int commands_user_ran_command(struct commands *c,
        struct tgdb_list *client_command_list);

#endif
