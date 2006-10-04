#include "tgdb_client_interface.h"

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif /* HAVE_STDLIB_H */

#include "gdbmi_tgdb.h"
#include "a2-tgdb.h"
#include "sys_util.h"

/**
 * This is a list of all of the client interfaces that TGDB supports.
 */
static struct tgdb_client_debugger_interfaces {

	/**
	 * This represents the type of debugger to support.
	 */
	enum tgdb_client_supported_debuggers debugger;

	/**
	 * This represents the type of protocol to support.
	 */
	enum tgdb_client_supported_protocols protocol;

	void *(*tgdb_client_create_context) ( 
			const char *debugger_path, 
			int argc, char **argv,
			const char *config_dir,
			struct logger *logger );

	int (*tgdb_client_initialize_context)(
			void *ctx,
			int *debugger_stdin, int *debugger_stdout,
			int *inferior_stdin, int *inferior_stdout );

	int (*tgdb_client_destroy_context)( 
			void *ctx );

	int (*tgdb_client_err_msg)( 
			void *ctx );

	int (*tgdb_client_is_client_ready) ( 
			void *ctx );

	int (*tgdb_client_tgdb_ran_command) ( 
			void *ctx );

	int (*tgdb_client_prepare_for_command) (
			void *ctx,
			struct tgdb_command *com );

	int (*tgdb_client_can_tgdb_run_commands) ( 
			void *ctx );

	int (*tgdb_client_parse_io) ( 
			void *ctx,
			const char *input_data, const size_t input_data_size,
			char *debugger_output, size_t *debugger_output_size,
			char *inferior_output, size_t *inferior_output_size,
		    struct tgdb_list *list);

	struct tgdb_list *(*tgdb_client_get_client_commands) ( 
			void *ctx );

	int (*tgdb_client_get_filename_pair) ( 
			void *ctx, 
			const char *path );

	int (*tgdb_client_get_current_location) (void *ctx, int on_startup);

	int (*tgdb_client_get_inferior_source_files) ( 
			void *ctx );

	int (*tgdb_client_completion_callback) (
			void *ctx,
			const char *completion_command);

	char *(*tgdb_client_return_command) (
			void *ctx,
			enum tgdb_command_type c );

	char *(*tgdb_client_modify_breakpoint) ( 
			void *ctx, 
			const char *file, 
			int line, 
			enum tgdb_breakpoint_action b );

	pid_t (*tgdb_client_get_debugger_pid) ( 
			void *ctx );

	int (*tgdb_client_open_new_tty) ( 
			void *ctx,
			int *inferior_stdin, 
			int *inferior_stdout );

