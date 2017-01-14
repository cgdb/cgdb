/* clog: Extremely simple logger for C.
 *
 * Features:
 * - Implemented purely as a single header file.
 * - Create multiple loggers.
 * - Four log levels (debug, info, warn, error).
 * - Custom formats.
 * - Fast.
 *
 * Dependencies:
 * - Should conform to C89, C++98 (but requires vsnprintf, unfortunately).
 * - POSIX environment.
 *
 * USAGE:
 *
 * Include this header in any file that wishes to write to logger(s).  In
 * exactly one file (per executable), define CLOG_MAIN first (e.g. in your
 * main .c file).
 *
 *     #define CLOG_MAIN
 *     #include "clog.h"
 *
 * This will define the actual objects that all the other units will use.
 *
 * Loggers are identified by integers (0 - 15).  It's expected that you'll
 * create meaningful constants and then refer to the loggers as such.
 *
 * Example:
 *
 *  const int MY_LOGGER = 0;
 *
 *  int main() {
 *      int r;
 *      r = clog_init_path(MY_LOGGER, "my_log.txt");
 *      if (r != 0) {
 *          fprintf(stderr, "Logger initialization failed.\n");
 *          return 1;
 *      }
 *      clog_info(CLOG(MY_LOGGER), "Hello, world!");
 *      clog_free(MY_LOGGER);
 *      return 0;
 *  }
 *
 * The CLOG macro used in the call to clog_info is a helper that passes the
 * __FILE__ and __LINE__ parameters for you, so you don't have to type them
 * every time. (It could be prettier with variadic macros, but that requires
 * C99 or C++11 to be standards compliant.)
 *
 * Errors encountered by clog will be printed to stderr.  You can suppress
 * these by defining a macro called CLOG_SILENT before including clog.h.
 *
 * License: Do whatever you want. It would be nice if you contribute
 * improvements as pull requests here:
 *
 *   https://github.com/mmueller/clog
 *
 * Copyright 2013 Mike Mueller <mike@subfocal.net>.
 *
 * As is; no warranty is provided; use at your own risk.
 */

#ifndef __CLOG_H__
#define __CLOG_H__

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/* Number of loggers that can be defined. */
#define CLOG_MAX_LOGGERS 16

/* Format strings cannot be longer than this. */
#define CLOG_FORMAT_LENGTH 256

/* Formatted times and dates should be less than this length. If they are not,
 * they will not appear in the log. */
#define CLOG_DATETIME_LENGTH 256

/* Default format strings. */
#define CLOG_DEFAULT_FORMAT "%d %t %f(%n): %l: %m\n"
#define CLOG_DEFAULT_DATE_FORMAT "%Y-%m-%d"
#define CLOG_DEFAULT_TIME_FORMAT "%H:%M:%S"

/* GCC defines __va_copy, but does not define va_copy unless in c99 mode
   or -ansi is not specified, since it was not part of C90. */
#if !defined(va_copy) && defined(__va_copy)
#define va_copy(d,s)    __va_copy(d,s)
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum clog_level {
    CLOG_DEBUG,
    CLOG_INFO,
    CLOG_WARN,
    CLOG_ERROR
};

struct clog;

/**
 * Create a new logger writing to the given file path.  The file will always
 * be opened in append mode.
 *
 * @param id
 * A constant integer between 0 and 15 that uniquely identifies this logger.
 *
 * @param path
 * Path to the file where log messages will be written.
 *
 * @return
 * Zero on success, non-zero on failure.
 */
int clog_init_path(int id, const char *const path);

/**
 * Create a new logger writing to a file descriptor.
 *
 * @param id
 * A constant integer between 0 and 15 that uniquely identifies this logger.
 *
 * @param fd
 * The file descriptor where log messages will be written.
 *
 * @return
 * Zero on success, non-zero on failure.
 */
int clog_init_fd(int id, int fd);

/**
 * Destroy (clean up) a logger.  You should do this at the end of execution,
 * or when you are done using the logger.
 *
 * @param id
 * The id of the logger to destroy.
 */
void clog_free(int id);

#define CLOG(id) __FILE__, __LINE__, id

