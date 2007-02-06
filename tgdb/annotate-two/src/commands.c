#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif /* HAVE_STDLIB_H */

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */

#if HAVE_REGEX_H
#include <regex.h>
#endif /* HAVE_REGEX_H */

/* Local includes */
#include "commands.h"
#include "data.h"
#include "io.h"
#include "tgdb_types.h"
#include "globals.h"
#include "logger.h"
#include "sys_util.h"
#include "queue.h"
#include "ibuf.h"
#include "a2-tgdb.h"
#include "queue.h"
#include "tgdb_list.h"
#include "annotate_two.h"

/**
 * This structure represents most of the I/O parsing state of the 
 * annotate_two subsytem.
 */
struct commands
{

  /** The current absolute path the debugger is at in the inferior.  */
  struct ibuf *absolute_path;

  /** The current line number the debugger is at in the inferior.  */
  struct ibuf *line_number;

  /** The state of the command context.  */
  enum COMMAND_STATE cur_command_state;

  /**
   * This is related to parsing the breakpoint annotations.
   * It keeps track of the current field we are in.
   */
  int cur_field_num;

  /** breakpoint information */
  /*@{*/

  /** A list of breakpoints already parsed.  */
  struct tgdb_list *breakpoint_list;

  /** The current breakpoint being parsed.  */
  struct ibuf *breakpoint_string;

  /** ???  */
  int breakpoint_table;

  /** If the current breakpoint is enabled */
  int breakpoint_enabled;

  /** ???  */
  int breakpoint_started;

  /* The regular expression matching the breakpoint GDB output */
  regex_t regex_bp;

  /*@}*/

  /** 'info source' information */

  /*@{*/

  /** The current info source line being parsed */
  struct ibuf *info_source_string;

  /** The discovered relative path, found from the info source output.  */
  struct ibuf *info_source_relative_path;

  /** The discovered absolute path, found from the info source output.  */
  struct ibuf *info_source_absolute_path;

  /** Finished parsing the line being looked for.  */
  int info_source_ready;

  /** The name of the file requested to have 'info source' run on.  */
  struct ibuf *last_info_source_requested;

  /*@}*/

  /* info sources information {{{*/
  /*@{*/

  /** ??? Finished parsing the data being looked for.  */
  int sources_ready;

  /** All of the sources.  */
  struct ibuf *info_sources_string;

  /** All of the source, parsed in put in a list, 1 at a time.  */
  struct tgdb_list *inferior_source_files;

  /*@}*/
  /* }}}*/

  /* tab completion information {{{*/
  /*@{*/

  /** ??? Finished parsing the data being looked for.  */
  int tab_completion_ready;

  /** A tab completion item */
  struct ibuf *tab_completion_string;

  /** All of the tab completion items, parsed in put in a list, 1 at a time. */
  struct tgdb_list *tab_completions;

  /*@}*/
  /* }}}*/

  /** The absolute path prefix output by GDB when 'info source' is given */
  const char *source_prefix;

  /** The length of the line above.  */
  int source_prefix_length;

  /** The relative path prefix output by GDB when 'info source' is given */
  const char *source_relative_prefix;

  /** The length of the line above.  */
  int source_relative_prefix_length;
};

struct commands *
commands_initialize (void)
{
  struct commands *c = (struct commands *) cgdb_malloc (sizeof (struct commands));
  const char *regex = "[io]n (.*) at (.*):([0-9]+)";

  c->absolute_path = ibuf_init ();
  c->line_number = ibuf_init ();

  c->cur_command_state = VOID_COMMAND;
  c->cur_field_num = 0;

  c->breakpoint_list = tgdb_list_init ();
  c->breakpoint_string = ibuf_init ();
  c->breakpoint_table = 0;
  c->breakpoint_enabled = 0;
  c->breakpoint_started = 0;
  if (regcomp (&c->regex_bp, regex, REG_EXTENDED) != 0)
    return NULL;

  c->info_source_string = ibuf_init ();
  c->info_source_relative_path = ibuf_init ();
  c->info_source_absolute_path = ibuf_init ();
  c->info_source_ready = 0;
  c->last_info_source_requested = ibuf_init ();

  c->sources_ready = 0;
  c->info_sources_string = ibuf_init ();
  c->inferior_source_files = tgdb_list_init ();

  c->tab_completion_ready = 0;
  c->tab_completion_string = ibuf_init ();
  c->tab_completions = tgdb_list_init ();

  c->source_prefix = "Located in ";
  c->source_prefix_length = 11;

  c->source_relative_prefix = "Current source file is ";
  c->source_relative_prefix_length = 23;

  return c;
}

