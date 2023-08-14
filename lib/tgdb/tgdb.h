#ifndef __TGDB_H__
#define __TGDB_H__

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

/*! 
 * \file
 * tgdb.h
 *
 * \brief
 * This interface is intended to be the abstraction layer between a front end 
 * and the low level debugger the front end is trying to communicate with.
 */

#include "sys_util.h"

/* Basic interface types {{{  */

    // The client can give any of these commands to TGDB through 
    // tgdb_run_debugger_command.
    enum tgdb_command_type {
        TGDB_CONTINUE = 0,
        TGDB_FINISH,
        TGDB_NEXT,
        TGDB_NEXTI,
        TGDB_START,
        TGDB_RUN,
        TGDB_KILL,
        TGDB_STEP,
        TGDB_STEPI,
        TGDB_UNTIL,
        TGDB_UP,
        TGDB_DOWN,
    };

    // This gives the client the ability to add or remove breakpoints.
    // Currently, enable/disable are not supported.
    enum tgdb_breakpoint_action {
        // Add a breakpoint
        TGDB_BREAKPOINT_ADD,
        // Add a temporary breakpoint
        TGDB_TBREAKPOINT_ADD,
        // Delete a breakpoint
        TGDB_BREAKPOINT_DELETE,
    };

    // This structure represents a breakpoint
    struct tgdb_breakpoint {
         // The path to the file.
         //
         // This will usually be absolute. If the absolute path is not
         // available for GDB it will be a relative path
        char *path;

        // The line number where the breakpoint is set
        int line;

        // Line number corresponding to the $pc or 0 if unknown
        uint64_t addr;

        //0 if it is not enabled or 1 if it is enabled
        int enabled;
    };

    // This structure currently represents a file position.
    //
    // Either path or addr will be non-NULL. Never both.
    //
    // If the source location is available, path and line number will be valid.
    // If the source information can not be determined, the addr will be
    // available. It is possible they are both available.
    struct tgdb_file_position {
         // The path to the file.
         //
         // This will usually be absolute. If the absolute path is not
         // available for GDB it will be a relative path.
         //
         // Will be NULL if the source information is not available
        char *path;

        // The line number in the file or 0 if unknown
        int line_number;

        // Line number corresponding to the $pc or 0 if unknown
        uint64_t addr;

        // Shared library where this function is defined or NULL if unknown
        char *from;

        // Function name or NULL if unknown
        char *func;
    };

    enum tgdb_request_type {
        // Get a list of all the source files in the program being debugged
        TGDB_REQUEST_INFO_SOURCES,

         // Determine the current location of the debugged program.
         // 
         // This is a filename and line number for source code.
        TGDB_REQUEST_INFO_SOURCE_FILE,

        // Get the list of existing breakpoints.
        TGDB_REQUEST_BREAKPOINTS,

        // Set which terminal to use for future runs of program being debugged.
        // 
        // This allows tgdb to separate the output of gdb from the output
        // of the program being debugged.
        TGDB_REQUEST_TTY,

        // Get information about the current frame.
        //
        // Generally useful for finding the current location of the program
        // that is being debugged.
        TGDB_REQUEST_INFO_FRAME,

        // Query if the CLI disassemble command supports mixed source+assembly.
        // 
        // Mixed source+assembly mode was added as the /s flag to the CLI
        // disassemble command and as mode 4 to the MI -data-disassemble
        // command.
        //
        // We query the MI command to determine if it supports mode 4, and
        // if it does, we also know that the CLI disassemble command supports
        // /s.
        // 
        // The passing case,
        //   (gdb) interpreter-exec mi "-data-disassemble -s 0 -e 0 -- 4"
        //   ^done,asm_insns=[]
        // 
        // The failing case,
        //   (gdb) interpreter-exec mi "-data-disassemble -s 0 -e 0 -- 4"
        //   ^error,msg="-data-disassemble: Mode argument must
        //   be 0, 1, 2, or 3."
        //
        // If the command comes back as an MI error, we assume /s is not
        // supported.
        // 
        // This functionality was added in gdb in commit 6ff0ba5f.
        TGDB_REQUEST_DATA_DISASSEMBLE_MODE_QUERY,

        // Run a debugger command through gdb.
        // 
        // This is when the caller wants to run a command through gdb,
        // next, step, finish, but without the user typing it at the
        // console. For instance, a cgdb shortcut like F8.
        TGDB_REQUEST_DEBUGGER_COMMAND,

        // Request that a breakpoint be modified.
        //
        // Useful for creating, deleting and disabling breakpoints.
        TGDB_REQUEST_MODIFY_BREAKPOINT,

        // Request GDB to disassemble the function surrounding the pc of the
        // selected frame.
        TGDB_REQUEST_DISASSEMBLE_PC,

        // Request GDB to disassemble a function.
        TGDB_REQUEST_DISASSEMBLE_FUNC,

        // Request GDB to skip to the given line.
        TGDB_REQUEST_UNTIL_LINE
    };

    // This is the commands interface used between the front end and TGDB.
    // When TGDB is responding to a request or when an event is being generated
    // the front end will find out about it through one of these enums.
    enum tgdb_response_type {

        // All breakpoints that are set
        TGDB_UPDATE_BREAKPOINTS,

        // This tells the gui what filename/line number the debugger is on.
        // It gets generated whenever it changes.
        // This is a 'struct tgdb_file_position *'.
        TGDB_UPDATE_FILE_POSITION,

        // This returns a list of all the source files that make up the 
        // inferior program.
        TGDB_UPDATE_SOURCE_FILES,

        // Disassemble $pc output
        TGDB_DISASSEMBLE_PC,

        // Disassemble function output
        TGDB_DISASSEMBLE_FUNC,

        // This happens when gdb quits.
        // You will get no more responses after this one.
        // This is a 'struct tgdb_quit_status *'
        TGDB_QUIT
    };

    // A single TGDB response for the front end.
    struct tgdb_response {
        // This is the type of response
        enum tgdb_response_type header;

        union {
            // header == TGDB_UPDATE_BREAKPOINTS
            struct {
                // This list has elements of 'struct tgdb_breakpoint *' 
                // representing each breakpoint
                struct tgdb_breakpoint *breakpoints;
            } update_breakpoints;

            // header == TGDB_UPDATE_FILE_POSITION
            struct {
                struct tgdb_file_position *file_position;
            } update_file_position;

            // header == TGDB_UPDATE_SOURCE_FILES
            struct {
                // This list has elements of 'const char *' representing each 
                // filename. The filename may be relative or absolute.
                char **source_files;
            } update_source_files;

            // header == TGDB_INFERIOR_EXITED
            struct {
                int exit_status;
            } inferior_exited;

            // header == TGDB_DISASSEMBLE_FUNC
            struct {
                uint64_t addr_start;
                uint64_t addr_end;
                int error;
                char **disasm;
            } disassemble_function;

            // header == TGDB_QUIT
            struct {
                // If the GDB being used is pre new-ui, before GDB 7.12
                // then it is unsupported, and this will be set to true.
                // Otherwise, this will be set to false.
                bool new_ui_unsupported;
            } quit;

        } choice;
    };

