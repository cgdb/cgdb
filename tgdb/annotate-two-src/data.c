#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_STDIO_H 
#include <stdio.h>
#endif  /* HAVE_STDLIB_H */

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#include "data.h"
#include "commands.h"
#include "globals.h"
#include "io.h"
#include "error.h"

static int car_ret_counter = 0;
static int server_counter = 8;
static int temp_data_state = 0;

static enum internal_state data_state = VOID;

#define GDB_PROMPT_SIZE 1024
static char gdb_prompt[GDB_PROMPT_SIZE];
static char gdb_prompt_last[GDB_PROMPT_SIZE];
static int gdb_prompt_size = 0;

int COMMAND_TYPED_AT_PROMPT = 0; 

/* Used to determine if the car ret is from readline */
static enum CAR_RET_STATE {
   OTHER,
   WHITE_SPACE
} local_car_ret = OTHER;

enum internal_state data_get_state(void){
   return data_state;
}

void data_prepare_run_command(void){
   car_ret_counter = 0;
   server_counter = 8;
   temp_data_state = 1;
}

void data_set_state(enum internal_state state){
   /* if tgdb is at an internal command, than nothing changes that
    * state unless tgdb gets to the prompt annotation. This means that
    * the internal command is done */
   if(data_state == INTERNAL_COMMAND && state != USER_AT_PROMPT)
      return;

   data_state = state;
   switch(data_state){
      case VOID:              break;
      case AT_PROMPT:         
         gdb_prompt_size = 0;
         break;
      case USER_AT_PROMPT:    

         /* Null-Terminate the prompt */
         gdb_prompt[gdb_prompt_size] = '\0';  

         if ( strcmp(gdb_prompt, gdb_prompt_last) != 0 ) {
            strcpy(gdb_prompt_last, gdb_prompt);
            /* Update the prompt */
            a2_tgdb_change_prompt(gdb_prompt_last);
         }

         commands_set_state(VOID, NULL);
         COMMAND_TYPED_AT_PROMPT = 0;
         global_set_signal_recieved(FALSE);

         break;
      case POST_PROMPT:    
            data_state = VOID;
            local_car_ret = OTHER;
            break;
      case GUI_COMMAND:    break;
      case INTERNAL_COMMAND: break;
      case USER_COMMAND: break;
   } /* end switch */
}

void data_process(char a, char *buf, int *n, struct queue *q){
    switch(data_state){
        case VOID:    buf[(*n)++] = a;   break;
        case AT_PROMPT:         
            gdb_prompt[gdb_prompt_size++] = a;  
            buf[(*n)++] = a;
            break;
        case USER_AT_PROMPT:             break;
        case GUI_COMMAND:
        case INTERNAL_COMMAND:
            if(data_state == INTERNAL_COMMAND)
                commands_process(a, q);
            else if(data_state == GUI_COMMAND)
                buf[(*n)++] = a;
      
            break; /* do nothing */
        case USER_COMMAND: break;
        case POST_PROMPT:  break;
    } /* end switch */
}

char *data_get_prompt(void) {
    if ( gdb_prompt[0] )
        return gdb_prompt;
    return "(tgdb) ";
}
