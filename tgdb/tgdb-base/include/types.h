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

//enum INTERFACE_COMMANDS {
//   BREAKPOINTS_BEGIN,   /* starts a breakpoint session */
//   BREAKPOINT,          /* a single breakpoint */
//   BREAKPOINTS_END,     /* ends a breakpoint session */
//   SOURCE_FILE_UPDATE,  /* tells the gui the current source file */
//   CURRENT_FILE_UPDATE, /* tells the gui the current relative source file */
//   LINE_NUMBER_UPDATE,  /* tells the gui the current line number */
//   SOURCES_START,       /* marks the beggining of a list of source files */
//   SOURCE_FILE,         /* a source file */
//   SOURCES_END,         /* marks the end of a list of source files */
//   SOURCES_DENIED,      /* program not compiled with debug ( annotate error ) */
//   ABSOLUTE_SOURCE_ACCEPTED, /* Returns the absolute path of the file requested */
//   ABSOLUTE_SOURCE_DENIED,  /* gdb could not find the source file */
//   DISPLAY_UPDATE,      /* Updates the value of a variable */
//   QUIT
//};

/* INTERFACE_COMMANDS:
 * -------------------
 *
 *  This is the interface mechanism used between the gui/tgdb.
 *  When tgdb is responding to a request or when an event is being generated
 *  the gui will find out about it through one of these enums.
 *
 *  TGDB_UPDATE_BREAKPOINTS: 
 *  ------------------------
 *
 *      All breakpoints that are set.
 *
 *      This is a struct queue. 
 *          It containes a 'struct string' for each filename. The filename is
 *          represented with a relative path.
 *      
 *          The client should call string_free on this.
 *
 *  TGDB_UPDATE_FILE_POSITION:
 *  --------------------------
 *
 *      This tells the gui what filename/line number the debugger is on.
 *      It gets generated whenever it changes.
 *
 *      This is a struct queue. 
 *          First string is the absolute path
 *          Second string is the relative path
 *          Third int is the line number
 *
 *          The client should call string_clear on these when finished.
 *
 *  TGDB_UPDATE_SOURCE_FILES:
 *  -------------------------
 *
 *      This returns a list of all the source files that make up the 
 *      debugged program.
 *
 *      This is a struct queue. 
 *          It contains a char* from the help for each filename. The filename may
 *          be represented as a relative or absolute path. The client must free
 *          this memory.
 *
 *  TGDB_SOURCES_DENIED:
 *  --------------------
 *
 *      This is a response to the tgdb_get_sources function call.
 *      If the sources can not be recieved you will get this response.
 *
 *          The data will be NULL.
 *
 *  TGDB_ABSOLUTE_SOURCE_ACCEPTED:
 *  ------------------------------
 *      
 *      This is a response to the function call tgdb_get_source_absolute_filename.
 *      It returns the absolute path to the relative path asked for.
 *
 *      This is a struct string representing the absolute filename.
 *      The client should call string_free on this.
 *
 *  TGDB_ABSOLUTE_SOURCE_DENIED:
 *  ----------------------------
 *
 *      This is a response to the function call tgdb_get_source_absolute_filename.
 *      It happens when gdb failed to know what the absolute path to the relative
 *      path asked for was.
 *
 *      This is a struct string representing the absolute filename.
 *      The client should call string_free on this.
 *
 *  TGDB_DISPLAY_UPDATE:
 *  --------------------
 *
 *      Currently not used.
 *
 *  TGDB_QUIT:
 *  ----------
 *
 *      libtgdb is done. You will get no more responses after this one.
 */
enum INTERFACE_COMMANDS {       
    TGDB_UPDATE_BREAKPOINTS,
    TGDB_UPDATE_FILE_POSITION,
    TGDB_UPDATE_SOURCE_FILES,
    TGDB_SOURCES_DENIED,
    TGDB_ABSOLUTE_SOURCE_ACCEPTED,
    TGDB_ABSOLUTE_SOURCE_DENIED,
    TGDB_DISPLAY_UPDATE,
    TGDB_QUIT
};

struct Command {
   enum INTERFACE_COMMANDS header;
   void *data;
};

void tgdb_delete_command(void *item);

/* tgdb_delete_command: Free's the memory from command com.
 *    
 *    NOTE: This functions MUST be called after tgdb_recv is called.  
 */
void tgdb_delete_commands(struct queue *q);

/* tgdb_append_command: 
 * --------------------
 *
 *  This appends a new command onto the com structure.
 *
 *  q:              
 *  new_header:      
 *  ndata:          
 */
void tgdb_append_command(
            struct queue *q, 
            enum INTERFACE_COMMANDS new_header, 
            void *ndata);

/* tgdb_traverse_command: Traverses com and outputs data to fd. 
 *    This is mainly used for debugging information.
 */
void tgdb_traverse_command(struct queue *q);

#ifdef __cplusplus
}
#endif

#endif
