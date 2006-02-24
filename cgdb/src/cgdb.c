/* cgdb.c:
 * -------
 */
#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif /* HAVE_FCNTL_H */

#if HAVE_UNISTD_H
#include <unistd.h>
#endif  /* HAVE_UNISTD_H */

#if HAVE_STDLIB_H 
#include <stdlib.h>
#endif  /* HAVE_STDLIB_H */

#if HAVE_STDIO_H
#include <stdio.h> 
#endif /* HAVE_STDIO_H */

#if HAVE_CURSES_H
#include <curses.h>
#endif /* HAVE_CURSES_H */

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif  /* HAVE_SYS_SELECT_H */

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#if HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif /* HAVE_SYS_IOCTL_H */

#if HAVE_ERRNO_H
#include <errno.h>
#endif /* HAVE_ERRNO_H */

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif 

/* Local Includes */
#include "cgdb.h"
#include "logger.h"
#include "interface.h"
#include "scroller.h"
#include "sources.h"
#include "tgdb.h"
#include "helptext.h"
#include "kui.h"
#include "kui_term.h"
#include "fs_util.h"
#include "cgdbrc.h"
#include "io.h"
#include "tgdb_list.h"
#include "fork_util.h"
#include "terminal.h"
#include "queue.h"
#include "rline.h"
#include "ibuf.h"
#include "usage.h"

/* --------------- */
/* Local Variables */
/* --------------- */

struct tgdb *tgdb; 			  /* The main TGDB context */
struct tgdb_request *last_request = NULL;

char cgdb_home_dir[MAXLINE];  /* Path to home directory with trailing slash */
char cgdb_help_file[MAXLINE]; /* Path to home directory with trailing slash */
const char *readline_history_filename = "readline_history.txt";
char *readline_history_path;  /* After readline init is called, this will 
			       * contain the path to the readline history 
			       * file. */

static int gdb_fd = -1;       /* File descriptor for GDB communication */
static int tty_fd = -1;       /* File descriptor for process being debugged */

/** Master/Slave PTY used to keep readline off of stdin/stdout. */
static pty_pair_ptr pty_pair;

static char *debugger_path = NULL;  /* Path to debugger to use */

struct kui_manager *kui_ctx = NULL; /* The key input package */

int resize_pipe[2] = { -1, -1 };

/* Readline interface */
static struct rline *rline;
static int is_tab_completing = 0;

/* Current readline line. If the user enters 'b ma\<NL>in<NL>', where 
 * NL is the same as the user hitting the enter key, then 2 commands are
 * received by readline. The first is 'b ma\' and the second is 'in'.
 *
 * In general, the user can put a \ as the last char on the line and GDB
 * accepts this as meaning, "allow me to enter more data on the next line".
 * The trailing \, does not belong in the command to GDB, it is purely there
 * to tell GDB that the user wants to enter more data on the next line.
 *
 * For this reason, CGDB keeps this buffer. It adds the command the user
 * is typeing to this buffer. When a line comes that does not end in a \,
 * then the command is sent to GDB.
 */
static struct ibuf *current_line = NULL;

/**
 * When the user is entering a multi-line command in the GDB window
 * via the \ char at the end of line, the prompt get's set to "". This
 * variable is used to remember what the previous prompt was so that it
 * can be set back when the user has finished entering the line.
 */
static char *rline_last_prompt = NULL;

/* Original terminal attributes */
static struct termios term_attributes;

/**
 * If the TGDB instance is not busy, it will run the requested command.
 * Otherwise, the command will get queued to run later.
 *
 * \param tgdb
 * An instance of the tgdb library to operate on.
 *
 * \param request
 * The requested command to have TGDB process.
 *
 * \return
 * 0 on success or -1 on error
 */
int
handle_request (struct tgdb *tgdb, struct tgdb_request *request)
{
  int val, is_busy;

  if (!tgdb || !request)
    return -1;

  val = tgdb_is_busy (tgdb, &is_busy);
  if (val == -1)
    return -1;

  if (is_busy)
    tgdb_queue_append (tgdb, request);
  else {
    last_request = request;
    tgdb_process_command (tgdb, request);
  }

  return 0;
}

// readline code {{{

/* Please forgive me for adding all the comment below. This function
 * has some strange bahaviors that I thought should be well explained.
 */
