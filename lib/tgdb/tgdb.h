#ifndef __TGDB_H__
#define __TGDB_H__

/*! 
 * \file
 * tgdb.h
 *
 * \brief
 * This interface is intended to be the abstraction layer between a front end 
 * and the low level debugger the front end is trying to communicate with.
 */

#include "sys_util.h"
#include "tgdb_types.h"

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
   * This initializes a tgdb library instance. It starts up the debugger and 
   * returns all file descriptors the client must select on.
   *
   * The client must call this function before any other function in the 
   * tgdb library.
   *
   * \param debugger
   * The path to the desired debugger to use. If this is NULL, then just
   * "gdb" is used.
   *
   * \param argc
   * The number of arguments to pass to the debugger
   *
   * \param argv
   * The arguments to pass to the debugger    
   *
   * \param gdb_win_rows
   * The number of rows in the gdb console
   *
   * \param gdb_win_cols
   * The number of columns in the gdb console
   *
   * \param gdb_console_fd
   * The gdb console file descriptor
   *
   * \param gdb_mi_fd
   * The gdb machine interface file descriptor
   *
   * \param callbacks
   * Callback functions for event driven notifications
   *
   * @return
   * NULL on error, a valid context on success.
   */
    struct tgdb *tgdb_initialize(const char *debugger,
            int argc, char **argv, int gdb_win_rows, int gdb_win_cols,
            int *gdb_console_fd, int *gdb_mi_fd, tgdb_callbacks callbacks);

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
     * \param tgdb
     * An instance of the tgdb library to operate on.
     *
     * \param rows
     * The number of rows in the new gdb console
     *
     * \param cols
     * The number of columns in the new gdb console
     */
    int tgdb_resize(struct tgdb *tgdb, int rows, int cols);

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
