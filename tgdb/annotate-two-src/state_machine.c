/* From configure */
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "state_machine.h"
#include "annotate.h"
#include "error.h"
#include "data.h"
#include "globals.h"

static char tgdb_buffer[MAXLINE];
static size_t tgdb_size = 0;

enum state {
   DATA,          /* data from debugger */
   CAR_RET,       /* got '\r' */
   NEW_LINE,      /* got '\n' */
   CONTROL_Z,     /* got first ^Z '\032' */
   ANNOTATION,    /* got second ^Z '\032' */
   NL_DATA        /* got a nl at the end of annotation */
} tgdb_state = DATA;

int a2_handle_data(char *data, size_t size,
                     char *gui_data, size_t gui_size, 
                     struct queue *q){
   int i, counter = 0;
   
   /* track state to find next file and line number */
   for(i = 0; i < size; ++i){
      switch(data[i]){
         case '\r':

            /* This makes sure that '\r' is always returned when
             * the user is talking with the readline library. It is 
             * not part of a gdb annotation.
             */
//            if ( data_get_state() == USER_AT_PROMPT && 
//                 (!global_did_user_press_enter() && 
//                  !global_signal_recieved() &&
//                  !global_did_user_press_special_control_char()) &&
//                  !global_has_implicit_enter_benen_recieved())
//                goto default_;

             switch(tgdb_state){
               case DATA:        
                  tgdb_state = CAR_RET;                          
                  break;
               case CAR_RET:  
                  data_process('\r', gui_data, &counter, q);      
                  break;
               case NEW_LINE:    
                  tgdb_state = CAR_RET;
                  data_process('\r', gui_data, &counter, q);
                  data_process('\n', gui_data, &counter, q);      
                  break;
               case CONTROL_Z:   
                  tgdb_state = CAR_RET;
                  data_process('\r', gui_data, &counter, q);
                  data_process('\n', gui_data, &counter, q);  
                  data_process('\032', gui_data, &counter, q);    
                  break;
               case ANNOTATION:  
                  tgdb_buffer[tgdb_size++] = data[i];          
                  break;
               case NL_DATA:
                  tgdb_state = CAR_RET; 
                  break;
               default:                                                       
                  err_msg("%s:%d -> Bad state transition", __FILE__, __LINE__);
                  break;
            } /* end switch */
            break;
         case '\n':     
               /* This can happen if the user does not hit the enter key
                * but a key they hit made return line decide that it would
                * return the '\n' key. ex. \t\t twice...
                * This is an error. gdb does not output the post-prompt
                * annotation. This saves tgdb from not understanding the 
                * output at all.
                */
//               if ( data_get_state() == USER_AT_PROMPT )
//                   global_set_implicit_enter(1);

            switch(tgdb_state){
               case DATA:        
                  data_process('\n', gui_data, &counter, q);
                  break;
               case CAR_RET:
                  tgdb_state = NEW_LINE;
                  break;
               case NL_DATA:     
                  tgdb_state = NEW_LINE;
                  break; 
               case NEW_LINE:    
                  tgdb_state = DATA;
                  data_process('\r', gui_data, &counter, q);
                  data_process('\n', gui_data, &counter, q);    
                  break;
               case CONTROL_Z:   
                  tgdb_state = DATA;
                  data_process('\r', gui_data, &counter, q);  
                  data_process('\n', gui_data, &counter, q);  
                  data_process('\032', gui_data, &counter, q);    
                  break;
               case ANNOTATION:  /* Found an annotation */
                  tgdb_state = NL_DATA;
                  tgdb_parse_annotation(tgdb_buffer, tgdb_size, q);
                  tgdb_size = 0;                               
                  memset(tgdb_buffer, '\0', MAXLINE);             
                  break;
               default:                                                       
                  err_msg("%s:%d -> Bad state transition", __FILE__, __LINE__);
                  break;
            } /* end switch */
            break;
         case '\032':
            switch(tgdb_state){
               case DATA:        
                  tgdb_state = DATA;
                  data_process('\032', gui_data, &counter, q);  
                  break;
               case CAR_RET:
                  tgdb_state = DATA;
                  data_process('\r', gui_data, &counter, q);  
                  data_process('\032', gui_data, &counter, q);  
                  break;
               case NEW_LINE:    
                  tgdb_state = CONTROL_Z;          
                  break;
               case NL_DATA:     
                  tgdb_state = CONTROL_Z;          
                  break;
               case CONTROL_Z:   
                  tgdb_state = ANNOTATION;         
                  break;
               case ANNOTATION:  
                  tgdb_buffer[tgdb_size++] = data[i];    
                  break;
               default:                                                       
                  err_msg("%s:%d -> Bad state transition", __FILE__, __LINE__);
                  break;
            } /* end switch */
            break;
         default:
         default_:
            switch(tgdb_state){
               case DATA:        
                  data_process(data[i], gui_data, &counter, q);  
                  break;
               case NL_DATA:     
                  tgdb_state = DATA;
                  data_process(data[i], gui_data, &counter, q);  
                  break;
               case CAR_RET:
                  tgdb_state = DATA;
                  data_process('\r', gui_data, &counter, q);     
                  data_process(data[i], gui_data, &counter, q);  
                  break;
               case NEW_LINE:    
                  tgdb_state = DATA;
                  data_process('\r', gui_data, &counter, q);     
                  data_process('\n', gui_data, &counter, q);     
                  data_process(data[i], gui_data, &counter, q);  
                  break;
               case CONTROL_Z:   
                  tgdb_state = DATA;
                  data_process('\r', gui_data, &counter, q);     
                  data_process('\n', gui_data, &counter, q);                 
                  data_process('\032', gui_data, &counter, q);                 
                  data_process(data[i], gui_data, &counter, q);                 
                  break;
               case ANNOTATION:  
                  tgdb_buffer[tgdb_size++] = data[i];    
                  break;
               default:                                                       
                  err_msg("%s:%d -> Bad state transition", __FILE__, __LINE__);
                  break;
            } /* end switch */
            break;
      } /* end switch */
   }  /* end for */

   gui_data[counter] = '\0';
   return counter;
}