void 
rlctx_send_user_command (char *line)
{
  char *cline;
  int length;
  char *rline_prompt;
  tgdb_request_ptr request_ptr;

  /* This happens when rl_callback_read_char gets EOF */
  if (line == NULL)
    return;

  /* Add the line passed in to the current line */
  ibuf_add (current_line, line);

  /* Get current line, and current line length */
  cline = ibuf_get (current_line);
  length = ibuf_length (current_line);
  
  /* Check to see if the user is escaping the line, to use a 
   * multi line command. If so, return so that the user can
   * continue the command. This data should not go into the history
   * buffer, or be sent to gdb yet. 
   *
   * Also, notice the length > 0 condition. (length == 0) can happen 
   * when the user simply hits Enter on the keyboard. */
  if (length > 0 && cline[length-1] == '\\') {
    /* The \ char is for continuation only, it is not meant to be sent
     * to GDB as a character. */
    ibuf_delchar (current_line);

    /* Only need to change the prompt the first time the \ char is used.
     * Doing it a second time would erase the real rline_last_prompt,
     * since the prompt has already been set to "".
     */
    if (!rline_last_prompt) {
      rline_get_prompt (rline, &rline_prompt);
      rline_last_prompt = strdup (rline_prompt);
      /* GDB set's the prompt to nothing when doing continuation.
       * This is here just for compatibility. */
      rline_set_prompt (rline, "");
    }

    return;
  }

  /* If here, a full command has been sent. Restore the prompt. */
  if (rline_last_prompt) {
    rline_set_prompt (rline, rline_last_prompt);
    free (rline_last_prompt);
    rline_last_prompt = NULL;
  }

  /* Don't add the enter command to the history */
  if (length > 0)
    rline_add_history (rline, cline);

  request_ptr = tgdb_request_run_console_command (tgdb, cline);

  /* Send this command to TGDB */
  if (!request_ptr)
    logger_write_pos ( logger, __FILE__, __LINE__, "rlctx_send_user_command\n"); 

  handle_request (tgdb, request_ptr);

  ibuf_clear (current_line);
}

static int 
tab_completion (int a, int b)
{
  char *cur_line;
  int ret;
  tgdb_request_ptr request_ptr;

  is_tab_completing = 1;

  ret = rline_get_current_line (rline, &cur_line);
  if (ret == -1)
    logger_write_pos ( logger, __FILE__, __LINE__, "rline_get_current_line error\n"); 

  request_ptr = tgdb_request_complete (tgdb, cur_line);
  if (!request_ptr)
    logger_write_pos ( logger, __FILE__, __LINE__, "tgdb_request_complete error\n"); 
  
  handle_request (tgdb, request_ptr);
  return 0;
}

/**
 * This is the function responsible for display the readline completions when there
 * is more than 1 item to complete. Currently it prints 1 per line.
 */
static void
readline_completion_display_func (char **matches, int num_matches, int max_length)
{
  int i;
  if_print ("\n");
  for (i=1; i<=num_matches; ++i) {
    if_print (matches[i]);
    if_print ("\n");
  }
}

int
do_tab_completion (struct tgdb_list *list)
{
  if (rline_rl_complete (rline, list, &readline_completion_display_func) == -1) {
    logger_write_pos (logger, __FILE__, __LINE__, "rline_rl_complete error\n"); 
    return -1;
  }
  is_tab_completing = 0;

  return 0;
}

void rl_sigint_recved (void){
  rline_clear (rline);
  is_tab_completing = 0;
}

void rl_resize (int rows, int cols)
{
  rline_resize_terminal_and_redisplay (rline, rows, cols);
}

static void 
change_prompt (const char *new_prompt)
{
  rline_set_prompt (rline, new_prompt);
}

// }}}

/* ------------------------ */
/* Initialization functions */
/* ------------------------ */

/* version_info: Returns version information about cgdb.
 * ------------- 
 *
 * Return:  A pointer to a static buffer, containing version info.
 */
static char *version_info(void) {
    static char buf[MAXLINE];
    sprintf(buf, "%s %s\r\n%s", "CGDB", VERSION, 
            "Copyright 2002-2006 Bob Rossi and Mike Mueller.\n"
            "CGDB is free software, covered by the GNU General Public License, and you are\n"
            "welcome to change it and/or distribute copies of it under certain conditions.\n"
            "There is absolutely no warranty for CGDB.\n");
    return buf;
}

