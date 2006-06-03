#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "gdbmi_oc.h"

#if 0
static const char * const async_reason_string_lookup[] =
{
  "breakpoint-hit",
  "watchpoint-trigger",
  "read-watchpoint-trigger",
  "access-watchpoint-trigger",
  "function-finished",
  "location-reached",
  "watchpoint-scope",
  "end-stepping-range",
  "exited-signalled",
  "exited",
  "exited-normally",
  "signal-received",
  NULL
};

/* Represents the reason why GDB is sending an asynchronous command to the
   front end.  */
enum async_reply_reason
{
  EXEC_ASYNC_BREAKPOINT_HIT = 0,
  EXEC_ASYNC_WATCHPOINT_TRIGGER,
  EXEC_ASYNC_READ_WATCHPOINT_TRIGGER,
  EXEC_ASYNC_ACCESS_WATCHPOINT_TRIGGER,
  EXEC_ASYNC_FUNCTION_FINISHED,
  EXEC_ASYNC_LOCATION_REACHED,
  EXEC_ASYNC_WATCHPOINT_SCOPE,
  EXEC_ASYNC_END_STEPPING_RANGE,
  EXEC_ASYNC_EXITED_SIGNALLED,
  EXEC_ASYNC_EXITED,
  EXEC_ASYNC_EXITED_NORMALLY,
  EXEC_ASYNC_SIGNAL_RECEIVED,
  /* This is here only to represent the number of enums.  */
  EXEC_ASYNC_LAST
};

static const char *
async_reason_lookup (enum async_reply_reason reason)
{
  return async_reason_string_lookup[reason];
}
#endif

struct gdbmi_input_command_hash
{
  const char * const cstring_val;
  enum gdbmi_input_command enum_val;
} gdbmi_input_lookup[] = {
  {"-file-list-exec-source-file", GDBMI_FILE_LIST_EXEC_SOURCE_FILE},
  {"-file-list-exec-source-files", GDBMI_FILE_LIST_EXEC_SOURCE_FILES},
  {"-break-list", GDBMI_BREAK_LIST},
  {NULL, GDBMI_LAST}
};

static enum gdbmi_input_command
gdbmi_input_command_lookup (const char *command)
{
  int i;
  for (i = 0; gdbmi_input_lookup[i].cstring_val; ++i)
    if (strcmp (command, gdbmi_input_lookup[i].cstring_val) == 0)
      return gdbmi_input_lookup[i].enum_val;

  return GDBMI_LAST;
}

/* Creating, Destroying and printing gdbmi_oc  */
gdbmi_oc_ptr
create_gdbmi_oc (void)
{
  gdbmi_oc_ptr oc_ptr = calloc (1, sizeof (struct gdbmi_oc));
  if (!oc_ptr)
    return NULL;

  oc_ptr->input_command = GDBMI_LAST;

  return oc_ptr;
}

int
destroy_gdbmi_oc (gdbmi_oc_ptr param)
{
  if (!param)
    return 0;

  if (destroy_gdbmi_cstring_ll (param->console_output) == -1)
    return -1;
  param->console_output = NULL;

  switch (param->input_command)
  {
    case GDBMI_FILE_LIST_EXEC_SOURCE_FILE:
      free (param->input_commands.file_list_exec_source_file.file);
      param->input_commands.file_list_exec_source_file.file = NULL;
      free (param->input_commands.file_list_exec_source_file.fullname);
      param->input_commands.file_list_exec_source_file.fullname = NULL;
      break;
    case GDBMI_FILE_LIST_EXEC_SOURCE_FILES:
      if (destroy_gdbmi_file_path_info (param->input_commands.file_list_exec_source_files.file_name_pair) == -1)
        return -1;
      break;
    case GDBMI_BREAK_LIST:
      if (destroy_gdbmi_breakpoint (param->input_commands.break_list.breakpoint_ptr) == -1)
        return -1;
      break;
    case GDBMI_LAST:
      break;
  };

  if (destroy_gdbmi_oc (param->next) == -1)
    return -1;
  param->next = NULL;

  free (param);
  param = NULL;

  return 0;
}

