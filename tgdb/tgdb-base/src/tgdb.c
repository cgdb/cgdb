/* Includes {{{ */
#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif /* HAVE_STDLIB_H */

#if HAVE_SIGNAL_H
#include <signal.h>		/* sig_atomic_t */
#endif /* HAVE_SIGNAL_H */

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#if HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#include "tgdb.h"
#include "tgdb_command.h"
#include "tgdb_client_interface.h"
#include "fs_util.h"
#include "ibuf.h"
#include "io.h"
#include "queue.h"

#include "pseudo.h"		/* SLAVE_SIZE constant */
#include "fork_util.h"
#include "sys_util.h"
#include "tgdb_list.h"
#include "logger.h"

/* }}} */

static int num_loggers = 0;

/* struct tgdb {{{ */

/**
 * The TGDB context data structure.
 */
struct tgdb
{

  /** A client context to abstract the debugger.  */
  struct tgdb_client_context *tcc;

  /** Reading from this will read from the debugger's output */
  int debugger_stdout;

  /** Writing to this will write to the debugger's stdin */
  int debugger_stdin;

  /** Reading from this will read the stdout from the program being debugged */
  int inferior_stdout;

  /** Writing to this will write to the stdin of the program being debugged */
  int inferior_stdin;

  /***************************************************************************
   * All the queue's the clients can run commands through
   * The different queue's can be slightly confusing.
   **************************************************************************/

  /**
   * The commands that need to be run through GDB.
   *
   * This is a buffered queue that represents all of the commands that TGDB
   * needs to execute. These commands will be executed one at a time. That is,
   * 1 command will be issued, and TGDB will wait for the entire response before
   * issuing any more commands. It is even possible that while this command is
   * executing, that the client context will add commands to the 
   * oob_command_queue. If this happens TGDB will execute all of the commands
   * in the oob_command_queue before executing the next command in this queue. 
   */
  struct queue *gdb_input_queue;

  /** 
   * The commands that the client has requested to run.
   *
   * TGDB buffers all of the commands that the FE wants to run. That is,
   * if the FE is sending commands faster than TGDB can pass the command to
   * GDB and get a response, then the commands are buffered until it is there
   * turn to run. These commands are run in the order that they are recieved.
   *
   * This is here as a convience to the FE. TGDB currently does not access 
   * these commands. It provides the push/pop functionality and it erases the
   * queue when a control_c is received.
   */
  struct queue *gdb_client_request_queue;

  /** 
   * The out of band input queue. 
   *
   * These commands are used by the client context. When a single client context 
   * command is run, sometimes it will discover that it needs to run other commands
   * in order to satisfy the functionality requested by the GUI. For instance, a 
   * single GUI function request could take N client context commands to generate
   * a response. Also, to make things worse, sometimes the client context doesn't 
   * know if it will take 1 or N commands to satisfy the request until after it has 
   * sent and recieved the information from the first command. Thus, while the 
   * client context is processing the first command, it may add more commands to 
   * this queue to specify that these commands need to be run before any other 
   * commands are sent to GDB.
   *
   * These commands should *always* be run first.
   */
  struct queue *oob_input_queue;

  /** These are 2 very important state variables.  */

  /**
   * If set to 1, libtgdb thinks the lower level subsystem is capable of 
   * recieving another command. It needs this so that it doesn't send 2
   * commands to the lower level before it can say it can't recieve a command.
   * At some point, maybe this can be removed?
   * When its set to 0, libtgdb thinks it can not send the lower level another
   * command.  */
  int IS_SUBSYSTEM_READY_FOR_NEXT_COMMAND;

  /** If ^c was hit by user */
  sig_atomic_t control_c;

  /**
   * This is the last GUI command that has been run.
   * It is used to display to the client the GUI commands.
   *
   * It will either be NULL when it is not set or it should be
   * the last GUI command run. If it is non-NULL it should be from the heap.
   * As anyone is allowed to call free on it.  */
  char *last_gui_command;

  /**
   * This is a TGDB option.
   * It determines if the user wants to see the commands the GUI is running.
   * 
   * If it is 0, the user does not want to see the commands the GUI is 
   * running. Otherwise, if it is 1, it does.  */
  int show_gui_commands;

  /**
   * This is the queue of commands TGDB has currently made to give to the 
   * front end.  */
  struct tgdb_list *command_list;

  /** An iterator into command_list. */
  tgdb_list_iterator *command_list_iterator;

  /**
   * When GDB dies (purposely or not), the SIGCHLD is sent to the application controlling TGDB.
   * This data structure represents the fact that SIGCHLD has been sent.
   *
   * This currently does not need to track if more than 1 SIGCHLD has been received. So
   * no matter how many are receieved, this will only be 1. Otherwise if none have been
   * received this will be 0.  */
  int has_sigchld_recv;
};

/* }}} */

/* Temporary prototypes {{{ */
static int tgdb_deliver_command (struct tgdb *tgdb,
				 struct tgdb_command *command);
static int tgdb_unqueue_and_deliver_command (struct tgdb *tgdb);
static int tgdb_run_or_queue_command (struct tgdb *tgdb,
				  struct tgdb_command *com);

/* }}} */

/**
 * Process the commands that were created by the client
 *
 * \param tgdb
 * The TGDB context
 *
 * \return
 * -1 on error, 0 on success
 */