static void parse_long_options(int *argc, char ***argv) {
    int c, option_index = 0, n = 1;
    const char *args = "d:hv";
     
#ifdef HAVE_GETOPT_H
    static struct option long_options[] = {
        { "version", 0, 0, 0 },
        { "help", 0, 0, 0 },
		{ 0, 0, 0, 0 }
    };
#endif

	while ( 1 ) {
    	opterr = 0;
#ifdef HAVE_GETOPT_H
        c = getopt_long(*argc, *argv, args, long_options, &option_index);
#else
        c = getopt(*argc, *argv, args);
#endif
   		if ( c == -1 )
			break;

        if ( ((char)c) == '?' )
            break;

        switch (c) {
            case 0:
                switch ( option_index ) {
                    case 0:
                        printf("%s",version_info());
                        exit(0);
                    case 1:
                        usage();
                        exit(0);
                    default:
                        break;
                }
                break;
            case 'v':
                printf("%s",version_info());
                exit(0);
            case 'd':
                debugger_path = strdup ( optarg );
                n+=2;
                break;
            case 'h':
                usage();
                exit(0);
            default:
                break;
        }
    }

    *argc -= n;
    *argv += n;
}

/* init_home_dir: Attempts to create a config directory in the user's home
 * -------------  directory.
 *
 * Return:  0 on sucess or -1 on error
 */

static int init_home_dir(void) {
    /* Get the home directory */
    char *home_dir = getenv("HOME");
    const char *cgdb_dir = ".cgdb";

    /* Create the config directory */
    if ( ! fs_util_create_dir_in_base ( home_dir, cgdb_dir ) ) {
        logger_write_pos ( logger, __FILE__, __LINE__, "fs_util_create_dir_in_base error");
        return -1; 
    }

    fs_util_get_path ( home_dir, cgdb_dir, cgdb_home_dir );

    return 0;
}

/* create_help_file: Creates the file help.txt in $HOME/.cgdb
 *
 * Return:  0 on sucess or -1 on error
 */
static int create_help_file(void) {
   int i, length = 0;
   static FILE *fd = NULL;
   char *dashes = NULL;

   fs_util_get_path ( cgdb_home_dir, "help.txt", cgdb_help_file );
   
   if ( (fd = fopen(cgdb_help_file, "w")) == NULL)
       return -1;

   /* Write cgdb version number */
   length = 5 + strlen(VERSION);
   dashes = (char *)malloc(sizeof(char) * (length+1));
   memset(dashes, '-', length);
   dashes[length] = 0;
   fprintf(fd, "CGDB %s\n%s\n\n", VERSION, dashes);
   free(dashes);

   /* Write help file */
   for( i = 0; cgdb_help_text[i] != NULL; i++)
       fprintf(fd, "%s\n", cgdb_help_text[i]); 

   if ( fclose(fd) == -1 )
       return -1;

   return 0;
}

/* start_gdb: Starts up libtgdb
 *  Returns:  -1 on error, 0 on success
 */
static int 
start_gdb(int argc, char *argv[]) {
  tgdb_request_ptr request_ptr;
  //int val;

  tgdb = tgdb_initialize(debugger_path, argc, argv, &gdb_fd, &tty_fd);
  if (tgdb == NULL)
    return -1;

  /* Run some initialize commands */

  /* gdb may already have some breakpoints when it starts. This could happen
   * if the user puts breakpoints in there .gdbinit.
   * This makes sure that TGDB asks for the breakpoints on start up.
   */

  /**
   * Get the filename GDB defaults to. 
   */ 
  request_ptr = tgdb_request_current_location (tgdb);
  if (handle_request (tgdb, request_ptr) == -1)
    return -1;

  return 0;
}

static void send_key(int focus, char key) {
    if ( focus == 1 ) {
        int size;
	int masterfd;
	masterfd = pty_pair_get_masterfd (pty_pair);
	if (masterfd == -1)
          logger_write_pos ( logger, __FILE__, __LINE__, "send_key error");
        size = write (masterfd, &key, sizeof (char));
        if (size!= 1)
            logger_write_pos ( logger, __FILE__, __LINE__, "send_key error");
    } else if ( focus == 2 ) {
        tgdb_send_inferior_char (tgdb, key);
    }
}

