#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */

#if HAVE_STDIO_H
#include <stdio.h>
#endif

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif /* HAVE_STDLIB_H */

/* Local includes */
#include "tgdb_types.h"
#include "logger.h"
#include "sys_util.h"
#include "ibuf.h"
#include "tgdb_list.h"
#include "queue.h"

static int
tgdb_types_print_item (void *command)
{
  struct tgdb_response *com = (struct tgdb_response *) command;
  FILE *fd = stderr;

  if (!com)
    {
      logger_write_pos (logger, __FILE__, __LINE__, "item is null");
      return -1;
    }

  switch (com->header)
    {
    case TGDB_UPDATE_BREAKPOINTS:
      {
	struct tgdb_list *list =
	  com->choice.update_breakpoints.breakpoint_list;
	tgdb_list_iterator *iterator;
	struct tgdb_breakpoint *tb;

	fprintf (fd, "Breakpoint start\n");

	iterator = tgdb_list_get_first (list);

	while (iterator)
	  {
	    tb = (struct tgdb_breakpoint *) tgdb_list_get_item (iterator);

	    if (tb == NULL)
	      logger_write_pos (logger, __FILE__, __LINE__,
				"breakpoint is NULL");

	    fprintf (fd,
		     "\tFILE(%s) FUNCNAME(%s) LINE(%d) ENABLED(%d)\n",
		     tb->file, tb->funcname, tb->line, tb->enabled);

	    iterator = tgdb_list_next (iterator);
	  }

	fprintf (fd, "Breakpoint end\n");
	break;
      }
    case TGDB_UPDATE_FILE_POSITION:
      {
	struct tgdb_file_position *tfp =
	  com->choice.update_file_position.file_position;

	fprintf (fd,
		 "TGDB_UPDATE_FILE_POSITION ABSOLUTE(%s)RELATIVE(%s)LINE(%d)\n",
		 tfp->absolute_path, tfp->relative_path, tfp->line_number);
	break;
      }
    case TGDB_UPDATE_SOURCE_FILES:
      {
	struct tgdb_list *list = com->choice.update_source_files.source_files;
	tgdb_list_iterator *i;
	char *s;

	fprintf (fd, "Inferior source files start\n");
	i = tgdb_list_get_first (list);

	while (i)
	  {
	    s = (char *) tgdb_list_get_item (i);
	    fprintf (fd, "TGDB_SOURCE_FILE (%s)\n", s);
	    i = tgdb_list_next (i);
	  }
	fprintf (fd, "Inferior source files end\n");
	break;
      }
    case TGDB_SOURCES_DENIED:
      fprintf (fd, "TGDB_SOURCES_DENIED\n");
      break;
    case TGDB_FILENAME_PAIR:
      {
	const char *apath = com->choice.filename_pair.absolute_path;
	const char *rpath = com->choice.filename_pair.relative_path;
	fprintf (fd, "TGDB_ABSOLUTE_SOURCE_ACCEPTED ABSOLUTE(%s) RELATIVE(%s)\n",
		 apath, rpath);
	break;
      }
    case TGDB_ABSOLUTE_SOURCE_DENIED:
      {
	struct tgdb_source_file *file = com->choice.absolute_source_denied.source_file;
	fprintf (fd, "TGDB_ABSOLUTE_SOURCE_DENIED(%s)\n",
		 file->absolute_path);
	break;
      }
    case TGDB_INFERIOR_EXITED:
      {
	int *status = com->choice.inferior_exited.exit_status;
	fprintf (fd, "TGDB_INFERIOR_EXITED(%d)\n", *status);
	break;
      }
    case TGDB_UPDATE_COMPLETIONS:
      {
	struct tgdb_list *list = com->choice.update_completions.completion_list;
	tgdb_list_iterator *i;
	char *s;

	fprintf (fd, "completions start\n");
	i = tgdb_list_get_first (list);

	while (i)
	  {
	    s = (char *) tgdb_list_get_item (i);
	    fprintf (fd, "TGDB_UPDATE_COMPLETION (%s)\n", s);
	    i = tgdb_list_next (i);
	  }
	fprintf (fd, "completions end\n");
	break;
      }
    case TGDB_UPDATE_CONSOLE_PROMPT_VALUE: 
      {
	const char *value = com->choice.update_console_prompt_value.prompt_value;
	fprintf (fd, "TGDB_UPDATE_CONSOLE_PROMPT_VALUE(%s)\n", value);
	break;
      }
    case TGDB_QUIT:
      {
	struct tgdb_debugger_exit_status *status = com->choice.quit.exit_status;
	fprintf (fd, "TGDB_QUIT EXIT_STATUS(%d)RETURN_VALUE(%d)\n",
		 status->exit_status, status->return_value);
	break;
      }
    }

  return 0;
}

