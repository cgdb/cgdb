#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#include "types.h"
#include "globals.h"

/* This turns true if tgdb gets a misc prompt. This is so that we do not 
 * send commands to gdb at this point.
 */
static unsigned short misc_prompt_command = FALSE;
   
int globals_is_misc_prompt(void){
   return misc_prompt_command;
}
   
void globals_set_misc_prompt_command(unsigned short set){
   misc_prompt_command = set;
}  

static unsigned short tgdb_recieved_signal = FALSE;

int global_signal_recieved(void){
   return tgdb_recieved_signal;
}
/* if 1, then tgdb can issue a command, 0 can not */
void global_set_signal_recieved(unsigned short set){
   tgdb_recieved_signal = set;
}

/* This determines if the char enter has been typed by the
 * user since the prompt annotation has been sent by gdb
 */
static unsigned short info_sources_started = FALSE;

void global_set_start_info_sources(void){
   info_sources_started = TRUE;
}

int global_has_info_sources_started(void){
   return info_sources_started;
}

void global_reset_info_sources_started(void){
   info_sources_started = FALSE;
}

/* These check to see if the gui is working on getting an absolute path to
 * a source file in the debugged program.
 */
static unsigned short info_source_started = FALSE;

void global_set_start_info_source(void){
   info_source_started = TRUE;
}

int global_has_info_source_started(void){
   return info_source_started;
}

void global_reset_info_source_started(void){
   info_source_started = FALSE;
}

 /* Lists a file */
static unsigned short list_started = FALSE;

void global_set_start_list(void){
   list_started = TRUE;
}

int global_has_list_started(void){
   return list_started;
}

void global_reset_list_started(void){
   list_started = FALSE;
}