/* user_input: This function will get a key from the user and process it.
 *
 *  Returns:  -1 on error, 0 on success
 */
static int user_input(void) {
    static int key, val;
    
	key = kui_manager_getkey (kui_ctx);
    if (key == -1) {
        logger_write_pos ( logger, __FILE__, __LINE__, "kui_manager_getkey error");
        return -1;
    }

    /* Process the key */
    switch ( ( val = if_input(key)) ) {
        case 1:
        case 2:
            if ( kui_term_is_cgdb_key ( key ) ) {
				char *seqbuf = (char*)kui_manager_get_raw_data( kui_ctx );

				if ( seqbuf == NULL ) {
					logger_write_pos ( logger, __FILE__, __LINE__, "input_get_last_seq error");
					return -1;
				} else {
					int length = strlen(seqbuf), i;
					for ( i = 0; i < length; i++ )
						send_key(val, seqbuf[i]);
				}
            } else
                send_key(val, key);
    
            break;
        case -1:
            return -1;
        default:
            break;
    }

    return 0;
}

static void 
process_commands(struct tgdb *tgdb)
{
  struct tgdb_response *item;

  while ( (item = tgdb_get_response (tgdb)) != NULL) {
    switch (item -> header){
      /* This updates all the breakpoints */
      case TGDB_UPDATE_BREAKPOINTS:
      {
	struct sviewer *sview = if_get_sview();
	char *file; 
	struct tgdb_list *list = item->choice.update_breakpoints.breakpoint_list;
	tgdb_list_iterator *iterator;
	struct tgdb_breakpoint *tb;

	source_clear_breaks(if_get_sview());
	iterator = tgdb_list_get_first ( list );

	while (iterator) {
	  /* For each breakpoint */
	  tb = (struct tgdb_breakpoint *)tgdb_list_get_item ( iterator );

	  file = tb->file;

	  if (tb->enabled)
	    source_enable_break(sview, file, tb->line);
	  else
	    source_disable_break(sview, file, tb->line);

	  iterator = tgdb_list_next ( iterator );
	}

	if_show_file(NULL, 0);
	break;
      }

      /* This means a source file or line number changed */
      case TGDB_UPDATE_FILE_POSITION:
      {
	struct tgdb_file_position *tfp;
	tfp = item->choice.update_file_position.file_position;

	/* Update the file */
	source_reload (if_get_sview(), tfp->absolute_path, 0);

	if_show_file (tfp->absolute_path, tfp->line_number);

	source_set_relative_path ( 
                    if_get_sview(), 
                    tfp->absolute_path, 
                    tfp->relative_path);

	break;
      }
                
      /* This is a list of all the source files */
      case TGDB_UPDATE_SOURCE_FILES:
      {
	struct tgdb_list *list = item->choice.update_source_files.source_files;
	tgdb_list_iterator *i = tgdb_list_get_first ( list );
	char *s;

	if_clear_filedlg();

	while (i) {
	  s = tgdb_list_get_item ( i );
	  if_add_filedlg_choice( s );
	  i = tgdb_list_next ( i );
	}

	if_set_focus(FILE_DLG);
	break;
      }
                
      /* The user is trying to get a list of source files that make up
       * the debugged program but libtgdb is claiming that gdb knows
       * none. */
      case TGDB_SOURCES_DENIED:
	if_display_message("Error:", 0,  
                       " No sources available! Was the program compiled with debug?");
	break;

      /* This is the absolute path to the last file the user requested */
      case TGDB_FILENAME_PAIR:
      {
	const char *apath = item->choice.filename_pair.absolute_path;
	const char *rpath = item->choice.filename_pair.relative_path;
	if_show_file ((char*)apath, 1);
	source_set_relative_path(if_get_sview(), apath, rpath);
	break;
      }

      /* The source file requested does not exist */
      case TGDB_ABSOLUTE_SOURCE_DENIED:
      {
	struct tgdb_source_file *file = item->choice.absolute_source_denied.source_file;
	if_show_file(NULL, 0 );
	/* com can be NULL when tgdb orig requests main file */
	if (file->absolute_path != NULL)
	  if_display_message("No such file:", 0, " %s", file->absolute_path);
	break;
      }
      case TGDB_INFERIOR_EXITED:
      {
	/*
	 * int *status = item->data;
	 * This could eventually go here, but for now, the update breakpoint 
	 * display function makes the status bar go back to the name of the file.
	 *
	 * if_display_message ( "Program exited with value", 0, " %d", *status );
	 */

	/* Clear the cache */
	break;
      }
      case TGDB_UPDATE_COMPLETIONS:
      {
	struct tgdb_list *list = item->choice.update_completions.completion_list;
	do_tab_completion (list);
	break;
      }
      case TGDB_UPDATE_CONSOLE_PROMPT_VALUE:
      {
        const char *new_prompt = item->choice.update_console_prompt_value.prompt_value;
	change_prompt (new_prompt);
        break;
      }
      case TGDB_QUIT:
	cleanup();            
	exit(0);
	break;
      /* Default */
      default:
	break;
    }
  }
}

