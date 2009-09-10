#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif /* HAVE_STDLIB_H */

#include "annotate.h"
#include "sys_util.h"
#include "logger.h"
#include "data.h"
#include "commands.h"
#include "globals.h"
#include "io.h"

static int 
handle_source (struct annotate_two *a2, const char *buf, size_t n, struct tgdb_list *list)
{
  int ret = commands_parse_source (a2->c, a2->client_command_list, buf, n, list);

  /* This tells the annotate subsystem if the source annotation has been
   * reached. This could be useful to the application using TGDB to determine if a single
   * source annotation has been reached yet. If so, it doesn't have to probe TGDB for the
   * initiale source file when GDB starts up. 
   *
   * As of 12/17/2005 it is no longer being used within TGDB. I moved the responsibilty of
   * probing for the first file to the calling application. If the variable ends up being
   * useless, it can be removed.
   */
   a2->source_already_received = 1;

   return ret;
}

static int handle_misc_pre_prompt(struct annotate_two *a2, const char *buf, size_t n, struct tgdb_list *list){
   /* If tgdb is sending a command, then continue past it */
   if(data_get_state(a2->data) == INTERNAL_COMMAND){
      if(io_write_byte(a2->debugger_stdin, '\n') == -1)
         logger_write_pos ( logger, __FILE__, __LINE__, "Could not send command");
   } else {
      data_set_state(a2, AT_PROMPT);
   }

   return 0;
}

static int handle_misc_prompt(struct annotate_two *a2, const char *buf, size_t n, struct tgdb_list *list){
   globals_set_misc_prompt_command( a2->g, 1);
   data_set_state(a2, USER_AT_PROMPT );
   a2->command_finished = 1;
   return 0;
}

static int handle_misc_post_prompt(struct annotate_two *a2, const char *buf, size_t n, struct tgdb_list *list){
   globals_set_misc_prompt_command( a2->g, 0);
   data_set_state(a2, POST_PROMPT );

   return 0;
}

static int handle_pre_prompt(struct annotate_two *a2, const char *buf, size_t n, struct tgdb_list *list){
   data_set_state(a2, AT_PROMPT );
   
   return 0;
}

static int handle_prompt(struct annotate_two *a2, const char *buf, size_t n, struct tgdb_list *list){
   /* This return TGDB_ABSOLUTE_SOURCE_DENIED if there was no absolute path
	* given when 'info source' was run.
	* It tells the gui that the source is not available.
	*/
   commands_finalize_command ( a2->c, list );
   
   /* All done. */
   data_set_state(a2, USER_AT_PROMPT );

   /* 'info sources' is done, return the sources to the gui */
   if(global_has_info_sources_started(a2->g) == 1){
      global_reset_info_sources_started(a2->g);
      commands_send_gui_sources(a2->c, list);
      return 0;
   } 

   /* 'complete' is done, return the completions to the gui */
   if(global_has_completion_started(a2->g) == 1){
      global_reset_completion_started(a2->g);
      commands_send_gui_completions(a2->c, list);
      return 0;
   } 

   return 0;
}

static int handle_post_prompt(struct annotate_two *a2, const char *buf, size_t n, struct tgdb_list *list){
   data_set_state(a2, POST_PROMPT );
   return 0;
}

static int handle_breakpoints_invalid(struct annotate_two *a2, const char *buf, size_t n, struct tgdb_list *list){
  /* Don't use anymore because GDB is buggy */
  return 0;
}

static int handle_breakpoints_headers(struct annotate_two *a2, const char *buf, size_t n, struct tgdb_list *list){
   commands_set_state(a2->c, BREAKPOINT_HEADERS, list);
   return 0;
}

static int handle_breakpoints_table(struct annotate_two *a2, const char *buf, size_t n, struct tgdb_list *list){
   commands_set_state(a2->c, BREAKPOINT_TABLE_BEGIN, list);
   return 0;
}

static int handle_breakpoints_table_end(struct annotate_two *a2, const char *buf, size_t n, struct tgdb_list *list){
   commands_set_state(a2->c, BREAKPOINT_TABLE_END, list);
   return 0;
}

static int handle_unwanted_fields(struct annotate_two *a2, const char *buf, size_t n, struct tgdb_list *list){
  return 0;
}

static int handle_field(struct annotate_two *a2, const char *buf, size_t n, struct tgdb_list *list){
   int i = -1;
   
   commands_parse_field(a2->c, buf, n, &i);
   commands_set_field_num(a2->c, i);
   commands_set_state(a2->c, FIELD, list);

   return 0;
}

static int handle_record(struct annotate_two *a2, const char *buf, size_t n, struct tgdb_list *list){
   commands_set_state(a2->c, RECORD, list);
   return 0;
}

static int handle_error(struct annotate_two *a2, const char *buf, size_t n, struct tgdb_list *list){
   data_set_state(a2, POST_PROMPT );  /* TEMPORARY */
   return 0;
}

