#ifndef __TYPES_H__
#define __TYPES_H__

/*
 * This interface is intended to declare and document the ADT's that TGDB 
 * exports to the front ends.
 *
 * The front end can interrogate these data structures to discover what TGDB
 * knows about the debugger. This is currently the only way the front end gets
 * any information about the current debugging session.
 */

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

/* INTERFACE_COMMANDS
 * ------------------
 *
 *  This is the interface mechanism used between the gui/tgdb.
 *  When tgdb is responding to a request or when an event is being generated
 *  the gui will find out about it through one of these enums.
 *
 *  TGDB_UPDATE_BREAKPOINTS
 *  -----------------------
 *
 *      All breakpoints that are set.
 *
 *      This is a 'struct tgdb_list *'
 *          It contains a 'struct tgdb_breakpoint *' for each breakpoint.
 *
 *  TGDB_UPDATE_FILE_POSITION
 *  -------------------------
 *
 *      This tells the gui what filename/line number the debugger is on.
 *      It gets generated whenever it changes.
 *
 *		This is a 'struct tgdb_file_position *'
 *
 *  TGDB_UPDATE_SOURCE_FILES
 *  ------------------------
 *
 *      This returns a list of all the source files that make up the 
 *      inferior program.
 *
 * 	    This is a 'struct tgdb_list *'
 *          It contains a const char* for each filename. The filename may
 *          be represented as a relative or absolute path. 
 *
 *  TGDB_SOURCES_DENIED
 *  -------------------
 *
 *      This is a response to the tgdb_get_sources function call.
 *      If the sources can not be recieved you will get this response.
 *
 *          The data will be NULL.
 *
 *  TGDB_ABSOLUTE_SOURCE_ACCEPTED
 *  -----------------------------
 *      
 *      This is a response to the function call tgdb_get_source_absolute_filename.
 *      It returns the absolute path to the relative path asked for.
 *
 *      This is a struct string representing the absolute filename.
 *
 *  TGDB_ABSOLUTE_SOURCE_DENIED
 *  ---------------------------
 *
 *      This is a response to the function call tgdb_get_source_absolute_filename.
 *      It happens when gdb failed to know what the absolute path to the relative
 *      path asked for was.
 *
 *      This is a struct string representing the absolute filename.
 *
 *  TGDB_DISPLAY_UPDATE
 *  -------------------
 *
 *      Currently not used.
 *
 *  TGDB_QUIT_ABNORMAL
 *  ------------------
 *
 * 		This happens when gdb quits abnormally.
 *      libtgdb is done. 
 *      You will get no more responses after this one.
 *
 *  TGDB_QUIT_NORMAL
 *  ----------------
 *
 * 		This happens when gdb quits normally.
 *      libtgdb is done. 
 *      You will get no more responses after this one.
 */
enum INTERFACE_COMMANDS {       
    TGDB_UPDATE_BREAKPOINTS,
    TGDB_UPDATE_FILE_POSITION,
    TGDB_UPDATE_SOURCE_FILES,
    TGDB_SOURCES_DENIED,
    TGDB_ABSOLUTE_SOURCE_ACCEPTED,
    TGDB_ABSOLUTE_SOURCE_DENIED,
    TGDB_DISPLAY_UPDATE,
	TGDB_QUIT_ABNORMAL,
    TGDB_QUIT_NORMAL
};

struct tgdb_command {
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
void tgdb_traverse_command_queue (struct queue *q);

/* tgdb_command
 * ------------
 *
 *  The client can choose any of these as actions that libtgdb should take.
 */
enum tgdb_command_type {
	TGDB_CONTINUE = 0,
	TGDB_FINISH,
	TGDB_NEXT,
	TGDB_RUN,
	TGDB_STEP,
	TGDB_UP,
	TGDB_DOWN,
	TGDB_ERROR
};


/* TGDB_MODIFY_BREAKPOINT:
 *
 * This gives the client the ability to add or remove breakpoints.
 *
 * Currently, enable/disable are not supported.
 */
enum tgdb_breakpoint_action {
	TGDB_BREAKPOINT_ADD,
	TGDB_BREAKPOINT_DELETE,
	TGDB_BREAKPOINT_DISABLE,
	TGDB_BREAKPOINT_ENABLE
};

/* TGDB_BREAKPOINT
 * ---------------
 *
 * This structure represents a breakpoint.
 * Each field represents a portion of the breakpoint.
 *
 * file
 * ----
 *    This is the file that the breakpoint is set in. This path name can be
 *    relative.
 *
 * funcname
 * --------
 *    The name of the function the breakpoint is set at.
 *
 * line
 * ----
 *    The line number where the breakpoint is set.
 *
 * enabled
 * -------
 *    0 if it is not enabled or 1 if it is enabled.
 */
struct tgdb_breakpoint {
	char *file;
	char *funcname;
	int line;
	int enabled;
};

/* 
 * This structure currently represents a file position.
 *
 * absolute_path
 *     The absolute path to the file.
 *
 * relative_path
 *     The relative path to the file.
 *
 * line_number
 *     The line number in the file.
 */
struct tgdb_file_position {
	char *absolute_path;
	char *relative_path;
	int line_number;
};

#ifdef __cplusplus
}
#endif

#endif