	const char *(*tgdb_client_get_tty_name) ( 
			void *ctx );

} tgdb_client_debugger_interfaces[] = {
	{ 
		TGDB_CLIENT_DEBUGGER_GNU_GDB,
		TGDB_CLIENT_PROTOCOL_GNU_GDB_ANNOTATE_TWO,

		/* tgdb_client_create_context */
		a2_create_context,
		/* tgdb_client_initialize_context */
		a2_initialize,
		/* tgdb_client_destroy_context */
		a2_shutdown,
		/* tgdb_client_err_msg */
		a2_err_msg,
		/* tgdb_client_is_client_ready */
		a2_is_client_ready,
	    /* tgdb_client_tgdb_ran_command */
		a2_user_ran_command,
		/* tgdb_client_prepare_for_command */
		a2_prepare_for_command,
		/* tgdb_client_can_tgdb_run_commands */
		a2_is_misc_prompt,
		/* tgdb_client_parse_io */
		a2_parse_io,
		/* tgdb_client_get_client_commands */
		a2_get_client_commands,
		/* tgdb_client_get_filename_pair */
		a2_get_source_filename_pair,
		/* tgdb_client_get_current_location */
		a2_get_current_location,
		/* tgdb_client_get_inferior_source_files */
		a2_get_inferior_sources,
		/* tgdb_client_completion_callback */
		a2_completion_callback,
		/* tgdb_client_return_command*/
		a2_return_client_command,
		/* tgdb_client_modify_breakpoint */
		a2_client_modify_breakpoint,
		/* tgdb_client_get_debugger_pid */
		a2_get_debugger_pid,
		/* tgdb_client_open_new_tty */
		a2_open_new_tty,
		/* tgdb_client_get_tty_name */
		a2_get_tty_name
	},
	{ 
		TGDB_CLIENT_DEBUGGER_GNU_GDB,
		TGDB_CLIENT_PROTOCOL_GNU_GDB_GDBMI,

		/* tgdb_client_create_context */
		gdbmi_create_context,
		/* tgdb_client_initialize_context */
		gdbmi_initialize,
		/* tgdb_client_destroy_context */
		gdbmi_shutdown,
		/* tgdb_client_err_msg */
		NULL,
		/* tgdb_client_is_client_ready */
		gdbmi_is_client_ready,
	    /* tgdb_client_tgdb_ran_command */
		gdbmi_user_ran_command,
		/* tgdb_client_prepare_for_command */
		gdbmi_prepare_for_command,
		/* tgdb_client_can_tgdb_run_commands */
		NULL,
		/* tgdb_client_parse_io */
		gdbmi_parse_io,
		/* tgdb_client_get_client_commands */
		gdbmi_get_client_commands,
		/* tgdb_client_get_filename_pair */
		NULL,
		/* tgdb_client_get_current_location */
		NULL,
		/* tgdb_client_get_inferior_source_files */
		NULL,
		/* tgdb_client_completion_callback */
		NULL,
		/* tgdb_client_return_command*/
		NULL,
		/* tgdb_client_modify_breakpoint */
		NULL,
		/* tgdb_client_get_debugger_pid */
		gdbmi_get_debugger_pid,
		/* tgdb_client_open_new_tty */
		NULL,
		/* tgdb_client_get_tty_name */
		NULL
	},
	{
		TGDB_CLIENT_DEBUGGER_UNSUPPORTED,
		TGDB_CLIENT_PROTOCOL_UNSUPPORTED,

		/* tgdb_client_create_context */
		NULL,
		/* tgdb_client_initialize_context */
		NULL,
		/* tgdb_client_destroy_context */
		NULL,
		/* tgdb_client_err_msg */
		NULL,
		/* tgdb_client_is_client_ready */
		NULL,
	    /* tgdb_client_tgdb_ran_command */
		NULL,
		/* tgdb_client_prepare_for_command */
		NULL,
		/* tgdb_client_can_tgdb_run_commands */
		NULL,
		/* tgdb_client_parse_io */
		NULL,
		/* tgdb_client_get_filename_pair */
		NULL,
		/* tgdb_client_get_current_location */
		NULL,
		/* tgdb_client_get_inferior_source_files */
		NULL,
		/* tgdb_client_completion_callback */
		NULL,
		/* tgdb_client_return_command*/
		NULL,
		/* tgdb_client_modify_breakpoint */
		NULL,
		/* tgdb_client_get_debugger_pid */
		NULL,
		/* tgdb_client_open_new_tty */
		NULL,
		/* tgdb_client_get_tty_name */
		NULL
	}
};

/**
 * This is a context that abstracts the lower level interface from TGDB.
 * By doing this, all of the lower levels can communicate here, and TGDB
 * just makes sure that it calls the functions in this header.
 */
struct tgdb_client_context {

	/**
 	 * The current debugger being used.
	 */
	enum tgdb_client_supported_debuggers debugger;

	/**
     * The current protocol begin used.
	 */
	enum tgdb_client_supported_protocols protocol;

	/**
     * The current client debugger being used.
	 */
	void *tgdb_debugger_context;

	/**
	 * A client interface. All of the functions that implement the client.
	 */
	struct tgdb_client_debugger_interfaces *tgdb_client_interface;

	struct logger *logger;
};

struct tgdb_client_context *tgdb_client_create_context ( 
	const char *debugger_path, 
	int argc, char **argv,
	const char *config_dir,
	enum tgdb_client_supported_debuggers debugger,
	enum tgdb_client_supported_protocols protocol,
	struct logger *logger) {

	struct tgdb_client_context *tcc = NULL;

	/* Try to initialize the annotate 2 protocol */
	if ( debugger == TGDB_CLIENT_DEBUGGER_GNU_GDB &&
		 ( 
		  protocol == TGDB_CLIENT_PROTOCOL_GNU_GDB_ANNOTATE_TWO ||
		  protocol == TGDB_CLIENT_PROTOCOL_GNU_GDB_GDBMI
		 ) 
	   ){

		tcc = ( struct tgdb_client_context *) 
			cgdb_malloc ( sizeof ( struct tgdb_client_context) );
		tcc->debugger = debugger;
		tcc->protocol = protocol;
		tcc->logger   = logger;

		if ( protocol == TGDB_CLIENT_PROTOCOL_GNU_GDB_ANNOTATE_TWO )
			tcc->tgdb_client_interface = &tgdb_client_debugger_interfaces[0];
		else if ( protocol == TGDB_CLIENT_PROTOCOL_GNU_GDB_GDBMI )
			tcc->tgdb_client_interface = &tgdb_client_debugger_interfaces[1];

		tcc->tgdb_debugger_context =
			tcc->tgdb_client_interface->tgdb_client_create_context (
					debugger_path, argc, argv, config_dir, logger );

		if ( tcc->tgdb_debugger_context == NULL ) {
			free ( tcc );
			logger_write_pos ( tcc->logger, __FILE__, __LINE__, "a2_create_instance failed" );
			return NULL; 
		}
	} else {
		logger_write_pos ( tcc->logger, __FILE__, __LINE__, "tgdb_client_create_context protocol not recognized" );
	}

	return tcc;
}

