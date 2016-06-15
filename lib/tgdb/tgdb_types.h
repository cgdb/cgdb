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

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#include "tgdb_list.h"

/******************************************************************************/
/**
 * @name Utilitly commands to run on a command.
 * These functions take a 'struct tgdb_response' as a parameter.
 */
/******************************************************************************/

/*@{*/

 /**
  * This will print a client generated command to stderr.
  * These are the commands that are returned to the front end.
  * It is currently used for debugging purposes.
  *
  * \param command
  * The command to print. It should be of type 'struct tgdb_response'
  *
  * @return
  * Will return -1 if the print command failed. Otherwise, 0.
  */
    int tgdb_types_print_command(void *command);

 /**
  * This will free a client generated command.
  * These are the commands that are returned to the front end.
  *
  * \param command
  * The command to print. It should be of type 'struct tgdb_response'
  * 
  * @return
  * Will return -1 if free'ing failed. Otherwise, 0.
  */
    int tgdb_types_free_command(void *command);

 /**
  * This will append a new command into TGDB's queue.
  *
  * \param command_list
  * \param response
  */
    void tgdb_types_append_command(struct tgdb_list *command_list,
            struct tgdb_response *response);

/*@}*/

 /**
  * The client can give any of these commands to TGDB through 
  * tgdb_run_debugger_command.
  */
    enum tgdb_command_type {

    /** This will instruct TGDB to tell the debugger to continue.  */
        TGDB_CONTINUE = 0,

    /** This will instruct TGDB to tell the debugger to finish.  */
        TGDB_FINISH,

    /** 
     * This will instruct TGDB to tell the debugger to go to the next 
     * source level instruction.
     */
        TGDB_NEXT,

    /** 
     * This will instruct TGDB to tell the debugger to (re-)start the program. 
     */
        TGDB_START,

    /** 
     * This will instruct TGDB to tell the debugger to (re-)run the program.
     */
        TGDB_RUN,

    /** 
     * This will instruct TGDB to tell the debugger to kill the program.
     */
        TGDB_KILL,

    /** This will instruct TGDB to tell the debugger to step.  */
        TGDB_STEP,

    /** 
     * This will instruct TGDB to tell the debugger to continue running
     * until a source line past the current line.  This is used to avoid
     * single stepping through loops.
     */
        TGDB_UNTIL,

    /** This will instruct TGDB to tell the debugger to go up a frame.  */
        TGDB_UP,

    /** This will instruct TGDB to tell the debugger to go down a frame.  */
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

    /** Add a breakpoint.  */
        TGDB_BREAKPOINT_ADD,

    /** Add a temporary breakpoint */
        TGDB_TBREAKPOINT_ADD,

    /** Delete a breakpoint.  */
        TGDB_BREAKPOINT_DELETE,
    };

 /**
  * This structure represents a breakpoint.
  */
    struct tgdb_breakpoint {

    /** This is the file that the breakpoint is set in. */
        char *file;

    /** This is the fullname to the file that the breakpoint is set in. */
        char *fullname;

    /** The line number where the breakpoint is set.  */
        int line;

    /** 0 if it is not enabled or 1 if it is enabled.  */
        int enabled;
    };

 /**
  * This structure currently represents a file position.
  *
  * Either path or addr will be non-NULL. Never both.
  *
  * If the source location is available, path and line number will be valid.
  * If the source information can not be determined, the addr will be
  * available. It is possible they are both available.
  */
    struct tgdb_file_position {

    /**
     * The path to the file.
     *
     * This will usually be absolute. If the absolute path is not available
     * for GDB it will be a relative path.
     */
        char *path;

    /** The line number in the file.  */
        int line_number;

    /** Line number corresponding to the $pc or 0 if unknown.  */
        uint64_t addr;

    /** Shared library where this function is defined or NULL if unknown.  */
        char *from;

    /** Function name or NULL if unknown.  */
        char *func;
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

    enum INTERFACE_REQUEST_COMMANDS {
    /** Request for TGDB to run a console command through the debugger */
        TGDB_REQUEST_CONSOLE_COMMAND,
    /** 
     * Request for TGDB to get all of the source files that the debugger 
     * currently knows about the inferior. */
        TGDB_REQUEST_INFO_SOURCES,
    /**
     * This asks TGDB to determine the current fullname, filename and line 
     * number that the debugger is currently at, in the inferior. */
        TGDB_REQUEST_CURRENT_LOCATION,
    /** Run a debugger command (ie next, step, finish) */
        TGDB_REQUEST_DEBUGGER_COMMAND,
    /** Modify a breakpoint (ie delete/create/disable) */
        TGDB_REQUEST_MODIFY_BREAKPOINT,
    /** Ask GDB to give a list of tab completions for a given string */
        TGDB_REQUEST_COMPLETE,
    /** Ask GDB to disassemble a $pc */
        TGDB_REQUEST_DISASSEMBLE_PC,
    /** Ask GDB to disassemble a function */
        TGDB_REQUEST_DISASSEMBLE_FUNC
    };

    struct tgdb_request {
    /** This is the type of request.  */
        enum INTERFACE_REQUEST_COMMANDS header;

        union {
            struct {
    /** The null terminated console command to pass to GDB */
                const char *command;
            } console_command;

            struct {
    /** This is the command that libtgdb should run through the debugger */
                enum tgdb_command_type c;
            } debugger_command;

            struct {
                /* The filename to set the breakpoint in */
                const char *file;
                /* The corresponding line number */
                int line;
                /* The action to take */
                enum tgdb_breakpoint_action b;
            } modify_breakpoint;

            struct {
                /* The line to ask GDB for completions for */
                const char *line;
            } complete;

            struct {
                int lines;
            } disassemble;

            struct {
                int source;
                int raw;
                const char *file;
                const char *function;
            } disassemble_func;
        } choice;
    };

    typedef struct tgdb_request *tgdb_request_ptr;

 /**
  *  This is the commands interface used between the front end and TGDB.
  *  When TGDB is responding to a request or when an event is being generated
  *  the front end will find out about it through one of these enums.
  */
    enum INTERFACE_RESPONSE_COMMANDS {

    /** All breakpoints that are set.  */
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
     */
        TGDB_UPDATE_SOURCE_FILES,

    /**
     * This happens when the program being debugged by GDB exits. 
     * It can be used be the front end to clear any cache that it might have
     * obtained by debugging the last program. The data represents the exit
     * status.
     */
        TGDB_INFERIOR_EXITED,

    /**
     * This returns a list of all the completions.
     *
     */
        TGDB_UPDATE_COMPLETIONS,

    /**
     * Disassemble $pc output
     *
     */
        TGDB_DISASSEMBLE_PC,

    /**
     * Disassemble function output
     *
     */
        TGDB_DISASSEMBLE_FUNC,

    /** The prompt has changed, here is the new value.  */
        TGDB_UPDATE_CONSOLE_PROMPT_VALUE,

    /** A debugger command was run */
    TGDB_DEBUGGER_COMMAND_DELIVERED,

    /**
     * This happens when gdb quits.
     * libtgdb is done. 
     * You will get no more responses after this one.
     * This is a 'struct tgdb_quit_status *'
     */
        TGDB_QUIT
    };

 /**
  * A single TGDB response for the front end.
  * This is the smallest unit of information that TGDB can return to the front 
  * end.
  */
    struct tgdb_response {
    /** This is the type of response.  */
        enum INTERFACE_RESPONSE_COMMANDS header;

        union {
            /* header == TGDB_UPDATE_BREAKPOINTS */
            struct {
                /* This list has elements of 'struct tgdb_breakpoint *' 
                 * representing each breakpoint. */
                struct tgdb_list *breakpoint_list;
            } update_breakpoints;

            /* header == TGDB_UPDATE_FILE_POSITION */
            struct {
                struct tgdb_file_position *file_position;
            } update_file_position;

            /* header == TGDB_UPDATE_SOURCE_FILES */
            struct {
                /* This list has elements of 'const char *' representing each 
                 * filename. The filename may be relative or absolute. */
                struct tgdb_list *source_files;
            } update_source_files;

            /* header == TGDB_INFERIOR_EXITED */
            struct {
                int exit_status;
            } inferior_exited;

            /* header == TGDB_UPDATE_COMPLETIONS */
            struct {
                /* This list has elements of 'const char *' 
                 * representing each possible completion. */
                struct tgdb_list *completion_list;
            } update_completions;

            /* header == TGDB_DISASSEMBLE_FUNC */
            struct {
                uint64_t addr_start;
                uint64_t addr_end;
                int error;
                char **disasm;
            } disassemble_function;

            /* header == TGDB_UPDATE_CONSOLE_PROMPT_VALUE */
            struct {
                /* The new prompt GDB has reported */
                const char *prompt_value;
            } update_console_prompt_value;

            /* header == TGDB_DEBUGGER_COMMAND_DELIVERED */
            struct {
                /**
                 * This will be 1 if it is a debugger command. That is a
                 * command that was issued through 
                 *   tgdb_request_run_debugger_command
                 * This will be 0 if it is a tgdb command. That is a command
                 * that cgdb did not request directly, but tgdb required to
                 * run it to update the state of cgdb. For instance,
                 * getting the breakpoints or the currently active line.
                 */
                int debugger_command;

                /* The debugger command */
                const char *command;
            } debugger_command_delivered;

            /* header == TGDB_QUIT */
            struct {
                struct tgdb_debugger_exit_status *exit_status;
            } quit;

        } choice;
    };

#endif                          /* __TGDB_TYPES_H__ */
