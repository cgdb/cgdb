#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gdbmi_pt.h"
#include "gdbmi_oc.h"
#include "gdbmi_parser.h"

static void
usage (char *progname)
{

  printf ("%s mi.inputfile mi.outputfile\n", progname);
  exit (-1);
}

int
main (int argc, char **argv)
{
  gdbmi_parser_ptr parser_ptr;
  gdbmi_output_ptr output_ptr;
  gdbmi_oc_ptr oc_ptr;
  int result, parse_failed;
  gdbmi_oc_cstring_ll_ptr mi_input_cmds = NULL;

  if (argc != 3)
    usage (argv[0]);

  /* Parse the input file, 1 command per line. */
  {
    FILE *fd = fopen (argv[1], "r");
    int val = 1;
    if (!fd)
      {
	fprintf (stderr, "%s:%d\n", __FILE__, __LINE__);
	return -1;
      }

    while (!feof (fd) || val == 0)
      {
	gdbmi_oc_cstring_ll_ptr cur;
	char line[4096];
	if (fgets (line, 4096, fd)==NULL)
	  break;
	cur = create_gdbmi_cstring_ll ();
	line[strlen(line)-1] = '\0';
	cur->cstring = strdup (line);
	mi_input_cmds = append_gdbmi_cstring_ll (mi_input_cmds, cur);
      }

    fclose (fd);
  }


  parser_ptr = gdbmi_parser_create ();

  result = gdbmi_parser_parse_file (parser_ptr,
				    argv[2], &output_ptr, &parse_failed);

  if (result == -1)
    {
      fprintf (stderr, "%s:%d\n", __FILE__, __LINE__);
      return -1;
    }

  if (parse_failed)
    {
      output_ptr = NULL;
      if (result == -1)
	{
	  fprintf (stderr, "%s:%d\n", __FILE__, __LINE__);
	  return -1;
	}
    }
  else
    {
      /* Print the parse tree for testing purposes. */
      print_gdbmi_output (output_ptr);

      /* Turn the parse tree into an MI output command. */
      result = gdbmi_get_output_commands (output_ptr, mi_input_cmds, &oc_ptr);
      if (result == -1)
        {
	  fprintf (stderr, "%s:%d\n", __FILE__, __LINE__);
	  return -1;
	}
      else
        {
	  if (print_gdbmi_oc (oc_ptr) == -1)
	    {
	      fprintf (stderr, "%s:%d\n", __FILE__, __LINE__);
	      return -1;
	    }

	  if (destroy_gdbmi_oc (oc_ptr) == -1)
	    {
	      fprintf (stderr, "%s:%d\n", __FILE__, __LINE__);
	      return -1;
	    }
	}

      if (destroy_gdbmi_output (output_ptr) == -1)
	{
	  fprintf (stderr, "%s:%d\n", __FILE__, __LINE__);
	  return -1;
	}
    }

  /* Free the parser */
  if (gdbmi_parser_destroy (parser_ptr) == -1)
    {
      fprintf (stderr, "%s:%d\n", __FILE__, __LINE__);
      return -1;
    }

  if (destroy_gdbmi_cstring_ll (mi_input_cmds) == -1)
    {
      fprintf (stderr, "%s:%d\n", __FILE__, __LINE__);
      return -1;
    }

  return 0;
}