/**
 * This function looks at the request that CGDB has made and determines if it 
 * effects the GDB console window. For instance, if the request makes output go
 * to that window, then the user would like to see another prompt there when the 
 * command finishes. However, if the command is 'o', to get all the sources and 
 * display them, then this doesn't effect the GDB console window and the window
 * should not redisplay the prompt.
 *
 * \param request
 * The request to analysize
 *
 * \param update
 * Will return as 1 if the console window should be updated, or 0 otherwise
 *
 * \return
 * 0 on success or -1 on error
 */
static int
does_request_require_console_update (struct tgdb_request *request, int *update)
{
  if (!request || !update)
    return -1;

  switch (request->header)
    {
      case TGDB_REQUEST_CONSOLE_COMMAND:
        *update = 1;
	break;
      case TGDB_REQUEST_INFO_SOURCES:
      case TGDB_REQUEST_FILENAME_PAIR:
      case TGDB_REQUEST_CURRENT_LOCATION:
	*update = 0;
	break;
      case TGDB_REQUEST_DEBUGGER_COMMAND:
      case TGDB_REQUEST_MODIFY_BREAKPOINT:
      case TGDB_REQUEST_COMPLETE:
	*update = 1;
	break;
      default:
	return -1;
    }

  return 0;
}

/* gdb_input: Recieves data from tgdb:
 *
 *  Returns:  -1 on error, 0 on success
 */
static int 
gdb_input()
{
  char *buf = malloc(GDB_MAXBUF+1);
  int size;
  int is_finished;

  /* Read from GDB */
  size = tgdb_process (tgdb, buf, GDB_MAXBUF, &is_finished);
  if (size == -1){
    logger_write_pos ( logger, __FILE__, __LINE__, "tgdb_recv_debugger_data error");
    free( buf );
    buf = NULL;
    return -1;
  }

  buf[size] = 0;

  process_commands(tgdb);

  /* Display GDB output 
   * The strlen check is here so that if_print does not get called
   * when displaying the filedlg. If it does get called, then the 
   * gdb window gets displayed when the filedlg is up
   */
  if ( strlen( buf ) > 0 )
    if_print(buf);

  free( buf );
  buf = NULL;

  /* Check to see if GDB is ready to recieve another command. If it is, then
   * readline should redisplay what it currently contains. There are 2 special
   * case's here.
   *
   * If there are no commands in the queue to send to GDB then  readline 
   * should simply redisplay what it has currently in it's buffer.
   * rl_forced_update_display does this.
   *
   * However, if there are commands in the queue to send to GDB, that means
   * the user already entered these through readline. In that case, simply 
   * write the command entered through readline directly, instead of having
   * readline do it (readline already processed the data). This is important
   * also because rl_forced_update_display doesn't write the data right away.
   * It writes data to rl_outstream, and then the main_loop handles both the
   * readline data and the data from the TGDB command being sent. This could
   * result in a race condition.
   */
  if (is_finished) {
    int size;

    tgdb_queue_size (tgdb, &size);
    /* This is the second case, this command was queued. */
    if (size > 0) {
      struct tgdb_request *request = tgdb_queue_pop (tgdb);
      char *prompt;
      rline_get_prompt (rline, &prompt);
      if_print (prompt);

      if (request->header == TGDB_REQUEST_CONSOLE_COMMAND) {
	if_print (request->choice.console_command.command);
	if_print ("\n");
      }

      last_request = request;
      tgdb_process_command (tgdb, request);
    /* This is the first case */
    } else {
      int update = 1, ret_val;
      
      if (last_request) {
	ret_val = does_request_require_console_update (last_request, &update);
	if (ret_val == -1)
	  return -1;
	last_request = NULL;
      }
	
      if (update)
	rline_rl_forced_update_display(rline);
    }
  }

  return 0;
}

