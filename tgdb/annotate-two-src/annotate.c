/* From configure */
#include "config.h"

#include <string.h>
#include "annotate.h"
#include "error.h"
#include "data.h"
#include "commands.h"
#include "globals.h"
#include "io.h"

extern int masterfd;

static int handle_source(const char *buf,  size_t n, struct queue *q){
   return commands_parse_source(buf, n, q);
}

static int handle_misc_pre_prompt(const char *buf, size_t n, struct queue *q){
   extern int misc_pre_prompt;
   
   /* If tgdb is sending a command, then continue past it */
   if(data_get_state() == INTERNAL_COMMAND){
      if(io_write_byte(masterfd, '\n') == -1)
         err_msg("%s:%d -> Could not send command", __FILE__, __LINE__);
   } else {
      data_set_state(AT_PROMPT);
      misc_pre_prompt = 1;
   }

   return 0;
}

static int handle_misc_prompt(const char *buf, size_t n, struct queue *q){
   globals_set_misc_prompt_command(TRUE);
   return 0;
}

static int handle_misc_post_prompt(const char *buf, size_t n, struct queue *q){
   globals_set_misc_prompt_command(FALSE);

   return 0;
}

static int handle_pre_prompt(const char *buf, size_t n, struct queue *q){
   data_set_state(AT_PROMPT);
   
   return 0;
}

static int handle_prompt(const char *buf, size_t n, struct queue *q){
   data_set_state(USER_AT_PROMPT);

   /* 'info sources' is done, return the sources to the gui */
   if(global_has_info_sources_started() == TRUE){
      global_reset_info_sources_started();
      commands_send_gui_sources(q);
      return 0;
   } 

   if(global_has_list_started() == TRUE){
      global_reset_list_started();
      commands_list_command_finished(q, 1);
      return 0;
   }

   if(global_has_info_source_started() == TRUE){
      global_reset_info_source_started();
      commands_send_source_absolute_source_file(q);
      return 0;
   }
   
   return 0;
}

static int handle_post_prompt(const char *buf, size_t n, struct queue *q){
   data_set_state(POST_PROMPT);
   return 0;
}

static int handle_breakpoints_invalid(const char *buf, size_t n, struct queue *q){
   /*commands_set_command_to_run(BREAKPOINTS);*/
   /* Tgdb checks for breakpoints after every command. 
    * This is because of buggy gdb versions. gdb does not output this 
    * annotation when it is suppossed to.
    */
   return 0;
}

static int handle_breakpoints_headers(const char *buf, size_t n, struct queue *q){
   commands_set_state(BREAKPOINT_HEADERS, q);
   return 0;
}

static int handle_breakpoints_table(const char *buf, size_t n, struct queue *q){
   commands_set_state(BREAKPOINT_TABLE_BEGIN, q);
   return 0;
}

static int handle_breakpoints_table_end(const char *buf, size_t n, struct queue *q){
   commands_set_state(BREAKPOINT_TABLE_END, q);
   return 0;
}

static int handle_field(const char *buf, size_t n, struct queue *q){
   int i = -1;
   
   commands_parse_field(buf, n, &i);
   commands_set_field_num(i);
   commands_set_state(FIELD, q);

   return 0;
}

static int handle_record(const char *buf, size_t n, struct queue *q){
   commands_set_state(RECORD, q);
   return 0;
}

static int handle_error(const char *buf, size_t n, struct queue *q){
   data_set_state(POST_PROMPT);  /* TEMPORARY */
   return 0;
}

static int handle_error_begin(const char *buf, size_t n, struct queue *q){
   /* If the user listed the files ( info sources ) and there is an 
    * annotate error ( usually meaning that gdb can not find the symbols
    * for the debugged program ) then send a denied response. */
   if ( commands_get_state() == INFO_SOURCES ) {
        tgdb_append_command(q, SOURCES_DENIED, NULL, NULL, NULL);
        return 0;
   }

   /* if the user tried to list a file that does not exist */
   if(global_has_list_started() == TRUE){
      global_reset_list_started();
      commands_list_command_finished(q, 0);
      return 0;
   }

   data_set_state(POST_PROMPT);  /* TEMPORARY */
   return 0;
}

static int handle_quit(const char *buf, size_t n, struct queue *q){
   data_set_state(POST_PROMPT);  /* TEMPORARY */
   return 0;
}

static int handle_display_begin(const char *buf, size_t n, struct queue *q){
   return 0;
}

static int handle_display_number_end(const char *buf, size_t n, struct queue *q){
   return 0;
}

static int handle_display_format(const char *buf, size_t n, struct queue *q){
   return 0;
}

static int handle_display_expression(const char *buf, size_t n, struct queue *q){
   return 0;
}

static int handle_display_expression_end(const char *buf, size_t n, struct queue *q){
   return 0;
}

static int handle_display_value(const char *buf, size_t n, struct queue *q){
   return 0;
}

static int NOT_SUPPORTED(const char *buf, size_t n, struct queue *q){
   return 0;
}

static struct annotation {
   const char *data;
   size_t size;
   int (*f)(const char *buf, size_t n, struct queue *q);
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

int tgdb_parse_annotation(char *data, size_t size, struct queue *q){
   int i;
   for(i = 0; annotations[i].data != NULL; ++i){
      if(strncmp(data, annotations[i].data, annotations[i].size) == 0){
         if(annotations[i].f){
            if(annotations[i].f(data, size, q) == -1){
               err_msg("%s:%d -> parsing annotation failed\n", __FILE__, __LINE__);
            } else 
               break; /* only match one annotation */
         }
      }
   }
   
   /*err_msg("ANNOTION(%s)", data);*/
   return 0;
}