int
free_breakpoint (void *item)
{
  struct tgdb_breakpoint *bp = (struct tgdb_breakpoint *) item;
  if (bp->file)
    {
      free (bp->file);
      bp->file = NULL;
    }

  if (bp->funcname)
    {
      free (bp->funcname);
      bp->funcname = NULL;
    }

  free (bp);
  bp = NULL;

  return 0;
}

int
free_char_star (void *item)
{
  char *s = (char *) item;

  free (s);
  s = NULL;

  return 0;
}

void
commands_shutdown (struct commands *c)
{
  if (c == NULL)
    return;

  ibuf_free (c->absolute_path);
  c->absolute_path = NULL;

  ibuf_free (c->line_number);
  c->line_number = NULL;

  tgdb_list_free (c->breakpoint_list, free_breakpoint);
  tgdb_list_destroy (c->breakpoint_list);

  ibuf_free (c->breakpoint_string);
  c->breakpoint_string = NULL;
  regfree(&c->regex_bp);

  ibuf_free (c->info_source_string);
  c->info_source_string = NULL;

  ibuf_free (c->info_source_relative_path);
  c->info_source_relative_path = NULL;

  ibuf_free (c->info_source_absolute_path);
  c->info_source_absolute_path = NULL;

  ibuf_free (c->info_sources_string);
  c->info_sources_string = NULL;

  ibuf_free (c->tab_completion_string);
  c->tab_completion_string = NULL;

  tgdb_list_destroy (c->tab_completions);

  tgdb_list_free (c->inferior_source_files, free_char_star);
  tgdb_list_destroy (c->inferior_source_files);

  /* TODO: free source_files queue */

  free (c);
  c = NULL;
}

int
commands_parse_field (struct commands *c, const char *buf, size_t n,
		      int *field)
{
  if (sscanf (buf, "field %d", field) != 1)
    logger_write_pos (logger, __FILE__, __LINE__,
		      "parsing field annotation failed (%s)\n", buf);

  return 0;
}

/* source filename:line:character:middle:addr */
int
commands_parse_source (struct commands *c,
		       struct tgdb_list *client_command_list,
		       const char *buf, size_t n, struct tgdb_list *list)
{
  int i = 0;
  char copy[n + 1];
  char *cur = copy + n;
  struct ibuf *file = ibuf_init (), *line = ibuf_init ();
  strncpy (copy, buf, n + 1);	/* modify local copy */

  while (cur != copy && i <= 3)
    {
      if (*cur == ':')
	{
	  if (i == 3)
	    {
	      int length = strlen (cur + 1);
	      char *temp = cgdb_malloc (sizeof (char) * (length + 1));

	      if (sscanf (cur + 1, "%s", temp) != 1)
		logger_write_pos (logger, __FILE__, __LINE__,
				  "Could not get line number");

	      ibuf_add (line, temp);
	      free (temp);
	      temp = NULL;
	    }

	  *cur = '\0';
	  ++i;
	}
      --cur;
    }				/* end while */

  {
    int length = strlen (copy);
    char *temp = cgdb_malloc (sizeof (char) * (length + 1));
    if (strncmp ("source ", copy, 7) == 0 && length > 7) {
      int j;
      for (j = 7; j < length; j++)
	ibuf_addchar (file, copy[j]);
    } else {
      logger_write_pos (logger, __FILE__, __LINE__, "Could not get file name out of line (%s)", buf);
    }

    free (temp);
    temp = NULL;
  }

  ibuf_clear (c->absolute_path);
  ibuf_add (c->absolute_path, ibuf_get (file));
  ibuf_clear (c->line_number);
  ibuf_add (c->line_number, ibuf_get (line));

  ibuf_free (file);
  ibuf_free (line);

  /* set up the info_source command to get the relative path */
  if (commands_issue_command (c,
			      client_command_list,
			      ANNOTATE_INFO_SOURCE_RELATIVE, NULL, 1) == -1)
    {
      logger_write_pos (logger, __FILE__, __LINE__,
			"commands_issue_command error");
      return -1;
    }

  return 0;
}

