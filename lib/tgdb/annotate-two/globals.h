#ifndef __GLOBALS_H__
#define __GLOBALS_H__

/* This unit holds global data to tgdb. It helps keep track of obscure states */

struct globals;

int globals_is_misc_prompt(struct globals *g);
void globals_set_misc_prompt_command(struct globals *g, unsigned short set);

struct globals *globals_initialize(void);
void globals_shutdown(struct globals *g);

/* These check to see if the gui is working on getting the source files that 
 * make up the program being debugged.
 */
void global_set_start_info_sources(struct globals *g);
int global_has_info_sources_started(struct globals *g);
void global_reset_info_sources_started(struct globals *g);

void global_set_start_completion(struct globals *g);
int global_has_completion_started(struct globals *g);
void global_reset_completion_started(struct globals *g);

 /* Lists a file */
void global_set_start_list(struct globals *g);
int global_has_list_started(struct globals *g);
void global_list_finished(struct globals *g);
unsigned short global_list_had_error(struct globals *g);
void global_set_list_error(struct globals *g, unsigned short error);

#endif
