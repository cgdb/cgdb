#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#include "annotate.h"
#include "error.h"
#include "data.h"
#include "commands.h"
#include "globals.h"
#include "io.h"

static int handle_source(struct annotate_two *a2, const char *buf,  size_t n, struct queue *q){
   int ret = commands_parse_source(a2->c, a2->command_container, buf, n, q);

   /* This tells the annotate subsystem if the source annotation has been
	* reached. This is important because if the source annotation has been 
	* reached before tgdb is initialized, then the annotate subsystem doesn't
	* have to probe gdb for the first file to open.
	*/
   a2->source_already_received = 1;
   return ret;
}

static int handle_misc_pre_prompt(struct annotate_two *a2, const char *buf, size_t n, struct queue *q){
   /* If tgdb is sending a command, then continue past it */
   if(data_get_state(a2->data) == INTERNAL_COMMAND){
      if(io_write_byte(a2->debugger_stdin, '\n') == -1)
         err_msg("%s:%d -> Could not send command", __FILE__, __LINE__);
   } else {
      data_set_state(a2, AT_PROMPT);
   }

   return 0;
}

static int handle_misc_prompt(struct annotate_two *a2, const char *buf, size_t n, struct queue *q){
   globals_set_misc_prompt_command( a2->g, TRUE);
   data_set_state(a2, USER_AT_PROMPT );
   a2->command_finished = 1;
   return 0;
}

static int handle_misc_post_prompt(struct annotate_two *a2, const char *buf, size_t n, struct queue *q){
   globals_set_misc_prompt_command( a2->g, FALSE);
   data_set_state(a2, POST_PROMPT );

   return 0;
}

static int handle_pre_prompt(struct annotate_two *a2, const char *buf, size_t n, struct queue *q){
   data_set_state(a2, AT_PROMPT );
   
   return 0;
}

static int handle_prompt(struct annotate_two *a2, const char *buf, size_t n, struct queue *q){
   /* This return TGDB_ABSOLUTE_SOURCE_DENIED if there was no absolute path
	* given when 'info source' was run.
	* It tells the gui that the source is not available.
	*/
   commands_finalize_command ( a2->c, q );
   
   /* All done. */
   data_set_state(a2, USER_AT_PROMPT );

   /* 'info sources' is done, return the sources to the gui */
   if(global_has_info_sources_started(a2->g) == TRUE){
      global_reset_info_sources_started(a2->g);
      commands_send_gui_sources(a2->c, q);
      return 0;
   } 

   return 0;
}

static int handle_post_prompt(struct annotate_two *a2, const char *buf, size_t n, struct queue *q){
   data_set_state(a2, POST_PROMPT );
   return 0;
}

static int handle_breakpoints_invalid(struct annotate_two *a2, const char *buf, size_t n, struct queue *q){
   return 0;
}

static int handle_breakpoints_headers(struct annotate_two *a2, const char *buf, size_t n, struct queue *q){
   commands_set_state(a2->c, BREAKPOINT_HEADERS, q);
   return 0;
}

static int handle_breakpoints_table(struct annotate_two *a2, const char *buf, size_t n, struct queue *q){
   commands_set_state(a2->c, BREAKPOINT_TABLE_BEGIN, q);
   return 0;
}

static int handle_breakpoints_table_end(struct annotate_two *a2, const char *buf, size_t n, struct queue *q){
   commands_set_state(a2->c, BREAKPOINT_TABLE_END, q);
   return 0;
}

static int handle_field(struct annotate_two *a2, const char *buf, size_t n, struct queue *q){
   int i = -1;
   
   commands_parse_field(a2->c, buf, n, &i);
   commands_set_field_num(a2->c, i);
   commands_set_state(a2->c, FIELD, q);

   return 0;
}

static int handle_record(struct annotate_two *a2, const char *buf, size_t n, struct queue *q){
   commands_set_state(a2->c, RECORD, q);
   return 0;
}

static int handle_error(struct annotate_two *a2, const char *buf, size_t n, struct queue *q){
   data_set_state(a2, POST_PROMPT );  /* TEMPORARY */
   return 0;
}