gdbmi_oc_ptr
append_gdbmi_oc (gdbmi_oc_ptr list, gdbmi_oc_ptr item)
{
  if (!item)
    return NULL;

  if (!list)
    list = item;
  else
    {
      gdbmi_oc_ptr cur = list;
      while (cur->next)
	cur = cur->next;

      cur->next = item;
    }

  return list;
}

int
print_gdbmi_oc (gdbmi_oc_ptr param)
{
  gdbmi_oc_ptr cur = param;

  while (cur)
    {
      if (cur->is_asynchronous)
	printf ("asynchronous\n");
      else
	printf ("synchronous\n");

      if (print_gdbmi_cstring_ll (cur->console_output) == -1)
	return -1;

      switch (cur->input_command)
      {
	case GDBMI_FILE_LIST_EXEC_SOURCE_FILE:
	  printf ("file-list-exec-source-file\n");
	  printf ("line=%d\n", cur->input_commands.file_list_exec_source_file.line);
	  printf ("file=%s\n", cur->input_commands.file_list_exec_source_file.file);
	  printf ("fullname=%s\n", cur->input_commands.file_list_exec_source_file.fullname);
	  break;
	case GDBMI_FILE_LIST_EXEC_SOURCE_FILES:
	  {
	    gdbmi_oc_file_path_info_ptr file_ptr;
	    printf ("file-list-exec-source-files\n");
	    file_ptr = 
	      cur->input_commands.file_list_exec_source_files.file_name_pair;
	    if (print_gdbmi_file_path_info (file_ptr) == -1)
	      {
		fprintf (stderr, "%s:%d\n", __FILE__, __LINE__);
		return -1;
	      }
	  }
	  break;
	case GDBMI_BREAK_LIST:
	  {
	    gdbmi_oc_breakpoint_ptr breakpoint_ptr;
	    printf ("break-list\n");
	    breakpoint_ptr = cur->input_commands.break_list.breakpoint_ptr;
	    if (print_gdbmi_breakpoint (breakpoint_ptr) == -1)
	      {
		fprintf (stderr, "%s:%d\n", __FILE__, __LINE__);
		return -1;
	      }
	  }
	case GDBMI_LAST:
	  break;
      };

      cur = cur->next;
    }

  return 0;
}

/**
 * This will convert a cstring that was passed from GDB to the front end.
 * A typically cstring will look like,
 *   "Type \"show copying\" to see the conditions.\n"
 * The \" should be converted to a ", and the \n should be converted to a 
 * newline. Also, the first and last " should go away. The other cases are 
 * handled in the code bleow.
 *
 * \param orig
 * The string to convert
 *
 * \param new
 * The new string, the memory will be allocated in this function and passed back.
 *
 * \return
 * 0 on success, -1 on error.
 */
static int
convert_cstring (const char *orig, char **new)
{
  int length;
  char *nstring;
  int i, cur;

  if (!orig || !new)
    return -1;
 
  length = strlen (orig);
  nstring = malloc (sizeof (char)*(length+1));
  cur = 0;

  /* Loop from 1 to length -1 to skip the first and last char, 
   * they are the " chars. */
  for (i = 1; i < length-1; ++i)
  {
    if (orig[i] == '\\')
    {
      i++;
      switch (orig[i])
      {
	case 'n':
	  nstring[cur++] = '\r';
	  nstring[cur++] = '\n';
	  break;
	case '"':
	  nstring[cur++] = '"';
	  break;
	case 't':
	  nstring[cur++] = '\t';
	  break;
	case '\\':
	  nstring[cur++] = '\\';
	  break;
	default:
          fprintf (stderr, "%s:%d char(%d)\n", __FILE__, __LINE__, orig[i]);
	  return -1;
      };
    }
    else
      nstring[cur++] = orig[i];
  }
  nstring[cur] = '\0';

  *new = nstring;
   
  return 0;
}