static int readline_input()
{
    const int MAX = 1024;
    char *buf = malloc(MAX+1);
    int size;

    int masterfd = pty_pair_get_masterfd (pty_pair);
    if (masterfd == -1) {
      logger_write_pos ( logger, __FILE__, __LINE__, "pty_pair_get_masterfd error");
      return -1;
    }

    size = read (masterfd, buf, MAX);
    if (size == -1){
        logger_write_pos ( logger, __FILE__, __LINE__, "read error");
		free( buf );
		buf = NULL;
        return -1;
    }

    buf[size] = 0;

    /* Display GDB output 
     * The strlen check is here so that if_print does not get called
     * when displaying the filedlg. If it does get called, then the 
     * gdb window gets displayed when the filedlg is up
     */
    if ( size > 0 )
        if_print(buf);

	free( buf );
	buf = NULL;
    return 0;
}
/* child_input: Recieves data from the child application:
 *
 *  Returns:  -1 on error, 0 on success
 */
static int child_input()
{
    char *buf = malloc(GDB_MAXBUF+1);
    int size;

    /* Read from GDB */
    size = tgdb_recv_inferior_data ( tgdb, buf, GDB_MAXBUF);
    if (size == -1){
        logger_write_pos ( logger, __FILE__, __LINE__, "tgdb_recv_inferior_data error ");
		free( buf );
		buf = NULL;
        return -1;
    }
    buf[size] = 0;

    /* Display CHILD output */
    if_tty_print(buf);
	free( buf );
	buf = NULL;
    return 0;
}

static 
int cgdb_resize_term (int fd)
{
  int c, result;
  read (resize_pipe[0], &c, sizeof (int));

  /* If there is more input in the pipe, that means another resize has
   * been recieved, and we still have not handled this one. So, skip this
   * one and only handle the next one.
   */
  result = io_data_ready( resize_pipe[0], 0 );
  if (result == -1)
  {
    logger_write_pos ( logger, __FILE__, __LINE__, "io_data_ready");
    return -1;
  }

  if (result)
    return 0;

  if (if_resize_term () == -1)
  {
    logger_write_pos ( logger, __FILE__, __LINE__, "if_resize_term error");
    return -1;
  }

  return 0;
}

