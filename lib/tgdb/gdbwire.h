/**
 * This file is an amalgamation of the header files from gdbwire.
 *
 * It was created using gdbwire 1.0 and git revision 549b001.
 */

/***** Begin file gdbwire_sys.h **********************************************/
#ifndef __GDBWIRE_SYS_H__
#define __GDBWIRE_SYS_H__

/**
 * Supporting system functions.
 */

#ifdef __cplusplus 
extern "C" { 
#endif 

/**
 * Duplicate a string.
 *
 * @param str
 * The string to duplicate
 *
 * @return
 * An allocated string that must be freed.
 * Null if out of memory or str is NULL.
 */
char *gdbwire_strdup(const char *str);

#ifdef __cplusplus 
}
#endif 

#endif
/***** End of gdbwire_sys.h **************************************************/
/***** Begin file gdbwire_string.h *******************************************/
#ifndef __GDBWIRE_STRING_H__
#define __GDBWIRE_STRING_H__

#ifdef __cplusplus 
extern "C" { 
#endif 

#include <stdlib.h>

/**
 * A dynamic string representation.
 *
 * To create and destroy a string use gdbwire_string_create() and
 * gdbwire_string_destroy() respectively.
 *
 * This string is an abstraction of a low level C string. It supports being
 * used as a NULL terminated c string and also as an arbitrary array of
 * bytes. You can append to this string in either of these modes using
 * gdbwire_string_append_cstr() or gdbwire_string_append_data(). This string
 * automatically grows as you append data to it. Please note, the size of
 * the string will not include the NULL terminated character when using
 * the gdbwire_string_append_cstr() function to append data.
 *
 * To get access to the underlying bytes associated with this string
 * call gdbwire_string_data(). It is OK to modify the result as long as
 * you are careful to stay in it's valid bounds.
 *
 * The size (or length) of the string can be accessed through the
 * gdbwire_string_size() function. The character pointer returned from
 * gdbwire_string_data() is valid from the index range of 0 to
 * gdbwire_string_size() - 1.
 */
struct gdbwire_string;

/**
 * Create a string instance.
 *
 * @return
 * A valid string instance or NULL on error.
 */
struct gdbwire_string *gdbwire_string_create(void);

/**
 * Destroy the string instance and it's resources.
 *
 * @param string
 * The string to destroy.
 */
void gdbwire_string_destroy(struct gdbwire_string *string);

/**
 * Clear the contents of a string.
 *
 * Sets the string back to an empty string which also changes it's
 * size back to zero.
 *
 * The capacity remains unchanged.
 *
 * @param string
 * The string to clear
 */
void gdbwire_string_clear(struct gdbwire_string *string);

/**
 * Append a c string to the string instance.
 *
 * @param string
 * The string instance to append the c string to.
 *
 * @param cstr
 * The c string to append to the string instance.
 *
 * @return
 * 0 on success or -1 on failure.
 */
int gdbwire_string_append_cstr(struct gdbwire_string *string, const char *cstr);

/**
 * Append a sequence of bytes to the string instance.
 *
 * @param string
 * The string instance to append the sequence of bytes to.
 *
 * @param data
 * The sequence of bytes to append to the string instance. This may
 * contain NUL characters.
 *
 * @param size
 * The number of bytes in data to append to the string instance.
 *
 * @return
 * 0 on success or -1 on failure.
 */
int gdbwire_string_append_data(struct gdbwire_string *string,
        const char *data, size_t size);

/**
 * Get the data associated with this string.
 *
 * The data could be formatted as a NULL terminated C string or
 * as an arbitrary array of bytes. Use gdbwire_string_size() to
 * determine the size (or length) of the result of this function.
 * 
 * Modifying the return value of this function is acceptable as long as you
 * stay in the string's valid bounds.
 *
 * @param string
 * The string index to get the pointer data from.
 *
 * @return
 * The data that has been added to this string instance or "" after
 * creation or clear. The result is gdbwire_string_size() bytes long.
 */
char *gdbwire_string_data(struct gdbwire_string *string);

/**
 * Determine the size (the number of bytes) this string instance represents.
 *
 * Please note, the result of this function will not include the NULL
 * terminated character when using the gdbwire_string_append_cstr() function
 * to append data.
 *
 * @param string
 * The string instance to get the size for.
 *
 * @return
 * The number of bytes contained in this string instance. To access these
 * bytes see gdbwire_string_data(). Will be 0 after creation or clear.
 */
size_t gdbwire_string_size(struct gdbwire_string *string);

/**
 * Determine the maximum capacity (number of bytes) this string may hold.
 *
 * The max capacity of the string is automatically increased when data
 * is appended to this string through the gdbwire_string_append_*()
 * family of functions.
 *
 * @param string
 * The string to determine the capacity of.
 *
 * @return
 * The max number of bytes this string may hold.
 */
size_t gdbwire_string_capacity(struct gdbwire_string *string);

/**
 * Search for the first character in chars occuring in this string.
 *
 * @param string
 * The string to search for the characters in chars in.
 *
 * @param chars
 * A null terminated string of characters. This string is not searched
 * for directly but instead each individually character in the string
 * is searched for.
 *
 * @return
 * The index position of the first matched character in chars.
 * Will return gdbwire_string_size() if not found.
 */
size_t gdbwire_string_find_first_of(struct gdbwire_string *string,
        const char *chars);

/**
 * Erase characters from this string, reducing it's size.
 *
 * @param string
 * The string to erase characters from.
 *
 * @param pos
 * The index position of the first character to be erased.
 *
 * @param count
 * The number of characters to erase starting at position pos.
 * If count goes past the end of the string it is adjusted to erase
 * until the end of the string. This allows the caller to pass in
 * gdbwire_string_size() to erase the end of the string with out
 * doing index arithmetic.
 * 
 * @return
 * On success 0 will be returned otherwise -1. The string will remain
 * unmodified when an error occurs. Success can only occur if the entire
 * requested range can be erased.
 */
int gdbwire_string_erase(struct gdbwire_string *string, size_t pos,
        size_t count);

#ifdef __cplusplus 
}
#endif 

#endif
/***** End of gdbwire_string.h ***********************************************/
/***** Begin file gdbwire_assert.h *******************************************/
#ifndef GDBWIRE_ERROR_H
#define GDBWIRE_ERROR_H

/***** Include gdbwire_result.h in the middle of gdbwire_assert.h ************/
/***** Begin file gdbwire_result.h *******************************************/
#ifndef GDBWIRE_RESULT_H
#define GDBWIRE_RESULT_H

enum gdbwire_result {
    /* The result of the operation was successful */
    GDBWIRE_OK,

    /**
     * An assertion failed in the calling code.
     *
     * Functions are encouraged to assert expressions they expect
     * to be true. The macro GDBWIRE_ASSERT and GDBWIRE_ASSERT_ERRNO
     * are useful for asserting expressions, and upon failure, to
     * automatically log the assertion expression and return
     * this result status.
     */
    GDBWIRE_ASSERT,

    /**
     * An internal logic error has occurred.
     *
     * In general, this should be used when a function can no
     * longer carry out it's contract and must abort.
     *
     * This happens, for instance, when a called function returns
     * an error status, or when invalid input was provided, etc.
     */
    GDBWIRE_LOGIC,

    /**
     * The system is out of memory.
     *
     * Will occur when malloc, strdup, calloc, etc fail to allocate memory.
     */
    GDBWIRE_NOMEM
};

#endif /* GDBWIRE_RESULT_H */
/***** End of gdbwire_result.h ***********************************************/
/***** Continuing where we left off in gdbwire_assert.h **********************/
/***** Include gdbwire_logger.h in the middle of gdbwire_assert.h ************/
/***** Begin file gdbwire_logger.h *******************************************/
#ifndef __GDBWIRE_LOGGER_H__
#define __GDBWIRE_LOGGER_H__

/* #include "gdbwire_result.h" */

#ifdef __cplusplus 
extern "C" { 
#endif 

enum gdbwire_logger_level {
    GDBWIRE_LOGGER_DEBUG,
    GDBWIRE_LOGGER_INFO,
    GDBWIRE_LOGGER_WARN,
    GDBWIRE_LOGGER_ERROR
};

/**
 * Log a statement to the logger.
 *
 * This is typically not called directly. Use the below macros instead.
 * The macros automatically supply the file, line and level arguments.
 *
 * @param file
 * The filename the logger was invoked from.
 *
 * @param line
 * The line number the logger was invoked from.
 *
 * @param level
 * The level associated with the log message.
 *
 * @param fmt
 * The format string for the message (printf formatting).
 *
 * @param ...
 * Any additional format arguments.
 */
void gdbwire_logger_log(const char *file, int line,
        enum gdbwire_logger_level level, const char *fmt, ...);

/* The macros intended to be used for logging */
#define gdbwire_debug(fmt, ...)(gdbwire_logger_log(__FILE__, __LINE__, \
        GDBWIRE_LOGGER_DEBUG, fmt, ##__VA_ARGS__))
#define gdbwire_info(fmt, ...)(gdbwire_logger_log(__FILE__, __LINE__, \
        GDBWIRE_LOGGER_INFO, fmt, ##__VA_ARGS__))
#define gdbwire_warn(fmt, ...)(gdbwire_logger_log(__FILE__, __LINE__, \
        GDBWIRE_LOGGER_WARN, fmt, ##__VA_ARGS__))
#define gdbwire_error(fmt, ...)(gdbwire_logger_log(__FILE__, __LINE__, \
        GDBWIRE_LOGGER_ERROR, fmt, ##__VA_ARGS__))

#ifdef __cplusplus 
}
#endif 

#endif
/***** End of gdbwire_logger.h ***********************************************/
/***** Continuing where we left off in gdbwire_assert.h **********************/

/**
 * Validate that the expression evaluates to true.
 *
 * If the expression does not evaluate to true, log the error and
 * return a GDBWIRE_ASSERT status code.
 *
 * Otherwise, if the expression does evaluate to true, do nothing.
 *
 * @param expr
 * The expression to evaluate.
 */