/** 
 * Parse a breakpoint that GDB passes back when annotate=2 is set.
 * 
 * Unfortunatly, the line that this function has to parse is completly 
 * ambiguous. GDB does not output a line that can be read in a 
 * non-ambiguous way. Therefore, TGDB tries its best to read the line 
 * properly. The line is in this format.
 *
 *  '[io]n .* at .*:number'
 *
 * so, TGDB can parser the ':number' part without a problem. However,
 * it may not be able to get function name and filename correctly. If
 * the filename contains ' at ' in it, then TGDB will stop prematurly.
 */
static int
parse_breakpoint (struct commands *c)
{
#define BP_REGEX_SIZE (4)
  char *info_ptr;
  size_t nmatch = BP_REGEX_SIZE;
  regmatch_t pmatch[BP_REGEX_SIZE];
  char *matches[BP_REGEX_SIZE] = {NULL, NULL, NULL, NULL};
  int cur, val;
  struct tgdb_breakpoint *tb;

  info_ptr = ibuf_get (c->breakpoint_string);
  if (!info_ptr) /* This should never really happen */
    return -1;

  /* Check to see if this is a watchpoint, if it is,
   * don't parse for a breakpoint.  */
  if (strstr (info_ptr, " at ") == NULL)
    return 0;

  val = regexec(&c->regex_bp, info_ptr, nmatch, pmatch, 0);
  if (val != 0)
    {
      logger_write_pos (logger, __FILE__, __LINE__, "regexec failed");
      return -1;
    }

  /* Get the whole match, function, filename and number name */
  for (cur = 0; cur < BP_REGEX_SIZE; ++cur)
    if (pmatch[cur].rm_so != -1)
      {
	int size = pmatch[cur].rm_eo-pmatch[cur].rm_so;
	matches[cur] = cgdb_malloc (sizeof (char)*(size)+1);
	strncpy (matches[cur], &info_ptr[pmatch[cur].rm_so], size);
	matches[cur][size] = 0;
      }

  tb = (struct tgdb_breakpoint *) cgdb_malloc (sizeof (struct tgdb_breakpoint));
  tb->funcname = matches[1];
  tb->file = matches[2];
  tb->line = atoi (matches[3]);

  if (c->breakpoint_enabled == 1)
    tb->enabled = 1;
  else
    tb->enabled = 0;

  tgdb_list_append (c->breakpoint_list, tb);

  /* Matches 1 && 2 are freed by client. */
  free (matches[0]); matches[0] = NULL;
  free (matches[3]); matches[3] = NULL;

  return 0;
}

void
commands_set_state (struct commands *c,
		    enum COMMAND_STATE state, struct tgdb_list *list)
{
  c->cur_command_state = state;

  switch (c->cur_command_state)
    {
    case RECORD:
      if (ibuf_length (c->breakpoint_string) > 0)
	{
	  if (parse_breakpoint (c) == -1)
	    logger_write_pos (logger, __FILE__, __LINE__, "parse_breakpoint error");

	  ibuf_clear (c->breakpoint_string);
	  c->breakpoint_enabled = 0;
	}
      break;
    case BREAKPOINT_TABLE_END:
      if (ibuf_length (c->breakpoint_string) > 0)
	if (parse_breakpoint (c) == -1)
	  logger_write_pos (logger, __FILE__, __LINE__, "parse_breakpoint error");
      {
	struct tgdb_response *response = (struct tgdb_response *)
	  cgdb_malloc (sizeof (struct tgdb_response));
	response->header = TGDB_UPDATE_BREAKPOINTS;
	response->choice.update_breakpoints.breakpoint_list =
	  c->breakpoint_list;

	/* At this point, annotate needs to send the breakpoints to the gui.
	 * All of the valid breakpoints are stored in breakpoint_queue. */
	tgdb_types_append_command (list, response);
      }

      ibuf_clear (c->breakpoint_string);
      c->breakpoint_enabled = 0;

      c->breakpoint_started = 0;
      break;
    case BREAKPOINT_HEADERS:
      c->breakpoint_table = 0;
      break;
    case BREAKPOINT_TABLE_BEGIN:

      /* The breakpoint queue should be empty at this point */
      c->breakpoint_table = 1;
      c->breakpoint_started = 1;
      break;
    case INFO_SOURCE_FILENAME_PAIR:
      break;
    case INFO_SOURCE_RELATIVE:
      break;
    case INFO_SOURCES:
      break;
    default:
      break;
    }
}

void
commands_set_field_num (struct commands *c, int field_num)
{
  c->cur_field_num = field_num;

  /* clear buffer and start over */
  if (c->breakpoint_table && c->cur_command_state == FIELD
      && c->cur_field_num == 5)
    ibuf_clear (c->breakpoint_string);
}