static int handle_error_begin(struct annotate_two *a2, const char *buf, size_t n, struct tgdb_list *list){
   /* If the user listed the files ( info sources ) and there is an 
    * annotate error ( usually meaning that gdb can not find the symbols
    * for the debugged program ) then send a denied response. */
   if ( commands_get_state(a2->c) == INFO_SOURCES ) {
        struct tgdb_response *response = (struct tgdb_response *)
     	  cgdb_malloc (sizeof (struct tgdb_response));
	response->header = TGDB_SOURCES_DENIED;
        tgdb_types_append_command (list, response);
        return 0;
   }

   /* if the user tried to list a file that does not exist */
   if(global_has_list_started(a2->g) == 1){
      global_list_finished(a2->g);
      global_set_list_error ( a2->g, 1 );
      commands_list_command_finished(a2->c, list, 0);
      return 0;
   }

   /* After a signal is sent (^c), the debugger will then output 
	* something like "Quit\n", so that should be displayed to the user.
	* Unfortunatly, the debugger ( gdb ) isn't nice enough to return a 
	* post-prompt when a signal is recieved.
	*/
   data_set_state(a2, VOID);

   return 0;
}

static int handle_quit(struct annotate_two *a2, const char *buf, size_t n, struct tgdb_list *list){
   data_set_state(a2, POST_PROMPT );  /* TEMPORARY */
   return 0;
}

static int handle_exited(struct annotate_two *a2, const char *buf, size_t n, struct tgdb_list *list){
  char *tmp = (char*)malloc ( sizeof ( char )*(n+1));
  int *i = (int *)malloc ( sizeof ( int ) );
  struct tgdb_response *response;
  sprintf ( tmp, "%s", buf + 7 ); /* Skip the 'exited ' part */
  *i = atoi ( tmp ) ;
  free ( tmp );
  tmp = NULL;
  response = (struct tgdb_response *)
    cgdb_malloc (sizeof (struct tgdb_response));
  response->header = TGDB_INFERIOR_EXITED;
  response->choice.inferior_exited.exit_status = i;
  tgdb_types_append_command(list, response);
  return 0;
}

/**
 * The main annotation data structure.
 * It represents all of the supported annotataions that can be parsed.
 */
static struct annotation {

	/**
	 * The name of the annotation.
	 */
    const char *data;

	/**
	 * The size of the annotation.
	 */
    size_t size;

	/**
	 * The function to call when the annotation is found.
	 */
    int (*f)(struct annotate_two *a2, const char *buf, size_t n, struct tgdb_list *list);
} annotations[] = {
  {
      "source",
      6,
      handle_source
  },
  {
      "pre-commands",
      12,
      handle_misc_pre_prompt 
  },
  {
      "commands",
      8,
      handle_misc_prompt
  },
  {
      "post-commands",
      13,
      handle_misc_post_prompt
  },
  {
      "pre-overload-choice",
      19,
      handle_misc_pre_prompt 
  },
  {
      "overload-choice",
      15,
      handle_misc_prompt
  },
  {
      "post-overload-choice",
      20,
      handle_misc_post_prompt
  },
  {
      "pre-instance-choice",
      19,
      handle_misc_pre_prompt 
  },
  {
      "instance-choice",
      15,
      handle_misc_prompt
  },
  {
      "post-instance-choice",
      20,
      handle_misc_post_prompt
  },
  {
      "pre-query",
      9,
      handle_misc_pre_prompt 
  },
  {
      "query",
      5,
      handle_misc_prompt
  },
  {
      "post-query",
      10,
      handle_misc_post_prompt
  },
  {
      "pre-prompt-for-continue",
      23,
      handle_misc_pre_prompt 
  },
  {
      "prompt-for-continue",
      19,
      handle_misc_prompt
  },
  {
      "post-prompt-for-continue",
      24,
      handle_misc_post_prompt
  },
  {
      "pre-prompt",
      10,
      handle_pre_prompt 
  },
  {
      "prompt",
      6,
      handle_prompt 
  },
  {
      "post-prompt",
      11,
      handle_post_prompt 
  },
  {
      "breakpoints-invalid",
      19,
      handle_breakpoints_invalid
  },
  {
      "breakpoints-headers",
      19,
      handle_breakpoints_headers 
  },
  {
      "breakpoints-table-end",
      21,
      handle_breakpoints_table_end
  },
  {
      "breakpoints-table",
      17,
      handle_breakpoints_table
  },
  {
      "field-",
      6,
      handle_unwanted_fields
  },
  {
      "field",
      5,
      handle_field
  },
  {
      "record",
      6,
      handle_record
  },
  {
      "error-begin",
      11,
      handle_error_begin
  },
  {
      "error",
      5,
      handle_error
  },
  {
      "quit",
      4,
      handle_quit
  },
  {
      "exited",
      6,
      handle_exited
  },
  {
      NULL,
      0,
      NULL
  }
};

int tgdb_parse_annotation(struct annotate_two *a2, char *data, size_t size, struct tgdb_list *list) {
   int i;
   for(i = 0; annotations[i].data != NULL; ++i){
      if(strncmp(data, annotations[i].data, annotations[i].size) == 0){
         if(annotations[i].f){
            if(annotations[i].f(a2, data, size, list) == -1){
               logger_write_pos ( logger, __FILE__, __LINE__, "parsing annotation failed");
            } else 
               break; /* only match one annotation */
         }
      }
   }
   
   /*err_msg("ANNOTION(%s)", data);*/
   return 0;
}
