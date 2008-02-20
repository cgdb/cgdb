#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "gdbmi_grammar.h"
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
  gdbmi_pstate *mips;
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

  /* Create a new parser instance */
  parser->mips = gdbmi_pstate_new ();
  if (!parser) {
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

  if (parser->mips) {
    /* Free the parser instance */
    gdbmi_pstate_delete (parser->mips);
    parser->mips = NULL;
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
  int pattern;
  int mi_status;
  int finished = 0;

  if (!parser)
    return -1;

  if (!mi_command)
    return -1;

  if (!parse_failed)
    return -1;

  /* Initialize output parameters */
  *pt = 0;
  *parse_failed = 0;

  /* Create a new input buffer for flex. */
  state = gdbmi__scan_string (strdup (mi_command));

  /* Create a new input buffer for flex and
   * iterate over all the tokens. */
  do {
    pattern = gdbmi_lex ();
    if (pattern == 0)
      break;
    mi_status = gdbmi_push_parse (parser->mips, pattern, NULL, &finished);
  } while (mi_status == YYPUSH_MORE);

  /* Parser is done, this should never happen */
  if (mi_status != YYPUSH_MORE && mi_status != 0) {
    *parse_failed = 1;
  } else if (finished) {
      *pt = tree;
      tree = NULL;
  }

  /* Free the scanners buffer */
  gdbmi__delete_buffer (state);

  return 0;
}

int
gdbmi_parser_parse_file (gdbmi_parser_ptr parser,
			 const char *mi_command_file,
			 gdbmi_output_ptr *pt, int *parse_failed)
{
  int pattern;
  int mi_status;
  int found_one = 0;

  if (!parser)
    return -1;

  if (!mi_command_file)
    return -1;

  if (!parse_failed)
    return -1;

  *pt = 0;
  *parse_failed = 0;

  /* Initialize data */
  gdbmi_in = fopen (mi_command_file, "r");

  if (!gdbmi_in)
    {
      fprintf (stderr, "%s:%d", __FILE__, __LINE__);
      return -1;
    }

  /* Create a new input buffer for flex and
   * iterate over all the tokens. */
  do {
    pattern = gdbmi_lex ();
    if (pattern == 0)
      break;
    mi_status = gdbmi_push_parse (parser->mips, pattern, NULL, &found_one);
  } while (mi_status == YYPUSH_MORE);

  /* Parser is done, this should never happen */
  if (mi_status != YYPUSH_MORE && mi_status != 0) {
    *parse_failed = 1;
  } else {
      *pt = tree;
  }

  fclose (gdbmi_in);

  return 0;
}