enum COMMAND_STATE
commands_get_state (struct commands *c)
{
  return c->cur_command_state;
}

static void
commands_prepare_info_source (struct annotate_two *a2, struct commands *c,
			      enum COMMAND_STATE state)
{
  data_set_state (a2, INTERNAL_COMMAND);
  ibuf_clear (c->info_source_string);
  ibuf_clear (c->info_source_relative_path);
  ibuf_clear (c->info_source_absolute_path);

  if (state == INFO_SOURCE_FILENAME_PAIR)
    commands_set_state (c, INFO_SOURCE_FILENAME_PAIR, NULL);
  else if (state == INFO_SOURCE_RELATIVE)
    commands_set_state (c, INFO_SOURCE_RELATIVE, NULL);

  c->info_source_ready = 0;
}

void
commands_list_command_finished (struct commands *c,
				struct tgdb_list *list, int success)
{
  /* The file does not exist and it can not be opened.
   * So we return that information to the gui.  */
  struct tgdb_source_file *rejected = (struct tgdb_source_file *)
    cgdb_malloc (sizeof (struct tgdb_source_file));
  struct tgdb_response *response = (struct tgdb_response *)
    cgdb_malloc (sizeof (struct tgdb_response));

  if (c->last_info_source_requested == NULL)
    rejected->absolute_path = NULL;
  else
    rejected->absolute_path =
      strdup (ibuf_get (c->last_info_source_requested));

  response->header = TGDB_ABSOLUTE_SOURCE_DENIED;
  response->choice.absolute_source_denied.source_file = rejected;

  tgdb_types_append_command (list, response);
}

/* This will send to the gui the absolute path to the file being requested. 
 * Otherwise the gui will be notified that the file is not valid.
 */
static void
commands_send_source_absolute_source_file (struct commands *c,
					   struct tgdb_list *list)
{
  /*err_msg("Whats up(%s:%d)\r\n", info_source_buf, info_source_buf_pos); */
  unsigned long length = ibuf_length (c->info_source_absolute_path);

  /* found */
  if (length > 0)
    {
      char *apath = NULL, *rpath = NULL;
      struct tgdb_response *response;
      if (length > 0)
	apath = ibuf_get (c->info_source_absolute_path);
      if (ibuf_length (c->info_source_relative_path) > 0)
	rpath = ibuf_get (c->info_source_relative_path);

      response = (struct tgdb_response *)
	cgdb_malloc (sizeof (struct tgdb_response));
      response->header = TGDB_FILENAME_PAIR;
      response->choice.filename_pair.absolute_path = strdup (apath);
      response->choice.filename_pair.relative_path = strdup (rpath);
      tgdb_types_append_command (list, response);
      /* not found */
    }
  else
    {
      struct tgdb_source_file *rejected = (struct tgdb_source_file *)
	cgdb_malloc (sizeof (struct tgdb_source_file));
      struct tgdb_response *response = (struct tgdb_response *)
	cgdb_malloc (sizeof (struct tgdb_response));
      response->header = TGDB_ABSOLUTE_SOURCE_DENIED;

      if (c->last_info_source_requested == NULL)
	rejected->absolute_path = NULL;
      else
	rejected->absolute_path =
	  strdup (ibuf_get (c->last_info_source_requested));

      response->choice.absolute_source_denied.source_file = rejected;
      tgdb_types_append_command (list, response);
    }
}

static void
commands_send_source_relative_source_file (struct commands *c,
					   struct tgdb_list *list)
{
  /* So far, INFO_SOURCE_RELATIVE is only used when a 
   * TGDB_UPDATE_FILE_POSITION is needed.
   */
  /* This section allocates a new structure to add into the queue 
   * All of its members will need to be freed later.
   */
  struct tgdb_file_position *tfp = (struct tgdb_file_position *)
    cgdb_malloc (sizeof (struct tgdb_file_position));
  struct tgdb_response *response = (struct tgdb_response *)
    cgdb_malloc (sizeof (struct tgdb_response));
  tfp->absolute_path = strdup (ibuf_get (c->absolute_path));
  tfp->relative_path = strdup (ibuf_get (c->info_source_relative_path));
  tfp->line_number = atoi (ibuf_get (c->line_number));

  response->header = TGDB_UPDATE_FILE_POSITION;
  response->choice.update_file_position.file_position = tfp;

