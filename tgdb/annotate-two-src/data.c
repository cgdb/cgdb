#include "data.h"
#include "commands.h"
#include "globals.h"
#include "io.h"
#include "error.h"
#include <stdio.h>
#include <string.h>

static int car_ret_counter = 0;
static int server_counter = 8;
static int temp_data_state = 0;

static enum internal_state data_state = VOID;

#define GDB_PROMPT_SIZE 1024
static char gdb_prompt[GDB_PROMPT_SIZE];
static int gdb_prompt_size = 0;

int DATA_AT_PROMPT = 0; 

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
         memset(gdb_prompt, '\0', GDB_PROMPT_SIZE);
         gdb_prompt_size = 0;
         break;
      case USER_AT_PROMPT:    
         commands_set_state(VOID, NULL);
         DATA_AT_PROMPT = 1;
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

void data_process(char a, char *buf, int *n, struct Command ***com){
   switch(data_state){
      case VOID:              buf[(*n)++] = a;                    break;
      case AT_PROMPT:         gdb_prompt[gdb_prompt_size++] = a;  break;
      case USER_AT_PROMPT:    break;
      case GUI_COMMAND:
      case INTERNAL_COMMAND:
         if(data_state == INTERNAL_COMMAND)
            commands_process(a, com);
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
