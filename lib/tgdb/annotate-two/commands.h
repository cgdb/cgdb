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
    /* These are related to the 'info breakpoints' command */
    BREAKPOINT_HEADERS,
    BREAKPOINT_TABLE_BEGIN,
    BREAKPOINT_TABLE_END,
    FIELD,
    RECORD,

    /* Related to the 'info sources' command */
    INFO_SOURCES,
    /* Related to the 'server complete' command for tab completion */
    COMPLETE,

    /* Related to the 'info source' command */
    INFO_LIST,
    INFO_SOURCE_FILENAME_PAIR,
    INFO_SOURCE_RELATIVE
};

/* commands_initialize: Initialize the commands unit */
struct commands *commands_initialize(void);
void commands_shutdown(struct commands *c);

/* commands_parse_field: This is called when tgdb gets a field annotation
 *                       from gdb. 
 * buf -> The 'field' annotation recieved from gdb.
 * n   -> The size of the annotation.
 * field -> the field number outputted.
 */
int commands_parse_field(struct commands *c, const char *buf, size_t n,
        int *field);

/* commands_parse_source: This is called when tgdb gets a source annotation
 *                         from gdb. It parses the annotation and puts the 
 *                         correct information into com for the gui.
 * 
 * buf -> The 'source' annotation recieved from gdb.
 * n   -> The size of the annotation.
 */
int commands_parse_source(struct commands *c,
        struct tgdb_list *client_command_list,
        const char *buf, size_t n, struct tgdb_list *list);

/* command_set_state: Sets the state of the command package. This should usually be called
 *                      after an annotation has been read.
 */
void commands_set_state(struct commands *c,
        enum COMMAND_STATE state, struct tgdb_list *list);

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

/* commands_process: This function recieves the output from gdb when gdb
 *                   is running a command on behalf of this package.
 *
 *    a     -> the character recieved from gdb.
 *    com   -> commands to give back to gdb.
 */
void commands_process(struct commands *c, char a, struct tgdb_list *list);

/* commands_list_command_finished: Returns to the gui the absolute path of
 *                                  the filename requested.
 *
 *  if success is 0 then the list failed, otherwise it worked.
 */
void commands_list_command_finished(struct commands *c,
        struct tgdb_list *list, int success);

/* commands_send_gui_sources: This gives the gui all of the sources that were
 *                            just read from gdb through an 'info sources' prompt.
 *
 *    com   -> commands to give back to gdb.
 */
void commands_send_gui_sources(struct commands *c, struct tgdb_list *list);

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

/* commands_has_commnands_to_run: Determines if there is a command that 
 *                                tgdb needs run.
 *    RETURNS: 1 if there is commands to run, otherwise 0 
 */
int commands_has_commnands_to_run(struct commands *c);

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

/* commands_finalize_command:
 * --------------------------
 *
 *  This can be called to finalize work when the end of a command is reached.
 */
void commands_finalize_command(struct commands *c, struct tgdb_list *list);

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