static int
tgdb_process_client_commands (struct tgdb *tgdb)
{
  struct tgdb_list *client_command_list;
  tgdb_list_iterator *iterator;
  struct tgdb_command *command;

  client_command_list = tgdb_client_get_client_commands (tgdb->tcc);
  iterator = tgdb_list_get_first (client_command_list);

  while (iterator)
    {
      command =
	(struct tgdb_command *) tgdb_list_get_item (iterator);

      if (tgdb_run_or_queue_command (tgdb, command) == -1)
	{
	  logger_write_pos (logger, __FILE__, __LINE__,
			    "tgdb_run_or_queue_command failed");
	  return -1;
	}

      iterator = tgdb_list_next (iterator);
    }

  /* free the list of client commands */
  tgdb_list_clear (client_command_list);

  return 0;
}

static struct tgdb *
initialize_tgdb_context (void)
{
  struct tgdb *tgdb = (struct tgdb *) cgdb_malloc (sizeof (struct tgdb));

  tgdb->tcc = NULL;
  tgdb->control_c = 0;

  tgdb->debugger_stdout = -1;
  tgdb->debugger_stdin = -1;

  tgdb->inferior_stdout = -1;
  tgdb->inferior_stdin = -1;

  tgdb->gdb_client_request_queue = NULL;
  tgdb->gdb_input_queue = NULL;
  tgdb->oob_input_queue = NULL;

  tgdb->IS_SUBSYSTEM_READY_FOR_NEXT_COMMAND = 1;

  tgdb->last_gui_command = NULL;
  tgdb->show_gui_commands = 0;

  tgdb->command_list = tgdb_list_init ();
  tgdb->has_sigchld_recv = 0;

  logger = NULL;

  return tgdb;
}

/*******************************************************************************
 * This is the basic initialization
 ******************************************************************************/

/* 
 * Gets the users home dir and creates the config directory.
 *
 * \param tgdb
 * The tgdb context.
 *
 * \param config_dir 
 * Should be FSUTIL_PATH_MAX in size on way in.
 * On way out, it will be the path to the config dir
 *
 * \return
 * -1 on error, or 0 on success
 */
static int
tgdb_initialize_config_dir (struct tgdb *tgdb, char *config_dir)
{
  /* Get the home directory */
  char *home_dir = getenv ("HOME");
  const char *tgdb_dir = ".tgdb";

  /* Create the config directory */
  if (!fs_util_create_dir_in_base (home_dir, tgdb_dir))
    {
      logger_write_pos (logger, __FILE__, __LINE__,
			"fs_util_create_dir_in_base error");
      return -1;
    }

  fs_util_get_path (home_dir, tgdb_dir, config_dir);

  return 0;
}

/**
 * Knowing the user's home directory, TGDB can initialize the logger interface
 *
 * \param tgdb
 * The tgdb context.
 *
 * \param config_dir 
 * The path to the user's config directory
 *
 * \return
 * -1 on error, or 0 on success
 */
static int
tgdb_initialize_logger_interface (struct tgdb *tgdb, char *config_dir)
{

  /* Get the home directory */
  const char *tgdb_log_file = "tgdb_log.txt";
  char tgdb_log_path[FSUTIL_PATH_MAX];

  fs_util_get_path (config_dir, tgdb_log_file, tgdb_log_path);

  /* Initialize the logger */
  if (num_loggers == 0)
    {
      logger = logger_create ();

      if (!logger)
	{
	  printf ("Error: Could not create log file\n");
	  return -1;
	}
    }

  ++num_loggers;

  if (logger_set_file (logger, tgdb_log_path) == -1)
    {
      printf ("Error: Could not open log file\n");
      return -1;
    }

  return 0;
}

/* Createing and Destroying a libtgdb context. {{{*/

struct tgdb *
tgdb_initialize (const char *debugger,
		 int argc, char **argv, int *debugger_fd, int *inferior_fd)
{
  /* Initialize the libtgdb context */
  struct tgdb *tgdb = initialize_tgdb_context ();
  char config_dir[FSUTIL_PATH_MAX];

  /* Create config directory */
  if (tgdb_initialize_config_dir (tgdb, config_dir) == -1)
    {
      logger_write_pos (logger, __FILE__, __LINE__, "tgdb_initialize error");
      return NULL;
    }

  if (tgdb_initialize_logger_interface (tgdb, config_dir) == -1)
    {
      printf ("Could not initialize logger interface\n");
      return NULL;
    }

  tgdb->gdb_client_request_queue  = queue_init ();
  tgdb->gdb_input_queue		  = queue_init ();
  tgdb->oob_input_queue		  = queue_init ();

  tgdb->tcc = tgdb_client_create_context (debugger, argc, argv, config_dir,
					  TGDB_CLIENT_DEBUGGER_GNU_GDB,
					  TGDB_CLIENT_PROTOCOL_GNU_GDB_ANNOTATE_TWO,
					  logger);

  /* create an instance and initialize a tgdb_client_context */
  if (tgdb->tcc == NULL)
    {
      logger_write_pos (logger, __FILE__, __LINE__,
			"tgdb_client_create_context failed");
      return NULL;
    }

  if (tgdb_client_initialize_context (tgdb->tcc,
				      &(tgdb->debugger_stdin),
				      &(tgdb->debugger_stdout),
				      &(tgdb->inferior_stdin),
				      &(tgdb->inferior_stdout)) == -1)
    {
      logger_write_pos (logger, __FILE__, __LINE__,
			"tgdb_client_initialize failed");
      return NULL;
    }

  tgdb_process_client_commands (tgdb);

  *debugger_fd = tgdb->debugger_stdout;
  *inferior_fd = tgdb->inferior_stdout;

  return tgdb;
}