/**
 * Log functions (one per level).  Call these to write messages to the log
 * file.  The first three arguments can be replaced with a call to the CLOG
 * macro defined above, e.g.:
 *
 *     clog_debug(CLOG(MY_LOGGER_ID), "This is a log message.");
 *
 * @param sfile
 * The name of the source file making this log call (e.g. __FILE__).
 *
 * @param sline
 * The line number of the call in the source code (e.g. __LINE__).
 *
 * @param id
 * The id of the logger to write to.
 *
 * @param fmt
 * The format string for the message (printf formatting).
 *
 * @param ...
 * Any additional format arguments.
 */
void clog_debug(const char *sfile, int sline, int id, const char *fmt, ...);
void clog_info(const char *sfile, int sline, int id, const char *fmt, ...);
void clog_warn(const char *sfile, int sline, int id, const char *fmt, ...);
void clog_error(const char *sfile, int sline, int id, const char *fmt, ...);

/**
 * Set the minimum level of messages that should be written to the log.
 * Messages below this level will not be written.  By default, loggers are
 * created with level == CLOG_DEBUG.
 *
 * @param id
 * The identifier of the logger.
 *
 * @param level
 * The new minimum log level.
 *
 * @return
 * Zero on success, non-zero on failure.
 */
int clog_set_level(int id, enum clog_level level);

/**
 * Set the format string used for times.  See strftime(3) for how this string
 * should be defined.  The default format string is CLOG_DEFAULT_TIME_FORMAT.
 *
 * @param fmt
 * The new format string, which must be less than CLOG_FORMAT_LENGTH bytes.
 *
 * @return
 * Zero on success, non-zero on failure.
 */
int clog_set_time_fmt(int id, const char *fmt);

/**
 * Set the format string used for dates.  See strftime(3) for how this string
 * should be defined.  The default format string is CLOG_DEFAULT_DATE_FORMAT.
 *
 * @param fmt
 * The new format string, which must be less than CLOG_FORMAT_LENGTH bytes.
 *
 * @return
 * Zero on success, non-zero on failure.
 */
int clog_set_date_fmt(int id, const char *fmt);

/**
 * Set the format string for log messages.  Here are the substitutions you may
 * use:
 *
 *     %f: Source file name generating the log call.
 *     %n: Source line number where the log call was made.
 *     %m: The message text sent to the logger (after printf formatting).
 *     %d: The current date, formatted using the logger's date format.
 *     %t: The current time, formatted using the logger's time format.
 *     %l: The log level (one of "DEBUG", "INFO", "WARN", or "ERROR").
 *     %%: A literal percent sign.
 *
 * The default format string is CLOG_DEFAULT_FORMAT.
 *
 * @param fmt
 * The new format string, which must be less than CLOG_FORMAT_LENGTH bytes.
 * You probably will want to end this with a newline (\n).
 *
 * @return
 * Zero on success, non-zero on failure.
 */
int clog_set_fmt(int id, const char *fmt);

/*
 * No need to read below this point.
 */

/*
 * Portability stuff.
 */

/* This is not portable, but should work on old Visual C++ compilers. Visual
 * Studio 2013 defines va_copy, but older versions do not. */
#ifdef _MSC_VER
#if _MSC_VER < 1800
#define va_copy(a,b) ((a) = (b))
#endif
#endif

/**
 * The C logger structure.
 */
struct clog {

    /* The current level of this logger. Messages below it will be dropped. */
    enum clog_level level;

    /* The file being written. */
    int fd;

    /* The format specifier. */
    char fmt[CLOG_FORMAT_LENGTH];

    /* Date format */
    char date_fmt[CLOG_FORMAT_LENGTH];

    /* Time format */
    char time_fmt[CLOG_FORMAT_LENGTH];

    /* Tracks whether the fd needs to be closed eventually. */
    int opened;
};

void _clog_err(const char *fmt, ...);

#ifdef CLOG_MAIN
struct clog *_clog_loggers[CLOG_MAX_LOGGERS] = { 0 };
#else
extern struct clog *_clog_loggers[CLOG_MAX_LOGGERS];
#endif

#ifdef CLOG_MAIN

const char *const CLOG_LEVEL_NAMES[] = {
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR",
};