  tgdb_types_append_command (list, response);
}

/* commands_process_info_source:
 * -----------------------------
 *
 * This function is capable of parsing the output of 'info source'.
 * It can get both the absolute and relative path to the source file.
 */
static void
commands_process_info_source (struct commands *c,
			      struct tgdb_list *list, char a)
{
  unsigned long length;
  static char *info_ptr;

  if (c->info_source_ready)	/* Already found */
    return;

  info_ptr = ibuf_get (c->info_source_string);
  length = ibuf_length (c->info_source_string);

  if (a == '\r')
    return;

  if (a == '\n')
    {
      /* This is the line containing the absolute path to the source file */
      if (length >= c->source_prefix_length &&
	  strncmp (info_ptr, c->source_prefix, c->source_prefix_length) == 0)
	{
	  ibuf_add (c->info_source_absolute_path,
		    info_ptr + c->source_prefix_length);
	  c->info_source_ready = 1;
	  ibuf_clear (c->info_source_string);

	  /* commands_finalize_command will use the populated data */

	  /* This is the line contatining the relative path to the source file */
	}
      else if (length >= c->source_relative_prefix_length &&
	       strncmp (info_ptr, c->source_relative_prefix,
			c->source_relative_prefix_length) == 0)
	{
	  ibuf_add (c->info_source_relative_path,
		    info_ptr + c->source_relative_prefix_length);
	  ibuf_clear (c->info_source_string);
	}
      else
	ibuf_clear (c->info_source_string);
    }
  else
    ibuf_addchar (c->info_source_string, a);
}

static void
commands_process_source_line (struct commands *c)
{
  unsigned long length = ibuf_length (c->info_sources_string), i, start = 0;
  static char *info_ptr;
  static char *nfile;
  info_ptr = ibuf_get (c->info_sources_string);

  for (i = 0; i < length; ++i)
    {
      if (i > 0 && info_ptr[i - 1] == ',' && info_ptr[i] == ' ')
	{
	  nfile = calloc (sizeof (char), i - start);
	  strncpy (nfile, info_ptr + start, i - start - 1);
	  start += ((i + 1) - start);
	  tgdb_list_append (c->inferior_source_files, nfile);
	}
      else if (i == length - 1)
	{
	  nfile = calloc (sizeof (char), i - start + 2);
	  strncpy (nfile, info_ptr + start, i - start + 1);
	  tgdb_list_append (c->inferior_source_files, nfile);
	}
    }
}


/* process's source files */
static void
commands_process_sources (struct commands *c, char a)
{
  static const char *sourcesReadyString = "Source files for which symbols ";
  static const int sourcesReadyStringLength = 31;
  static char *info_ptr;
  ibuf_addchar (c->info_sources_string, a);

  if (a == '\n')
    {
      ibuf_delchar (c->info_sources_string);	/* remove '\n' and null terminate */
      /* valid lines are 
       * 1. after the first line,
       * 2. do not end in ':' 
       * 3. and are not empty 
       */
      info_ptr = ibuf_get (c->info_sources_string);

      if (strncmp (info_ptr, sourcesReadyString, sourcesReadyStringLength) ==
	  0)
	c->sources_ready = 1;

      /* is this a valid line */
      if (ibuf_length (c->info_sources_string) > 0 && c->sources_ready
	  && info_ptr[ibuf_length (c->info_sources_string) - 1] != ':')
	commands_process_source_line (c);

      ibuf_clear (c->info_sources_string);
    }
}

static void
commands_process_completion (struct commands *c)
{
  const char *ptr = ibuf_get (c->tab_completion_string);
  const char *scomplete = "server complete ";
  /* Do not add the "server complete " matches, which is returned with 
   * GNAT 3.15p version of GDB. Most likely this could happen with other 
   * implementations that are derived from GDB.
   */
  if (strncmp (ptr, scomplete, strlen (scomplete)) != 0)
    tgdb_list_append (c->tab_completions, strdup (ptr));
}

/* process's source files */
static void
commands_process_complete (struct commands *c, char a)
{
  ibuf_addchar (c->tab_completion_string, a);

  if (a == '\n')
    {
      ibuf_delchar (c->tab_completion_string);	/* remove '\n' and null terminate */

      if (ibuf_length (c->tab_completion_string) > 0)
	commands_process_completion (c);

      ibuf_clear (c->tab_completion_string);
    }
}

void
commands_free (struct commands *c, void *item)
{
  free ((char *) item);
}