int
tgdb_shutdown (struct tgdb *tgdb)
{
  /* Free the logger */
  if (num_loggers == 1)
    {
      if (logger_destroy (logger) == -1)
	{
	  printf ("Could not destroy logger interface\n");
	  return -1;
	}
    }

  --num_loggers;

  return tgdb_client_destroy_context (tgdb->tcc);
}

/* }}}*/

char *
tgdb_err_msg (struct tgdb *tgdb)
{
  return NULL;
}

void
command_completion_callback (struct tgdb *tgdb)
{
  tgdb->IS_SUBSYSTEM_READY_FOR_NEXT_COMMAND = 1;
}

static char *
tgdb_get_client_command (struct tgdb *tgdb, enum tgdb_command_type c)
{
  return tgdb_client_return_command (tgdb->tcc, c);
}

static char *
tgdb_client_modify_breakpoint_call (struct tgdb *tgdb, const char *file,
				    int line, enum tgdb_breakpoint_action b)
{
  return tgdb_client_modify_breakpoint (tgdb->tcc, file, line, b);
}


/* These functions are used to determine the state of libtgdb */

/**
 * Determines if tgdb should send data to gdb or put it in a buffer. This 
 * is when the debugger is ready and there are no commands to run.
 *
 * \return
 * 1 if can issue directly to gdb. Otherwise 0.
 */
static int
tgdb_can_issue_command (struct tgdb *tgdb)
{
  if (tgdb->IS_SUBSYSTEM_READY_FOR_NEXT_COMMAND &&
      tgdb_client_is_client_ready (tgdb->tcc) &&
      (queue_size (tgdb->gdb_input_queue) == 0))
    return 1;

  return 0;
}

/**
 * Determines if tgdb has commands it needs to run.
 *
 * \return
 * 1 if can issue directly to gdb. Otherwise 0.
 */
static int
tgdb_has_command_to_run (struct tgdb *tgdb)
{
  if (tgdb_client_is_client_ready (tgdb->tcc) && 
	  ((queue_size (tgdb->gdb_input_queue) > 0) ||
	   (queue_size (tgdb->oob_input_queue) > 0)))
    return 1;

  return 0;
}

int
tgdb_is_busy (struct tgdb *tgdb, int *is_busy)
{
  /* Validate parameters */
  if (!tgdb || !is_busy)
    {
      logger_write_pos (logger, __FILE__, __LINE__, "tgdb_is_busy failed");
      return -1;
    }

  if (tgdb_can_issue_command (tgdb) == 1)
    *is_busy = 0;
  else
    *is_busy = 1;

  return 0;
}

static void 
tgdb_request_destroy (void *item)
{
  tgdb_request_ptr request_ptr = (tgdb_request_ptr) item;

  switch (request_ptr->header) {
    case TGDB_REQUEST_CONSOLE_COMMAND:
      free ((char*)request_ptr->choice.console_command.command);
      request_ptr->choice.console_command.command = NULL;
      break;
    case TGDB_REQUEST_INFO_SOURCES:
      break;
    case TGDB_REQUEST_FILENAME_PAIR:
      free ((char*)request_ptr->choice.filename_pair.file);
      request_ptr->choice.filename_pair.file = NULL;
      break;
    case TGDB_REQUEST_DEBUGGER_COMMAND:
      break;
    case TGDB_REQUEST_MODIFY_BREAKPOINT:
      free ((char*)request_ptr->choice.modify_breakpoint.file);
      request_ptr->choice.modify_breakpoint.file = NULL;
      break;
    case TGDB_REQUEST_COMPLETE:
      free ((char*)request_ptr->choice.complete.line);
      request_ptr->choice.complete.line = NULL;
      break;
    default:
      break;
  }

  free (request_ptr);
  request_ptr = NULL;
}

/* tgdb_handle_signals
 */
static int
tgdb_handle_signals (struct tgdb *tgdb)
{
  if (tgdb->control_c)
    {
      queue_free_list (tgdb->gdb_input_queue, tgdb_command_destroy);
      queue_free_list (tgdb->gdb_client_request_queue, tgdb_request_destroy);
      tgdb->control_c = 0;
    }

  return 0;
}

/*******************************************************************************
 * This is the main_loop stuff for tgdb-base
 ******************************************************************************/

/* 
 * Sends a command to the debugger. This function gets called when the GUI
 * wants to run a command.
 *
 * \param tgdb
 * An instance of the tgdb library to operate on.
 *
 * \param command
 * This is the command that should be sent to the debugger.
 *
 * \param command_choice
 * This tells tgdb_send who is sending the command.
 *
 * \return
 * 0 on success, or -1 on error.
 */
static int
tgdb_send (struct tgdb *tgdb, char *command, 
	   enum tgdb_command_choice command_choice)
{

  struct tgdb_command *tc;
  struct ibuf *temp_command = ibuf_init ();
  int length = strlen (command);