static int handle_error_begin(struct annotate_two *a2, const char *buf, size_t n, struct queue *q){
   /* If the user listed the files ( info sources ) and there is an 
    * annotate error ( usually meaning that gdb can not find the symbols
    * for the debugged program ) then send a denied response. */
   if ( commands_get_state(a2->c) == INFO_SOURCES ) {
        tgdb_append_command(q, TGDB_SOURCES_DENIED, NULL );
        return 0;
   }

   /* if the user tried to list a file that does not exist */
   if(global_has_list_started(a2->g) == TRUE){
      global_list_finished(a2->g);
      global_set_list_error ( a2->g, TRUE );
      commands_list_command_finished(a2->c, q, 0);
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

static int handle_quit(struct annotate_two *a2, const char *buf, size_t n, struct queue *q){
   data_set_state(a2, POST_PROMPT );  /* TEMPORARY */
   return 0;
}

static int handle_display_begin(struct annotate_two *a2, const char *buf, size_t n, struct queue *q){
   return 0;
}

static int handle_display_number_end(struct annotate_two *a2, const char *buf, size_t n, struct queue *q){
   return 0;
}

static int handle_display_format(struct annotate_two *a2, const char *buf, size_t n, struct queue *q){
   return 0;
}

static int handle_display_expression(struct annotate_two *a2, const char *buf, size_t n, struct queue *q){
   return 0;
}

static int handle_display_expression_end(struct annotate_two *a2, const char *buf, size_t n, struct queue *q){
   return 0;
}

static int handle_display_value(struct annotate_two *a2, const char *buf, size_t n, struct queue *q){
   return 0;
}

static int NOT_SUPPORTED(struct annotate_two *a2, const char *buf, size_t n, struct queue *q){
   return 0;
}

static struct annotation {
   const char *data;
   size_t size;
   int (*f)(struct annotate_two *a2, const char *buf, size_t n, struct queue *q);
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
      "value-history-begin",
      19,
      NOT_SUPPORTED
  },
  {
      "value-history-value",
      19,
      NOT_SUPPORTED
  },
  {
      "value-history-end",
      17,
      NOT_SUPPORTED
  },
  {
      "value-begin",
      11,
      NOT_SUPPORTED
  },
  {
      "value-end",
      9,
      NOT_SUPPORTED
  },
  {
      "arg-begin",
      9,
      NOT_SUPPORTED
  },
  {
      "arg-name-end",
      12,
      NOT_SUPPORTED
  },
  {
      "arg-value",
      9,
      NOT_SUPPORTED
  },
  {
      "arg-end",
      7,
      NOT_SUPPORTED
  },
  {
      "field-begin",
      11,
      NOT_SUPPORTED
  },
  {
      "field-name-end",
      14,
      NOT_SUPPORTED
  },
  {
      "field-value",
      11,
      NOT_SUPPORTED
  },
  {
      "field-end",
      9,
      NOT_SUPPORTED
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
      "display-begin",
      13,
      handle_display_begin
  },
  {
     "display-number-end",
     18,
     handle_display_number_end
  },
  {
     "display-format",
      14,
      handle_display_format
  },
  {
     "display-expression",
     18,
     handle_display_expression
  },
  {
     "display-expression-end",
     22,
     handle_display_expression_end
  },
  {
     "display-value",
     13,
     handle_display_value
  },
  {
      NULL,
      (int)NULL,
      NULL
  }
};

int tgdb_parse_annotation(struct annotate_two *a2, char *data, size_t size, struct queue *q) {
   int i;
   for(i = 0; annotations[i].data != NULL; ++i){
      if(strncmp(data, annotations[i].data, annotations[i].size) == 0){
         if(annotations[i].f){
            if(annotations[i].f(a2, data, size, q) == -1){
               err_msg("%s:%d -> parsing annotation failed\n", __FILE__, __LINE__);
            } else 
               break; /* only match one annotation */
         }
      }
   }
   
   /*err_msg("ANNOTION(%s)", data);*/
   return 0;
}
