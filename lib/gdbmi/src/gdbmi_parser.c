#include <stdlib.h>
#include <stdio.h>

#include "gdbmi_parser.h"

int gdbmi_parse (void);

/* flex */
typedef struct yy_buffer_state *YY_BUFFER_STATE;
extern YY_BUFFER_STATE gdbmi__scan_string (const char *yy_str);
extern void gdbmi__delete_buffer (YY_BUFFER_STATE state);
extern FILE *gdbmi_in;
extern int gdbmi_lex (void);
extern char *gdbmi_text;
extern int gdbmi_lineno;
extern gdbmi_output_ptr tree;

struct gdbmi_parser
{
  char *last_error;
};

gdbmi_parser_ptr
gdbmi_parser_create (void)
{
  gdbmi_parser_ptr parser;

  parser = (gdbmi_parser_ptr) malloc (sizeof (struct gdbmi_parser));
  parser->last_error = NULL;

  if (!parser)
    {
      fprintf (stderr, "%s:%d", __FILE__, __LINE__);
      return NULL;
    }

  return parser;
}

int
gdbmi_parser_destroy (gdbmi_parser_ptr parser)
{

  if (!parser)
    return -1;

  if (parser->last_error)
    {
      free (parser->last_error);
      parser->last_error = NULL;
    }

  free (parser);
  parser = NULL;
  return 0;
}

int
gdbmi_parser_parse_string (gdbmi_parser_ptr parser,
			   const char *mi_command,
			   gdbmi_output_ptr * pt, int *parse_failed)
{
  YY_BUFFER_STATE state;

  if (!parser)
    return -1;

  if (!mi_command)
    return -1;

  if (!parse_failed)
    return -1;

  *parse_failed = 0;

  /* Create a new input buffer for flex. */
  state = gdbmi__scan_string ((char *) mi_command);

  /* Create a new input buffer for flex. */
  if (gdbmi_parse () == 1)
    *parse_failed = 1;
  else
    *parse_failed = 0;

  *pt = tree;

  if (*parse_failed == 0)
    *pt = tree;
  else
    {
      if (destroy_gdbmi_output (tree) == -1)
	{
	  fprintf (stderr, "%s:%d", __FILE__, __LINE__);
	  return -1;
	}
    }

  gdbmi__delete_buffer (state);

  return 0;
}

int
gdbmi_parser_parse_file (gdbmi_parser_ptr parser,
			 const char *mi_command_file,
			 gdbmi_output_ptr * pt, int *parse_failed)
{

  if (!parser)
    return -1;

  if (!mi_command_file)
    return -1;

  if (!parse_failed)
    return -1;

  *parse_failed = 0;

  /* Initialize data */
  gdbmi_in = fopen (mi_command_file, "r");

  if (!gdbmi_in)
    {
      fprintf (stderr, "%s:%d", __FILE__, __LINE__);
      return -1;
    }

  if (gdbmi_parse () == 1)
    *parse_failed = 1;
  else
    *parse_failed = 0;

  if (*parse_failed == 0)
    *pt = tree;
  else
    {
      if (destroy_gdbmi_output (tree) == -1)
	{
	  fprintf (stderr, "%s:%d", __FILE__, __LINE__);
	  return -1;
	}
    }


  fclose (gdbmi_in);

  return 0;
}
