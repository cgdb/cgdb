#ifndef __TGDB_TYPES_H__
#define __TGDB_TYPES_H__

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
 *      This is a struct tgdb_source_file representing the absolute filename.
 *
 *  TGDB_ABSOLUTE_SOURCE_DENIED
 *  ---------------------------
 *
 *      This is a response to the function call tgdb_get_source_absolute_filename.
 *      It happens when gdb failed to know what the absolute path to the relative
 *      path asked for was.
 *
 *      This is a struct tgdb_source_file representing the absolute filename.
 *
 *  TGDB_DISPLAY_UPDATE
 *  -------------------
 *
 *      Currently not used.
 *
 *  TGDB_QUIT
 *  ---------
 *
 * 		This happens when gdb quits.
 *      libtgdb is done. 
 *      You will get no more responses after this one.
 *
 *      This is a 'struct tgdb_quit_status *'
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

/*
 * A single TGDB command for the front end.
 *
 * header
 *    This is the type of command.
 *
 * data
 *    This is a particular structure, based off of header.
 */
struct tgdb_command {
   enum INTERFACE_COMMANDS header;
   void *data;
};

/*
 * This will print a command to stderr.
 * It is currently used for debugging purposes.
 */
void tgdb_types_print_command ( void *command );

/*
 * This will free a command.
 */
void tgdb_types_free_command ( void *command );

/* 
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

/* 
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

/* 
 * This is used to return a path to the front end.
 *
 * path
 *    The absolute path to the file of interest.
 */
struct tgdb_source_file {
	char *absolute_path;
};

/**
 * 
 * This tells the front end how the debugger terminated.
 *
 * \field exit_status
 *     If this is 0, the debugger terminated normally and return_value is valid
 *     If this is -1, the debugger terminated abnormally and return_value is 
 *     invalid
 *
 * \field return_value
 *     This is the return value of the debugger upon normal termination.
 */
struct tgdb_debugger_exit_status {
	int exit_status;
	int return_value;
};

#ifdef __cplusplus
}
#endif

#endif /* __TGDB_TYPES_H__ */