  /* Add a newline to the end of the command if it doesn't exist */
  ibuf_add (temp_command, command);

  if (command[length - 1] != '\n')
    ibuf_addchar (temp_command, '\n');

  /* Create the client command */
  tc = tgdb_command_create (ibuf_get (temp_command), command_choice, NULL);

  ibuf_free (temp_command);
  temp_command = NULL;

  if (tgdb_run_or_queue_command (tgdb, tc) == -1)
    {
      logger_write_pos (logger, __FILE__, __LINE__,
			"tgdb_run_or_queue_command failed");
      return -1;
    }

  if (tgdb_client_tgdb_ran_command (tgdb->tcc) == -1)
    {
      logger_write_pos (logger, __FILE__, __LINE__,
			"tgdb_client_tgdb_ran_command failed");
      return -1;
    }

  tgdb_process_client_commands (tgdb);

  return 0;
}

/**
 * If TGDB is ready to process another command, then this command will be
 * sent to the debugger. However, if TGDB is not ready to process another command,
 * then the command will be queued and run when TGDB is ready.
 *
 * \param tgdb
 * The TGDB context to use.
 *
 * \param command
 * The command to run or queue.
 *
 * \return
 * 0 on success or -1 on error
 */
static int
tgdb_run_or_queue_command (struct tgdb *tgdb, struct tgdb_command *command)
{
  int can_issue;

  can_issue = tgdb_can_issue_command (tgdb);

  if (can_issue)
  {
    if (tgdb_deliver_command (tgdb, command) == -1)
      return -1;
  } else {
    /* Make sure to put the command into the correct queue. */
    switch (command->command_choice)
      {
      case TGDB_COMMAND_FRONT_END:
      case TGDB_COMMAND_TGDB_CLIENT:
	queue_append (tgdb->gdb_input_queue, command);
	break;
      case TGDB_COMMAND_TGDB_CLIENT_PRIORITY:
	queue_append (tgdb->oob_input_queue, command);
	break;
      case TGDB_COMMAND_CONSOLE:
	logger_write_pos (logger, __FILE__, __LINE__, "unimplemented command");
	return -1;
	break;
      default:
	logger_write_pos (logger, __FILE__, __LINE__, "unimplemented command");
	return -1;
      }
  }

  return 0;
}

/** 
 * Will send a command to the debugger immediatly. No queueing will be done
 * at this point.
 *
 * \param tgdb
 * The TGDB context to use.
 *
 * \param command 
 * The command to run.
 *
 *  NOTE: This function assummes valid commands are being sent to it. 
 *        Error checking should be done before inserting into queue.
 */
static int
tgdb_deliver_command (struct tgdb *tgdb, struct tgdb_command *command)
{
  tgdb->IS_SUBSYSTEM_READY_FOR_NEXT_COMMAND = 0;

  /* Here is where the command is actually given to the debugger.
   * Before this is done, if the command is a GUI command, we save it,
   * so that later, it can be printed to the client. Its for debugging
   * purposes only, or for people who want to know the commands there
   * debugger is being given.
   */
  if (command->command_choice == TGDB_COMMAND_FRONT_END)
    tgdb->last_gui_command = cgdb_strdup (command->tgdb_command_data);

  /* A command for the debugger */
  if (tgdb_client_prepare_for_command (tgdb->tcc, command) == -1)
    return -1;

  /* A regular command from the client */
  io_debug_write_fmt ("<%s>", command->tgdb_command_data);

  io_writen (tgdb->debugger_stdin, command->tgdb_command_data,
	     strlen (command->tgdb_command_data));

  /* Uncomment this if you wish to see all of the commands, that are 
   * passed to GDB. */
#if 0
  {
    char *s = strdup (client_command->tgdb_client_command_data);
    int length = strlen (s);
    s[length - 1] = '\0';
    fprintf (stderr, "[%s]\n", s);
    s[length - 1] = ' ';
    free (s);
    s = NULL;
  }
#endif

  return 0;
}

/**
 * TGDB will search it's command queue's and determine what the next command
 * to deliever to GDB should be.
 *
 * \return
 * 0 on success, -1 on error
 */
static int
tgdb_unqueue_and_deliver_command (struct tgdb *tgdb)
{
tgdb_unqueue_and_deliver_command_tag:

  /* This will redisplay the prompt when a command is run
   * through the gui with data on the console.
   */
  /* The out of band commands should always be run first */
  if (queue_size (tgdb->oob_input_queue) > 0)
    {
      /* These commands are always run. 
       * However, if an assumption is made that a misc
       * prompt can never be set while in this spot.
       */
      struct tgdb_command *item = NULL;
      item = queue_pop (tgdb->oob_input_queue);
      tgdb_deliver_command (tgdb, item);
      tgdb_command_destroy (item);
    }
  /* If the queue is not empty, run a command */
  else if (queue_size (tgdb->gdb_input_queue) > 0)
    {
      struct tgdb_command *item = NULL;
      item = queue_pop (tgdb->gdb_input_queue);

      /* If at the misc prompt, don't run the internal tgdb commands,
       * In fact throw them out for now, since they are only 
       * 'info breakpoints' */
      if (tgdb_client_can_tgdb_run_commands (tgdb->tcc) == 1)
	{
	  if (item->command_choice != TGDB_COMMAND_CONSOLE)
	    {
	      tgdb_command_destroy (item);
	      goto tgdb_unqueue_and_deliver_command_tag;
	    }
	}

      /* This happens when a command was skipped because the client no longer
       * needs the command to be run */
      if (tgdb_deliver_command (tgdb, item) == -1)
	goto tgdb_unqueue_and_deliver_command_tag;

      tgdb_command_destroy (item);
    }

  return 0;
}