/**
 * This will take in a single MI output command parse tree and return a 
 * single MI output commands data structure.
 *
 * \param output_ptr
 * The MI parse tree
 *
 * \param oc_ptr
 * On return, this will be the MI output command that were derived from the 
 * MI output command parse tree.
 *
 * \return
 * 0 on success, -1 on error.
 */
static int
gdbmi_get_output_command (gdbmi_output_ptr output_ptr, gdbmi_oc_ptr *oc_ptr)
{
  if (!output_ptr || !oc_ptr)
    return -1;

  *oc_ptr = create_gdbmi_oc ();
  if (!(*oc_ptr))
    {
      fprintf (stderr, "%s:%d\n", __FILE__, __LINE__);
      return -1;
    }

  /* Check to see if the output command is synchronous or asynchronous */
  if (!output_ptr->result_record)
    (*oc_ptr)->is_asynchronous = 1;

  /* Walk the output_ptr to get the MI stream record's */
  if (output_ptr->oob_record)
  {
    gdbmi_oob_record_ptr cur = output_ptr->oob_record;
    while (cur)
      {
	if (cur->record == GDBMI_STREAM)
	  {
	    if (cur->option.stream_record->stream_record == GDBMI_CONSOLE)
	      {
		char *orig = cur->option.stream_record->cstring;
		gdbmi_oc_cstring_ll_ptr ncstring = create_gdbmi_cstring_ll ();
		if (convert_cstring (orig, &(ncstring->cstring)) == -1)
		    {
		      fprintf (stderr, "%s:%d\n", __FILE__, __LINE__);
		      return -1;
		    }

		(*oc_ptr)->console_output = append_gdbmi_cstring_ll (
		  (*oc_ptr)->console_output, ncstring);
	      }
	  }
	cur = cur->next;
      }

  }

  return 0;
}