static int
tgdb_types_breakpoint_free (void *data)
{
  struct tgdb_breakpoint *tb;
  tb = (struct tgdb_breakpoint *) data;

  /* Free the structure */
  free ((char *) tb->file);
  tb->file = NULL;
  free ((char *) tb->funcname);
  tb->funcname = NULL;
  free (tb);
  tb = NULL;
  return 0;
}

static int
tgdb_types_source_files_free (void *data)
{
  char *s = (char *) data;
  free (s);
  s = NULL;
  return 0;
}

static int
tgdb_types_delete_item (void *command)
{
  struct tgdb_response *com = (struct tgdb_response *) command;

  if (!com)
    return -1;

  switch (com->header)
    {
    case TGDB_UPDATE_BREAKPOINTS:
      {
	struct tgdb_list *list = com->choice.update_breakpoints.breakpoint_list;

	tgdb_list_free (list, tgdb_types_breakpoint_free);
	break;
      }
    case TGDB_UPDATE_FILE_POSITION:
      {
	struct tgdb_file_position *tfp = com->choice.update_file_position.file_position;

	free (tfp->absolute_path), tfp->absolute_path = NULL;
	free (tfp->relative_path), tfp->relative_path = NULL;

	free (tfp);
	tfp = NULL;
	break;
      }
    case TGDB_UPDATE_SOURCE_FILES:
      {
	struct tgdb_list *list = com->choice.update_source_files.source_files;
	tgdb_list_free (list, tgdb_types_source_files_free);
	break;
      }
    case TGDB_SOURCES_DENIED:
      /* Nothing to do */
      break;
    case TGDB_FILENAME_PAIR:
      {
	free ((char*)com->choice.filename_pair.absolute_path);
	com->choice.filename_pair.absolute_path = NULL;
	free ((char*)com->choice.filename_pair.relative_path);
	com->choice.filename_pair.relative_path = NULL;
	break;
      }
    case TGDB_ABSOLUTE_SOURCE_DENIED:
      {
	struct tgdb_source_file *file = com->choice.absolute_source_denied.source_file;
	free (file->absolute_path);
	file->absolute_path = NULL;
	free (file);
	file = NULL;
	break;
      }
    case TGDB_INFERIOR_EXITED:
      {
	int *status = com->choice.inferior_exited.exit_status;
	free (status);
	status = NULL;
      }
      break;
    case TGDB_UPDATE_COMPLETIONS:
      {
	struct tgdb_list *list = com->choice.update_completions.completion_list;
	tgdb_list_free (list, tgdb_types_source_files_free);
	break;
      }
    case TGDB_UPDATE_CONSOLE_PROMPT_VALUE: 
      {
	const char *value = com->choice.update_console_prompt_value.prompt_value;
	free ((char*)value);
	value = NULL;
	break;
      }
    case TGDB_QUIT:
      {
	struct tgdb_debugger_exit_status *status = com->choice.quit.exit_status;
	free (status);
	status = NULL;
	break;
      }
    }

  free (com);
  com = NULL;
  return 0;
}

int
tgdb_types_print_command (void *command)
{
  return tgdb_types_print_item ((void *) command);
}

int
tgdb_types_free_command (void *command)
{
  return tgdb_types_delete_item ((void *) command);
}

void
tgdb_types_append_command (struct tgdb_list *command_list,
			   struct tgdb_response *response)
{
  tgdb_list_append (command_list, response);
}