/* }}} */

/* Creating and Destroying a libtgdb context. {{{*/
/******************************************************************************/
/**
 * @name Creating and Destroying a libtgdb context.
 * These functions are for createing and destroying a tgdb context.
 */
/******************************************************************************/

/*@{*/

  /**
   *  This struct is a reference to a libtgdb instance.
   */
    struct tgdb;


    struct tgdb_callbacks {
        /**
         * An arbitrary pointer to associate with the callbacks.
         */
        void *context;

        /**
         * Output is available for the console.
         *
         * @param context
         * The context pointer
         *
         * @param str
         * The console output
         */
        void (*console_output_callback)(void *context, const std::string &str);

        /**
         * A command response is available for consumption.
         *
         * @param context
         * The tgdb instance to operate on
         *
         * @param response
         * The response to consume. This response is only valid for use
         * during this callback function. It is freed by tgdb afterwards.
         */
        void (*command_response_callback)(void *context,
                struct tgdb_response *response);
    };

  /**
   * This initializes a tgdb library instance.
   *
   * \param callbacks
   * Callback functions for event driven notifications
   *
   * @return
   * NULL on error, a valid context on success.
   */
    struct tgdb *tgdb_initialize(tgdb_callbacks callbacks);

  /**
   * This will terminate a libtgdb session. No functions should be called on
   * the tgdb context passed into this function after this call.
   *
   * \param tgdb
   * An instance of the tgdb library to operate on.
   *
   * @return
   * 0 on success or -1 on error
   */
    int tgdb_shutdown(struct tgdb *tgdb);

    /* Close tgdb logfiles. This should happen after tgdb_shutdown() and all
     * other shutdown which might use logfiles. Right before exit() works great.
     */
    void tgdb_close_logfiles();