int
clog_init_path(int id, const char *const path)
{
    int fd = open(path, O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (fd == -1) {
        _clog_err("Unable to open %s: %s\n", path, strerror(errno));
        return 1;
    }
    if (clog_init_fd(id, fd)) {
        close(fd);
        return 1;
    }
    _clog_loggers[id]->opened = 1;
    return 0;
}

int
clog_init_fd(int id, int fd)
{
    struct clog *logger;

    if (_clog_loggers[id] != NULL) {
        _clog_err("Logger %d already initialized.\n", id);
        return 1;
    }

    logger = (struct clog *) malloc(sizeof(struct clog));
    if (logger == NULL) {
        _clog_err("Failed to allocate logger: %s\n", strerror(errno));
        return 1;
    }

    logger->level = CLOG_DEBUG;
    logger->fd = fd;
    logger->opened = 0;
    strcpy(logger->fmt, CLOG_DEFAULT_FORMAT);
    strcpy(logger->date_fmt, CLOG_DEFAULT_DATE_FORMAT);
    strcpy(logger->time_fmt, CLOG_DEFAULT_TIME_FORMAT);

    _clog_loggers[id] = logger;
    return 0;
}

void
clog_free(int id)
{
    if (_clog_loggers[id]) {
        if (_clog_loggers[id]->opened) {
            close(_clog_loggers[id]->fd);
        }
        free(_clog_loggers[id]);
        _clog_loggers[ id ] = 0;
    }
}

int
clog_set_level(int id, enum clog_level level)
{
    if (_clog_loggers[id] == NULL) {
        return 1;
    }
    if ((unsigned) level > CLOG_ERROR) {
        return 1;
    }
    _clog_loggers[id]->level = level;
    return 0;
}

int
clog_set_time_fmt(int id, const char *fmt)
{
    struct clog *logger = _clog_loggers[id];
    if (logger == NULL) {
        _clog_err("clog_set_time_fmt: No such logger: %d\n", id);
        return 1;
    }
    if (strlen(fmt) >= CLOG_FORMAT_LENGTH) {
        _clog_err("clog_set_time_fmt: Format specifier too long.\n");
        return 1;
    }
    strcpy(logger->time_fmt, fmt);
    return 0;
}

int
clog_set_date_fmt(int id, const char *fmt)
{
    struct clog *logger = _clog_loggers[id];
    if (logger == NULL) {
        _clog_err("clog_set_date_fmt: No such logger: %d\n", id);
        return 1;
    }
    if (strlen(fmt) >= CLOG_FORMAT_LENGTH) {
        _clog_err("clog_set_date_fmt: Format specifier too long.\n");
        return 1;
    }
    strcpy(logger->date_fmt, fmt);
    return 0;
}

int
clog_set_fmt(int id, const char *fmt)
{
    struct clog *logger = _clog_loggers[id];
    if (logger == NULL) {
        _clog_err("clog_set_fmt: No such logger: %d\n", id);
        return 1;
    }
    if (strlen(fmt) >= CLOG_FORMAT_LENGTH) {
        _clog_err("clog_set_fmt: Format specifier too long.\n");
        return 1;
    }
    strcpy(logger->fmt, fmt);
    return 0;
}

/* Internal functions */

size_t
_clog_append_str(char **dst, char *orig_buf, const char *src, size_t cur_size)
{
    size_t new_size = cur_size;

    while (strlen(*dst) + strlen(src) >= new_size) {
        new_size *= 2;
    }
    if (new_size != cur_size) {
        if (*dst == orig_buf) {
            *dst = (char *) malloc(new_size);
            strcpy(*dst, orig_buf);
        } else {
            *dst = (char *) realloc(*dst, new_size);
        }
    }

    strcat(*dst, src);
    return new_size;
}

size_t
_clog_append_int(char **dst, char *orig_buf, long int d, size_t cur_size)
{
    char buf[40]; /* Enough for 128-bit decimal */
    if (snprintf(buf, 40, "%ld", d) >= 40) {
        return cur_size;
    }
    return _clog_append_str(dst, orig_buf, buf, cur_size);
}

size_t
_clog_append_time(char **dst, char *orig_buf, struct tm *lt,
                  const char *fmt, size_t cur_size)
{
    char buf[CLOG_DATETIME_LENGTH];
    size_t result = strftime(buf, CLOG_DATETIME_LENGTH, fmt, lt);

    if (result > 0) {
        return _clog_append_str(dst, orig_buf, buf, cur_size);
    }

    return cur_size;
}

const char *
_clog_basename(const char *path)
{
    const char *slash = strrchr(path, '/');
    if (slash) {
        path = slash + 1;
    }
#ifdef _WIN32
    slash = strrchr(path, '\\');
    if (slash) {
        path = slash + 1;
    }
#endif
    return path;
}

char *
_clog_format(const struct clog *logger, char buf[], size_t buf_size,
             const char *sfile, int sline, const char *level,
             const char *message)
{
    size_t cur_size = buf_size;
    char *result = buf;
    enum { NORMAL, SUBST } state = NORMAL;
    size_t fmtlen = strlen(logger->fmt);
    size_t i;
    time_t t = time(NULL);
    struct tm *lt = localtime(&t);

    sfile = _clog_basename(sfile);
    result[0] = 0;
    for (i = 0; i < fmtlen; ++i) {
        if (state == NORMAL) {
            if (logger->fmt[i] == '%') {
                state = SUBST;
            } else {
                char str[2] = { 0 };
                str[0] = logger->fmt[i];
                cur_size = _clog_append_str(&result, buf, str, cur_size);
            }
        } else {
            switch (logger->fmt[i]) {
                case '%':
                    cur_size = _clog_append_str(&result, buf, "%", cur_size);
                    break;
                case 't':
                    cur_size = _clog_append_time(&result, buf, lt,
                                                 logger->time_fmt, cur_size);
                    break;
                case 'd':
                    cur_size = _clog_append_time(&result, buf, lt,
                                                 logger->date_fmt, cur_size);
                    break;
                case 'l':
                    cur_size = _clog_append_str(&result, buf, level, cur_size);
                    break;
                case 'n':
                    cur_size = _clog_append_int(&result, buf, sline, cur_size);
                    break;
                case 'f':
                    cur_size = _clog_append_str(&result, buf, sfile, cur_size);
                    break;
                case 'm':
                    cur_size = _clog_append_str(&result, buf, message,
                                                cur_size);
                    break;
            }
            state = NORMAL;
        }
    }

    return result;
}

void
_clog_log(const char *sfile, int sline, enum clog_level level,
          int id, const char *fmt, va_list ap)
{
    /* For speed: Use a stack buffer until message exceeds 4096, then switch
     * to dynamically allocated.  This should greatly reduce the number of
     * memory allocations (and subsequent fragmentation). */
    char buf[4096];
    size_t buf_size = 4096;
    char *dynbuf = buf;
    char *message;
    va_list ap_copy;
    int result;
    struct clog *logger = _clog_loggers[id];

    if (!logger) {
        _clog_err("No such logger: %d\n", id);
        return;
    }

    if (level < logger->level) {
        return;
    }

    /* Format the message text with the argument list. */
    va_copy(ap_copy, ap);
    result = vsnprintf(dynbuf, buf_size, fmt, ap);
    if ((size_t) result >= buf_size) {
        buf_size = result + 1;
        dynbuf = (char *) malloc(buf_size);
        result = vsnprintf(dynbuf, buf_size, fmt, ap_copy);
        if ((size_t) result >= buf_size) {
            /* Formatting failed -- too large */
            _clog_err("Formatting failed (1).\n");
            va_end(ap_copy);
            free(dynbuf);
            return;
        }
    }
    va_end(ap_copy);

    /* Format according to log format and write to log */
    {
        char message_buf[4096];
        message = _clog_format(logger, message_buf, 4096, sfile, sline,
                               CLOG_LEVEL_NAMES[level], dynbuf);
        if (!message) {
            _clog_err("Formatting failed (2).\n");
            if (dynbuf != buf) {
                free(dynbuf);
            }
            return;
        }
        result = write(logger->fd, message, strlen(message));
        if (result == -1) {
            _clog_err("Unable to write to log file: %s\n", strerror(errno));
        }
        if (message != message_buf) {
            free(message);
        }
        if (dynbuf != buf) {
            free(dynbuf);
        }
    }
}

void
clog_debug(const char *sfile, int sline, int id, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    _clog_log(sfile, sline, CLOG_DEBUG, id, fmt, ap);
    va_end(ap);
}

void
clog_info(const char *sfile, int sline, int id, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    _clog_log(sfile, sline, CLOG_INFO, id, fmt, ap);
    va_end(ap);
}

void
clog_warn(const char *sfile, int sline, int id, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    _clog_log(sfile, sline, CLOG_WARN, id, fmt, ap);
    va_end(ap);
}

void
clog_error(const char *sfile, int sline, int id, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    _clog_log(sfile, sline, CLOG_ERROR, id, fmt, ap);
    va_end(ap);
}

void
_clog_err(const char *fmt, ...)
{
#ifdef CLOG_SILENT
    (void) fmt;
#else
    va_list ap;

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
#endif
}

#endif /* CLOG_MAIN */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __CLOG_H__ */