void
commands_send_gui_sources (struct commands *c, struct tgdb_list *list)
{
  /* If the inferior program was not compiled with debug, then no sources
   * will be available. If no sources are available, do not return the
   * TGDB_UPDATE_SOURCE_FILES command. */
  if (tgdb_list_size (c->inferior_source_files) > 0)
    {
      struct tgdb_response *response = (struct tgdb_response *)
	cgdb_malloc (sizeof (struct tgdb_response));
      response->header = TGDB_UPDATE_SOURCE_FILES;
      response->choice.update_source_files.source_files =
	c->inferior_source_files;
      tgdb_types_append_command (list, response);
    }
}


void
commands_send_gui_completions (struct commands *c, struct tgdb_list *list)
{
  /* If the inferior program was not compiled with debug, then no sources
   * will be available. If no sources are available, do not return the
   * TGDB_UPDATE_SOURCE_FILES command. */
/*  if (tgdb_list_size ( c->tab_completions ) > 0)*/
  struct tgdb_response *response = (struct tgdb_response *)
    cgdb_malloc (sizeof (struct tgdb_response));
  response->header = TGDB_UPDATE_COMPLETIONS;
  response->choice.update_completions.completion_list = c->tab_completions;
  tgdb_types_append_command (list, response);
}

void
commands_process (struct commands *c, char a, struct tgdb_list *list)
{
  if (commands_get_state (c) == INFO_SOURCES)
    {
      commands_process_sources (c, a);
    }
  else if (commands_get_state (c) == COMPLETE)
    {
      commands_process_complete (c, a);
    }
  else if (commands_get_state (c) == INFO_LIST)
    {
      /* do nothing with data */
    }
  else if (commands_get_state (c) == INFO_SOURCE_FILENAME_PAIR
	   || commands_get_state (c) == INFO_SOURCE_RELATIVE)
    {
      commands_process_info_source (c, list, a);
    }
  else if (c->breakpoint_table && c->cur_command_state == FIELD
	   && c->cur_field_num == 5)
    { /* the file name and line num */
      if (a != '\n' || a != '\r')
        ibuf_addchar (c->breakpoint_string, a);
    }
  else if (c->breakpoint_table && c->cur_command_state == FIELD
	   && c->cur_field_num == 3 && a == 'y')
    {
      c->breakpoint_enabled = 1;
    }
}

/*******************************************************************************
 * This must be translated to just return the proper command.
 ******************************************************************************/

/* commands_prepare_info_breakpoints: 
 * ----------------------------------
 *  
 *  This prepares the command 'info breakpoints' 
 */
static void
commands_prepare_info_breakpoints (struct commands *c)
{
  ibuf_clear (c->breakpoint_string);
}

/* commands_prepare_tab_completion:
 * --------------------------------
 *
 * This prepares the tab completion command
 */
static void
commands_prepare_tab_completion (struct annotate_two *a2, struct commands *c)
{
  c->tab_completion_ready = 0;
  ibuf_clear (c->tab_completion_string);
  commands_set_state (c, COMPLETE, NULL);
  global_set_start_completion (a2->g);
}

/* commands_prepare_info_sources: 
 * ------------------------------
 *
 *  This prepares the command 'info sources' by setting certain variables.
 */
static void
commands_prepare_info_sources (struct annotate_two *a2, struct commands *c)
{
  c->sources_ready = 0;
  ibuf_clear (c->info_sources_string);
  commands_set_state (c, INFO_SOURCES, NULL);
  global_set_start_info_sources (a2->g);
}

/* commands_prepare_list: 
 * -----------------------------
 *  This runs the command 'list filename:1' and then runs
 *  'info source' to find out what the absolute path to filename is.
 * 
 *    filename -> The name of the file to check the absolute path of.
 */
static void
commands_prepare_list (struct annotate_two *a2, struct commands *c,
		       char *filename)
{
  commands_set_state (c, INFO_LIST, NULL);
  global_set_start_list (a2->g);
  c->info_source_ready = 0;
}