/*@}*/
/* }}}*/

/* Input/Output commands {{{*/
/******************************************************************************/
/**
 * @name Input/Output commands
 * These functions are for communicating I/O with the tgdb context.
 */
/******************************************************************************/

/*@{*/

    // Start the debugger
    // 
    // Returns all file descriptors the client must select on.
    //
    // @param tgdb
    // An instance of the tgdb library to operate on.
    //
    // @param debugger
    // The path to the desired debugger to use.
    // If this is NULL, then just "gdb" is used.
    //
    // @param argc
    // The number of arguments to pass to the debugger
    //
    // @param argv
    // The arguments to pass to the debugger    
    //
    // @param gdb_win_rows
    // The number of rows in the gdb console
    //
    // @param gdb_win_cols
    // The number of columns in the gdb console
    //
    // @param gdb_console_fd
    // The gdb console file descriptor
    //
    // @param gdb_mi_fd
    // The gdb machine interface file descriptor
    int tgdb_start_gdb(struct tgdb *tgdb,
            const char *debugger, int argc, char **argv,
            int gdb_win_rows, int gdb_win_cols, int *gdb_console_fd,
            int *gdb_mi_fd);

  /**
   * This function does most of the dirty work in TGDB. It is capable of 
   * processing the output of the debugger, to either satisfy a previously 
   * made request, or to simply get console output for the caller to have.
   *
   * The data returned from this function is the console output of the 
   * debugger.
   * 
   * \param tgdb
   * An instance of the tgdb library to operate on.
   *
   * \param fd
   * The file descriptor that has input (either the console or mi).
   *
   * @return
   * 0 on sucess, or -1 on error
   */
    int tgdb_process(struct tgdb *tgdb, int fd);

    /**
     * Send a character to the gdb console.
     *
     * \param tgdb
     * An instance of the tgdb library to operate on.
     *
     * \param c
     * The character to send to the gdb console
     *
     * \return
     * 0 on sucess, or -1 on error
     */
    int tgdb_send_char(struct tgdb *tgdb, char c);

    /**
     * Resize the gdb console.
     *
     * If tgdb_start_gdb has not been called yet, this function will be a
     * no-op.
     *
     * \param tgdb
     * An instance of the tgdb library to operate on.
     *
     * \param rows
     * The number of rows in the new gdb console
     *
     * \param cols
     * The number of columns in the new gdb console
     */
    int tgdb_resize_console(struct tgdb *tgdb, int rows, int cols);

/*@}*/
/* }}}*/

/* Functional commands {{{*/
/******************************************************************************/
/**
 * @name Functional commands
 * These functinos are used to ask the TGDB context to perform a task.
 */
