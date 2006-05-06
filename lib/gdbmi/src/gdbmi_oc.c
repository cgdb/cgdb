#include <stdlib.h>
#include <stdio.h>
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

struct gdbmi_oc
{
  /* If this is 1, then the command was asynchronous, otherwise it wasn't */
  int is_asynchronous;

  /* The gdbmi_stream_record's, in the order they appeared in the output. */
  gdbmi_stream_record_ptr stream_record_ptr;

  /* The next MI output command */
  gdbmi_oc_ptr next;
};

/* Creating, Destroying and printing gdbmi_oc  */
gdbmi_oc_ptr
create_gdbmi_oc (void)
{
  return calloc (1, sizeof (struct gdbmi_oc));
}

int
destroy_gdbmi_oc (gdbmi_oc_ptr param)
{
  if (!param)
    return 0;

  if (destroy_gdbmi_stream_record (param->stream_record_ptr) == -1)
    return -1;

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

      if (print_gdbmi_stream_record (cur->stream_record_ptr) == -1)
	return -1;

      cur = cur->next;
    }

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
    return -1;

  /* Check to see if the output command is synchronous or asynchronous */
  if (output_ptr->oob_record)
  {
    gdbmi_oob_record_ptr cur = output_ptr->oob_record;
    while (cur)
      {
	if (cur->record == GDBMI_ASYNC)
	  (*oc_ptr)->is_asynchronous = 1;
	cur = cur->next;
      }
  }

  /* Walk the output_ptr to get the MI stream record's */
  if (output_ptr->oob_record)
  {
    gdbmi_oob_record_ptr cur = output_ptr->oob_record;
    while (cur)
      {
	if (cur->record == GDBMI_STREAM)
	  {
	    gdbmi_stream_record_ptr nptr = dup_gdbmi_stream_record (cur->option.stream_record);
	    (*oc_ptr)->stream_record_ptr = append_gdbmi_stream_record ((*oc_ptr)->stream_record_ptr, 
								       nptr);
	  }
	cur = cur->next;
      }

  }

  if (print_gdbmi_oc (*oc_ptr) == -1)
    return -1;

  return 0;
}

int
gdbmi_get_output_commands (gdbmi_output_ptr output_ptr, gdbmi_oc_ptr *oc_ptr)
{
  gdbmi_output_ptr cur = output_ptr;
  gdbmi_oc_ptr cur_oc_ptr = NULL;
  int result;

  if (!output_ptr)
    return -1;


  while (cur)
    {
      result = gdbmi_get_output_command (cur, &cur_oc_ptr);
      if (result == -1)
	return -1;

      cur = cur->next;
    }

  return 0;
}
