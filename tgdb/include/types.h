#ifndef __TYPES_H__
#define __TYPES_H__

#ifdef __cplusplus
extern "C" {
#endif 

#define MAXLINE 4096

#define SLAVE_SIZE 64

#ifndef TRUE
#define TRUE  1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */

#if HAVE_STDIO_H
#include <stdio.h>
#endif

#include "queue.h"

enum INTERFACE_COMMANDS {
   BREAKPOINTS_BEGIN,   /* starts a breakpoint session */
   BREAKPOINT,          /* a single breakpoint */
   BREAKPOINTS_END,     /* ends a breakpoint session */
   SOURCE_FILE_UPDATE,  /* tells the gui the current source file */
   CURRENT_FILE_UPDATE, /* tells the gui the current relative source file */
   LINE_NUMBER_UPDATE,  /* tells the gui the current line number */
   SOURCES_START,       /* marks the beggining of a list of source files */
   SOURCE_FILE,         /* a source file */
   SOURCES_END,         /* marks the end of a list of source files */
   SOURCES_DENIED,      /* program not compiled with debug ( annotate error ) */
   ABSOLUTE_SOURCE_ACCEPTED, /* Returns the absolute path of the file requested */
   ABSOLUTE_SOURCE_DENIED,  /* gdb could not find the source file */
   DISPLAY_UPDATE,      /* Updates the value of a variable */
   QUIT
};

struct Command {
   enum INTERFACE_COMMANDS header;
   char data[MAXLINE];
};

/* This type determines what type of command is next to be given to gdb.
 * BUFFER_GUI_COMMAND:  A command given by the gui.
 * BUFFER_TGDB_COMMAND: A command given by tgdb itself.
 * BUFFER_USER_COMMAND: A command given by the user.
 * BUFFER_READLINE_COMMAND: This is to run a command through readline.
 */
enum buffer_command_type {
   BUFFER_VOID = 0,
   BUFFER_GUI_COMMAND,
   BUFFER_TGDB_COMMAND,
   BUFFER_USER_COMMAND,
   BUFFER_READLINE_COMMAND
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
 * COMMANDS_INFO_SOURCES: Gets a list of all the source files.
 * COMMANDS_INFO_LIST:    Runs the list command on a given file. ( This helps
 *                        get the absolute path to the source file.
 * COMMANDS_INFO_BREAKPOINTS: This will get all the current breakpoints.
 * COMMANDS_TTY:          sets the input/output of the program gdb is running
 *                        to the tty is given
 * COMMANDS_SET_PROMPT:   This tells readline what the new prompt is.
 * COMMANDS_REDISPLAY:    This forces readline to redisplay the current line.
 * COMMANDS_TAB_COMPLETION: This tab completes the current line.
 */
enum buffer_command_to_run {
   COMMANDS_INFO_SOURCES = 1,
   COMMANDS_INFO_LIST,
   COMMANDS_INFO_SOURCE_ABSOLUTE,
   COMMANDS_INFO_SOURCE_RELATIVE,
   COMMANDS_INFO_BREAKPOINTS,
   COMMANDS_TTY, 
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
};

void tgdb_delete_command(void *item);

/* tgdb_delete_command: Free's the memory from command com.
 *    
 *    NOTE: This functions MUST be called after tgdb_recv is called.  
 */
void tgdb_delete_commands(struct queue *q);

/* tgdb_append_command: This appends a new command onto the com structure.
 *
 * new_header is the typed of command being added.
 * buf, buf2 and buf3 are all concatinated in order with a space between to
 * the data section.
 *
 * Return: currently only returns 0 for success.
 */
int tgdb_append_command(struct queue *q, 
                        enum INTERFACE_COMMANDS new_header, 
                        char *buf, char *buf2, char *buf3);

/* tgdb_traverse_command: Traverses com and outputs data to fd. 
 *    This is mainly used for debugging information.
 */
void tgdb_traverse_command(struct queue *q);

#ifdef __cplusplus
}
#endif

#endif