static int
gdbmi_get_specific_output_command (gdbmi_output_ptr output_ptr, 
				   gdbmi_oc_ptr oc_ptr,
				   gdbmi_oc_cstring_ll_ptr mi_input_cmds)
{
  /* If the command is synchronous, then it is a response to an MI input command. */
  char *mi_input_cmd;
  enum gdbmi_input_command mi_input_cmd_kind;

  if (!mi_input_cmds)
    {
      fprintf (stderr, "%s:%d\n", __FILE__, __LINE__);
      return -1;
    }

  mi_input_cmd = mi_input_cmds->cstring;
  mi_input_cmd_kind = gdbmi_input_command_lookup (mi_input_cmd);

  oc_ptr->input_command = mi_input_cmd_kind;
  switch (mi_input_cmd_kind)
  {
    case GDBMI_FILE_LIST_EXEC_SOURCE_FILE:
      {
	gdbmi_result_ptr result_ptr = output_ptr->result_record->result;
	while (result_ptr)
	  {
	    if (strcmp (result_ptr->variable, "line") == 0)
	    {
	      char *nline;
	      if (convert_cstring (result_ptr->value->option.cstring, &nline) == -1)
	        {
		  fprintf (stderr, "%s:%d\n", __FILE__, __LINE__);
		  return -1;
	        }

	      oc_ptr->input_commands.file_list_exec_source_file.line = atoi (nline);
	      free (nline);
	      nline = NULL;
	    }
	    else if (strcmp (result_ptr->variable, "file") == 0)
	    {
	      char *nline;
	      if (convert_cstring (result_ptr->value->option.cstring, &nline) == -1)
	        {
		  fprintf (stderr, "%s:%d\n", __FILE__, __LINE__);
		  return -1;
	        }

	      oc_ptr->input_commands.file_list_exec_source_file.file = nline;
	    }
	    else if (strcmp (result_ptr->variable, "fullname") == 0)
	    {
	      char *nline;
	      if (convert_cstring (result_ptr->value->option.cstring, &nline) == -1)
	        {
		  fprintf (stderr, "%s:%d\n", __FILE__, __LINE__);
		  return -1;
	        }
	      oc_ptr->input_commands.file_list_exec_source_file.fullname = nline;
	    }

	    result_ptr = result_ptr->next;
	  }
      }
      break;
    case GDBMI_FILE_LIST_EXEC_SOURCE_FILES:
      {
	gdbmi_result_ptr result_ptr = output_ptr->result_record->result;
	while (result_ptr)
	  {
	    if (strcmp (result_ptr->variable, "files") == 0)
	    {
	      if (result_ptr->value->value_choice == GDBMI_LIST)
	      {
		gdbmi_list_ptr list = result_ptr->value->option.list;
		while (list)
		{
		  if (list->list_choice == GDBMI_VALUE)
		  {
		    gdbmi_value_ptr value_ptr = list->option.value;
		    while (value_ptr)
		    {
		      if (value_ptr->value_choice == GDBMI_TUPLE)
		      {
			gdbmi_oc_file_path_info_ptr ptr = create_gdbmi_file_path_info ();
			gdbmi_result_ptr result = value_ptr->option.tuple->result;
			while (result)
			{
			  if (strcmp (result->variable, "file") == 0)
			  {
			    if (convert_cstring (result->value->option.cstring, &(ptr->file)) == -1)
			      {
				fprintf (stderr, "%s:%d\n", __FILE__, __LINE__);
				return -1;
			      }
			  } 
			  else if (strcmp (result->variable, "fullname") == 0)
			  {
			    if (convert_cstring (result->value->option.cstring, &(ptr->fullname)) == -1)
			      {
				fprintf (stderr, "%s:%d\n", __FILE__, __LINE__);
				return -1;
			      }
			  }
			  result = result->next;
			}

			oc_ptr->input_commands.file_list_exec_source_files.file_name_pair =
			   append_gdbmi_file_path_info (oc_ptr->input_commands.file_list_exec_source_files.file_name_pair, ptr);
		      }
		      value_ptr = value_ptr->next;
		    }
		  }

		  list = list->next;
		}
	      }
	    }

	    result_ptr = result_ptr->next;
	  }
      }
      break;
    case GDBMI_BREAK_LIST:
      {
	gdbmi_result_ptr result_ptr = output_ptr->result_record->result;
	if (strcmp (result_ptr->variable, "BreakpointTable") == 0)
	{
	  if (result_ptr->value->value_choice == GDBMI_TUPLE)
	  {
	    result_ptr = result_ptr->value->option.tuple->result;
	    while (result_ptr)
	    {
	      if (strcmp (result_ptr->variable, "body") == 0)
	      {
		if (result_ptr->value->value_choice == GDBMI_LIST)
		{
		  gdbmi_list_ptr list_ptr = result_ptr->value->option.list;
		  if (list_ptr && list_ptr->list_choice == GDBMI_RESULT)
		  {
		    gdbmi_result_ptr result_ptr = list_ptr->option.result;
		    while (result_ptr)
		    {
		      if (strcmp (result_ptr->variable, "bkpt") == 0)
		      {
			gdbmi_oc_breakpoint_ptr ptr = create_gdbmi_breakpoint ();

			gdbmi_value_ptr value_ptr = result_ptr->value;
			if (value_ptr->value_choice == GDBMI_TUPLE)
			{
			  gdbmi_result_ptr result_ptr = value_ptr->option.tuple->result;
			  while (result_ptr)
			  {
			    if (strcmp (result_ptr->variable, "number") == 0)
			    {
			      if (result_ptr->value->value_choice == GDBMI_CSTRING)
			      {
				char *nstr;
				if (convert_cstring (result_ptr->value->option.cstring, &nstr) == -1)
				{
				  fprintf (stderr, "%s:%d\n", __FILE__, __LINE__);
				  return -1;
				}

				ptr->number = atoi (nstr);
				free (nstr);
				nstr = NULL;
			      }
			    } 
			    else if (strcmp (result_ptr->variable, "type") == 0)
			    {
			      if (result_ptr->value->value_choice == GDBMI_CSTRING)
			      {
				if (strcmp (result_ptr->value->option.cstring, "\"breakpoint\"") == 0)
				  ptr->type = GDBMI_BREAKPOINT;
				else if (strcmp (result_ptr->value->option.cstring, "\"watchpoint\"") == 0)
				  ptr->type = GDBMI_WATCHPOINT;
			      }
			    }
			    else if (strcmp (result_ptr->variable, "disp") == 0)
			    {
			      if (result_ptr->value->value_choice == GDBMI_CSTRING)
			      {
				if (strcmp (result_ptr->value->option.cstring, "\"keep\"") == 0)
				  ptr->disposition = GDBMI_KEEP;
				else if (strcmp (result_ptr->value->option.cstring, "\"nokeep\"") == 0)
				  ptr->disposition = GDBMI_NOKEEP;
			      }
			    }
			    else if (strcmp (result_ptr->variable, "enabled") == 0)
			    {
			      if (result_ptr->value->value_choice == GDBMI_CSTRING)
			      {
				if (strcmp (result_ptr->value->option.cstring, "\"y\"") == 0)
				  ptr->enabled = 1;
				else
				  ptr->enabled = 0;
			      }
			    }
			    else if (strcmp (result_ptr->variable, "addr") == 0)
			    {
			      if (result_ptr->value->value_choice == GDBMI_CSTRING)
			      {
				if (convert_cstring (result_ptr->value->option.cstring, &ptr->address) == -1)
				{
				  fprintf (stderr, "%s:%d\n", __FILE__, __LINE__);
				  return -1;
				}
			      }
			    }
			    else if (strcmp (result_ptr->variable, "func") == 0)
			    {
			      if (result_ptr->value->value_choice == GDBMI_CSTRING)
			      {
				if (convert_cstring (result_ptr->value->option.cstring, &ptr->func) == -1)
				{
				  fprintf (stderr, "%s:%d\n", __FILE__, __LINE__);
				  return -1;
				}
			      }
			    }
			    else if (strcmp (result_ptr->variable, "file") == 0)
			    {
			      if (result_ptr->value->value_choice == GDBMI_CSTRING)
			      {
				if (convert_cstring (result_ptr->value->option.cstring, &ptr->file) == -1)
				{
				  fprintf (stderr, "%s:%d\n", __FILE__, __LINE__);
				  return -1;
				}
			      }
			    }
			    else if (strcmp (result_ptr->variable, "fullname") == 0)
			    {
			      if (result_ptr->value->value_choice == GDBMI_CSTRING)
			      {
				if (convert_cstring (result_ptr->value->option.cstring, &ptr->fullname) == -1)
				{
				  fprintf (stderr, "%s:%d\n", __FILE__, __LINE__);
				  return -1;
				}
			      }
			    }
			    else if (strcmp (result_ptr->variable, "line") == 0)
			    {
			      if (result_ptr->value->value_choice == GDBMI_CSTRING)
			      {
				char *nstr;
				if (convert_cstring (result_ptr->value->option.cstring, &nstr) == -1)
				{
				  fprintf (stderr, "%s:%d\n", __FILE__, __LINE__);
				  return -1;
				}

				ptr->line = atoi (nstr);
				free (nstr);
				nstr = NULL;
			      }
			    } 
			    else if (strcmp (result_ptr->variable, "times") == 0)
			    {
			      if (result_ptr->value->value_choice == GDBMI_CSTRING)
			      {
				char *nstr;
				if (convert_cstring (result_ptr->value->option.cstring, &nstr) == -1)
				{
				  fprintf (stderr, "%s:%d\n", __FILE__, __LINE__);
				  return -1;
				}

				ptr->times = atoi (nstr);
				free (nstr);
				nstr = NULL;
			      }
			    } 

			    result_ptr = result_ptr->next;
			  }
			}

			oc_ptr->input_commands.break_list.breakpoint_ptr =
			   append_gdbmi_breakpoint (oc_ptr->input_commands.break_list.breakpoint_ptr, ptr);
		      }
		      result_ptr = result_ptr->next;
		    }
		  }
		}
	      }
	      result_ptr = result_ptr->next;
	    }
	  }
	}
      }
      break;
    case GDBMI_LAST:
      fprintf (stderr, "%s:%d\n", __FILE__, __LINE__);
      return -1;
  };

  return 0;
}

