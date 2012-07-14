#ifndef __LOGGER_H__
#define __LOGGER_H__

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_STDIO_H
#include <stdio.h>
#endif /* HAVE_STDIO_H */

/* Doxygen headers {{{ */
/*!
 * \file
 * logger.h
 *
 * \brief
 * This interface is intended to be the abstraction layer between an application 
 * and any data logging that application needs to perform.
 */
/* }}} */

/* struct logger {{{ */

/**
 * @name Creating and destroying a logger
 *
 * A log context is basically a way to store data into a file.
 * You can initialize the logger, give it data, and then close it.
 */

/*@{*/

struct logger;
typedef struct logger *logger_ptr;

/**
 * Create a logger context.
 *
 * \return
 * The new instance of a logger, or NULL on error
 */
struct logger *logger_create(void);

/**
 * Destroy a logger context.
 *
 * \param log
 * The instance to destroy
 *
 * \return
 * 0 on succes, or -1 on error.
 */
int logger_destroy(struct logger *log);

/*@}*/

/*@{*/

/**
 * @name Setting file names
 * These functions control where the logger writes to
 */

/**
 * Tells the logger which file to write to.
 *
 * If a file was already being written to, this will close that 
 * file and start writing to the new file.
 *
 * If a descriptor was being written to, the descriptor will be 
 * discarded, and the new file will be written to.
 *
 * \param log
 * The logger to set the file to.
 *
 * \param file
 * The name of the file to write to
 *
 * \return
 * 0 on success, or -1 on error
 */
int logger_set_file(struct logger *log, const char *file);

/**
 * Get's the file that the logger is writting to.
 * This could be used to tell the user what file the error's
 * are being sent to.
 *
 * \param log
 * The logger context
 *
 * \param file
 * On return, this will be set to the file that the logger is 
 * writing to. The memory returned should not be modified.
 *
 * \return
 * 0 on success, or -1 on error
 */
int logger_get_file(struct logger *log, char **file);

/**
 * Set's the stream that the logger should write to.
 * The stream should be open, before being passed in, the 
 * logger doesn't attempt to open or close it in any way, 
 * it only writes to the stream. 
 *
 * If a file was already being written to, this will close that 
 * file and start writing to the new descriptor.
 *
 * If a descriptor was being written to, the descriptor will be 
 * discarded, and the new descriptor will be used.
 *
 * \param log
 * The logger context
 *
 * \param fd
 * The file descriptor the logger should write to.
 *
 * \return
 * 0 on success, or -1 on error
 */
int logger_set_fd(struct logger *log, FILE * fd);

/*@}*/

/*@{*/

/**
 * @name Writing to the logger
 * These functions allow the client to write data to the logger
 */

/**
 * Write data to the logger, including a position
 *
 * \param log
 * The logger context to write to.
 *
 * \param file
 * The name of the file the error was produced in
 *
 * \param line
 * The line number the error came from
 *
 * \param fmt
 * The format of the error message
 *
 * \param ...
 * The data to write
 *
 * \return
 * 0 on succes, -1 on error
 */
int logger_write_pos(struct logger *log,
        const char *file, int line, const char *fmt, ...);

/*@}*/

/*@{*/

/**
 * @name Querying the status of the logger
 * These functions query the logger to find out information about it.
 */

/**
 * Checks to see if the logger has had any data written to it.
 *
 * \param log
 * The logger context
 *
 * \param has_recv_data
 * This will return as 1 if data has been written to the logger since
 * it was created, otherwise, it will return as 0.
 *
 * \return
 * 0 on succes, -1 on error
 */
int logger_has_recv_data(struct logger *log, int *has_recv_data);

/**
 * This tells the logger if it should be recording the data that
 * is sent to it or not. By default, the logger will record data
 * that is written to it. 
 *
 * \param log
 * The logger context
 *
 * \param record
 * 1 if you wish to have the logger record the data ( default )
 * 0 if you wish the logger to ignore the data it recieves.
 *
 * \return
 * 0 on succes, -1 on error
 */
int logger_set_record(struct logger *log, int record);

/**
 * Checks to see if the logger is currently recording.
 *
 * \param log
 * The logger context
 *
 * \return
 * 1 if it is recording, 0 otherwise.
 */
int logger_is_recording(struct logger *log);

/*@}*/

extern struct logger *logger;

/* }}} */

#endif
