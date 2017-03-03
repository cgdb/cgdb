#ifndef __TGDB_TYPES_H__
#define __TGDB_TYPES_H__

#include <list>

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
    /** This will instruct TGDB to tell the debugger to (re-)start the program. */
        TGDB_START,
    /** This will instruct TGDB to tell the debugger to (re-)run the program. */
        TGDB_RUN,
    /** This will instruct TGDB to tell the debugger to kill the program. */
        TGDB_KILL,
    /** This will instruct TGDB to tell the debugger to step. */
        TGDB_STEP,
    /** 
     * This will instruct TGDB to tell the debugger to continue running
     * until a source line past the current line.  This is used to avoid
     * single stepping through loops.
     */
        TGDB_UNTIL,
    /** This will instruct TGDB to tell the debugger to go up a frame. */
        TGDB_UP,
    /** This will instruct TGDB to tell the debugger to go down a frame. */
        TGDB_DOWN,
    };

 /**
  * This gives the client the ability to add or remove breakpoints.
  * Currently, enable/disable are not supported.
  */
    enum tgdb_breakpoint_action {
    /** Add a breakpoint. */
        TGDB_BREAKPOINT_ADD,
    /** Add a temporary breakpoint */
        TGDB_TBREAKPOINT_ADD,
    /** Delete a breakpoint. */
        TGDB_BREAKPOINT_DELETE,
    };

 /** This structure represents a breakpoint. */
    struct tgdb_breakpoint {
        /**
         * The path to the file.
         *
         * This will usually be absolute. If the absolute path is not available
         * for GDB it will be a relative path.
         */
        char *path;
        /** The line number where the breakpoint is set. */
        int line;
        /** Line number corresponding to the $pc or 0 if unknown.  */
        uint64_t addr;
        /** 0 if it is not enabled or 1 if it is enabled. */
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

        /** Shared library where this function is defined or NULL if unknown. */
        char *from;

        /** Function name or NULL if unknown.  */
        char *func;
    };

    enum tgdb_request_type {
        /**
         * A command the user typed at the console.
         */
        TGDB_REQUEST_CONSOLE_COMMAND,

        /**
         * Get a list of all the source files in the program being debugged.
         */
        TGDB_REQUEST_INFO_SOURCES,

        /**
         * Determine the current location of the debugged program.
         *
         * This is a filename and line number for source code.
         */
        TGDB_REQUEST_INFO_SOURCE_FILE,

        /**
         * Get the list of existing breakpoints.
         */
        TGDB_REQUEST_BREAKPOINTS,

        /**
         * Set which terminal to use for future runs of program being debugged.
         *
         * This allows tgdb to separate the output of gdb from the output
         * of the program being debugged.
         */
        TGDB_REQUEST_TTY,

        /**
         * Get information about the current frame.
         *
         * Generally useful for finding the current location of the program
         * that is being debugged.
         */
        TGDB_REQUEST_INFO_FRAME,

        /**
         * Query if the CLI disassemble command supports mixed source+assembly.
         *
         * Mixed source+assembly mode was added as the /s flag to the CLI
         * disassemble command and as mode 4 to the MI -data-disassemble
         * command.
         *
         * We query the MI command to determine if it supports mode 4, and
         * if it does, we also know that the CLI disassemble command supports
         * /s.
         *
         * The passing case,
         *   (gdb) interpreter-exec mi "-data-disassemble -s 0 -e 0 -- 4"
         *   ^done,asm_insns=[]
         *
         * The failing case,
         *   (gdb) interpreter-exec mi "-data-disassemble -s 0 -e 0 -- 4"
         *   ^error,msg="-data-disassemble: Mode argument must be 0, 1, 2, or 3."
         *
         * If the command comes back as an MI error, we assume /s is not
         * supported.
         *
         * This functionality was added in gdb in commit 6ff0ba5f.
         */
        TGDB_REQUEST_DATA_DISASSEMBLE_MODE_QUERY,


        /**
         * Run a debugger command through gdb.
         *
         * This is when the caller wants to run a command through gdb,
         * next, step, finish, but without the user typing it at the
         * console. For instance, a cgdb shortcut like F8.
         */
        TGDB_REQUEST_DEBUGGER_COMMAND,

        /**
         * Request that a breakpoint be modified.
         *
         * Useful for creating, deleting and disabling breakpoints.
         */
        TGDB_REQUEST_MODIFY_BREAKPOINT,

        /**
         * Request GDB to give a list of tab completions for a given string
         *
         * This request is usually invoked when the user types <tab>
         * at the gdb console.
         */
        TGDB_REQUEST_COMPLETE,

        /**
         * Request GDB to disassemble the function surrounding the pc of the
         * selected frame.
         */
        TGDB_REQUEST_DISASSEMBLE_PC,

        /**
         * Request GDB to disassemble a function.
         */
        TGDB_REQUEST_DISASSEMBLE_FUNC
    };

    struct tgdb_request {
    /** This is the type of request.  */
        enum tgdb_request_type header;

        union {
            struct {
                /** The null terminated console command to pass to GDB */
                const char *command;
                /**
                 * Track if the request went into the request queue or not.
                 *
                 * True if request went into the queue, false if run
                 * immediately
                 */
                bool queued;
            } console_command;

            struct {
                const char *slavename;
            } tty_command;

            struct {
    /** This is the command that libtgdb should run through the debugger */
                enum tgdb_command_type c;
            } debugger_command;

            struct {
                /* The filename to set the breakpoint in */
                const char *file;
                /* The corresponding line number */
                int line;
                /* The address to set breakpoint in (if file is null). */
                uint64_t addr;
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
            } disassemble_func;
        } choice;
    };

    typedef struct tgdb_request *tgdb_request_ptr;
    typedef std::list<tgdb_request_ptr> tgdb_request_ptr_list;

 /**
  *  This is the commands interface used between the front end and TGDB.
  *  When TGDB is responding to a request or when an event is being generated
  *  the front end will find out about it through one of these enums.
  */
    enum tgdb_response_type {

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
        enum tgdb_response_type header;

        union {
            /* header == TGDB_UPDATE_BREAKPOINTS */
            struct {
                /* This list has elements of 'struct tgdb_breakpoint *' 
                 * representing each breakpoint. */
                struct tgdb_breakpoint *breakpoints;
            } update_breakpoints;

            /* header == TGDB_UPDATE_FILE_POSITION */
            struct {
                struct tgdb_file_position *file_position;
            } update_file_position;

            /* header == TGDB_UPDATE_SOURCE_FILES */
            struct {
                /* This list has elements of 'const char *' representing each 
                 * filename. The filename may be relative or absolute. */
                char **source_files;
            } update_source_files;

            /* header == TGDB_INFERIOR_EXITED */
            struct {
                int exit_status;
            } inferior_exited;

            /* header == TGDB_UPDATE_COMPLETIONS */
            struct {
                /* This sb array has elements of 'const char *'
                 * representing each possible completion. */
                char **completions;
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

            /* header == TGDB_QUIT */
            struct {
                /* Currently not telling the front end how GDB quit. */
            } quit;

        } choice;
    };

    struct tgdb_response *tgdb_create_response(enum tgdb_response_type header);
    void tgdb_send_response(struct tgdb *tgdb, struct tgdb_response *response);


#endif                          /* __TGDB_TYPES_H__ */
