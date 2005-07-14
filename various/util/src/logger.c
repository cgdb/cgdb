#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#if HAVE_STDIO_H
#include <stdio.h> 
#endif /* HAVE_STDIO_H */

#if HAVE_STDARG_H
#include <stdarg.h>     /* ANSI C header file */
#endif /* HAVE_STDARG_H */

#if HAVE_STDLIB_H 
#include <stdlib.h>
#endif  /* HAVE_STDLIB_H */

#if HAVE_ERRNO_H
#include <errno.h>
#endif /* HAVE_ERRNO_H */

#define MAXLINE 4096

#include "logger.h"

struct logger *logger = NULL;

struct logger {
	/** The name of the file that is being logged to. */
	char *log_file;

	/** 
	 * The file descriptor being written to. 
	 * NULL if there is no current file being written to.
	 */
	FILE *fd;

	/**
	 * A flag telling if this logger has had any data written to it
	 * since it was opened.
	 */
	int used;

	/**
	 * A flag to tell if the logger is currently recording data
	 * passed to it. 
	 * 1 if it is recording.
	 * 0 if it is not.
	 */
	int recording;
};

struct logger *logger_create ( void ) {
	struct logger *log;

	log = (struct logger*)malloc ( sizeof ( struct logger ) );

	if ( !log )
		return NULL;

	log->log_file = NULL;
	log->fd = NULL;
	log->used = 0;
	log->recording = 1;

	return log;
}

int logger_destroy ( struct logger *log ) {

	if ( !log )
		return 0;

	if ( log->log_file ) {
		free ( log->log_file );
		log->log_file = NULL;
	}

	if ( log->fd ) {
		fclose ( log->fd );
		log->fd = NULL;
	}

	free ( log );
	log = NULL;

	return 0;
}

/**
 * This is used to close or discard the data was being used to
 * write to a file, descriptor or whatever else.
 * After this is called, another file or descriptor can be setup, 
 * and the logger should use that from then on.
 *
 * \param log
 * The logger context.
 *
 * \return
 * 0 on success, or -1 on error.
 */
static int logger_close_writer ( struct logger *log ) {
	if ( !log )
		return -1;

	/* A file is being written to */
	if ( log->log_file ) {
		free ( log->log_file );
		log->log_file = NULL;

		fclose ( log->fd );
		log->fd = NULL;
	/* A descriptor was passed in, discard it */
	} else if ( log->fd ) {
		log->fd = NULL;
	}

	log->used = 0;

	return 0;
}

int logger_set_file ( struct logger *log, const char *file ) {

	if ( !log )
		return -1;

	if ( !file )
		return -1;

	/* prepare for writing */
	if ( logger_close_writer ( log ) == -1 )
		return -1;

	log->fd = fopen ( file, "w" );

	/* If the open failed, just return */
	if ( !log->fd ) {
        printf ( "Error: Could not open file %s for writing\n", file );
		return -1;
    }

	log->log_file = strdup ( file );

	return 0;
}

int logger_set_fd ( struct logger *log, FILE *fd ) {
	if ( !log )
		return -1;

	if ( !fd )
		return -1;


	/* prepare for writing */
	if ( logger_close_writer ( log ) == -1 )
		return -1;

	log->fd = fd;

	return 0;
}

int logger_get_file ( struct logger *log, char **file ) {

	if ( !file )
		return -1;

	*file = 0;

	if ( !log )
		return -1;

	*file = log->log_file;

	return 0;
}

int logger_write_pos ( 
		struct logger *log, 
		const char *file,
	    int line,
		const char *fmt,
		...) {
    va_list ap;
    char va_buf[MAXLINE];

	if ( !log )
		return -1;

	/* It's OK to write nothing */
	if ( !fmt )
		return 0;

	if ( !log->recording )
		return 0;

    /* Get the buffer with format */
    va_start(ap, fmt);
#ifdef   HAVE_VSNPRINTF
    vsnprintf(va_buf, sizeof(va_buf), fmt, ap);  /* this is safe */
#else
    vsprintf(va_buf, fmt, ap);             /* this is not safe */
#endif
    va_end(ap);

	fprintf ( log->fd, "%s:%d %s\n", file, line, va_buf );

	log->used = 1;

	return 0;
}

int logger_has_recv_data ( struct logger *log, int *has_recv_data ) {
	if ( !has_recv_data )
		return -1;

	*has_recv_data = 0;

	if ( !log )
		return 0;


	*has_recv_data = log->used;

	return 0;
}

int logger_set_record ( struct logger *log, int record ) {
	if ( !log )
		return -1;

	if ( record == 1 )
		log->recording = 1;

	if ( record == 0 )
		log->recording = 0;

	return 0;
}

int logger_is_recording ( struct logger *log ) {
	if ( !log )
		return -1;

	return log->recording;
}
