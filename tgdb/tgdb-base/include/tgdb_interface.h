#ifndef __TGDB_INTERFACE_H__
#define __TGDB_INTERFACE_H__

/* tgdb_interface:
 * ---------------
 *
 *  This unit is dedicated to the clients trying to integrate into libtgdb.
 *  The clients will recieve all data from the debugger.
 *  They can use this data to setup up functions for libtgdb-base to call.
 */

#include "queue.h"
#include "types.h"

/* This type determines what type of command is next to be given to gdb.
 * BUFFER_GUI_COMMAND:      A command given by the gui.
 * BUFFER_TGDB_COMMAND:     A command given by tgdb itself.
 * BUFFER_USER_COMMAND:     A command given by the user.
 * BUFFER_READLINE_COMMAND: A command given for readline
 * BUFFER_OOB_COMMAND:      This command will be run next
 */
enum buffer_command_type {
    BUFFER_VOID = 0,
    BUFFER_GUI_COMMAND,
    BUFFER_TGDB_COMMAND,
    BUFFER_USER_COMMAND,
    BUFFER_READLINE_COMMAND,
    BUFFER_OOB_COMMAND
};

/* This type determines what the user will see when a particular gdb command
 * is run.
 * COMMANDS_SHOW_USER_OUTPUT: This will show all of gdb's output to the user.
 * COMMANDS_HIDE_OUTPUT:      This will not show gdb's output to the user.
 */
enum buffer_output_type {
    COMMANDS_SHOW_USER_OUTPUT = 1,
    COMMANDS_HIDE_OUTPUT
};

/* There are several commands that tgdb knows how to give to gdb. They are:
 * COMMANDS_SET_PROMPT:     This tells readline what the new prompt is.
 * COMMANDS_REDISPLAY:      This forces readline to redisplay the current line.
 * COMMANDS_TAB_COMPLETION: This tab completes the current line.
 */
enum buffer_command_to_run {
    COMMANDS_SET_PROMPT,
    COMMANDS_REDISPLAY,
    COMMANDS_TAB_COMPLETION,
    COMMANDS_VOID
};

/* This is the type that is used to make up 1 complete request from a client 
 * to gdb. With this, tgdb can run a command, capture output and return values.
 */
struct command {
    char *data;
    enum buffer_command_type com_type;
    enum buffer_output_type out_type;
    enum buffer_command_to_run com_to_run;
    void *client_data;
};

/* The functions below are not meant for the clients to use */

/* tgdb_interface_new_item: 
 * ------------------------
 *
 *  Creates a new command and initializes it 
 *
 * data         - The command to run
 * com_type     - Who is running the command
 * out_type     - Where the output should go
 * com_to_run   - What readline command to run
 * client_data  - Data that the client can use when 
 *                prepare_client_for_command is called
 *
 *  Returns: Always is successfull, will call exit on failed malloc
 */
struct command *tgdb_interface_new_command(    
        const char *data, 
        enum buffer_command_type    com_type, 
        enum buffer_output_type     out_type,
        enum buffer_command_to_run  com_to_run,
        void *client_data);

/* tgdb_interface_free_item: 
 * -------------------------
 *
 *  Free a command 
 *
 *  item - The command to free
 */
void tgdb_interface_free_command ( void* item);

/* tgdb_interface_print_item: 
 * --------------------------
 *
 *  This is a function for debugging. It will print a command.
 *
 *  item - The command to print
 */
void tgdb_interface_print_command ( struct command *item );

#endif
