#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_STDIO_H
#include <stdio.h> 
#endif /* HAVE_STDIO_H */

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#if HAVE_STDLIB_H 
#include <stdlib.h>
#endif  /* HAVE_STDLIB_H */

#include "state_machine.h"
#include "annotate.h"
#include "error.h"
#include "data.h"
#include "globals.h"

/* This package looks for annotations coming from gdb's output.
 * The program that is being debugged does not have its ouput pass
 * through here. So only gdb's ouput is filtered here.
 *
 * This is a simple state machine that is looking for annotations
 * in gdb's output. Annotations are of the form
 * '\n\032\032annotation\n'
 * However, on windows \n gets mapped to \r\n So we take account
 * for that by matching the form
 * '\r+\n\032\032annotation\r+\n'
 * 
 * When an annotation is found, this unit passes the annotation to the 
 * annotate unit and this unit is free of all responsibility :)
 */

static char tgdb_buffer[MAXLINE];
static size_t tgdb_size = 0;

enum state {
   DATA,          /* data from debugger */
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
         /* Ignore all car returns outputted by gdb */
         case '\r':
            break;
         case '\n':     
            switch(tgdb_state){
               case DATA:        
                  tgdb_state = NEW_LINE;
                  break;
               case NEW_LINE:    
                  tgdb_state = NEW_LINE;
                  data_process('\n', gui_data, &counter, q);    
                  break;
               case CONTROL_Z:   
                  tgdb_state = DATA;
                  data_process('\n', gui_data, &counter, q);  
                  data_process('\032', gui_data, &counter, q);    
                  break;
               case ANNOTATION:  /* Found an annotation */
                  tgdb_state = NL_DATA;
                  tgdb_parse_annotation(tgdb_buffer, tgdb_size, q);
                  tgdb_size = 0;                               
                  memset(tgdb_buffer, '\0', MAXLINE);             
                  break;
               case NL_DATA:     
                  tgdb_state = NEW_LINE;
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
            switch(tgdb_state){
               case DATA:        
                  data_process(data[i], gui_data, &counter, q);  
                  break;
               case NL_DATA:     
                  tgdb_state = DATA;
                  data_process(data[i], gui_data, &counter, q);  
                  break;
               case NEW_LINE:    
                  tgdb_state = DATA;
                  data_process('\n', gui_data, &counter, q);     
                  data_process(data[i], gui_data, &counter, q);  
                  break;
               case CONTROL_Z:   
                  tgdb_state = DATA;
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