#define GDBWIRE_ASSERT(expr) \
    do { \
        if (!(expr)) { \
            gdbwire_error("Assertion failure, expr[%s]", #expr); \
            return GDBWIRE_ASSERT; \
        } \
    } while (0)

/**
 * Validate that the expression evaluates to true.
 *
 * If the expression does not evaluate to true, log the error,
 * set the variable provided to GDBWIRE_ASSERT and goto the label
 * provided.
 *
 * Otherwise, if the expression does evaluate to true, do nothing.
 *
 * @param expr
 * The expression to evaluate.
 *
 * @param variable
 * The result variable to assign the value GDBWIRE_ASSERT to.
 *
 * @param label
 * The label to jump to if the expression evaluates to False.
 */
#define GDBWIRE_ASSERT_GOTO(expr, variable, label) \
    do { \
        if (!(expr)) { \
            gdbwire_error("Assertion failure, expr[%s], " \
                "label[%s]", #expr, #label); \
            variable = GDBWIRE_ASSERT; \
            goto label; \
        } \
    } while (0)

/**
 * Validate that the expression evaluates to true.
 *
 * This particular assertion macro is used when a system library
 * call fails and that library call has an associated errno status
 * to describe the failure reason.
 *
 * If the expression does not evaluate to true, log the error,
 * along with the errno value and message and return a GDBWIRE_ASSERT
 * status code.
 *
 * Otherwise, if the expression does evaluate to true, do nothing.
 *
 * @param expr
 * The expression to evaluate.
 */
#define GDBWIRE_ASSERT_ERRNO(expr) \
    do { \
        if (!(expr)) { \
            gdbwire_error("Assertion failure, expr[%s]," \
                "errno[%d], strerror[%s]", \
                #expr, errno, strerror(errno)); \
            return GDBWIRE_ASSERT; \
        } \
    } while (0)

#endif /* GDBWIRE_ERROR_H */
/***** End of gdbwire_assert.h ***********************************************/
/***** Begin file gdbwire_result.h *******************************************/
#ifndef GDBWIRE_RESULT_H
#define GDBWIRE_RESULT_H

enum gdbwire_result {
    /* The result of the operation was successful */
    GDBWIRE_OK,

    /**
     * An assertion failed in the calling code.
     *
     * Functions are encouraged to assert expressions they expect
     * to be true. The macro GDBWIRE_ASSERT and GDBWIRE_ASSERT_ERRNO
     * are useful for asserting expressions, and upon failure, to
     * automatically log the assertion expression and return
     * this result status.
     */
    GDBWIRE_ASSERT,

    /**
     * An internal logic error has occurred.
     *
     * In general, this should be used when a function can no
     * longer carry out it's contract and must abort.
     *
     * This happens, for instance, when a called function returns
     * an error status, or when invalid input was provided, etc.
     */
    GDBWIRE_LOGIC,

    /**
     * The system is out of memory.
     *
     * Will occur when malloc, strdup, calloc, etc fail to allocate memory.
     */
    GDBWIRE_NOMEM
};

#endif /* GDBWIRE_RESULT_H */
/***** End of gdbwire_result.h ***********************************************/
/***** Begin file gdbwire_logger.h *******************************************/
#ifndef __GDBWIRE_LOGGER_H__
#define __GDBWIRE_LOGGER_H__

/* #include "gdbwire_result.h" */

#ifdef __cplusplus 
extern "C" { 
#endif 

enum gdbwire_logger_level {
    GDBWIRE_LOGGER_DEBUG,
    GDBWIRE_LOGGER_INFO,
    GDBWIRE_LOGGER_WARN,
    GDBWIRE_LOGGER_ERROR
};

/**
 * Log a statement to the logger.
 *
 * This is typically not called directly. Use the below macros instead.
 * The macros automatically supply the file, line and level arguments.
 *
 * @param file
 * The filename the logger was invoked from.
 *
 * @param line
 * The line number the logger was invoked from.
 *
 * @param level
 * The level associated with the log message.
 *
 * @param fmt
 * The format string for the message (printf formatting).
 *
 * @param ...
 * Any additional format arguments.
 */
void gdbwire_logger_log(const char *file, int line,
        enum gdbwire_logger_level level, const char *fmt, ...);

/* The macros intended to be used for logging */
#define gdbwire_debug(fmt, ...)(gdbwire_logger_log(__FILE__, __LINE__, \
        GDBWIRE_LOGGER_DEBUG, fmt, ##__VA_ARGS__))
#define gdbwire_info(fmt, ...)(gdbwire_logger_log(__FILE__, __LINE__, \
        GDBWIRE_LOGGER_INFO, fmt, ##__VA_ARGS__))
#define gdbwire_warn(fmt, ...)(gdbwire_logger_log(__FILE__, __LINE__, \
        GDBWIRE_LOGGER_WARN, fmt, ##__VA_ARGS__))
#define gdbwire_error(fmt, ...)(gdbwire_logger_log(__FILE__, __LINE__, \
        GDBWIRE_LOGGER_ERROR, fmt, ##__VA_ARGS__))

#ifdef __cplusplus 
}
#endif 

#endif
/***** End of gdbwire_logger.h ***********************************************/
/***** Begin file gdbwire_mi_pt.h ********************************************/
#ifndef GDBWIRE_MI_PT_H
#define GDBWIRE_MI_PT_H

#ifdef __cplusplus 
extern "C" { 
#endif 

/**
 * The position of a token in a GDB/MI line.
 *
 * Note that a string in C is zero based and the token column
 * position is 1 based. For example,
 *   char *str = "hello world";
 * The "hello" token would have a start_column as 1 and an end
 * column as 5.
 *
 * The start_column and end_column will be the same column number for
 * a token of size 1.
 */
struct gdbwire_mi_position {
    /* The starting column position of the token */
    int start_column;
    /* The ending column position of the token */
    int end_column;
};

/** The gdbwire_mi output kinds. */
enum gdbwire_mi_output_kind {
    /**
     * The GDB/MI output contains an out of band record.
     *
     * The out of band record is not necessarily associated with any
     * particular GDB/MI input command.
     */
    GDBWIRE_MI_OUTPUT_OOB,

    /**
     * The GDB/MI output contains a gdbwire_mi result record.
     *
     * This record typically contains the result data from a request
     * made by the client in a previous GDB/MI input command.
     */
    GDBWIRE_MI_OUTPUT_RESULT,

    /**
     * The GDB/MI output represents a prompt. (ie. (gdb) )
     * 
     * TODO: Document when GDB is ready to receive a command. Only if
     * the prompt is received and at *stopped?
     */
    GDBWIRE_MI_OUTPUT_PROMPT,

    /**
     * A parse error occurred.
     */
    GDBWIRE_MI_OUTPUT_PARSE_ERROR
};

/**
 * The GDB/MI output command.
 *
 * A GDB/MI output command is the main mechanism in which GDB
 * corresponds with a front end.
 */
struct gdbwire_mi_output {
    enum gdbwire_mi_output_kind kind;

    union {
        /** When kind == GDBWIRE_MI_OUTPUT_OOB, never NULL. */
        struct gdbwire_mi_oob_record *oob_record;
        /** When kind == GDBWIRE_MI_OUTPUT_RESULT, never NULL. */
        struct gdbwire_mi_result_record *result_record;
        /** When kind == GDBWIRE_MI_OUTPUT_PARSE_ERROR, never NULL. */
        struct {
            /** The token the error occurred on */
            char *token;
            /** The position of the token where the error occurred. */
            struct gdbwire_mi_position pos;
        } error;
    } variant;

    /**
     * The GDB/MI output line that was used to create this output instance.
     *
     * Each gdbwire_mi output structure is created from exactly one line of
     * MI output from GDB. This field represents the line that created 
     * this particular output structure.
     *
     * This field is always available and never NULL, even for a parse error.
     */
    char *line;

    /** The next GDB/MI output command or NULL if none */
    struct gdbwire_mi_output *next;
};

/**
 * A GDB/MI token.
 *
 * A string made up of one or more digits.
 * The regular expression [0-9]+ will match this types contents.
 */
typedef char *gdbwire_mi_token_t;

/**
 * A GDB/MI output command may contain one of the following result indications.
 */
enum gdbwire_mi_result_class {
    /**
     * The synchronous operation was successful (^done).
     */
    GDBWIRE_MI_DONE,

    /**
     * Equivalent to GDBWIRE_MI_DONE (^running).
     *
     * Historically, was output by GDB instead of ^done if the command
     * resumed the target.
     *
     * Do not rely on or use this result class in the front end to determine
     * the state of the target. Use the async *running output record to
     * determine which threads have resumed running.
     *
     * TODO: Ensure that early versions of GDB can depend on the async
     * *running or if front ends DO have to rely on ^running.
     */
    GDBWIRE_MI_RUNNING,

    /**
     * GDB has connected to a remote target (^connected).
     *
     * This is in response to the -target-select command.
     *
     * A comment in the GDB source code says,
     *   There's no particularly good reason why target-connect results
     *   in not ^done.  Should kill ^connected for MI3.
     *
     * With this in mind, it makes sense to assume that GDBWIRE_MI_CONNECTED
     * and GDBWIRE_MI_DONE are equivalent.
     */
    GDBWIRE_MI_CONNECTED,

    /**
     * An error has occurred (^error).
     *
     * This can occur if the user provides an improper command to GDB.
     * In this case, the user will be provided the standard error output but
     * the front end will also be provided this information independently.
     */
    GDBWIRE_MI_ERROR,

    /**
     * GDB has terminated (^exit).
     *
     * When GDB knows it is about to exit, it provides this notification
     * in the GDB/MI output command. However, on all other circumstances,
     * the front end should be prepared to have GDB exit and not provide
     * this information.
     */
    GDBWIRE_MI_EXIT,

    /* An unsupported result class */
    GDBWIRE_MI_UNSUPPORTED
};

/**
 * The GDB/MI result record in an output command.
 *
 * The result record represents the result data in the GDB/MI output
 * command sent by GDB. This typically contains the content the client
 * was requesting when it sent a GDB/MI input command to GDB.
 */
struct gdbwire_mi_result_record {
    /**
     * The token associated with the corresponding GDB/MI input command.
     *
     * The client may provide a unique string of digits at the beginning of a
     * GDB/MI input command. For example,
     *   0000-foo
     * When GDB finally gets around to responding to the GDB/MI input command,
     * it takes the token provided in the input command and puts it into the
     * result record of the corresponding GDB/MI output command. For
     * example, the output commmand associated with the above input command is,
     *   0000^error,msg="Undefined MI command: foo",code="undefined-command"
     * and the result record would have the below token field set to "0000".
     *
     * This is intended to allow the front end to correlate the GDB/MI input
     * command it sent with the GDB/MI output command GDB responded with.
     *
     * This represents the token value the front end provided to the
     * corresponding GDB/MI input command or NULL if no token was provided.
     */
    gdbwire_mi_token_t token;

    /** The result records result class. */
    enum gdbwire_mi_result_class result_class;

    /**
     * An optional list of results for this result record.
     *
     * Will be NULL if there is no results for this result record.
     *
     * This is typically where the result data is that the client
     * is looking for.
     */
    struct gdbwire_mi_result *result;
};

/** The out of band record kinds. */
enum gdbwire_mi_oob_record_kind {
    /**
     * An asyncronous out of band record.
     *
     * An asyncronous record occurs when GDB would like to update the
     * client with information that it has not asked for.
     *
     * For instance, if the inferior has stopped, or a new thread has
     * started.
     */
    GDBWIRE_MI_ASYNC,

    /**
     * A stream out of band record.
     *
     * This is the result of normal output from the console, target or GDB.
     */
    GDBWIRE_MI_STREAM
};

/* This is an out of band record.  */
struct gdbwire_mi_oob_record {
    /** The kind of out of band record. */
    enum gdbwire_mi_oob_record_kind kind;

    union {
        /** When kind == GDBWIRE_MI_ASYNC. */
        struct gdbwire_mi_async_record *async_record;
        /** When kind == GDBWIRE_MI_STREAM. */
        struct gdbwire_mi_stream_record *stream_record;
    } variant;
};

/** The asynchronous out of band record kinds */
enum gdbwire_mi_async_record_kind {
    /**
     * The asynchronous status record kind.
     *
     * Contains on-going status information about the progress of a slow
     * operation. It can be discarded.
     *
     * This output is prepended by the + character.
     */
    GDBWIRE_MI_STATUS,

    /**
     * The asynchronous exec record kind.
     *
     * Contains asynchronous state change regarding the target:
     *  (stopped, started, disappeared).
     *
     * This output is prepended by the * character.
     */
    GDBWIRE_MI_EXEC,

    /**
     * The asyncronous notify record kind.
     *
     * Contains supplementary information that the client should handle 
     * (e.g., a new breakpoint information).
     *
     * This output is prepended by the = character.
     */
    GDBWIRE_MI_NOTIFY
};

/** The stream out of band record kinds */
enum gdbwire_mi_stream_record_kind {
    /**
     * The console output.
     *
     * Output that should be displayed as is in the console.
     * It is the textual response to a CLI command.
     *
     * This output is prepended by the ~ character.
     */
    GDBWIRE_MI_CONSOLE,

    /**
     * The target output.
     *
     * Output produced by the target program.
     *
     * This output is prepended by the @ character.
     */
    GDBWIRE_MI_TARGET,

    /**
     * The GDB log output.
     *
     * Output text coming from GDB's internals. For instance messages 
     * that should be displayed as part of an error log.
     *
     * This output is prepended by the & character.
     */
    GDBWIRE_MI_LOG
};

/**
 * The GDB/MI asyncronous class.
 *
 * 
 */
enum gdbwire_mi_async_class {
    /**
     * Loading the executable onto the remote target.
     *
     * This was undocumented in the GDB manual as far as GDB 7.7.
     *
     * This occurs if the async record is GDBWIRE_MI_STATUS as +download.
     */
    GDBWIRE_MI_ASYNC_DOWNLOAD,

    /**
     * The target has stopped.
     *
     * This occurs if the async record is GDBWIRE_MI_EXEC as *stopped.
     */
    GDBWIRE_MI_ASYNC_STOPPED,

    /**
     * The target is now running.
     *
     * This occurs if the async record is GDBWIRE_MI_EXEC as *running.
     */
    GDBWIRE_MI_ASYNC_RUNNING,

    /**
     * Reports that a thread group was added.
     *
     * When a thread group is added, it generally might not be associated
     * with a running process.
     *
     * This occurs if the async record is GDBWIRE_MI_NOTIFY
     * as =thread-group-added.
     */
    GDBWIRE_MI_ASYNC_THREAD_GROUP_ADDED,

    /**
     * Reports that a thread group was removed.
     *
     * When a thread group is removed, its id becomes invalid and cannot be
     * used in any way. 
     *
     * This occurs if the async record is GDBWIRE_MI_NOTIFY
     * as =thread-group-removed.
     */
    GDBWIRE_MI_ASYNC_THREAD_GROUP_REMOVED,

    /**
     * Reports that a thread group was started.
     *
     * A thread group became associated with a running program.
     *
     * This occurs if the async record is GDBWIRE_MI_NOTIFY
     * as =thread-group-started.
     */
    GDBWIRE_MI_ASYNC_THREAD_GROUP_STARTED,

    /**
     * Reports that a thread group was exited.
     *
     * A thread group is no longer associated with a running program.
     *
     * This occurs if the async record is GDBWIRE_MI_NOTIFY
     * as =thread-group-exited.
     */
    GDBWIRE_MI_ASYNC_THREAD_GROUP_EXITED,

    /**
     * Reports that a thread was created.
     *
     * This occurs if the async record is GDBWIRE_MI_NOTIFY
     * as =thread-created.
     */
    GDBWIRE_MI_ASYNC_THREAD_CREATED,

    /**
     * Reports that a thread was exited.
     *
     * This occurs if the async record is GDBWIRE_MI_NOTIFY as =thread-exited.
     */
    GDBWIRE_MI_ASYNC_THREAD_EXITED,

    /**
     * Reports that a thread was selected.
     *
     * This occurs if the async record is GDBWIRE_MI_NOTIFY as =thread-selected.
     */
    GDBWIRE_MI_ASYNC_THREAD_SELECTED,

    /**
     * Reports that a new library was loaded.
     *
     * This occurs if the async record is GDBWIRE_MI_NOTIFY as =library-loaded.
     */
    GDBWIRE_MI_ASYNC_LIBRARY_LOADED,

    /**
     * Reports that a new library was unloaded.
     *
     * This occurs if the async record is GDBWIRE_MI_NOTIFY
     * as =library-unloaded.
     */
    GDBWIRE_MI_ASYNC_LIBRARY_UNLOADED,

    /**
     * Reports that a trace frame was changed.
     *
     * This occurs if the async record is GDBWIRE_MI_NOTIFY
     * as =traceframe-changed.
     */
    GDBWIRE_MI_ASYNC_TRACEFRAME_CHANGED,

    /**
     * Reports that a trace state variable was created.
     *
     * This occurs if the async record is GDBWIRE_MI_NOTIFY as =tsv-created.
     */
    GDBWIRE_MI_ASYNC_TSV_CREATED,

    /**
     * Reports that a trace state variable was modified.
     *
     * This occurs if the async record is GDBWIRE_MI_NOTIFY as =tsv-modified.
     */
    GDBWIRE_MI_ASYNC_TSV_MODIFIED,

    /**
     * Reports that a trace state variable was deleted.
     *
     * This occurs if the async record is GDBWIRE_MI_NOTIFY as =tsv-deleted.
     */
    GDBWIRE_MI_ASYNC_TSV_DELETED,

    /**
     * Reports that a breakpoint was created.
     *
     * Only user-visible breakpoints are reported to the MI user. 
     *
     * If a breakpoint is emitted in the result record of a
     * command, then it will not also be emitted in an async record. 
     *
     * This occurs if the async record is GDBWIRE_MI_NOTIFY
     * as =breakpoint-created.
     */
    GDBWIRE_MI_ASYNC_BREAKPOINT_CREATED,

    /**
     * Reports that a breakpoint was modified.
     *
     * Only user-visible breakpoints are reported to the MI user. 
     *
     * If a breakpoint is emitted in the result record of a
     * command, then it will not also be emitted in an async record. 
     *
     * This occurs if the async record is GDBWIRE_MI_NOTIFY
     * as =breakpoint-modified.
     */
    GDBWIRE_MI_ASYNC_BREAKPOINT_MODIFIED,

    /**
     * Reports that a breakpoint was deleted.
     *
     * Only user-visible breakpoints are reported to the MI user. 
     *
     * If a breakpoint is emitted in the result record of a
     * command, then it will not also be emitted in an async record. 
     *
     * This occurs if the async record is GDBWIRE_MI_NOTIFY
     * as =breakpoint-deleted.
     */
    GDBWIRE_MI_ASYNC_BREAKPOINT_DELETED,

    /**
     * Reports that execution log recording was started on an inferior.
     *
     * This occurs if the async record is GDBWIRE_MI_NOTIF
     *  as =record-started.
     */
    GDBWIRE_MI_ASYNC_RECORD_STARTED,

    /**
     * Reports that execution log recording was stopped on an inferior.
     *
     * This occurs if the async record is GDBWIRE_MI_NOTIFY
     * as =record-stopped.
     */
    GDBWIRE_MI_ASYNC_RECORD_STOPPED,

    /**
     * Reports that a parameter of the command set param is changed to value.
     *
     * For example, when the user runs a command like 'set print pretty on',
     * this async command will be invoked with the parameter reported as
     * 'print pretty' and the value as 'on'.
     *
     * This occurs if the async record is GDBWIRE_MI_NOTIFY
     * as =cmd-param-changed.
     */
    GDBWIRE_MI_ASYNC_CMD_PARAM_CHANGED,

    /**
     * Reports that bytes from addr to data + len were written in an inferior.
     *
     * This occurs if the async record is GDBWIRE_MI_NOTIFY
     * as =memory-changed.
     */
    GDBWIRE_MI_ASYNC_MEMORY_CHANGED,

    /* An unsupported async class */
    GDBWIRE_MI_ASYNC_UNSUPPORTED
};

/**
 * The GDB/MI asyncronous record in an output command.
 *
 * An asyncronous record occurs when GDB would like to update the
 * client with information that it has not asked for.
 */
struct gdbwire_mi_async_record {
    /**
     * The result record token.
     *
     * Please note that the GDB/MI manual says that asyncronous records
     * do not currently populate this token on output but reserve the right
     * to do so. For that reason, token here should always be NULL.
     *
     * From the GDB documentation:
     *   Note that for all async output, while the token is allowed by the
     *   grammar and may be output by future versions of gdb for select async
     *   output messages, it is generally omitted. Frontends should treat all
     *   async output as reporting general changes in the state of the target
     *   and there should be no need to associate async output to any prior
     *   command. 
     *
     * After further investigation, I determined that newer GDB's will no
     * longer ever output this information. Older GDB's will. The commit
     * that made this change in GDB is 721c02de on April 24th, 2008.
     * The next GDB that was released was on October 6th, 2009, version 7.0.
     *
     * Before the above mentioned commit async *stopped commands would
     * sometimes output the token associated with the last token provided in
     * a GDB/MI input command. After that change, the token is never
     * associated with an async output command, even though the
     * documentation says it might be.
     *
     * Finally, even before that change when the token was output in the
     * async *stopped command, the developers of GDB felt that it was not
     * useful and should be avoided by front ends.
     *
     * With this information, I've determined that front ends should never
     * use this value to determine logic. However, the value is parsed in
     * order to accurately handle and represent the cases where this value
     * occurs.
     *
     * This represents the token value the front end provided to the
     * corresponding GDB/MI input command or NULL if no token was provided.
     */
    gdbwire_mi_token_t token;

    /** The kind of asynchronous record. */
    enum gdbwire_mi_async_record_kind kind;

    /** The asynchronous output class */
    enum gdbwire_mi_async_class async_class;

    /**
     * An optional list of results for this async output.
     *
     * Will be NULL if there is no results.
     */
    struct gdbwire_mi_result *result;
};

/** The GDB/MI result kind */
enum gdbwire_mi_result_kind {
    /** The result is a cstring */
    GDBWIRE_MI_CSTRING,
    /** The result is a tuple */
    GDBWIRE_MI_TUPLE,
    /** The result is a list */
    GDBWIRE_MI_LIST
};

/**
 * A GDB/MI result list.
 *
 * This is one of the important GDB/MI data structures. GDB communicates many
 * of it's values to the front end through this key/value data structure.
 *
 * It is basically a list of key/value pairs, where the key is a
 * variable name and the value expands to a string, a tuple of results or
 * a list of results.
 *
 * This can be thought of as a custom json object.
 */
struct gdbwire_mi_result {
    /** The kind of result this represents. */
    enum gdbwire_mi_result_kind kind;

    /** The key being described by the result. */
    char *variable;

    union {
        /** When kind is GDBWIRE_MI_CSTRING */
        char *cstring;

        /**
         * When kind is GDBWIRE_MI_TUPLE or GDBWIRE_MI_LIST.
         *
         * If kind is GDBWIRE_MI_TUPLE, each result in the tuple should have a
         * valid key according to the GDB/MI specification. That is, for
         * each result, result->variable should not be NULL.
         *   Note: GDBWIRE currently relaxes the above rule. It allows tuple's
         *   with out a key in each member. For instance, {key="value"}
         *   is what the GDB/MI specification advocates for, but some
         *   variations of GDB emit {"value"} and so GDBWIRE allows it.
         *
         * If kind is GDBWIRE_MI_LIST, the GDB/MI specification allows
         * results in this list to not have keys. That is, for each result,
         * result->variable may be NULL.
         *
         * Will be NULL if the tuple or list is empty.
         */
        struct gdbwire_mi_result *result;
    } variant;

    /** The next result or NULL if none */
    struct gdbwire_mi_result *next;
};

/**
 * An out of band GDB/MI stream record.
 *
 * A stream record is intended to provide the front end with information
 * from the console, the target or from GDB itself.
 */
struct gdbwire_mi_stream_record {
    /** The kind of stream record. */
    enum gdbwire_mi_stream_record_kind kind;
    /** The buffer provided in this stream record. */
    char *cstring;
};

void gdbwire_mi_output_free(struct gdbwire_mi_output *param);

struct gdbwire_mi_output *append_gdbwire_mi_output(
        struct gdbwire_mi_output *list, struct gdbwire_mi_output *item);

struct gdbwire_mi_result *append_gdbwire_mi_result(
        struct gdbwire_mi_result *list, struct gdbwire_mi_result *item);

#ifdef __cplusplus 
}
#endif 

#endif
/***** End of gdbwire_mi_pt.h ************************************************/
/***** Begin file gdbwire_mi_pt_alloc.h **************************************/
#ifndef GDBWIRE_MI_PT_ALLOC_H
#define GDBWIRE_MI_PT_ALLOC_H

#ifdef __cplusplus 
extern "C" { 
#endif 

/**
 * Responsible for allocating and deallocating gdbwire_mi_pt objects.
 */

/* struct gdbwire_mi_output */
struct gdbwire_mi_output *gdbwire_mi_output_alloc(void);
void gdbwire_mi_output_free(struct gdbwire_mi_output *param);

/* struct gdbwire_mi_result_record */
struct gdbwire_mi_result_record *gdbwire_mi_result_record_alloc(void);
void gdbwire_mi_result_record_free(struct gdbwire_mi_result_record *param);

/* struct gdbwire_mi_result */
struct gdbwire_mi_result *gdbwire_mi_result_alloc(void);
void gdbwire_mi_result_free(struct gdbwire_mi_result *param);

/* struct gdbwire_mi_oob_record */
struct gdbwire_mi_oob_record *gdbwire_mi_oob_record_alloc(void);
void gdbwire_mi_oob_record_free(struct gdbwire_mi_oob_record *param);

/* struct gdbwire_mi_async_record */
struct gdbwire_mi_async_record *gdbwire_mi_async_record_alloc(void);
void gdbwire_mi_async_record_free(struct gdbwire_mi_async_record *param);

/* struct gdbwire_mi_stream_record */
struct gdbwire_mi_stream_record *gdbwire_mi_stream_record_alloc(void);
void gdbwire_mi_stream_record_free(struct gdbwire_mi_stream_record *param);

#ifdef __cplusplus 
}
#endif 

#endif /* GDBWIRE_MI_PT_ALLOC_H */
/***** End of gdbwire_mi_pt_alloc.h ******************************************/
/***** Begin file gdbwire_mi_parser.h ****************************************/
#ifndef GDBWIRE_MI_PARSER_H
#define GDBWIRE_MI_PARSER_H

#ifdef __cplusplus 
extern "C" { 
#endif 

/* #include "gdbwire_result.h" */
/***** Include gdbwire_mi_pt.h in the middle of gdbwire_mi_parser.h **********/
/***** Begin file gdbwire_mi_pt.h ********************************************/
#ifndef GDBWIRE_MI_PT_H
#define GDBWIRE_MI_PT_H

#ifdef __cplusplus 
extern "C" { 
#endif 

/**
 * The position of a token in a GDB/MI line.
 *
 * Note that a string in C is zero based and the token column
 * position is 1 based. For example,
 *   char *str = "hello world";
 * The "hello" token would have a start_column as 1 and an end
 * column as 5.
 *
 * The start_column and end_column will be the same column number for
 * a token of size 1.
 */
struct gdbwire_mi_position {
    /* The starting column position of the token */
    int start_column;
    /* The ending column position of the token */
    int end_column;
};

/** The gdbwire_mi output kinds. */
enum gdbwire_mi_output_kind {
    /**
     * The GDB/MI output contains an out of band record.
     *
     * The out of band record is not necessarily associated with any
     * particular GDB/MI input command.
     */
    GDBWIRE_MI_OUTPUT_OOB,

    /**
     * The GDB/MI output contains a gdbwire_mi result record.
     *
     * This record typically contains the result data from a request
     * made by the client in a previous GDB/MI input command.
     */
    GDBWIRE_MI_OUTPUT_RESULT,

    /**
     * The GDB/MI output represents a prompt. (ie. (gdb) )
     * 
     * TODO: Document when GDB is ready to receive a command. Only if
     * the prompt is received and at *stopped?
     */
    GDBWIRE_MI_OUTPUT_PROMPT,

    /**
     * A parse error occurred.
     */
    GDBWIRE_MI_OUTPUT_PARSE_ERROR
};

/**
 * The GDB/MI output command.
 *
 * A GDB/MI output command is the main mechanism in which GDB
 * corresponds with a front end.
 */
struct gdbwire_mi_output {
    enum gdbwire_mi_output_kind kind;

    union {
        /** When kind == GDBWIRE_MI_OUTPUT_OOB, never NULL. */
        struct gdbwire_mi_oob_record *oob_record;
        /** When kind == GDBWIRE_MI_OUTPUT_RESULT, never NULL. */
        struct gdbwire_mi_result_record *result_record;
        /** When kind == GDBWIRE_MI_OUTPUT_PARSE_ERROR, never NULL. */
        struct {
            /** The token the error occurred on */
            char *token;
            /** The position of the token where the error occurred. */
            struct gdbwire_mi_position pos;
        } error;
    } variant;

    /**
     * The GDB/MI output line that was used to create this output instance.
     *
     * Each gdbwire_mi output structure is created from exactly one line of
     * MI output from GDB. This field represents the line that created 
     * this particular output structure.
     *
     * This field is always available and never NULL, even for a parse error.
     */
    char *line;

    /** The next GDB/MI output command or NULL if none */
    struct gdbwire_mi_output *next;
};

/**
 * A GDB/MI token.
 *
 * A string made up of one or more digits.
 * The regular expression [0-9]+ will match this types contents.
 */
typedef char *gdbwire_mi_token_t;

/**
 * A GDB/MI output command may contain one of the following result indications.
 */
enum gdbwire_mi_result_class {
    /**
     * The synchronous operation was successful (^done).
     */
    GDBWIRE_MI_DONE,

    /**
     * Equivalent to GDBWIRE_MI_DONE (^running).
     *
     * Historically, was output by GDB instead of ^done if the command
     * resumed the target.
     *
     * Do not rely on or use this result class in the front end to determine
     * the state of the target. Use the async *running output record to
     * determine which threads have resumed running.
     *
     * TODO: Ensure that early versions of GDB can depend on the async
     * *running or if front ends DO have to rely on ^running.
     */
    GDBWIRE_MI_RUNNING,

    /**
     * GDB has connected to a remote target (^connected).
     *
     * This is in response to the -target-select command.
     *
     * A comment in the GDB source code says,
     *   There's no particularly good reason why target-connect results
     *   in not ^done.  Should kill ^connected for MI3.
     *
     * With this in mind, it makes sense to assume that GDBWIRE_MI_CONNECTED
     * and GDBWIRE_MI_DONE are equivalent.
     */
    GDBWIRE_MI_CONNECTED,

    /**
     * An error has occurred (^error).
     *
     * This can occur if the user provides an improper command to GDB.
     * In this case, the user will be provided the standard error output but
     * the front end will also be provided this information independently.
     */
    GDBWIRE_MI_ERROR,

    /**
     * GDB has terminated (^exit).
     *
     * When GDB knows it is about to exit, it provides this notification
     * in the GDB/MI output command. However, on all other circumstances,
     * the front end should be prepared to have GDB exit and not provide
     * this information.
     */
    GDBWIRE_MI_EXIT,

    /* An unsupported result class */
    GDBWIRE_MI_UNSUPPORTED
};

/**
 * The GDB/MI result record in an output command.
 *
 * The result record represents the result data in the GDB/MI output
 * command sent by GDB. This typically contains the content the client
 * was requesting when it sent a GDB/MI input command to GDB.
 */
struct gdbwire_mi_result_record {
    /**
     * The token associated with the corresponding GDB/MI input command.
     *
     * The client may provide a unique string of digits at the beginning of a
     * GDB/MI input command. For example,
     *   0000-foo
     * When GDB finally gets around to responding to the GDB/MI input command,
     * it takes the token provided in the input command and puts it into the
     * result record of the corresponding GDB/MI output command. For
     * example, the output commmand associated with the above input command is,
     *   0000^error,msg="Undefined MI command: foo",code="undefined-command"
     * and the result record would have the below token field set to "0000".
     *
     * This is intended to allow the front end to correlate the GDB/MI input
     * command it sent with the GDB/MI output command GDB responded with.
     *
     * This represents the token value the front end provided to the
     * corresponding GDB/MI input command or NULL if no token was provided.
     */
    gdbwire_mi_token_t token;

    /** The result records result class. */
    enum gdbwire_mi_result_class result_class;

    /**
     * An optional list of results for this result record.
     *
     * Will be NULL if there is no results for this result record.
     *
     * This is typically where the result data is that the client
     * is looking for.
     */
    struct gdbwire_mi_result *result;
};

/** The out of band record kinds. */
enum gdbwire_mi_oob_record_kind {
    /**
     * An asyncronous out of band record.
     *
     * An asyncronous record occurs when GDB would like to update the
     * client with information that it has not asked for.
     *
     * For instance, if the inferior has stopped, or a new thread has
     * started.
     */
    GDBWIRE_MI_ASYNC,

    /**
     * A stream out of band record.
     *
     * This is the result of normal output from the console, target or GDB.
     */
    GDBWIRE_MI_STREAM
};

/* This is an out of band record.  */
struct gdbwire_mi_oob_record {
    /** The kind of out of band record. */
    enum gdbwire_mi_oob_record_kind kind;

    union {
        /** When kind == GDBWIRE_MI_ASYNC. */
        struct gdbwire_mi_async_record *async_record;
        /** When kind == GDBWIRE_MI_STREAM. */
        struct gdbwire_mi_stream_record *stream_record;
    } variant;
};

/** The asynchronous out of band record kinds */
enum gdbwire_mi_async_record_kind {
    /**
     * The asynchronous status record kind.
     *
     * Contains on-going status information about the progress of a slow
     * operation. It can be discarded.
     *
     * This output is prepended by the + character.
     */
    GDBWIRE_MI_STATUS,

    /**
     * The asynchronous exec record kind.
     *
     * Contains asynchronous state change regarding the target:
     *  (stopped, started, disappeared).
     *
     * This output is prepended by the * character.
     */
    GDBWIRE_MI_EXEC,

    /**
     * The asyncronous notify record kind.
     *
     * Contains supplementary information that the client should handle 
     * (e.g., a new breakpoint information).
     *
     * This output is prepended by the = character.
     */
    GDBWIRE_MI_NOTIFY
};

/** The stream out of band record kinds */
enum gdbwire_mi_stream_record_kind {
    /**
     * The console output.
     *
     * Output that should be displayed as is in the console.
     * It is the textual response to a CLI command.
     *
     * This output is prepended by the ~ character.
     */
    GDBWIRE_MI_CONSOLE,

    /**
     * The target output.
     *
     * Output produced by the target program.
     *
     * This output is prepended by the @ character.
     */
    GDBWIRE_MI_TARGET,

    /**
     * The GDB log output.
     *
     * Output text coming from GDB's internals. For instance messages 
     * that should be displayed as part of an error log.
     *
     * This output is prepended by the & character.
     */
    GDBWIRE_MI_LOG
};

/**
 * The GDB/MI asyncronous class.
 *
 * 
 */
enum gdbwire_mi_async_class {
    /**
     * Loading the executable onto the remote target.
     *
     * This was undocumented in the GDB manual as far as GDB 7.7.
     *
     * This occurs if the async record is GDBWIRE_MI_STATUS as +download.
     */
    GDBWIRE_MI_ASYNC_DOWNLOAD,

    /**
     * The target has stopped.
     *
     * This occurs if the async record is GDBWIRE_MI_EXEC as *stopped.
     */
    GDBWIRE_MI_ASYNC_STOPPED,

    /**
     * The target is now running.
     *
     * This occurs if the async record is GDBWIRE_MI_EXEC as *running.
     */
    GDBWIRE_MI_ASYNC_RUNNING,

    /**
     * Reports that a thread group was added.
     *
     * When a thread group is added, it generally might not be associated
     * with a running process.
     *
     * This occurs if the async record is GDBWIRE_MI_NOTIFY
     * as =thread-group-added.
     */
    GDBWIRE_MI_ASYNC_THREAD_GROUP_ADDED,

    /**
     * Reports that a thread group was removed.
     *
     * When a thread group is removed, its id becomes invalid and cannot be
     * used in any way. 
     *
     * This occurs if the async record is GDBWIRE_MI_NOTIFY
     * as =thread-group-removed.
     */
    GDBWIRE_MI_ASYNC_THREAD_GROUP_REMOVED,

    /**
     * Reports that a thread group was started.
     *
     * A thread group became associated with a running program.
     *
     * This occurs if the async record is GDBWIRE_MI_NOTIFY
     * as =thread-group-started.
     */
    GDBWIRE_MI_ASYNC_THREAD_GROUP_STARTED,

    /**
     * Reports that a thread group was exited.
     *
     * A thread group is no longer associated with a running program.
     *
     * This occurs if the async record is GDBWIRE_MI_NOTIFY
     * as =thread-group-exited.
     */
    GDBWIRE_MI_ASYNC_THREAD_GROUP_EXITED,

    /**
     * Reports that a thread was created.
     *
     * This occurs if the async record is GDBWIRE_MI_NOTIFY
     * as =thread-created.
     */
    GDBWIRE_MI_ASYNC_THREAD_CREATED,

    /**
     * Reports that a thread was exited.
     *
     * This occurs if the async record is GDBWIRE_MI_NOTIFY as =thread-exited.
     */
    GDBWIRE_MI_ASYNC_THREAD_EXITED,

    /**
     * Reports that a thread was selected.
     *
     * This occurs if the async record is GDBWIRE_MI_NOTIFY as =thread-selected.
     */
    GDBWIRE_MI_ASYNC_THREAD_SELECTED,

    /**
     * Reports that a new library was loaded.
     *
     * This occurs if the async record is GDBWIRE_MI_NOTIFY as =library-loaded.
     */
    GDBWIRE_MI_ASYNC_LIBRARY_LOADED,

    /**
     * Reports that a new library was unloaded.
     *
     * This occurs if the async record is GDBWIRE_MI_NOTIFY
     * as =library-unloaded.
     */
    GDBWIRE_MI_ASYNC_LIBRARY_UNLOADED,

    /**
     * Reports that a trace frame was changed.
     *
     * This occurs if the async record is GDBWIRE_MI_NOTIFY
     * as =traceframe-changed.
     */
    GDBWIRE_MI_ASYNC_TRACEFRAME_CHANGED,

    /**
     * Reports that a trace state variable was created.
     *
     * This occurs if the async record is GDBWIRE_MI_NOTIFY as =tsv-created.
     */
    GDBWIRE_MI_ASYNC_TSV_CREATED,

    /**
     * Reports that a trace state variable was modified.
     *
     * This occurs if the async record is GDBWIRE_MI_NOTIFY as =tsv-modified.
     */
    GDBWIRE_MI_ASYNC_TSV_MODIFIED,

    /**
     * Reports that a trace state variable was deleted.
     *
     * This occurs if the async record is GDBWIRE_MI_NOTIFY as =tsv-deleted.
     */
    GDBWIRE_MI_ASYNC_TSV_DELETED,

    /**
     * Reports that a breakpoint was created.
     *
     * Only user-visible breakpoints are reported to the MI user. 
     *
     * If a breakpoint is emitted in the result record of a
     * command, then it will not also be emitted in an async record. 
     *
     * This occurs if the async record is GDBWIRE_MI_NOTIFY
     * as =breakpoint-created.
     */
    GDBWIRE_MI_ASYNC_BREAKPOINT_CREATED,

    /**
     * Reports that a breakpoint was modified.
     *
     * Only user-visible breakpoints are reported to the MI user. 
     *
     * If a breakpoint is emitted in the result record of a
     * command, then it will not also be emitted in an async record. 
     *
     * This occurs if the async record is GDBWIRE_MI_NOTIFY
     * as =breakpoint-modified.
     */
    GDBWIRE_MI_ASYNC_BREAKPOINT_MODIFIED,

    /**
     * Reports that a breakpoint was deleted.
     *
     * Only user-visible breakpoints are reported to the MI user. 
     *
     * If a breakpoint is emitted in the result record of a
     * command, then it will not also be emitted in an async record. 
     *
     * This occurs if the async record is GDBWIRE_MI_NOTIFY
     * as =breakpoint-deleted.
     */
    GDBWIRE_MI_ASYNC_BREAKPOINT_DELETED,

    /**
     * Reports that execution log recording was started on an inferior.
     *
     * This occurs if the async record is GDBWIRE_MI_NOTIF
     *  as =record-started.
     */
    GDBWIRE_MI_ASYNC_RECORD_STARTED,

    /**
     * Reports that execution log recording was stopped on an inferior.
     *
     * This occurs if the async record is GDBWIRE_MI_NOTIFY
     * as =record-stopped.
     */
    GDBWIRE_MI_ASYNC_RECORD_STOPPED,

    /**
     * Reports that a parameter of the command set param is changed to value.
     *
     * For example, when the user runs a command like 'set print pretty on',
     * this async command will be invoked with the parameter reported as
     * 'print pretty' and the value as 'on'.
     *
     * This occurs if the async record is GDBWIRE_MI_NOTIFY
     * as =cmd-param-changed.
     */
    GDBWIRE_MI_ASYNC_CMD_PARAM_CHANGED,

    /**
     * Reports that bytes from addr to data + len were written in an inferior.
     *
     * This occurs if the async record is GDBWIRE_MI_NOTIFY
     * as =memory-changed.
     */
    GDBWIRE_MI_ASYNC_MEMORY_CHANGED,

    /* An unsupported async class */
    GDBWIRE_MI_ASYNC_UNSUPPORTED
};

/**
 * The GDB/MI asyncronous record in an output command.
 *
 * An asyncronous record occurs when GDB would like to update the
 * client with information that it has not asked for.
 */
struct gdbwire_mi_async_record {
    /**
     * The result record token.
     *
     * Please note that the GDB/MI manual says that asyncronous records
     * do not currently populate this token on output but reserve the right
     * to do so. For that reason, token here should always be NULL.
     *
     * From the GDB documentation:
     *   Note that for all async output, while the token is allowed by the
     *   grammar and may be output by future versions of gdb for select async
     *   output messages, it is generally omitted. Frontends should treat all
     *   async output as reporting general changes in the state of the target
     *   and there should be no need to associate async output to any prior
     *   command. 
     *
     * After further investigation, I determined that newer GDB's will no
     * longer ever output this information. Older GDB's will. The commit
     * that made this change in GDB is 721c02de on April 24th, 2008.
     * The next GDB that was released was on October 6th, 2009, version 7.0.
     *
     * Before the above mentioned commit async *stopped commands would
     * sometimes output the token associated with the last token provided in
     * a GDB/MI input command. After that change, the token is never
     * associated with an async output command, even though the
     * documentation says it might be.
     *
     * Finally, even before that change when the token was output in the
     * async *stopped command, the developers of GDB felt that it was not
     * useful and should be avoided by front ends.
     *
     * With this information, I've determined that front ends should never
     * use this value to determine logic. However, the value is parsed in
     * order to accurately handle and represent the cases where this value
     * occurs.
     *
     * This represents the token value the front end provided to the
     * corresponding GDB/MI input command or NULL if no token was provided.
     */
    gdbwire_mi_token_t token;

    /** The kind of asynchronous record. */
    enum gdbwire_mi_async_record_kind kind;

    /** The asynchronous output class */
    enum gdbwire_mi_async_class async_class;

    /**
     * An optional list of results for this async output.
     *
     * Will be NULL if there is no results.
     */
    struct gdbwire_mi_result *result;
};

/** The GDB/MI result kind */
enum gdbwire_mi_result_kind {
    /** The result is a cstring */
    GDBWIRE_MI_CSTRING,
    /** The result is a tuple */
    GDBWIRE_MI_TUPLE,
    /** The result is a list */
    GDBWIRE_MI_LIST
};

/**
 * A GDB/MI result list.
 *
 * This is one of the important GDB/MI data structures. GDB communicates many
 * of it's values to the front end through this key/value data structure.
 *
 * It is basically a list of key/value pairs, where the key is a
 * variable name and the value expands to a string, a tuple of results or
 * a list of results.
 *
 * This can be thought of as a custom json object.
 */
struct gdbwire_mi_result {
    /** The kind of result this represents. */
    enum gdbwire_mi_result_kind kind;

    /** The key being described by the result. */
    char *variable;

    union {
        /** When kind is GDBWIRE_MI_CSTRING */
        char *cstring;

        /**
         * When kind is GDBWIRE_MI_TUPLE or GDBWIRE_MI_LIST.
         *
         * If kind is GDBWIRE_MI_TUPLE, each result in the tuple should have a
         * valid key according to the GDB/MI specification. That is, for
         * each result, result->variable should not be NULL.
         *   Note: GDBWIRE currently relaxes the above rule. It allows tuple's
         *   with out a key in each member. For instance, {key="value"}
         *   is what the GDB/MI specification advocates for, but some
         *   variations of GDB emit {"value"} and so GDBWIRE allows it.
         *
         * If kind is GDBWIRE_MI_LIST, the GDB/MI specification allows
         * results in this list to not have keys. That is, for each result,
         * result->variable may be NULL.
         *
         * Will be NULL if the tuple or list is empty.
         */
        struct gdbwire_mi_result *result;
    } variant;

    /** The next result or NULL if none */
    struct gdbwire_mi_result *next;
};

/**
 * An out of band GDB/MI stream record.
 *
 * A stream record is intended to provide the front end with information
 * from the console, the target or from GDB itself.
 */
struct gdbwire_mi_stream_record {
    /** The kind of stream record. */
    enum gdbwire_mi_stream_record_kind kind;
    /** The buffer provided in this stream record. */
    char *cstring;
};

void gdbwire_mi_output_free(struct gdbwire_mi_output *param);

struct gdbwire_mi_output *append_gdbwire_mi_output(
        struct gdbwire_mi_output *list, struct gdbwire_mi_output *item);

struct gdbwire_mi_result *append_gdbwire_mi_result(
        struct gdbwire_mi_result *list, struct gdbwire_mi_result *item);

#ifdef __cplusplus 
}
#endif 

#endif
/***** End of gdbwire_mi_pt.h ************************************************/
/***** Continuing where we left off in gdbwire_mi_parser.h *******************/

/* The opaque GDB/MI parser context */
struct gdbwire_mi_parser;

/**
 * The primary mechanism to alert users of GDB/MI notifications.
 *
 * The flow is like this:
 * - create a parser context (gdbwire_mi_parser_create)
 * - push onto the parser arbitrary amounts of data (gdbwire_mi_parser_push)
 *   - receive callbacks from inside gdbwire_mi_parser_push when
 *     it discovers callbacks the user will find interesting
 * - destroy the parser (gdbwire_mi_parser_destroy)
 */
struct gdbwire_mi_parser_callbacks {
    /**
     * An arbitrary pointer to associate with the callbacks.
     *
     * If the calling api is C++ it is useful to make this an instance
     * of an object you want to bind to the callback functions below.
     */
    void *context;
    
    /**
     * A GDB/MI output command is available.
     *
     * @param context
     * The context pointer above.
     *
     * @param output
     * The gdbwire_mi output command. This output command is now owned by the
     * function being invoked and should be destroyed when necessary.
     */
    void (*gdbwire_mi_output_callback)(void *context,
        struct gdbwire_mi_output *output);
};

/**
 * Create a GDB/MI parser context.
 *
 * @param callbacks
 * The callback functions to invoke upon discovery of parse data.
 *
 * @return
 * A new GDB/MI parser instance or NULL on error.
 */
struct gdbwire_mi_parser *gdbwire_mi_parser_create(
        struct gdbwire_mi_parser_callbacks callbacks);

/**
 * Destroy a gdbwire_mi_parser context.
 *
 * This function will do nothing if parser is NULL.
 *
 * @param parser
 * The instance the parser to destroy
 */
void gdbwire_mi_parser_destroy(struct gdbwire_mi_parser *parser);

/**
 * Push a null terminated string onto the parser.
 *
 * During this function, if a gdbwire_mi output command is discovered by
 * the parser (or any other useful GDB/MI notification), it will invoke
 * the appropriate callbacks assigned during parser creation.
 *
 * @param parser
 * The gdbwire_mi parser context to operate on.
 *
 * @param data
 * The parse data to push onto the parser.
 * 
 * @return
 * GDBWIRE_OK on success or appropriate error result on failure.
 */
enum gdbwire_result gdbwire_mi_parser_push(struct gdbwire_mi_parser *parser,
        const char *data);

/**
 * Push some parse data onto the parser.
 *
 * See gdbwire_mi_parser_push for details on function behavior.
 *
 * @param parser
 * The gdbwire_mi parser context to operate on.
 *
 * @param data
 * The parse data to push onto the parser.
 *
 * @param size
 * The size of the data to push onto the parser.
 *
 * @return
 * GDBWIRE_OK on success or appropriate error result on failure.
 */
enum gdbwire_result gdbwire_mi_parser_push_data(
        struct gdbwire_mi_parser *parser, const char *data, size_t size);

#ifdef __cplusplus 
}
#endif 

#endif
/***** End of gdbwire_mi_parser.h ********************************************/
/***** Begin file gdbwire_mi_command.h ***************************************/
#ifndef GDBWIRE_MI_COMMAND_H
#define GDBWIRE_MI_COMMAND_H

#ifdef __cplusplus 
extern "C" { 
#endif 

/* #include "gdbwire_result.h" */
/* #include "gdbwire_mi_pt.h" */

/**
 * An enumeration representing the supported GDB/MI commands.
 */
enum gdbwire_mi_command_kind {
    /* -break-info */
    GDBWIRE_MI_BREAK_INFO,

    /* -stack-info-frame */
    GDBWIRE_MI_STACK_INFO_FRAME,

    /* -file-list-exec-source-file */
    GDBWIRE_MI_FILE_LIST_EXEC_SOURCE_FILE,
    /* -file-list-exec-source-files */
    GDBWIRE_MI_FILE_LIST_EXEC_SOURCE_FILES
};

/** A linked list of source files. */
struct gdbwire_mi_source_file {
    /** A relative path to a file, never NULL */
    char *file;
    /**An absolute path to a file, NULL if unavailable */
    char *fullname;
    /** The next file name or NULL if no more. */
    struct gdbwire_mi_source_file *next;
};

/** The disposition of a breakpoint. What to do after hitting it. */
enum gdbwire_mi_breakpoint_disp_kind {
    GDBWIRE_MI_BP_DISP_DELETE,           /** Delete on next hit */
    GDBWIRE_MI_BP_DISP_DELETE_NEXT_STOP, /** Delete on next stop, hit or not */
    GDBWIRE_MI_BP_DISP_DISABLE,          /** Disable on next hit */
    GDBWIRE_MI_BP_DISP_KEEP,             /** Leave the breakpoint in place */
    GDBWIRE_MI_BP_DISP_UNKNOWN           /** When GDB doesn't specify */
};

/**
 * A linked list of breakpoints.
 *
 * A breakpoint is a breakpoint, a tracepoint, a watchpoing or a
 * catchpoint. The GDB breakpoint model is quite sophisticated.
 * This structure can be extended when necessary.
 */
struct gdbwire_mi_breakpoint {
    /**
     * The breakpoint number.
     *
     * An integer, however, for a breakpoint that represents one location of
     * a multiple location breakpoint, this will be a dotted pair, like ‘1.2’. 
     *
     * Never NULL.
     */
    char *number;

    /**
     * Determines if this is a multiple location breakpoint.
     *
     * True for a multi-location breakpoint, false otherwise.
     *
     * It is possible that a breakpoint corresponds to several locations in
     * your program. For example, several functions may have the same name.
     * For the following source code,
     *   int foo(int p) { return p; }
     *   double foo(double p) { return p; }
     *   int main() { int i = 1; double d = 2.3; return foo(i) + foo(d); }
     * If the user sets a breakpoint at foo by typing,
     *   b foo
     * Then gdb will create 3 breakpoints. The multiple location breakpoint,
     * which is the parent of the two breakpoints created for each foo
     * function. Here is the output of gdb from the CLI perspective,
     *   Num     Type           Disp Enb Address            What
     *   1       breakpoint     keep y   <MULTIPLE>         
     *   1.1                         y     0x4004dd in foo(int) at main.cpp:1
     *   1.2                         y     0x4004eb in foo(double) at main.cpp:2
     *
     * However, if the user created a breakpoint for main by typing,
     *   b main
     * Then gdb will only create a single breakpoint which would look like,
     *   1       breakpoint     keep y   0x4004fa in main() at main.cpp:3
     *
     * When this is true, the address field will be "<MULTIPLE>" and
     * the field multi_breakpoints will represent the breakpoints that this
     * multiple location breakpoint has created.
     */
    unsigned char multi:1;
    
    /**
     * True for breakpoints of a multi-location breakpoint, otherwise false.
     *
     * For the example above, 1.1 and 1.2 would have this field set true.
     *
     * When this is true, the field multi_breakpoint will represent
     * the multiple location breakpoint that has created this breakpoint.
     */
    unsigned char from_multi:1;

    /**
     * The breakpoint type.
     *
     * Typically "breakpoint", "watchpoint" or "catchpoint", but can be
     * a variety of different values. In gdb, see breakpoint.c:bptype_string
     * to see all the different possibilities.
     *
     * This will be NULL for breakpoints of a multiple location breakpoint.
     * In this circumstance, check the multi_breakpoint field for the
     * multiple location breakpoint type field.
     */
    char *type;

    /**
     * The type of the catchpoint or NULL if not a catch point.
     *
     * This field is only valid when the breakpoint is a catchpoint.
     * Unfortuntely, gdb says the "type" of the breakpoint in the type field
     * is "breakpoint" not "catchpoint". So if this field is non-NULL, it is
     * safe to assume that this breakpoint represents at catch point.
     */
    char *catch_type;

    /**
     * The breakpoint disposition.
     *
     * For multiple location breakpoints, this will be
     * GDBWIRE_MI_BP_DISP_UNKNOWN. In this circumstance, check the
     * multi_breakpoint field for the multiple location breakpoint
     * disposition field.
     */
    enum gdbwire_mi_breakpoint_disp_kind disposition;

    /** True if enabled or False if disabled. */
    unsigned char enabled:1;

    /**
     * The address of the breakpoint.
     *
     * This may be
     * - a hexidecimal number, representing the address
     * - the string ‘<PENDING>’ for a pending breakpoint
     * - the string ‘<MULTIPLE>’ for a breakpoint with multiple locations
     *
     * This field will be NULL if no address can be determined.
     * For example, a watchpoint does not have an address. 
     */
    char *address;

    /** 
     * The name of the function or NULL if unknown.
     */
    char *func_name;

    /**
     * A relative path to the file the breakpoint is in or NULL if unknown.
     */
    char *file;

    /**
     * An absolute path to the file the breakpoint is in or NULL if unknown.
     */
    char *fullname;

    /**
     * The line number the breakpoint is at or 0 if unkonwn.
     */
    unsigned long line;

    /**
     * The number of times this breakpoint has been hit.
     *
     * For breakpoints of multi-location breakpoints, this will be 0.
     * Look at the multi-location breakpoint field instead.
     */
    unsigned long times;

    /**
     * The location of the breakpoint as originally specified by the user. 
     *
     * This may be NULL for instance, for breakpoints for multi-breakpoints.
     */
    char *original_location;

    /**
     * True for a pending breakpoint, otherwise false.
     *
     * When this is true, the address field will be "<PENDING>".
     */
    unsigned char pending:1;

    /**
     * The breakpoints for a multi-location breakpoint.
     *
     * If multi is true, this will be the breakpoints associated with the
     * multiple location breakpoint. Otherwise will be NULL.
     */
    struct gdbwire_mi_breakpoint *multi_breakpoints;

    /**
     * A pointer to the multi location breakpoint that created this breakpoint.
     *
     * When the field from_multi is true, this will be a pointer to the
     * multi-location breakpoint that created this breakpoint. Otherwise NULL.
     *
     * For the example above in the multi field, breakpoints 1.1 and 1.2
     * would have this field pointing to the breakpoint 1.
     */
    struct gdbwire_mi_breakpoint *multi_breakpoint;


    /** The next breakpoint or NULL if no more. */
    struct gdbwire_mi_breakpoint *next;
};

struct gdbwire_mi_stack_frame {
    /**
     * The frame number.
     *
     * Where 0 is the topmost frame, i.e., the innermost function. 
     *
     * Always present.
     */
    unsigned level;

    /**
     * The address ($pc value) of the frame.
     *
     * May be NULL if GDB can not determine the frame address.
     */
    char *address;

   /**
    * The function name for the frame. May be NULL if unknown.
    */
   char *func;

   /**
    * The file name for the frame. May be NULL if unknown.
    */
   char *file;

   /**
    * The fullname name for the frame. May be NULL if unknown.
    */
   char *fullname;

   /**
    * Line number corresponding to the $pc. Maybe be 0 if unknown.
    */
   int line;

   /**
    * The shared library where this function is defined.
    *
    * This is only given if the frame's function is not known. 
    * May be NULL if unknown.
    */
   char *from;
};

/**
 * Represents a GDB/MI command.
 */
struct gdbwire_mi_command {
    /**
     * The kind of mi command this represents.
     */
    enum gdbwire_mi_command_kind kind;

    union {
        /** When kind == GDBWIRE_MI_BREAK_INFO */
        struct {
            /** The list of breakpoints, NULL if none exist.  */
            struct gdbwire_mi_breakpoint *breakpoints;
        } break_info;

        /** When kind == GDBWIRE_MI_STACK_INFO_FRAME */
        struct {
            struct gdbwire_mi_stack_frame *frame;
        } stack_info_frame;

        /** When kind == GDBWIRE_MI_FILE_LIST_EXEC_SOURCE_FILE */
        struct {
            /**
             * The line number the inferior is currently executing at.
             */
            int line;

            /**
             * The filename the inferior is currently executing at.
             *
             * This is usually a relative path.
             */
            char *file;

            /**
             * The filename the inferior is currently executing at.
             *
             * This is an absolute path.
             *
             * This command was addd in 2004, however, it was possible
             * at the time that only the "file" field would be put out and
             * the "fullname" field would be omitted. In 2012, in git commit,
             * f35a17b5, gdb was changed to always omit the "fullname" field.
             */
            char *fullname;

            /**
             * Determines if the file includes preprocessor macro information.
             *
             * This command was added in 2004. However, the macro-info
             * field was added to the output in 2008 in git commit 17784837.
             *
             * Only check this field if macro_info_exists is true.
             */
            char macro_info:1;

            /** True if macro-info field was in mi output, otherwise false */
            char macro_info_exists:1;
        } file_list_exec_source_file;

        /** When kind == GDBWIRE_MI_FILE_LIST_EXEC_SOURCE_FILES */
        struct {
            /**
             * A list of files that make up the inferior.
             *
             * When there are no files (if the gdb does not have an inferior
             * loaded) than files will be NULL.
             *
             * This command was addd in 2004, however, it was possible
             * at the time that only the "file" field would be put out and
             * the "fullname" field would be omitted. In 2012, in git commit,
             * f35a17b5, gdb was changed to always omit the "fullname" field.
             */
            struct gdbwire_mi_source_file *files;
        } file_list_exec_source_files;
        
    } variant;
};

/**
 * Get a gdbwire MI command from the result record.
 *
 * @param kind
 * The kind of command the result record is associated with.
 *
 * @param result_record
 * The result record to turn into a command.
 *
 * @param out_mi_command
 * Will return an allocated gdbwire mi command if GDBWIRE_OK is returned
 * from this function. You should free this memory with
 * gdbwire_mi_command_free when you are done with it.
 *
 * @return
 * The result of this function.
 */
enum gdbwire_result gdbwire_get_mi_command(
        enum gdbwire_mi_command_kind kind,
        struct gdbwire_mi_result_record *result_record,
        struct gdbwire_mi_command **out_mi_command);

/**
 * Free the gdbwire mi command.
 *
 * @param mi_command
 * The mi command to free.
 */
void gdbwire_mi_command_free(struct gdbwire_mi_command *mi_command);

#ifdef __cplusplus 
}
#endif 

#endif
/***** End of gdbwire_mi_command.h *******************************************/
/***** Begin file gdbwire_mi_grammar.h ***************************************/
/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_GDBWIRE_MI_SRC_GDBWIRE_MI_GRAMMAR_H_INCLUDED
# define YY_GDBWIRE_MI_SRC_GDBWIRE_MI_GRAMMAR_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int gdbwire_mi_debug;
#endif
/* "%code requires" blocks.  */


/* An opaque pointer. */
#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void *yyscan_t;
#endif

    struct gdbwire_mi_output;


/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    OPEN_BRACE = 258,
    CLOSED_BRACE = 259,
    OPEN_PAREN = 260,
    CLOSED_PAREN = 261,
    ADD_OP = 262,
    MULT_OP = 263,
    EQUAL_SIGN = 264,
    TILDA = 265,
    AT_SYMBOL = 266,
    AMPERSAND = 267,
    OPEN_BRACKET = 268,
    CLOSED_BRACKET = 269,
    NEWLINE = 270,
    INTEGER_LITERAL = 271,
    STRING_LITERAL = 272,
    CSTRING = 273,
    COMMA = 274,
    CARROT = 275
  };
#endif
/* Tokens.  */
#define OPEN_BRACE 258
#define CLOSED_BRACE 259
#define OPEN_PAREN 260
#define CLOSED_PAREN 261
#define ADD_OP 262
#define MULT_OP 263
#define EQUAL_SIGN 264
#define TILDA 265
#define AT_SYMBOL 266
#define AMPERSAND 267
#define OPEN_BRACKET 268
#define CLOSED_BRACKET 269
#define NEWLINE 270
#define INTEGER_LITERAL 271
#define STRING_LITERAL 272
#define CSTRING 273
#define COMMA 274
#define CARROT 275

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{

  struct gdbwire_mi_output *u_output;
  struct gdbwire_mi_oob_record *u_oob_record;
  struct gdbwire_mi_result_record *u_result_record;
  int u_result_class;
  int u_async_record_kind;
  struct gdbwire_mi_result *u_result;
  char *u_token;
  struct gdbwire_mi_async_record *u_async_record;
  struct gdbwire_mi_stream_record *u_stream_record;
  int u_async_class;
  char *u_variable;
  char *u_cstring;
  struct gdbwire_mi_result *u_tuple;
  struct gdbwire_mi_result *u_list;
  int u_stream_record_kind;

};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



#ifndef YYPUSH_MORE_DEFINED
# define YYPUSH_MORE_DEFINED
enum { YYPUSH_MORE = 4 };
#endif

typedef struct gdbwire_mi_pstate gdbwire_mi_pstate;

int gdbwire_mi_push_parse (gdbwire_mi_pstate *ps, int pushed_char, YYSTYPE const *pushed_val, yyscan_t yyscanner, struct gdbwire_mi_output **gdbwire_mi_output);

gdbwire_mi_pstate * gdbwire_mi_pstate_new (void);
void gdbwire_mi_pstate_delete (gdbwire_mi_pstate *ps);

#endif /* !YY_GDBWIRE_MI_SRC_GDBWIRE_MI_GRAMMAR_H_INCLUDED  */
/***** End of gdbwire_mi_grammar.h *******************************************/
/***** Begin file gdbwire.h **************************************************/
#ifndef GDBWIRE_H
#define GDBWIRE_H

#ifdef __cplusplus 
extern "C" { 
#endif 

#include <stdlib.h>
/* #include "gdbwire_result.h" */
/* #include "gdbwire_mi_pt.h" */
/***** Include gdbwire_mi_command.h in the middle of gdbwire.h ***************/
/***** Begin file gdbwire_mi_command.h ***************************************/
#ifndef GDBWIRE_MI_COMMAND_H
#define GDBWIRE_MI_COMMAND_H

#ifdef __cplusplus 
extern "C" { 
#endif 

/* #include "gdbwire_result.h" */
/* #include "gdbwire_mi_pt.h" */

/**
 * An enumeration representing the supported GDB/MI commands.
 */
enum gdbwire_mi_command_kind {
    /* -break-info */
    GDBWIRE_MI_BREAK_INFO,

    /* -stack-info-frame */
    GDBWIRE_MI_STACK_INFO_FRAME,

    /* -file-list-exec-source-file */
    GDBWIRE_MI_FILE_LIST_EXEC_SOURCE_FILE,
    /* -file-list-exec-source-files */
    GDBWIRE_MI_FILE_LIST_EXEC_SOURCE_FILES
};

/** A linked list of source files. */
struct gdbwire_mi_source_file {
    /** A relative path to a file, never NULL */
    char *file;
    /**An absolute path to a file, NULL if unavailable */
    char *fullname;
    /** The next file name or NULL if no more. */
    struct gdbwire_mi_source_file *next;
};

/** The disposition of a breakpoint. What to do after hitting it. */
enum gdbwire_mi_breakpoint_disp_kind {
    GDBWIRE_MI_BP_DISP_DELETE,           /** Delete on next hit */
    GDBWIRE_MI_BP_DISP_DELETE_NEXT_STOP, /** Delete on next stop, hit or not */
    GDBWIRE_MI_BP_DISP_DISABLE,          /** Disable on next hit */
    GDBWIRE_MI_BP_DISP_KEEP,             /** Leave the breakpoint in place */
    GDBWIRE_MI_BP_DISP_UNKNOWN           /** When GDB doesn't specify */
};

/**
 * A linked list of breakpoints.
 *
 * A breakpoint is a breakpoint, a tracepoint, a watchpoing or a
 * catchpoint. The GDB breakpoint model is quite sophisticated.
 * This structure can be extended when necessary.
 */
struct gdbwire_mi_breakpoint {
    /**
     * The breakpoint number.
     *
     * An integer, however, for a breakpoint that represents one location of
     * a multiple location breakpoint, this will be a dotted pair, like ‘1.2’. 
     *
     * Never NULL.
     */
    char *number;

    /**
     * Determines if this is a multiple location breakpoint.
     *
     * True for a multi-location breakpoint, false otherwise.
     *
     * It is possible that a breakpoint corresponds to several locations in
     * your program. For example, several functions may have the same name.
     * For the following source code,
     *   int foo(int p) { return p; }
     *   double foo(double p) { return p; }
     *   int main() { int i = 1; double d = 2.3; return foo(i) + foo(d); }
     * If the user sets a breakpoint at foo by typing,
     *   b foo
     * Then gdb will create 3 breakpoints. The multiple location breakpoint,
     * which is the parent of the two breakpoints created for each foo
     * function. Here is the output of gdb from the CLI perspective,
     *   Num     Type           Disp Enb Address            What
     *   1       breakpoint     keep y   <MULTIPLE>         
     *   1.1                         y     0x4004dd in foo(int) at main.cpp:1
     *   1.2                         y     0x4004eb in foo(double) at main.cpp:2
     *
     * However, if the user created a breakpoint for main by typing,
     *   b main
     * Then gdb will only create a single breakpoint which would look like,
     *   1       breakpoint     keep y   0x4004fa in main() at main.cpp:3
     *
     * When this is true, the address field will be "<MULTIPLE>" and
     * the field multi_breakpoints will represent the breakpoints that this
     * multiple location breakpoint has created.
     */
    unsigned char multi:1;
    
    /**
     * True for breakpoints of a multi-location breakpoint, otherwise false.
     *
     * For the example above, 1.1 and 1.2 would have this field set true.
     *
     * When this is true, the field multi_breakpoint will represent
     * the multiple location breakpoint that has created this breakpoint.
     */
    unsigned char from_multi:1;

    /**
     * The breakpoint type.
     *
     * Typically "breakpoint", "watchpoint" or "catchpoint", but can be
     * a variety of different values. In gdb, see breakpoint.c:bptype_string
     * to see all the different possibilities.
     *
     * This will be NULL for breakpoints of a multiple location breakpoint.
     * In this circumstance, check the multi_breakpoint field for the
     * multiple location breakpoint type field.
     */
    char *type;

    /**
     * The type of the catchpoint or NULL if not a catch point.
     *
     * This field is only valid when the breakpoint is a catchpoint.
     * Unfortuntely, gdb says the "type" of the breakpoint in the type field
     * is "breakpoint" not "catchpoint". So if this field is non-NULL, it is
     * safe to assume that this breakpoint represents at catch point.
     */
    char *catch_type;

    /**
     * The breakpoint disposition.
     *
     * For multiple location breakpoints, this will be
     * GDBWIRE_MI_BP_DISP_UNKNOWN. In this circumstance, check the
     * multi_breakpoint field for the multiple location breakpoint
     * disposition field.
     */
    enum gdbwire_mi_breakpoint_disp_kind disposition;

    /** True if enabled or False if disabled. */
    unsigned char enabled:1;

    /**
     * The address of the breakpoint.
     *
     * This may be
     * - a hexidecimal number, representing the address
     * - the string ‘<PENDING>’ for a pending breakpoint
     * - the string ‘<MULTIPLE>’ for a breakpoint with multiple locations
     *
     * This field will be NULL if no address can be determined.
     * For example, a watchpoint does not have an address. 
     */
    char *address;

    /** 
     * The name of the function or NULL if unknown.
     */
    char *func_name;

    /**
     * A relative path to the file the breakpoint is in or NULL if unknown.
     */
    char *file;

    /**
     * An absolute path to the file the breakpoint is in or NULL if unknown.
     */
    char *fullname;

    /**
     * The line number the breakpoint is at or 0 if unkonwn.
     */
    unsigned long line;

    /**
     * The number of times this breakpoint has been hit.
     *
     * For breakpoints of multi-location breakpoints, this will be 0.
     * Look at the multi-location breakpoint field instead.
     */
    unsigned long times;

    /**
     * The location of the breakpoint as originally specified by the user. 
     *
     * This may be NULL for instance, for breakpoints for multi-breakpoints.
     */
    char *original_location;

    /**
     * True for a pending breakpoint, otherwise false.
     *
     * When this is true, the address field will be "<PENDING>".
     */
    unsigned char pending:1;

    /**
     * The breakpoints for a multi-location breakpoint.
     *
     * If multi is true, this will be the breakpoints associated with the
     * multiple location breakpoint. Otherwise will be NULL.
     */
    struct gdbwire_mi_breakpoint *multi_breakpoints;

    /**
     * A pointer to the multi location breakpoint that created this breakpoint.
     *
     * When the field from_multi is true, this will be a pointer to the
     * multi-location breakpoint that created this breakpoint. Otherwise NULL.
     *
     * For the example above in the multi field, breakpoints 1.1 and 1.2
     * would have this field pointing to the breakpoint 1.
     */
    struct gdbwire_mi_breakpoint *multi_breakpoint;


    /** The next breakpoint or NULL if no more. */
    struct gdbwire_mi_breakpoint *next;
};

struct gdbwire_mi_stack_frame {
    /**
     * The frame number.
     *
     * Where 0 is the topmost frame, i.e., the innermost function. 
     *
     * Always present.
     */
    unsigned level;

    /**
     * The address ($pc value) of the frame.
     *
     * May be NULL if GDB can not determine the frame address.
     */
    char *address;

   /**
    * The function name for the frame. May be NULL if unknown.
    */
   char *func;

   /**
    * The file name for the frame. May be NULL if unknown.
    */
   char *file;

   /**
    * The fullname name for the frame. May be NULL if unknown.
    */
   char *fullname;

   /**
    * Line number corresponding to the $pc. Maybe be 0 if unknown.
    */
   int line;

   /**
    * The shared library where this function is defined.
    *
    * This is only given if the frame's function is not known. 
    * May be NULL if unknown.
    */
   char *from;
};

/**
 * Represents a GDB/MI command.
 */
struct gdbwire_mi_command {
    /**
     * The kind of mi command this represents.
     */
    enum gdbwire_mi_command_kind kind;

    union {
        /** When kind == GDBWIRE_MI_BREAK_INFO */
        struct {
            /** The list of breakpoints, NULL if none exist.  */
            struct gdbwire_mi_breakpoint *breakpoints;
        } break_info;

        /** When kind == GDBWIRE_MI_STACK_INFO_FRAME */
        struct {
            struct gdbwire_mi_stack_frame *frame;
        } stack_info_frame;

        /** When kind == GDBWIRE_MI_FILE_LIST_EXEC_SOURCE_FILE */
        struct {
            /**
             * The line number the inferior is currently executing at.
             */
            int line;

            /**
             * The filename the inferior is currently executing at.
             *
             * This is usually a relative path.
             */
            char *file;

            /**
             * The filename the inferior is currently executing at.
             *
             * This is an absolute path.
             *
             * This command was addd in 2004, however, it was possible
             * at the time that only the "file" field would be put out and
             * the "fullname" field would be omitted. In 2012, in git commit,
             * f35a17b5, gdb was changed to always omit the "fullname" field.
             */
            char *fullname;

            /**
             * Determines if the file includes preprocessor macro information.
             *
             * This command was added in 2004. However, the macro-info
             * field was added to the output in 2008 in git commit 17784837.
             *
             * Only check this field if macro_info_exists is true.
             */
            char macro_info:1;

            /** True if macro-info field was in mi output, otherwise false */
            char macro_info_exists:1;
        } file_list_exec_source_file;

        /** When kind == GDBWIRE_MI_FILE_LIST_EXEC_SOURCE_FILES */
        struct {
            /**
             * A list of files that make up the inferior.
             *
             * When there are no files (if the gdb does not have an inferior
             * loaded) than files will be NULL.
             *
             * This command was addd in 2004, however, it was possible
             * at the time that only the "file" field would be put out and
             * the "fullname" field would be omitted. In 2012, in git commit,
             * f35a17b5, gdb was changed to always omit the "fullname" field.
             */
            struct gdbwire_mi_source_file *files;
        } file_list_exec_source_files;
        
    } variant;
};

/**
 * Get a gdbwire MI command from the result record.
 *
 * @param kind
 * The kind of command the result record is associated with.
 *
 * @param result_record
 * The result record to turn into a command.
 *
 * @param out_mi_command
 * Will return an allocated gdbwire mi command if GDBWIRE_OK is returned
 * from this function. You should free this memory with
 * gdbwire_mi_command_free when you are done with it.
 *
 * @return
 * The result of this function.
 */
enum gdbwire_result gdbwire_get_mi_command(
        enum gdbwire_mi_command_kind kind,
        struct gdbwire_mi_result_record *result_record,
        struct gdbwire_mi_command **out_mi_command);

/**
 * Free the gdbwire mi command.
 *
 * @param mi_command
 * The mi command to free.
 */
void gdbwire_mi_command_free(struct gdbwire_mi_command *mi_command);

#ifdef __cplusplus 
}
#endif 

#endif
/***** End of gdbwire_mi_command.h *******************************************/
/***** Continuing where we left off in gdbwire.h *****************************/

/* The opaque gdbwire context */
struct gdbwire;

/**
 * The primary mechanism for gdbwire to send events to the caller.
 *
 * The flow is like this:
 * - create a gdbwire instance
 * - loop:
 *   - call gdbwire functions to send commands to gdb
 *   - receive callback events with results when they become available
 * - destroy the instance
 */
struct gdbwire_callbacks {
    /**
     * An arbitrary pointer to associate with the events.
     *
     * This pointer will be passed back to the caller in each event.
     */
    void *context;
    
    /**
     * A console, target or log output event has occured.
     *
     * @param context
     * The context pointer above.
     *
     * @param stream_record
     * The stream record to display to the user.
     */
    void (*gdbwire_stream_record_fn)(void *context,
            struct gdbwire_mi_stream_record *stream_record);

    /**
     * An asynchronous output event.
     *
     * @param context
     * The context pointer above.
     *
     * @param async_record
     * The asychronous record output by GDB.
     */
    void (*gdbwire_async_record_fn)(void *context,
            struct gdbwire_mi_async_record *async_record);

    /**
     * A result output event.
     *
     * @param context
     * The context pointer above.
     *
     * @param result_record
     * The result record output by GDB.
     */
    void (*gdbwire_result_record_fn)(void *context,
            struct gdbwire_mi_result_record *result_record);

    /**
     * A prompt output event.
     *
     * @param context
     * The context pointer above.
     *
     * @param prompt
     * The prompt output to display to the user.
     */
    void (*gdbwire_prompt_fn)(void *context, const char *prompt);

    /**
     * A gdbwire parse error occurred.
     *
     * If you receive this callback, that means the gdbwire parser
     * failed to parse some gdb/mi coming out of gdb.
     * Please send the parameters received in this callback to the
     * gdbwire develpment team.
     *
     * @param context
     * The context pointer above.
     *
     * @param mi
     * The mi string that gdbwire could not parse.
     *
     * @param token
     * The token the error occurred on.
     *
     * @param position
     * The position of the token the error occurred on.
     */
    void (*gdbwire_parse_error_fn)(void *context, const char *mi,
            const char *token, struct gdbwire_mi_position position);
};

/**
 * Create a gdbwire context.
 *
 * Each gdbwire structure is capable of talking to a single gdb instance.
 *
 * @param callbacks
 * The callback functions for when events should be sent. Be sure to
 * initialize all of the callback functions. If a callback event is
 * initialized to NULL, it will not be called.
 *
 * @return
 * A new gdbwire instance or NULL on error.
 */
struct gdbwire *gdbwire_create(struct gdbwire_callbacks callbacks);

/**
 * Destroy a gdbwire context.
 *
 * This function will do nothing if the instance is NULL.
 *
 * @param gdbwire
 * The instance of gdbwire to destroy
 */
void gdbwire_destroy(struct gdbwire *wire);

/**
 * Push some GDB output characters to gdbwire for processing.
 *
 * Currently, the calling application is responsible for reading the
 * output of GDB and sending it to gdbwire. This may change in the future.
 * Call this function with output from GDB when it is available.
 *
 * During this function, callback events may be invoked to alert the
 * caller of useful gdbwire_mi events.
 *
 * @param wire
 * The gdbwire context to operate on.
 *
 * @param data
 * The data to push to gdbwire for interpretation.
 *
 * @param size
 * The size of the data to push to gdbwire.
 *
 * @return
 * GDBWIRE_OK on success or appropriate error result on failure.
 */
enum gdbwire_result gdbwire_push_data(struct gdbwire *wire, const char *data,
        size_t size);

/**
 * Handle an interpreter-exec command.
 *
 * Typically, a front end would start gdb with the MI interface and create
 * a corresponding gdbwire instance. The front end would feed the gdbwire
 * instance all of the MI output. In this scenario, gdbwire triggers
 * callbacks when interesting events occur.
 *
 * Some GDB front ends use the annotate interface with gdb, and will
 * transition to using MI through the use of the interpreter-exec command.
 * In this scenario, the front end will send GDB a single interpreter-exec
 * command and will want to interpret the output of only that command.
 * For this use case, a gdbwire instance is not necessary for the front end,
 * nor any of the callbacks associated with that instance.
 *
 * This function provides a way for a front end to interpret the output
 * of a single interpreter-exec command with out the need for creating
 * a gdbwire instance or any gdbwire callbacks.
 *
 * @param interpreter_exec_output
 * The MI output from GDB for the interpreter exec command.
 *
 * @param kind
 * The interpreter-exec command kind.
 *
 * @param out_mi_command
 * Will return an allocated gdbwire mi command if GDBWIRE_OK is returned
 * from this function. You should free this memory with
 * gdbwire_mi_command_free when you are done with it.
 *
 * @return
 * The result of this function.
 */
enum gdbwire_result gdbwire_interpreter_exec(
        const char *interpreter_exec_output,
        enum gdbwire_mi_command_kind kind,
        struct gdbwire_mi_command **out_mi_command);

#ifdef __cplusplus 
}
#endif 

#endif
/***** End of gdbwire.h ******************************************************/