void
commands_finalize_command (struct commands *c, struct tgdb_list *list)
{
  switch (commands_get_state (c))
    {
    case INFO_SOURCE_RELATIVE:
    case INFO_SOURCE_FILENAME_PAIR:
      if (c->info_source_ready == 0)
	{
	  struct tgdb_source_file *rejected = (struct tgdb_source_file *)
	    cgdb_malloc (sizeof (struct tgdb_source_file));
	  struct tgdb_response *response = (struct tgdb_response *)
	    cgdb_malloc (sizeof (struct tgdb_response));
	  if (c->last_info_source_requested == NULL)
	    rejected->absolute_path = NULL;
	  else
	    rejected->absolute_path =
	      strdup (ibuf_get (c->last_info_source_requested));

	  response->header = TGDB_ABSOLUTE_SOURCE_DENIED;
	  response->choice.absolute_source_denied.source_file = rejected;
	  tgdb_types_append_command (list, response);
	}
      else
	{
	  if (commands_get_state (c) == INFO_SOURCE_RELATIVE)
	    commands_send_source_relative_source_file (c, list);
	  else if (commands_get_state (c) == INFO_SOURCE_FILENAME_PAIR)
	    commands_send_source_absolute_source_file (c, list);
	}
      break;
    default:
      break;
    }
}

int
commands_prepare_for_command (struct annotate_two *a2,
			      struct commands *c,
			      struct tgdb_command *com)
{

  enum annotate_commands *a_com =
    (enum annotate_commands *) com->tgdb_client_private_data;

  /* Set the commands state to nothing */
  commands_set_state (c, VOID, NULL);

  /* The list command is no longer running */
  global_list_finished (a2->g);

  if (global_list_had_error (a2->g) == 1
      && commands_get_state (c) == INFO_LIST)
    {
      global_set_list_error (a2->g, 0);
      return -1;
    }

  if (a_com == NULL)
    {
      data_set_state (a2, USER_COMMAND);
      return 0;
    }

  switch (*a_com)
    {
    case ANNOTATE_INFO_SOURCES:
      commands_prepare_info_sources (a2, c);
      break;
    case ANNOTATE_LIST:
      commands_prepare_list (a2, c, com->tgdb_command_data);
      break;
    case ANNOTATE_INFO_LINE:
      break;
    case ANNOTATE_INFO_SOURCE_RELATIVE:
      commands_prepare_info_source (a2, c, INFO_SOURCE_RELATIVE);
      break;
    case ANNOTATE_INFO_SOURCE_FILENAME_PAIR:
      commands_prepare_info_source (a2, c, INFO_SOURCE_FILENAME_PAIR);
      break;
    case ANNOTATE_INFO_BREAKPOINTS:
      commands_prepare_info_breakpoints (c);
      break;
    case ANNOTATE_TTY:
      break;			/* Nothing to do */
    case ANNOTATE_COMPLETE:
      commands_prepare_tab_completion (a2, c);
      io_debug_write_fmt ("<%s\n>", com->tgdb_command_data);
      break;			/* Nothing to do */
    case ANNOTATE_INFO_SOURCE:
    case ANNOTATE_SET_PROMPT:
    case ANNOTATE_VOID:
      break;
    default:
      logger_write_pos (logger, __FILE__, __LINE__,
			"commands_prepare_for_command error");
      break;
    };

  data_set_state (a2, INTERNAL_COMMAND);
  io_debug_write_fmt ("<%s\n>", com->tgdb_command_data);

  return 0;
}

/** 
 * This is responsible for creating a command to run through the debugger.
 *
 * \param com 
 * The annotate command to run
 *
 * \param data 
 * Information that may be needed to create the command
 *
 * \return
 * A command ready to be run through the debugger or NULL on error.
 * The memory is malloc'd, and must be freed.
 */