int tgdb_client_initialize_context ( 
	struct tgdb_client_context *tcc,
	int *debugger_stdin, int *debugger_stdout,
	int *inferior_stdin, int *inferior_stdout ) {

	if ( tcc == NULL || tcc->tgdb_client_interface == NULL ) {
		logger_write_pos ( logger, __FILE__, __LINE__, "tgdb_client_initilize_context unimplemented" );
		return -1;
	}

	return tcc->tgdb_client_interface->tgdb_client_initialize_context (
			tcc->tgdb_debugger_context, 
			debugger_stdin, debugger_stdout,
			inferior_stdin, inferior_stdout );
}

int tgdb_client_destroy_context ( struct tgdb_client_context *tcc ) {
	if ( tcc == NULL || tcc->tgdb_client_interface == NULL ) {
		logger_write_pos ( logger, __FILE__, __LINE__, "tgdb_client_destroy_context unimplemented" );
		return -1;
	}

	return tcc->tgdb_client_interface->tgdb_client_destroy_context ( 
			tcc->tgdb_debugger_context );
}

int tgdb_client_err_msg ( struct tgdb_client_context *tcc ) {
	return -1;
}

int tgdb_client_is_client_ready ( struct tgdb_client_context *tcc ) {
	if ( tcc == NULL || tcc->tgdb_client_interface == NULL ) {
		logger_write_pos ( logger, __FILE__, __LINE__, "tgdb_client_is_client_ready unimplemented" );
		return -1;
	}

	return tcc->tgdb_client_interface->tgdb_client_is_client_ready ( 
			tcc->tgdb_debugger_context );
}

int tgdb_client_tgdb_ran_command ( struct tgdb_client_context *tcc ) {
	if ( tcc == NULL || tcc->tgdb_client_interface == NULL ) {
		logger_write_pos ( logger, __FILE__, __LINE__, "tgdb_client_tgdb_ran_command unimplemented" );
		return -1;
	}

	return tcc->tgdb_client_interface->tgdb_client_tgdb_ran_command ( 
			tcc->tgdb_debugger_context );
}

int tgdb_client_prepare_for_command ( 
		struct tgdb_client_context *tcc, 
		struct tgdb_command *com ) {
	if ( tcc == NULL || tcc->tgdb_client_interface == NULL ) {
		logger_write_pos ( logger, __FILE__, __LINE__, "tgdb_client_prepare_for_command unimplemented" );
		return -1;
	}

	return tcc->tgdb_client_interface->tgdb_client_prepare_for_command ( 
			tcc->tgdb_debugger_context, com );
}

int tgdb_client_can_tgdb_run_commands ( struct tgdb_client_context *tcc ) {
	if ( tcc == NULL || tcc->tgdb_client_interface == NULL ) {
		logger_write_pos ( logger, __FILE__, __LINE__, "tgdb_client_can_tgdb_run_commands unimplemented" );
		return -1;
	}
	
	return tcc->tgdb_client_interface->tgdb_client_can_tgdb_run_commands ( 
			tcc->tgdb_debugger_context );
}

int tgdb_client_parse_io ( 
		struct tgdb_client_context *tcc,
		const char *input_data, const size_t input_data_size,
		char *debugger_output, size_t *debugger_output_size,
		char *inferior_output, size_t *inferior_output_size,
	    struct tgdb_list *command_list) {
	if ( tcc == NULL || tcc->tgdb_client_interface == NULL ) {
		logger_write_pos ( logger, __FILE__, __LINE__, "tgdb_client_parse_io error");
		return -1;
	}

	return tcc->tgdb_client_interface->tgdb_client_parse_io ( 
			tcc->tgdb_debugger_context, 
			input_data, input_data_size,
			debugger_output, debugger_output_size,
			inferior_output, inferior_output_size, command_list );
}

struct tgdb_list *tgdb_client_get_client_commands ( 
		struct tgdb_client_context *tcc ) {

	if ( tcc == NULL || tcc->tgdb_client_interface == NULL ) {
		logger_write_pos ( logger, __FILE__, __LINE__, "tgdb_client_get_client_commands error");
		return NULL;
	}

	return tcc->tgdb_client_interface->tgdb_client_get_client_commands (
			tcc->tgdb_debugger_context );
}