/**
 * There is a list of MI output commands, and a list of MI input commands.
 * For each MI input command, I need to find the corresponding output command. If there
 * are asynchronous output commands, they need to be detected and not counted as a response
 * to the input command.
 */
int
gdbmi_get_output_commands (gdbmi_output_ptr output_ptr, gdbmi_oc_cstring_ll_ptr mi_input_cmds,
			   gdbmi_oc_ptr *oc_ptr)
{
  gdbmi_output_ptr cur = output_ptr;
  gdbmi_oc_cstring_ll_ptr cur_mi_input_cmds = mi_input_cmds;
  int result;

  if (!output_ptr || !oc_ptr)
    return -1;

  *oc_ptr = NULL;

  while (cur)
    {
      gdbmi_oc_ptr cur_oc_ptr = NULL;
      result = gdbmi_get_output_command (cur, &cur_oc_ptr);
      if (result == -1)
        {
	  fprintf (stderr, "%s:%d\n", __FILE__, __LINE__);
	  return -1;
	}
      *oc_ptr = append_gdbmi_oc (*oc_ptr, cur_oc_ptr);

      /* If it is not asynchronous, then need to get the specific results */
      if (!cur_oc_ptr->is_asynchronous)
        {
	  result = gdbmi_get_specific_output_command (cur, cur_oc_ptr, cur_mi_input_cmds);
	  if (result == -1)
	    {
	      fprintf (stderr, "%s:%d\n", __FILE__, __LINE__);
	      return -1;
	    }

	  cur_mi_input_cmds = cur_mi_input_cmds->next;
	}

      cur = cur->next;
    }

  if (cur_mi_input_cmds != NULL)
    {
      fprintf (stderr, "%s:%d\n", __FILE__, __LINE__);
      return -1;
    }

  return 0;
}

