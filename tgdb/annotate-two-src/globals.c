/* From configure */
#include "config.h"

#include "types.h"
#include "globals.h"
#include <string.h>

/* This turns true if tgdb gets a misc prompt. This is so that we do not 
 * send commands to gdb at this point.
 */
static unsigned short internal_prompt_command = FALSE;
   
int globals_is_internal_prompt(void){
   return internal_prompt_command;
}
   
void globals_set_internal_prompt_command(unsigned short set){
   internal_prompt_command = set;
}  

/* This turns true if tgdb gets a prompt. Otherwise, something like
 * overload-prompt is no good, and tgdb can not issue a command. */
static unsigned short can_issue_command = TRUE;

int global_can_issue_command(void){
   return can_issue_command;
}

void global_set_can_issue_command(unsigned short set){
   can_issue_command = set;
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

/* Config directory */
static char global_config_dir[MAXLINE];
static int global_config_dir_length = 0;

void global_set_config_dir(const char *filename) {
   global_config_dir_length = strlen(filename) + 1;
   memset( global_config_dir, '\0', MAXLINE);
   strncpy( global_config_dir, filename, global_config_dir_length);
}

void global_get_config_dir(char *filename) {
   strncpy(filename, global_config_dir, global_config_dir_length);
}

void global_get_config_gdb_init_file(char *filename) {
   strncpy(filename, global_config_dir, global_config_dir_length);
#ifdef HAVE_CYGWIN
   strcat( filename, "\\gdb_init");
#else
   strcat( filename, "/gdb_init");
#endif
}

void global_get_config_gdb_debug_file(char *filename) {
   strncpy(filename, global_config_dir, global_config_dir_length);
#ifdef HAVE_CYGWIN
   strcat( filename, "\\tgdb_debug");
#else
   strcat( filename, "/tgdb_debug");
#endif
}