static char *
commands_create_command (struct commands *c, enum annotate_commands com,
			 const char *data)
{
  char *ncom = NULL;

  switch (com)
    {
    case ANNOTATE_INFO_SOURCES:
      ncom = strdup ("server info sources\n");
      break;
    case ANNOTATE_LIST:
      {
	struct ibuf *temp_file_name = NULL;
	if (data != NULL)
	  {
	    temp_file_name = ibuf_init ();
	    ibuf_add (temp_file_name, data);
	  }
	if (data == NULL)
	  ncom = (char *) cgdb_malloc (sizeof (char) * (16));
	else
	  ncom = (char *) cgdb_malloc (sizeof (char) * (16 + strlen (data)));
	strcpy (ncom, "server list ");

	if (temp_file_name != NULL)
	  {
	    strcat (ncom, ibuf_get (temp_file_name));
	    strcat (ncom, ":1");
	  }

	/* This happens when the user wants to get the absolute path of 
	 * the current file. They pass in NULL to represent that. */
	if (temp_file_name == NULL)
	  {
	    ibuf_free (c->last_info_source_requested);
	    c->last_info_source_requested = NULL;
	  }
	else
	  {
	    if (c->last_info_source_requested == NULL)
	      c->last_info_source_requested = ibuf_init ();

	    ibuf_clear (c->last_info_source_requested);
	    ibuf_add (c->last_info_source_requested,
		      ibuf_get (temp_file_name));
	  }

	strcat (ncom, "\n");

	ibuf_free (temp_file_name);
	temp_file_name = NULL;
	break;
      }
    case ANNOTATE_INFO_LINE:
      ncom = strdup ("server info line\n");
      break;
    case ANNOTATE_INFO_SOURCE_RELATIVE:
    case ANNOTATE_INFO_SOURCE_FILENAME_PAIR:
      ncom = strdup ("server info source\n");
      break;
    case ANNOTATE_INFO_BREAKPOINTS:
      ncom = strdup ("server info breakpoints\n");
      break;
    case ANNOTATE_TTY:
      {
	struct ibuf *temp_tty_name = ibuf_init ();
	ibuf_add (temp_tty_name, data);
	ncom = (char *) cgdb_malloc (sizeof (char) * (13 + strlen (data)));
	strcpy (ncom, "server tty ");
	strcat (ncom, ibuf_get (temp_tty_name));
	strcat (ncom, "\n");

	ibuf_free (temp_tty_name);
	temp_tty_name = NULL;
	break;
      }
    case ANNOTATE_COMPLETE:
      ncom = (char *) cgdb_malloc (sizeof (char) * (18 + strlen (data)));
      strcpy (ncom, "server complete ");
      strcat (ncom, data);
      strcat (ncom, "\n");
      break;
    case ANNOTATE_SET_PROMPT:
      ncom = strdup (data);
      break;
    case ANNOTATE_VOID:
    default:
      logger_write_pos (logger, __FILE__, __LINE__, "switch error");
      break;
    };

  return ncom;
}

int
commands_user_ran_command (struct commands *c,
			   struct tgdb_list *client_command_list)
{
  if (commands_issue_command (c,
			      client_command_list,
			      ANNOTATE_INFO_BREAKPOINTS, NULL, 0) == -1)
    {
      logger_write_pos (logger, __FILE__, __LINE__,
			"commands_issue_command error");
      return -1;
    }

#if 0
  /* This was added to allow support for TGDB to tell the FE when the user
   * switched locations due to a 'list foo:1' command. The info line would
   * get issued and the FE would know exactly what GDB was currently looking
   * at. However, it was noticed that the FE couldn't distinguish between when
   * a new file location should be displayed, or when a new file location 
   * shouldn't be displayed. For instance, if the user moves around in the
   * source window, and then types 'p argc' it would then get the original
   * position it was just at and the FE would show that spot again, but this
   * isn't necessarily what the FE wants.
   */
  if (commands_issue_command (c,
			      client_command_list,
			      ANNOTATE_INFO_LINE, NULL, 0) == -1)
    {
      logger_write_pos (logger, __FILE__, __LINE__,
			"commands_issue_command error");
      return -1;
    }
#endif

  return 0;
}

int
commands_issue_command (struct commands *c,
			struct tgdb_list *client_command_list,
			enum annotate_commands com, const char *data, int oob)
{
  char *ncom = commands_create_command (c, com, data);
  struct tgdb_command *client_command = NULL;
  enum annotate_commands *nacom =
    (enum annotate_commands *) cgdb_malloc (sizeof (enum annotate_commands));

  *nacom = com;

  if (ncom == NULL)
    {
      logger_write_pos (logger, __FILE__, __LINE__,
			"commands_issue_command error");
      return -1;
    }

  /* This should send the command to tgdb-base to handle */
  if (oob == 0)
    {
      client_command = tgdb_command_create (ncom, TGDB_COMMAND_TGDB_CLIENT,
					    (void *) nacom);
    }
  else if (oob == 1)
    {
      client_command = tgdb_command_create (ncom, 
					    TGDB_COMMAND_TGDB_CLIENT_PRIORITY,
					    (void *) nacom);
    }
  else if (oob == 4)
    {
      client_command = tgdb_command_create (ncom, TGDB_COMMAND_TGDB_CLIENT,
					    (void *) nacom);
    }

  if (ncom)
    {
      free (ncom);
      ncom = NULL;
    }

  /* Append to the command_container the commands */
  tgdb_list_append (client_command_list, client_command);

  return 0;
}