/* These functions are used to communicate with the inferior */
int
tgdb_send_inferior_char (struct tgdb *tgdb, char c)
{
  if (io_write_byte (tgdb->inferior_stdout, c) == -1)
    {
      logger_write_pos (logger, __FILE__, __LINE__, "io_write_byte failed");
      return -1;
    }

  return 0;
}

/* returns to the caller data from the child */
size_t
tgdb_recv_inferior_data (struct tgdb * tgdb, char *buf, size_t n)
{
  char local_buf[n + 1];
  ssize_t size;

  /* read all the data possible from the child that is ready. */
  if ((size = io_read (tgdb->inferior_stdin, local_buf, n)) < 0)
    {
      logger_write_pos (logger, __FILE__, __LINE__,
			"inferior_fd read failed");
      return -1;
    }

  strncpy (buf, local_buf, size);
  buf[size] = '\0';

  return size;
}

/**
 * TGDB is going to quit.
 *
 * \param tgdb
 * The tgdb context
 *
 * \return
 * 0 on success or -1 on error
 */
static int
tgdb_add_quit_command (struct tgdb *tgdb)
{
  struct tgdb_debugger_exit_status *tstatus;
  struct tgdb_response *response; 

  tstatus = (struct tgdb_debugger_exit_status *)
    cgdb_malloc (sizeof (struct tgdb_debugger_exit_status));

  /* Child did not exit normally */
  tstatus->exit_status = -1;
  tstatus->return_value = 0;

  response = (struct tgdb_response *) cgdb_malloc (sizeof (struct tgdb_response));
  response->header = TGDB_QUIT;
  response->choice.quit.exit_status = tstatus;

  tgdb_types_append_command (tgdb->command_list, response);

  return 0;
}

/**
 * This is called when GDB has finished.
 * Its job is to add the type of QUIT command that is appropriate.
 *
 * \param tgdb
 * The tgdb context
 *
 * \param tgdb_will_quit
 * This will return as 1 if tgdb sent the TGDB_QUIT command. Otherwise 0.
 * 
 * \return
 * 0 on success or -1 on error
 */
static int
tgdb_get_quit_command (struct tgdb *tgdb, int *tgdb_will_quit)
{
  pid_t pid = tgdb_client_get_debugger_pid (tgdb->tcc);
  int status = 0;
  pid_t ret;
  struct tgdb_debugger_exit_status *tstatus;
  struct tgdb_response *response = (struct tgdb_response *)
    cgdb_malloc (sizeof (struct tgdb_response));

  if (!tgdb_will_quit)
    return -1;

  *tgdb_will_quit = 0;

  tstatus = (struct tgdb_debugger_exit_status *)
    cgdb_malloc (sizeof (struct tgdb_debugger_exit_status));

  ret = waitpid (pid, &status, WNOHANG);

  if (ret == -1)
    {
      logger_write_pos (logger, __FILE__, __LINE__, "waitpid error");
      return -1;
    }
  else if (ret == 0)
    {
      /* This SIGCHLD wasn't for GDB */
      return 0;
    }

  if ((WIFEXITED (status)) == 0)
    {
      /* Child did not exit normally */
      tstatus->exit_status = -1;
      tstatus->return_value = 0;
    }
  else
    {
      tstatus->exit_status = 0;
      tstatus->return_value = WEXITSTATUS (status);
    }

  response->header = TGDB_QUIT;
  response->choice.quit.exit_status = tstatus;
  tgdb_types_append_command (tgdb->command_list, response);
  *tgdb_will_quit = 1;

  return 0;
}

