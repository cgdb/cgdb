#include "data.h"
#include "commands.h"
#include "globals.h"
#include "io.h"
#include "error.h"
#include <stdio.h>
#include <string.h>

static int num_chars_user_typed = 0;
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
         global_reset_command_line_data();
         DATA_AT_PROMPT = 1;
         global_set_signal_recieved(FALSE);
         global_set_implicit_enter(0);

         break;
      case POST_PROMPT:    
            num_chars_user_typed = 0;        
            data_state = VOID;
            local_car_ret = OTHER;
            break;
      case GUI_COMMAND:    break;
      case INTERNAL_COMMAND: break;
   } /* end switch */
}

int data_user_has_typed(void){
   return (num_chars_user_typed != 0);
}

void data_process(char a, char *buf, int *n, struct Command ***com){
   switch(data_state){
      case VOID:              buf[(*n)++] = a;                    break;
      case AT_PROMPT:         
         gdb_prompt[gdb_prompt_size++] = a;  
         buf[(*n)++] = a; /* give user the character typed */
      break;
      case USER_AT_PROMPT:    
         buf[(*n)++] = a; /* give user the character typed */

         /* if a '\r' was typed then the readline library just blasted
          * the gdb prompt ... lets be nice and put it back
          */
         if(a == '\r'){
            int i;
            buf[(*n)++] = '\r';
            for (i = 0; i < gdb_prompt_size; ++i)
                /* Only add the prompt if a '\n' has not been recieved 
                 * This is good for ^w or ^u or stuff like that.
                 * It should not put the prompt back if the user types \t\t*/
                if ( !global_has_implicit_enter_benen_recieved() )
                    buf[(*n)++] = gdb_prompt[i];

            if ( local_car_ret == OTHER ) {
               local_car_ret = WHITE_SPACE;
            } else {
               local_car_ret = OTHER;
            }
         }

         /* Lets keep track of how many char's the user has typed 
          * NOTE: here I am making a *BIG* assumption. That is, that 
          *       the readline library only uses the characters below
          *       to write to the terminal. So ^U -> \r ... \r
          */
         if(a == '\b' && num_chars_user_typed > 0)
            --num_chars_user_typed; 
         else if(a == '\r' || a == '\n')
            num_chars_user_typed = 0;
         else if(a != '\007') /* bell */
            ++num_chars_user_typed;

         break;
      case GUI_COMMAND:
      case INTERNAL_COMMAND:
         /* This is responsible for running a command internally on behalf of
          * the user application. It clears the gdb prompt that tgdb provides.
          * It also rewrites the prompt when necesary. It leaves whatever the 
          * user had on the prompt for the next command.
          */
         {
   
         /* return everything after 'server' and before '\031' (^Y) */
         if(a == '\r')
            car_ret_counter +=1;

         if((car_ret_counter == 2 && temp_data_state)){ /* after the '\r' */
            /* the user has typed something, must cover the prompt with spaces
             * and rewrite the prompt before the command is displayed 
             */
            int i; 

            for (i = 0; i < gdb_prompt_size; ++i)
               buf[(*n)++] = ' ';

            buf[(*n)++] = a;

            for (i = 0; i < gdb_prompt_size; ++i)
               buf[(*n)++] = gdb_prompt[i];
            
            server_counter = 0;
            temp_data_state = 0;
         } else if(!data_user_has_typed() && temp_data_state){
            if ( a != 's' ) {
               /* This is debug output upon error */
               err_msg("\n");
               io_display_char(stderr, a);
               err_msg("\n");
               car_ret_counter--;
            } else {
               /* the user has not typed anything */
               temp_data_state = 0; 
               server_counter = 1; /* the 's' was just recieved */
            }
         } else if(server_counter < 7){ /* this is the 'server ' response */
            server_counter++;
         } else if(data_state == INTERNAL_COMMAND){
            commands_process(a, com);
         } else if(data_state == GUI_COMMAND){
            buf[(*n)++] = a;
         }
      
         break; /* do nothing */
         }
     case POST_PROMPT:  break;
   } /* end switch */
}