int tgdb_client_get_filename_pair ( 
		struct tgdb_client_context *tcc, 
		const char *path ) {
	if ( tcc == NULL || tcc->tgdb_client_interface == NULL ) {
		logger_write_pos ( logger, __FILE__, __LINE__, "tgdb_client_get_absolute_path unimplemented");
		return -1;
	}
	
	return tcc->tgdb_client_interface->tgdb_client_get_filename_pair ( 
			tcc->tgdb_debugger_context, path );
}

int tgdb_client_get_current_location (struct tgdb_client_context *tcc, 
				      int on_startup)
{
  if (tcc == NULL || tcc->tgdb_client_interface == NULL)
  {
    logger_write_pos ( logger, __FILE__, __LINE__, "tgdb_client_get_current_location unimplemented");
    return -1;
  }
	
  return tcc->tgdb_client_interface->tgdb_client_get_current_location (
	    tcc->tgdb_debugger_context, on_startup);
}

int tgdb_client_get_inferior_source_files ( struct tgdb_client_context *tcc ) {
	if ( tcc == NULL || tcc->tgdb_client_interface == NULL ) {
		logger_write_pos ( logger, __FILE__, __LINE__, "tgdb_client_get_inferior_source_files unimplemented" );
		return -1;
	}
	
	return tcc->tgdb_client_interface->tgdb_client_get_inferior_source_files ( 
			tcc->tgdb_debugger_context );
}

int tgdb_client_completion_callback(
		struct tgdb_client_context *tcc,
		const char *completion_command) {
	if ( tcc == NULL || tcc->tgdb_client_interface == NULL ) {
		logger_write_pos ( logger, __FILE__, __LINE__, "tgdb_client_completion_callback unimplemented" );
		return -1;
	}
	
	return tcc->tgdb_client_interface->tgdb_client_completion_callback ( 
			tcc->tgdb_debugger_context, completion_command );
}

char *tgdb_client_return_command (
        struct tgdb_client_context *tcc,
        enum tgdb_command_type c ) {

	if ( tcc == NULL || tcc->tgdb_client_interface == NULL ) {
		logger_write_pos ( logger, __FILE__, __LINE__,  "tgdb_client_return_command unimplemented" );
		return NULL;
	}

	return tcc->tgdb_client_interface->tgdb_client_return_command ( 
			tcc->tgdb_debugger_context, c );
}

char *tgdb_client_modify_breakpoint ( 
		struct tgdb_client_context *tcc, 
		const char *file, 
		int line, 
		enum tgdb_breakpoint_action b ) {

	if ( tcc == NULL || tcc->tgdb_client_interface == NULL ) {
		logger_write_pos ( logger, __FILE__, __LINE__, "tgdb_client_modify_breakpoint unimplemented" );
		return NULL;
	}

	return tcc->tgdb_client_interface->tgdb_client_modify_breakpoint ( 
			tcc->tgdb_debugger_context, file, line, b );
}

pid_t tgdb_client_get_debugger_pid ( struct tgdb_client_context *tcc ) {
	if ( tcc == NULL || tcc->tgdb_client_interface == NULL ) {
		logger_write_pos ( logger, __FILE__, __LINE__, "tgdb_client_get_debugger_pid unimplemented" );
		return -1;
	}
	
	return tcc->tgdb_client_interface->tgdb_client_get_debugger_pid ( 
			tcc->tgdb_debugger_context );
}

int tgdb_client_open_new_tty ( 
		struct tgdb_client_context *tcc,
		int *inferior_stdin, 
		int *inferior_stdout ) {
	if ( tcc == NULL || tcc->tgdb_client_interface == NULL ) {
		logger_write_pos ( logger, __FILE__, __LINE__,  "tgdb_client_open_new_tty unimplemented" );
		return -1;
	}
	
	return tcc->tgdb_client_interface->tgdb_client_open_new_tty ( 
			tcc->tgdb_debugger_context, 
			inferior_stdin, inferior_stdout );
}

const char *tgdb_client_get_tty_name ( struct tgdb_client_context *tcc ) {
	if ( tcc == NULL || tcc->tgdb_client_interface == NULL ) {
		logger_write_pos ( logger, __FILE__, __LINE__, "tgdb_client_get_tty_name unimplemented" );
		return NULL;
	}
	
	return tcc->tgdb_client_interface->tgdb_client_get_tty_name ( 
			tcc->tgdb_debugger_context );
}