static int main_loop(void) {
    fd_set rset;
    int max;
    int masterfd, slavefd;

    masterfd = pty_pair_get_masterfd (pty_pair);
    if (masterfd == -1) {
      logger_write_pos ( logger, __FILE__, __LINE__, "pty_pair_get_masterfd error");
      return -1;
    }

    slavefd = pty_pair_get_slavefd (pty_pair);
    if (slavefd == -1) {
      logger_write_pos (logger, __FILE__, __LINE__, "pty_pair_get_slavefd error");
      return -1;
    }
    
    max = (gdb_fd > STDIN_FILENO) ? gdb_fd : STDIN_FILENO;
    max = (max > tty_fd) ? max : tty_fd;
    max = (max > resize_pipe[0]) ? max : resize_pipe[0];
    max = (max > slavefd) ? max : slavefd;
    max = (max > masterfd) ? max : masterfd;

    /* Main (infinite) loop:
     *   Sits and waits for input on either stdin (user input) or the
     *   GDB file descriptor.  When input is received, wrapper functions
     *   are called to process the input, and handle it appropriately.
     *   This will result in calls to the curses interface, typically. */
     
    for (;;){

        /* Reset the fd_set, and watch for input from GDB or stdin */
        FD_ZERO(&rset);
        
        FD_SET(STDIN_FILENO, &rset);
        FD_SET(gdb_fd, &rset);
        FD_SET(tty_fd, &rset);
        FD_SET(resize_pipe[0], &rset);
        FD_SET(slavefd, &rset);
        FD_SET(masterfd, &rset);

        /* Wait for input */
        if (select(max + 1, &rset, NULL, NULL, NULL) == -1){
            if (errno == EINTR)
                continue;
            else {
                logger_write_pos ( logger, __FILE__, __LINE__, "select failed: %s", strerror(errno));
				return -1;
			}
        }

        /* Input received through the pty:  Handle it 
         * Wrote to masterfd, now slavefd is ready, tell readline */
        if (!is_tab_completing && FD_ISSET(slavefd, &rset))
          rline_rl_callback_read_char(rline);

        /* Input received through the pty:  Handle it
         * Readline read from slavefd, and it wrote to the masterfd. 
         */
        if (FD_ISSET(masterfd, &rset))
          if ( readline_input() == -1 )
            return -1;

        /* Input received:  Handle it */
        if (FD_ISSET(STDIN_FILENO, &rset))
            if ( user_input() == -1 ) {
                logger_write_pos ( logger, __FILE__, __LINE__, "user_input failed");
                return -1;
            }

        /* child's ouptut -> stdout
         * The continue is important I think. It allows all of the child
         * output to get written to stdout before tgdb's next command.
         * This is because sometimes they are both ready.
         */
        if (FD_ISSET(tty_fd, &rset)) {
            if ( child_input() == -1 )
                return -1;
            continue;
        }

        /* gdb's output -> stdout */
        if (FD_ISSET(gdb_fd, &rset))
            if ( gdb_input() == -1 )
                return -1;

        /* A resize signal occured */
        if (FD_ISSET(resize_pipe[0], &rset))
            if ( cgdb_resize_term(resize_pipe[0]) == -1)
                return -1;
    }
    return 0;
}

/* ----------------- */
/* Exposed Functions */
/* ----------------- */

/* cleanup: Invoked by the various err_xxx funtions when dying.
 * -------- */
void cleanup()
{
	char *log_file, *tmp_log_file;
	int has_recv_data;

    ibuf_free (current_line);

    /* Cleanly scroll the screen up for a prompt */
    scrl(1);
    move(LINES-1, 0);
    printf("\n");

    rline_write_history (rline, readline_history_path);

    /* The order of these is important. They each must restore the terminal
     * the way they found it. Thus, the order in which curses/readline is 
     * started, is the reverse order in which they should be shutdown 
     */

    /* Shut down interface */
    if_shutdown();

#if 0
    if (masterfd != -1)
        util_free_tty (&masterfd, &slavefd, tty_name);
#endif

	/* Finally, should display the errors. 
	 * TGDB guarentees the logger to be open at this point.
	 * So, we can get the filename directly from the logger 
	 */
	logger_get_file ( logger, &tmp_log_file );
	log_file = strdup ( tmp_log_file );
	logger_has_recv_data ( logger, &has_recv_data );
   
    /* Shut down debugger */
    tgdb_shutdown( tgdb );

    if (tty_set_attributes (STDIN_FILENO, &term_attributes) == -1)
      logger_write_pos (logger, __FILE__, __LINE__, "tty_reset error");

	if ( has_recv_data )
		fprintf ( stderr, "CGDB had unexpected results, see %s for details.\n", log_file );

	free ( log_file );
	log_file = NULL;
}

int init_resize_pipe(void) {
    if ( pipe(resize_pipe) == -1 ) {
        logger_write_pos ( logger, __FILE__, __LINE__, "pipe error");
        return -1;
    }

    return 0;
}

int
init_readline (void)
{
  int slavefd, masterfd;
  int length;
  slavefd = pty_pair_get_slavefd (pty_pair);
  if (slavefd == -1)
    return -1;

  masterfd = pty_pair_get_masterfd (pty_pair);
  if (masterfd == -1)
    return -1;

  if (tty_off_xon_xoff (masterfd) == -1)
    return -1;
    
  /* The 16 is because I don't know how many char's the directory separator 
   * is going to be, I expect it to be 1, but who knows. */
  length = strlen (cgdb_home_dir) + strlen (readline_history_filename) + 16;
  readline_history_path = (char*)malloc (sizeof(char)*length);
  fs_util_get_path (cgdb_home_dir, readline_history_filename, readline_history_path);
  rline = rline_initialize (slavefd, rlctx_send_user_command, tab_completion);
  rline_read_history (rline, readline_history_path);
  return 0;
}

