#ifndef __TYPES_H__
#define __TYPES_H__

#ifdef __cplusplus
extern "C" {
#endif 

#define MAXLINE 4096

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