gdbmi_oc_cstring_ll_ptr 
create_gdbmi_cstring_ll (void)
{
  return calloc (1, sizeof (struct gdbmi_oc_cstring_ll));
}

int 
destroy_gdbmi_cstring_ll (gdbmi_oc_cstring_ll_ptr param)
{
  if (!param)
    return 0;

  free (param->cstring);
  param->cstring = NULL;

  if (destroy_gdbmi_cstring_ll (param->next) == -1)
    return -1;
  param->next = NULL;

  free (param);

  return 0;
}

gdbmi_oc_cstring_ll_ptr 
append_gdbmi_cstring_ll (gdbmi_oc_cstring_ll_ptr list,
			 gdbmi_oc_cstring_ll_ptr item)
{
  if (!item)
    return NULL;

  if (!list)
    list = item;
  else
    {
      gdbmi_oc_cstring_ll_ptr cur = list;
      while (cur->next)
	cur = cur->next;

      cur->next = item;
    }

  return list;
}

int 
print_gdbmi_cstring_ll (gdbmi_oc_cstring_ll_ptr param)
{
  gdbmi_oc_cstring_ll_ptr cur = param;

  while (cur)
    {
      printf ("cstring->(%s)\n", cur->cstring);
      cur = cur->next;
    }

  return 0;
}

