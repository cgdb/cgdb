#ifndef __GLOBALS_H__
#define __GLOBALS_H__

/* This unit holds global data to tgdb. It helps keep track of obscure states */

/* This turns true if tgdb gets a misc prompt. This is so that we do not 
 * send commands to gdb at this point.
 */
int globals_is_internal_prompt(void);
void globals_set_internal_prompt_command(unsigned short set);

/* global_can_issue_command: Tells tgdb if it can issue a command to gdb.
 * Return: 1 if tgdb can issue a command, otherwise 0.
 */
int global_can_issue_command(void);
/* if 1, then tgdb can issue a command, 0 can not */
void global_set_can_issue_command(unsigned short set);

/* if a signal was recieved by library. Once, the user reaches the prompt,
 * the signal is cleared.
 */
int global_signal_recieved(void);
void global_set_signal_recieved(unsigned short set);

/* These check to see if the gui is working on getting the source files that 
 * make up the program being debugged.
 */
void global_set_start_info_sources(void);
int global_has_info_sources_started(void);
void global_reset_info_sources_started(void);

/* These check to see if the gui is working on getting an absolute path to
 * a source file in the debugged program.
 */
void global_set_start_info_source(void);
int global_has_info_source_started(void);
void global_reset_info_source_started(void);

 /* Lists a file */
void global_set_start_list(void);
int global_has_list_started(void);
void global_reset_list_started(void);

#endif