size_t
tgdb_process (struct tgdb *tgdb, char *buf, size_t n, int *is_finished)
{
  char local_buf[10 * n];
  ssize_t size;
  size_t buf_size = 0;
  int is_busy;

  /* make the queue empty */
  tgdb_delete_responses (tgdb);

  /* TODO: This is kind of a hack.
   * Since I know that I didn't do a read yet, the next select loop will
   * get me back here. This probably shouldn't return, however, I have to
   * re-write a lot of this function. Also, I think this function should
   * return a malloc'd string, not a static buffer.
   *
   * Currently, I see it as a bigger hack to try to just append this to the
   * beggining of buf.
   */
  if (tgdb->last_gui_command != NULL)
    {
      int ret;

      if (tgdb_is_busy (tgdb, &is_busy) == -1)
	{
	  logger_write_pos (logger, __FILE__, __LINE__, 
			    "tgdb_is_busy failed");
	  return -1;
	}
      *is_finished = !is_busy;

      if (tgdb->show_gui_commands)
	{
	  strcpy (buf, tgdb->last_gui_command);
	  ret = strlen (tgdb->last_gui_command);
	}
      else
	{
	  strcpy (buf, "\n");
	  ret = 1;
	}

      free (tgdb->last_gui_command);
      tgdb->last_gui_command = NULL;
      return ret;
    }

  if (tgdb->has_sigchld_recv)
    {
      int tgdb_will_quit;
      /* tgdb_get_quit_command will return right away, it's asynchrounous.
       * We call it to determine if it was GDB that died.
       * If GDB didn't die, things will work like normal. ignore this.
       * If GDB did die, this get's the quit command and add's it to the list. It's
       * OK that the rest of this function get's executed, since the read will simply
       * return EOF.
       */
      int val = tgdb_get_quit_command (tgdb, &tgdb_will_quit);
      if (val == -1)
	{
	  logger_write_pos (logger, __FILE__, __LINE__,
			    "tgdb_get_quit_command error");
	  return -1;
	}
      tgdb->has_sigchld_recv = 0;
      if (tgdb_will_quit)
	goto tgdb_finish;
    }

  /* set buf to null for debug reasons */
  memset (buf, '\0', n);

  /* 1. read all the data possible from gdb that is ready. */
  if ((size = io_read (tgdb->debugger_stdout, local_buf, n)) < 0)
    {
      logger_write_pos (logger, __FILE__, __LINE__,
			"could not read from masterfd");
      buf_size = -1;
      tgdb_add_quit_command (tgdb);
      goto tgdb_finish;
    }
  else if (size == 0)
    {				/* EOF */
      tgdb_add_quit_command (tgdb);
      goto tgdb_finish;
    }

  local_buf[size] = '\0';

  /* 2. At this point local_buf has everything new from this read.
   * Basically this function is responsible for seperating the annotations
   * that gdb writes from the data. 
   *
   * buf and buf_size are the data to be returned from the user.
   */
  {
    /* unused for now */
    char *infbuf = NULL;
    size_t infbuf_size;
    int result;
    result = tgdb_client_parse_io (tgdb->tcc,
				   local_buf, size,
				   buf, &buf_size,
				   infbuf, &infbuf_size, tgdb->command_list);

    tgdb_process_client_commands (tgdb);

    if (result == 0)
      {
	/* success, and more to parse, ss isn't done */
      }
    else if (result == 1)
      {
	/* success, and finished command */
	command_completion_callback (tgdb);
      }
    else if (result == -1)
      {
	logger_write_pos (logger, __FILE__, __LINE__,
			  "tgdb_client_parse_io failed");
      }
  }

  /* 3. if ^c has been sent, clear the buffers.
   *        If a signal has been recieved, clear the queue and return
   */
  if (tgdb_handle_signals (tgdb) == -1)
    {
      logger_write_pos (logger, __FILE__, __LINE__,
			"tgdb_handle_signals failed");
      return -1;
    }

  /* 4. runs the users buffered command if any exists */
  if (tgdb_has_command_to_run (tgdb))
    tgdb_unqueue_and_deliver_command (tgdb);

tgdb_finish:

  /* Set the iterator to the beggining. So when the user
   * calls tgdb_get_command it, it will be in the right spot.
   */
  tgdb->command_list_iterator = tgdb_list_get_first (tgdb->command_list);

  if (tgdb_is_busy (tgdb, &is_busy) == -1)
    {
      logger_write_pos (logger, __FILE__, __LINE__, "tgdb_is_busy failed");
      return -1;
    }
  *is_finished = !is_busy;

  return buf_size;
}

/* Getting Data out of TGDB {{{*/

struct tgdb_response *
tgdb_get_response (struct tgdb *tgdb)
{
  struct tgdb_response *command;

  if (tgdb->command_list_iterator == NULL)
    return NULL;

  command =
    (struct tgdb_response *) tgdb_list_get_item (tgdb->command_list_iterator);

  tgdb->command_list_iterator = tgdb_list_next (tgdb->command_list_iterator);

  return command;
}

void
tgdb_traverse_responses (struct tgdb *tgdb)
{
  tgdb_list_foreach (tgdb->command_list, tgdb_types_print_command);
}

void
tgdb_delete_responses (struct tgdb *tgdb)
{
  tgdb_list_free (tgdb->command_list, tgdb_types_free_command);
}

/* }}}*/

/* Inferior tty commands {{{*/

int
tgdb_tty_new (struct tgdb *tgdb)
{
  int ret = tgdb_client_open_new_tty (tgdb->tcc,
				      &tgdb->inferior_stdin,
				      &tgdb->inferior_stdout);

  tgdb_process_client_commands (tgdb);

  return ret;
}

const char *
tgdb_tty_name (struct tgdb *tgdb)
{
  return tgdb_client_get_tty_name (tgdb->tcc);
}

/* }}}*/

/* Functional commands {{{ */

/* Request {{{*/

tgdb_request_ptr
tgdb_request_run_console_command (struct tgdb *tgdb, const char *command)
{
  tgdb_request_ptr request_ptr;
  if (!tgdb || !command)
    return NULL;

  request_ptr = (tgdb_request_ptr)
    cgdb_malloc (sizeof (struct tgdb_request));
  if (!request_ptr)
    return NULL;

  request_ptr->header = TGDB_REQUEST_CONSOLE_COMMAND;
  request_ptr->choice.console_command.command = (const char *)
    cgdb_strdup (command);

  
  return request_ptr;
}