int
create_and_init_pair ()
{
  struct winsize size;
  int slavefd;

  pty_pair = pty_pair_create ();
  if (!pty_pair) {
    fprintf ( stderr, "%s:%d Unable to create PTY pair", __FILE__, __LINE__); 
    return -1;
  }

  slavefd = pty_pair_get_slavefd (pty_pair);
  if (slavefd == -1) {
    logger_write_pos (logger, __FILE__, __LINE__, "pty_pair_get_slavefd error");
    return -1;
  }

  /* Set the pty winsize to the winsize of stdout */
  if (ioctl (0, TIOCGWINSZ, &size) < 0)
    return -1;

  if (ioctl (slavefd, TIOCSWINSZ, &size) < 0)
    return -1;

  return 0;
}

int main(int argc, char *argv[]) {
/* Uncomment to debug and attach */
#if 0
    int c;
    read(0, &c, 1);
#endif

    parse_long_options(&argc, &argv);

   if (tty_get_attributes (STDIN_FILENO, &term_attributes) == -1) {
     logger_write_pos ( logger, __FILE__, __LINE__, "tty_get_attributes error");
     return -1;
   }

    current_line = ibuf_init ();
    
    if (create_and_init_pair () == -1) {
        fprintf ( stderr, "%s:%d Unable to create PTY pair", __FILE__, __LINE__); 
        exit(-1);
      }

	/* First create tgdb, because it has the error log */
    if (start_gdb(argc, argv) == -1) {
        fprintf ( stderr, "%s:%d Unable to invoke GDB", __FILE__, __LINE__); 
        exit(-1);
    }

    /* From here on, the logger is initialized */

    /* Create the home directory */
    if ( init_home_dir() == -1 ) {
        logger_write_pos ( logger, __FILE__, __LINE__, "Unable to create home dir ~/.cgdb"); 
        cleanup();
        exit(-1);
    }

    if (init_readline () == -1) {
      logger_write_pos ( logger, __FILE__, __LINE__, "Unable to init readline"); 
      cleanup();
      exit(-1);
    }

    if ( create_help_file() == -1 ) {
        logger_write_pos ( logger, __FILE__, __LINE__,  "Unable to create help file"); 
        cleanup();
        exit(-1);
	}

    if ( tty_cbreak(STDIN_FILENO) == -1 ) {
        logger_write_pos ( logger, __FILE__, __LINE__, "tty_cbreak error");
        cleanup();
        exit(-1);
	}

	kui_ctx = kui_manager_create ( STDIN_FILENO );
	if ( !kui_ctx  ) {
        logger_write_pos ( logger, __FILE__, __LINE__, "Unable to initialize input library"); 
        cleanup();
        exit(-1);
	}

    /* Initialize the display */
    switch (if_init()){
        case 1:
            logger_write_pos ( logger, __FILE__, __LINE__, "Unable to initialize the curses library");
			cleanup();
			exit(-1);
        case 2:
            logger_write_pos ( logger, __FILE__, __LINE__, "Unable to handle signal: SIGWINCH");
			cleanup();
			exit(-1);
        case 3:
            logger_write_pos ( logger, __FILE__, __LINE__, "Unable to setup highlighting groups");
			cleanup();
			exit(-1);
        case 4:
            logger_write_pos ( logger, __FILE__, __LINE__, "New GDB window failed -- out of memory?");
			cleanup();
			exit(-1);
    }

    /* Initialize the pipe that is used for resize */
    if( init_resize_pipe() == -1 ) {
        logger_write_pos ( logger, __FILE__, __LINE__, "init_resize_pipe error"); 
		cleanup();
		exit(-1);
	}

    {
        char config_file[ FSUTIL_PATH_MAX ];
        FILE *config;
        fs_util_get_path( cgdb_home_dir, "cgdbrc", config_file );
        config = fopen( config_file, "r" );
        if( config ) { 
            command_parse_file( config );
            fclose( config );
        }
    }

    /* Enter main loop */
    main_loop();

    /* Shut down curses and exit */
    cleanup();
    return 0;
}

int cgdb_set_esc_sequence_timeout ( int msec ) {
	return kui_manager_set_terminal_escape_sequence_timeout ( kui_ctx, msec );
}