gdbmi_oc_file_path_info_ptr 
create_gdbmi_file_path_info (void)
{
  return calloc (1, sizeof (struct gdbmi_oc_file_path_info));
}


int 
destroy_gdbmi_file_path_info (gdbmi_oc_file_path_info_ptr param)
{
  if (!param)
    return 0;

  free (param->file);
  param->file = NULL;

  free (param->fullname);
  param->fullname = NULL;

  if (destroy_gdbmi_file_path_info (param->next) == -1)
    return -1;
  param->next = NULL;

  free (param);

  return 0;
}

gdbmi_oc_file_path_info_ptr 
append_gdbmi_file_path_info (gdbmi_oc_file_path_info_ptr list, gdbmi_oc_file_path_info_ptr item)
{
  if (!item)
    return NULL;

  if (!list)
    list = item;
  else
    {
      gdbmi_oc_file_path_info_ptr cur = list;
      while (cur->next)
	cur = cur->next;

      cur->next = item;
    }

  return list;
}

int 
print_gdbmi_file_path_info (gdbmi_oc_file_path_info_ptr param)
{
  gdbmi_oc_file_path_info_ptr cur = param;

  while (cur)
    {
      printf ("file->(%s)\n", cur->file);
      printf ("fullname->(%s)\n", cur->fullname);
      cur = cur->next;
    }

  return 0;
}

gdbmi_oc_breakpoint_ptr 
create_gdbmi_breakpoint (void)
{
  return calloc (1, sizeof (struct gdbmi_oc_breakpoint));
}

int 
destroy_gdbmi_breakpoint (gdbmi_oc_breakpoint_ptr param)
{
  if (!param)
    return 0;

  if (param->address)
  {
    free (param->address);
    param->address = NULL;
  }

  if (param->func)
  {
    free (param->func);
    param->func = NULL;
  }

  if (param->file)
  {
    free (param->file);
    param->file = NULL;
  }
  
  if (param->fullname)
  {
    free (param->fullname);
    param->fullname = NULL;
  }

  if (destroy_gdbmi_breakpoint (param->next) == -1)
    return -1;
  param->next = NULL;

  free (param);
  param = NULL;

  return 0;
}

gdbmi_oc_breakpoint_ptr 
append_gdbmi_breakpoint (gdbmi_oc_breakpoint_ptr list, gdbmi_oc_breakpoint_ptr item)
{
  if (!item)
    return NULL;

  if (!list)
    list = item;
  else
    {
      gdbmi_oc_breakpoint_ptr cur = list;
      while (cur->next)
	cur = cur->next;

      cur->next = item;
    }

  return list;
}

int 
print_gdbmi_breakpoint (gdbmi_oc_breakpoint_ptr param)
{
  gdbmi_oc_breakpoint_ptr cur = param;

  while (cur)
    {
      printf ("number=%d\n", cur->number);

      switch (cur->type)
      {
	case GDBMI_BREAKPOINT:
	  printf ("GDBMI_BREAKPOINT\n");
	  break;
	case GDBMI_WATCHPOINT:
	  printf ("GDBMI_WATCHPOINT\n");
	  break;
	default:
	  return -1;
      };

      switch (cur->disposition)
      {
	case GDBMI_KEEP:
	  printf ("GDBMI_KEEP\n");
	  break;
	case GDBMI_NOKEEP:
	  printf ("GDBMI_NOKEEP\n");
	  break;
	default:
	  return -1;
      };

      printf ("enabled=%d\n", cur->enabled);
      printf ("address->(%s)\n", cur->address);
      printf ("func->(%s)\n", cur->func);
      printf ("file->(%s)\n", cur->file);
      printf ("fullname->(%s)\n", cur->fullname);
      printf ("line=%d\n", cur->line);
      printf ("times=%d\n", cur->times);

      cur = cur->next;
    }

  return 0;
}