tgdb_request_ptr
tgdb_request_inferiors_source_files (struct tgdb *tgdb)
{
  tgdb_request_ptr request_ptr;
  if (!tgdb)
    return NULL;

  request_ptr = (tgdb_request_ptr)
    cgdb_malloc (sizeof (struct tgdb_request));
  if (!request_ptr)
    return NULL;

  request_ptr->header = TGDB_REQUEST_INFO_SOURCES;

  return request_ptr;
}

tgdb_request_ptr
tgdb_request_filename_pair (struct tgdb *tgdb, const char *file)
{
  tgdb_request_ptr request_ptr;
  if (!tgdb)
    return NULL;

  if (!file)
    return NULL;

  request_ptr = (tgdb_request_ptr)
    cgdb_malloc (sizeof (struct tgdb_request));
  if (!request_ptr)
    return NULL;

  request_ptr->header = TGDB_REQUEST_FILENAME_PAIR;

  if (file) {
    request_ptr->choice.filename_pair.file = (const char *)
      cgdb_strdup (file);
  } else
    request_ptr->choice.filename_pair.file = NULL;

    
  return request_ptr;
}

tgdb_request_ptr 
tgdb_request_current_location (struct tgdb *tgdb, int on_startup)
{
  tgdb_request_ptr request_ptr;
  if (!tgdb)
    return NULL;

  request_ptr = (tgdb_request_ptr)
    cgdb_malloc (sizeof (struct tgdb_request));
  if (!request_ptr)
    return NULL;

  request_ptr->header = TGDB_REQUEST_CURRENT_LOCATION;
  request_ptr->choice.current_location.on_startup = on_startup;

  return request_ptr;
}

tgdb_request_ptr
tgdb_request_run_debugger_command (struct tgdb *tgdb, enum tgdb_command_type c)
{
  tgdb_request_ptr request_ptr;
  if (!tgdb)
    return NULL;

  request_ptr = (tgdb_request_ptr)
    cgdb_malloc (sizeof (struct tgdb_request));
  if (!request_ptr)
    return NULL;

  request_ptr->header = TGDB_REQUEST_DEBUGGER_COMMAND;
  request_ptr->choice.debugger_command.c = c;
    
  return request_ptr;
}

tgdb_request_ptr
tgdb_request_modify_breakpoint (struct tgdb *tgdb, const char *file, int line,
				enum tgdb_breakpoint_action b)
{
  tgdb_request_ptr request_ptr;
  if (!tgdb)
    return NULL;

  request_ptr = (tgdb_request_ptr)
    cgdb_malloc (sizeof (struct tgdb_request));
  if (!request_ptr)
    return NULL;

  request_ptr->header = TGDB_REQUEST_MODIFY_BREAKPOINT;
  request_ptr->choice.modify_breakpoint.file = (const char *)
    cgdb_strdup (file);
  request_ptr->choice.modify_breakpoint.line = line;
  request_ptr->choice.modify_breakpoint.b = b;

  return request_ptr;
}

tgdb_request_ptr
tgdb_request_complete (struct tgdb *tgdb, const char *line)
{
  tgdb_request_ptr request_ptr;
  if (!tgdb)
    return NULL;

  request_ptr = (tgdb_request_ptr)
    cgdb_malloc (sizeof (struct tgdb_request));
  if (!request_ptr)
    return NULL;

  request_ptr->header = TGDB_REQUEST_COMPLETE;
  request_ptr->choice.complete.line = (const char *)
    cgdb_strdup (line);

  return request_ptr;
}

/* }}}*/

/* Process {{{*/

static int
tgdb_process_console_command (struct tgdb *tgdb, tgdb_request_ptr request)
{
  struct ibuf *command;

  if (!tgdb || !request)
    return -1;

  if (!tgdb_can_issue_command (tgdb))
    return -1;

  if (request->header != TGDB_REQUEST_CONSOLE_COMMAND)
    return -1;
  
  command = ibuf_init ();
  ibuf_add (command, request->choice.console_command.command);
  ibuf_addchar (command, '\n');

  if (tgdb_send (tgdb, ibuf_get (command), TGDB_COMMAND_CONSOLE) == -1)
  {
      logger_write_pos (logger, __FILE__, __LINE__, "tgdb_send failed");
      return -1;
  }

  ibuf_free (command);
  command = NULL;

  return 0;
}

static int
tgdb_process_info_sources (struct tgdb *tgdb, tgdb_request_ptr request)
{
  int ret;

  if (!tgdb || !request)
    return -1;

  if (request->header != TGDB_REQUEST_INFO_SOURCES)
    return -1;

  ret = tgdb_client_get_inferior_source_files (tgdb->tcc);
  tgdb_process_client_commands (tgdb);

  return ret;
}

static int
tgdb_process_filename_pair (struct tgdb *tgdb, tgdb_request_ptr request)
{
  int ret;

  if (!tgdb || !request)
    return -1;

  if (request->header != TGDB_REQUEST_FILENAME_PAIR)
    return -1;

  ret = tgdb_client_get_filename_pair (tgdb->tcc, request->choice.filename_pair.file);
  tgdb_process_client_commands (tgdb);

  return ret;
}

