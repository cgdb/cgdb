#ifndef __TGDB_TYPES_H__
#define __TGDB_TYPES_H__

/*! 
 * \file
 * tgdb_types.h
 *
 * \brief
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

/**
 * This should be depricated and removed from TGDB.
 */
#define MAXLINE 4096

/**
 * This should be depricated and removed from TGDB.
 */
#ifndef TRUE
#define TRUE  1
#endif

/**
 * This should be depricated and removed from TGDB.
 */
#ifndef FALSE
#define FALSE 0
#endif

/**
 *  This is the commands interface used between the front end and TGDB.
 *  When TGDB is responding to a request or when an event is being generated
 *  the front end will find out about it through one of these enums.
 */
enum INTERFACE_COMMANDS {       

	/** 
	 * All breakpoints that are set.
	 * This is a 'struct tgdb_list *'.
	 * It contains a 'struct tgdb_breakpoint *' for each breakpoint.
	 */
    TGDB_UPDATE_BREAKPOINTS,

 	/**
	 * This tells the gui what filename/line number the debugger is on.
	 * It gets generated whenever it changes.
	 * This is a 'struct tgdb_file_position *'.
	 */
    TGDB_UPDATE_FILE_POSITION,

 	/**
	 * This returns a list of all the source files that make up the 
     * inferior program.
	 * This is a 'struct tgdb_list *'
	 * It contains a const char* for each filename. The filename may
	 * be represented as a relative or absolute path. 
	 */
    TGDB_UPDATE_SOURCE_FILES,

 	/**
	 * This is a response to the tgdb_get_sources function call.
     * If the sources can not be recieved you will get this response.
     * The data will be NULL.
     */
    TGDB_SOURCES_DENIED,

 	/** 
	 * This is a response to the function call tgdb_get_source_absolute_filename.
 	 * It returns the absolute path to the relative path asked for.
     * This is a struct tgdb_source_file representing the absolute filename.
     */
    TGDB_ABSOLUTE_SOURCE_ACCEPTED,

 	/** This is a response to the function call tgdb_get_source_absolute_filename.
  	 * It happens when gdb failed to know what the absolute path to the relative
 	 * path asked for was.
     * This is a struct tgdb_source_file representing the absolute filename.
     */
    TGDB_ABSOLUTE_SOURCE_DENIED,

 	/**      Currently not used. */
    TGDB_DISPLAY_UPDATE,

 	/**
	 * This happens when gdb quits.
     * libtgdb is done. 
     * You will get no more responses after this one.
     * This is a 'struct tgdb_quit_status *'
	 */
	TGDB_QUIT
};

/**
 * A single TGDB command for the front end.
 * This is the smallest unit of information that TGDB can return to the front 
 * end.
 */
struct tgdb_command {

 	/** 
 	 * This is the type of command.
 	 */
   enum INTERFACE_COMMANDS header;

 	/**
 	 * This is a particular structure, based off of the header.
	 */
   void *data;
};

/******************************************************************************/
/**
 * @name Utilitly commands to run on a command.
 * These functions take a 'struct tgdb_command' as a parameter.
 */
/******************************************************************************/

//@{

/**
 * This will print a command to stderr.
 * It is currently used for debugging purposes.
 *
 * \param command
 *     The command to print. It should be of type 'struct tgdb_command'
 */
void tgdb_types_print_command ( void *command );

/**
 * This will free a command.
 *
 * \param command
 *     The command to print. It should be of type 'struct tgdb_command'
 */
void tgdb_types_free_command ( void *command );

//@}

/**
 * The client can give any of these commands to TGDB through 
 * tgdb_run_debugger_command.
 */
enum tgdb_command_type {

	/** 
	 * This will instruct TGDB to tell the debugger to continue.
	 */
	TGDB_CONTINUE = 0,

	/** 
	 * This will instruct TGDB to tell the debugger to finish.
	 */
	TGDB_FINISH,

	/** 
	 * This will instruct TGDB to tell the debugger to go to the next 
	 * source level instruction.
	 */
	TGDB_NEXT,

	/** 
	 * This will instruct TGDB to tell the debugger to run the program again.
	 */
	TGDB_RUN,

	/** 
	 * This will instruct TGDB to tell the debugger to step.
	 */
	TGDB_STEP,

	/** 
	 * This will instruct TGDB to tell the debugger to go up a frame.
	 */
	TGDB_UP,

	/** 
	 * This will instruct TGDB to tell the debugger to go down a frame.
	 */
	TGDB_DOWN,

	/** 
	 * Hmmm. This is probably bad :). Actually, I can't remember why its here.
	 */
	TGDB_ERROR
};

/**
 * This gives the client the ability to add or remove breakpoints.
 * Currently, enable/disable are not supported.
 */
enum tgdb_breakpoint_action {

	/**
	 * Add a breakpoint.
	 */
	TGDB_BREAKPOINT_ADD,

	/**
	 * Delete a breakpoint.
	 */
	TGDB_BREAKPOINT_DELETE,

	/**
	 * Disable a breakpoint.
	 */
	TGDB_BREAKPOINT_DISABLE,

	/**
	 * Enable a breakpoint.
	 */
	TGDB_BREAKPOINT_ENABLE
};

/**
 * This structure represents a breakpoint.
 */
struct tgdb_breakpoint {

	/**
 	 * This is the file that the breakpoint is set in. This path name can be
 	 * relative.
	 */
	char *file;

	/**
 	 * The name of the function the breakpoint is set at.
	 */
	char *funcname;

	/**
 	 * The line number where the breakpoint is set.
	 */
	int line;

	/**
 	 * 0 if it is not enabled or 1 if it is enabled.
	 */
	int enabled;
};

/**
 * This structure currently represents a file position.
 */
struct tgdb_file_position {

	/**
 	 * The absolute path to the file.
	 */
	char *absolute_path;

	/**
 	 * The relative path to the file.
	 */
	char *relative_path;

	/**
 	 * The line number in the file.
	 */
	int line_number;
};

/**
 * This is used to return a path to the front end.
 */
struct tgdb_source_file {
	/**
 	 * The absolute path to the file of interest.
	 */
	char *absolute_path;
};

/**
 * This tells the front end how the debugger terminated.
 */
struct tgdb_debugger_exit_status {

	/**
     * If this is 0, the debugger terminated normally and return_value is valid
 	 * If this is -1, the debugger terminated abnormally and return_value is 
 	 * invalid
	 */
	int exit_status;
	
	/**
     * This is the return value of the debugger upon normal termination.
	 */
	int return_value;
};

#ifdef __cplusplus
}
#endif

#endif /* __TGDB_TYPES_H__ */
