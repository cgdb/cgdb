#ifndef __COMMANDS_H__
#define __COMMANDS_H__

#include "types.h"
#include "buffer.h"

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

   /* Related to the 'info source' command */
   INFO_LIST,
   INFO_SOURCE
};

/* commands_parse_field: This is called when tgdb gets a field annotation
 *                       from gdb. 
 * buf -> The 'field' annotation recieved from gdb.
 * n   -> The size of the annotation.
 * field -> the field number outputted.
 */
int commands_parse_field(const char *buf, size_t n, int *field);

/* commands_parse_source: This is called when tgdb gets a source annotation
 *                         from gdb. It parses the annotation and puts the 
 *                         correct information into com for the gui.
 * 
 * buf -> The 'source' annotation recieved from gdb.
 * n   -> The size of the annotation.
 */
int commands_parse_source(const char *buf, size_t n, struct Command ***com);

/* command_set_state: Sets the state of the command package. This should usually be called
 *                      after an annotation has been read.
 */
void commands_set_state(enum COMMAND_STATE state, struct Command ***com);

/* commands_set_field_num: This is used for breakpoint annotations.
 *             field_num is the field that the breakpoint annotation is on.
 */
void commands_set_field_num(int field_num);

/* command_get_state:   Gets the state of the command package 
 * Returns:          The current state.
 */
enum COMMAND_STATE commands_get_state(void);

/* runs a simple command, output goes to user  */
int commands_run_command(int fd, char *com, enum buffer_command_type com_type);

/* commands_process: This function recieves the output from gdb when gdb
 *                   is running a command on behalf of this package.
 *
 *    a     -> the character recieved from gdb.
 *    com   -> commands to give back to gdb.
 */
void commands_process(char a, struct Command ***com);

/* commands_run_info_breakpoints: This runs the command 'info breakpoints' and prepares tgdb
 *                            by setting certain variables.
 * 
 *    fd -> The file descriptor to write the command to.
 */
int commands_run_info_breakpoints(int fd);

/* commands_run_info_sources: This runs the command 'info sources' and prepares tgdb
 *                            by setting certain variables.
 * 
 *    fd -> The file descriptor to write the command to.
 */
int commands_run_info_sources(int fd);

/* commands_run_info_source:  This runs the command 'list filename:1' and then runs
 *                            'info source' to find out what the absolute path to 
 *                            filename is.
 * 
 *    filename -> The name of the file to check the absolute path of.
 *    fd -> The file descriptor to write the command to.
 */
int commands_run_list(char *filename, int fd);

int commands_run_tty(char *tty, int fd);

/* commands_list_command_finished: Returns to the gui the absolute path of
 *                                  the filename requested.
 *
 *  if success is 0 then the list failed, otherwise it worked.
 */
void commands_list_command_finished(struct Command ***com, int success);

/* commands_send_gui_sources: This gives the gui all of the sources that were
 *                            just read from gdb through an 'info sources' prompt.
 *
 *    com   -> commands to give back to gdb.
 */
void commands_send_gui_sources(struct Command ***com);

/* commands_send_source_absolute_source_file: This will send to the gui the 
 *             absolute path to the file being requested. Otherwise the gui
 *             will be notified that the file is not valid.
 */
void commands_send_source_absolute_source_file(struct Command ***com);

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
int commands_has_commnands_to_run(void);

#endif