static int 
tgdb_process_current_location (struct tgdb *tgdb, tgdb_request_ptr request)
{
  int ret = 0;

  if (!tgdb || !request)
    return -1;

  if (request->header != TGDB_REQUEST_CURRENT_LOCATION)
    return -1;

  ret = tgdb_client_get_current_location (
	  tgdb->tcc, request->choice.current_location.on_startup);
  tgdb_process_client_commands (tgdb);
  
  return ret;
}

static int
tgdb_process_debugger_command (struct tgdb *tgdb, tgdb_request_ptr request)
{
  if (!tgdb || !request)
    return -1;

  if (request->header != TGDB_REQUEST_DEBUGGER_COMMAND)
    return -1;

  return tgdb_send (tgdb, tgdb_get_client_command (tgdb, request->choice.debugger_command.c),
		    TGDB_COMMAND_FRONT_END);
}

static int
tgdb_process_modify_breakpoint (struct tgdb *tgdb, tgdb_request_ptr request)
{
  char *val;

  if (!tgdb || !request)
    return -1;

  if (request->header != TGDB_REQUEST_MODIFY_BREAKPOINT)
    return -1;

  val = tgdb_client_modify_breakpoint_call (
	   tgdb, 
	   request->choice.modify_breakpoint.file, 
	   request->choice.modify_breakpoint.line, 
	   request->choice.modify_breakpoint.b);

  if (val == NULL)
    return -1;

  if (tgdb_send (tgdb, val, TGDB_COMMAND_FRONT_END) == -1)
    {
      logger_write_pos (logger, __FILE__, __LINE__, "tgdb_send failed");
      return -1;
    }

  free (val);

  return 0;
}

static int
tgdb_process_complete (struct tgdb *tgdb, tgdb_request_ptr request)
{
  int ret;

  if (!tgdb || !request)
    return -1;

  if (request->header != TGDB_REQUEST_COMPLETE)
    return -1;

  ret = tgdb_client_completion_callback (tgdb->tcc, request->choice.complete.line);
  tgdb_process_client_commands (tgdb);

  return ret;
}

int 
tgdb_process_command (struct tgdb *tgdb, tgdb_request_ptr request)
{
  if (!tgdb || !request)
    return -1;

  if (!tgdb_can_issue_command (tgdb))
    return -1;

  if (request->header == TGDB_REQUEST_CONSOLE_COMMAND)
    return tgdb_process_console_command (tgdb, request);
  else if (request->header == TGDB_REQUEST_INFO_SOURCES)
    return tgdb_process_info_sources (tgdb, request);
  else if (request->header == TGDB_REQUEST_FILENAME_PAIR)
    return tgdb_process_filename_pair (tgdb, request);
  else if (request->header == TGDB_REQUEST_CURRENT_LOCATION)
    return tgdb_process_current_location (tgdb, request);
  else if (request->header == TGDB_REQUEST_DEBUGGER_COMMAND)
    return tgdb_process_debugger_command (tgdb, request);
  else if (request->header == TGDB_REQUEST_MODIFY_BREAKPOINT)
    return tgdb_process_modify_breakpoint (tgdb, request);
  else if (request->header == TGDB_REQUEST_COMPLETE)
    return tgdb_process_complete (tgdb, request);

  return 0;
}

/* }}}*/

/* }}} */

/* TGDB Queue commands {{{*/

int 
tgdb_queue_append (struct tgdb *tgdb, tgdb_request_ptr request)
{
  if (!tgdb || !request)
    return -1;

  queue_append (tgdb->gdb_client_request_queue, request);

  return 0;
}

tgdb_request_ptr
tgdb_queue_pop (struct tgdb *tgdb)
{
  tgdb_request_ptr item;

  if (!tgdb)
    return NULL;

  item = queue_pop (tgdb->gdb_client_request_queue);

  return item;
}

int 
tgdb_queue_size (struct tgdb *tgdb, int *size)
{
  if (!tgdb || !size)
    return -1;

  *size = queue_size (tgdb->gdb_client_request_queue);

  return 0;
}

/* }}}*/

/* Signal Handling Support {{{*/

int
tgdb_signal_notification (struct tgdb *tgdb, int signum)
{
  struct termios t;
  cc_t *sig_char = NULL;

  tcgetattr (tgdb->debugger_stdin, &t);

  if (signum == SIGINT)
    {				/* ^c */
      tgdb->control_c = 1;
      sig_char = &t.c_cc[VINTR];
      write (tgdb->debugger_stdin, sig_char, 1);
    }
  else if (signum == SIGQUIT)
    {				/* ^\ */
      sig_char = &t.c_cc[VQUIT];
      write (tgdb->debugger_stdin, sig_char, 1);
    }
  else if (signum == SIGCHLD)
    {
      tgdb->has_sigchld_recv = 1;
    }

  return 0;
}

/* }}}*/

/* Config Options {{{*/
int
tgdb_set_verbose_gui_command_output (struct tgdb *tgdb, int value)
{
  if ((value == 0) || (value == 1))
    tgdb->show_gui_commands = value;

  if (tgdb->show_gui_commands == 1)
    return 1;

  return 0;
}

int
tgdb_set_verbose_error_handling (struct tgdb *tgdb, int value)
{
  if (value == -1)
    return logger_is_recording (logger);

  if (value == 1 || value == 0)
    logger_set_record (logger, value);

  if (value == 1)
    logger_set_fd (logger, stderr);

  return logger_is_recording (logger);
}

/* }}}*/