/******************************************************************************/

/*@{*/

  /**
   * Gets a list of source files that make up the program being debugged.
   *
   * \param tgdb
   * An instance of the tgdb library to operate on.
   */
   void tgdb_request_inferiors_source_files(struct tgdb *tgdb);

  /**
   * Get the current location of the inferior.
   *
   * \param tgdb
   * An instance of the tgdb library to operate on.
   */
   void tgdb_request_current_location(struct tgdb *tgdb);

   /**
    * Request an update of the breakpoints to the front end.
    *
    * @param tgdb
    * An instance of the tgdb library to operate on.
    */
   void tgdb_request_breakpoints(struct tgdb *tgdb);

  /**
   * This tells libtgdb to run a command through the debugger.
   *
   * \param tgdb
   * An instance of the tgdb library to operate on.
   *
   * \param c
   * This is the command that libtgdb should run through the debugger.
   */
    void tgdb_request_run_debugger_command(struct tgdb *tgdb,
            enum tgdb_command_type c);

  /**
   * Modify's a breakpoint.
   *
   * \param tgdb
   * An instance of the tgdb library to operate on.
   *
   * \param file
   * The file to set the breakpoint in.
   *
   * \param line
   * The line in FILE to set the breakpoint in.
   *
   * \param b
   * Determines what the user wants to do with the breakpoint.
   *
   * @return
   * Will return as a tgdb request command on success, otherwise NULL.
   */
    void tgdb_request_modify_breakpoint(struct tgdb *tgdb,
        const char *file, int line, uint64_t addr,
        enum tgdb_breakpoint_action b);

    
    /**
     * Used to get the disassemble of the current $pc.
     *
     * \param tgdb
     * An instance of the tgdb library to operate on.
     *
     * \param lines
     * The number of lines to disassemble after the pc.
     */
    void tgdb_request_disassemble_pc(struct tgdb *tgdb, int lines);

   /**
    * Get disassembly for entire function.
    *
    * \param tgdb
    * An instance of the tgdb library to operate on.
    */
    enum disassemble_func_type {
        DISASSEMBLE_FUNC_DISASSEMBLY,
        DISASSEMBLE_FUNC_SOURCE_LINES,
        DISASSEMBLE_FUNC_RAW_INSTRUCTIONS,
    };
    void tgdb_request_disassemble_func(struct tgdb *tgdb,
            enum disassemble_func_type type);

    /**
     * Request GDB skip 'until' the given file/line or address. Uses the same
     * basic functionality as 'break'. Only file/line *OR* addr should be set.
     *
     * \param tgdb
     * An instance of the tgdb library to operate on.
     *
     * \param file
     * The file name of the line to skip to.
     *
     * \param line
     * The line to skip until.
     *
     * \param addr
     * The address ($pc) to skip until.
     */
    void tgdb_request_until_line(struct tgdb *tgdb,
            const char *file, int line, uint64_t addr);

/*@}*/
/* }}}*/

/* Signal Handling Support {{{*/
/******************************************************************************/
/**
 * @name Signal Handling Support
 * These functinos are used to notify TGDB of signal received.
 */
/******************************************************************************/

/*@{*/

  /**
   * The front end can use this function to notify libtgdb that an
   * asynchronous event has occurred. If signal SIGNUM is relavant
   * to libtgdb, the appropriate processing will be done.
   * Currently, TGDB only handles SIGINT,SIGTERM and SIGQUIT.
   *
   * libtgdb will remove all elements from it's queue when a SIGINT
   * is received.
   *
   * \param tgdb
   * An instance of the tgdb library to operate on.
   *
   * \param signum
   * The signal number SIGNUM that has occurred.
   *
   * @return
   * 0 on success or -1 on error
   */
    int tgdb_signal_notification(struct tgdb *tgdb, int signum);

/*@}*/
/* }}}*/

#endif                          /* __TGDB_H__ */
