#ifndef __DATA_H__
#define __DATA_H__

#ifdef __cplusplus
extern "C" {
#endif 

#include "types.h"

enum internal_state {
   VOID,             /* not interesting */
   AT_PROMPT,        /* the prompt is being displayed */
   USER_AT_PROMPT,   /* the user is typing at prompt */
   POST_PROMPT,      /* the user is done at the prompt */
   GUI_COMMAND,      /* this is a command issued to gdb by tgdb (not the user) */
   INTERNAL_COMMAND  /* This is a command issued by tgdb */
};

/* data_set_state:   Sets the state of the data package. This should usually be called
 *                   after an annotation has been read.
 */
void data_set_state(enum internal_state state);

/* data_get_state:   Gets the state of the data package 
 * Returns:          The current state.
 */
enum internal_state data_get_state(void);

/* data_process:  This process's every character that is outputted from gdb not 
 *                including annotations.
 *
 *    a     -  A character read from gdb.
 *    buf   -  This is a buffer of information that will get returned to the user
 *             based on whatever the character a was.
 *    n     -  This is the current size of buf.
 */
void data_process(char a, char *buf, int *n, struct Command ***com);

/* This must be called before any command is written to gdb.  */
void data_prepare_run_command(void);

/* data_user_has_typed: This returns if the user has any characters present
 * at the gdb prompt.
 * Returns: 1 if char's are present, otherwise 0.
 */
int data_user_has_typed(void);

/* data_get_prompt: This returns the current gdb prompt
 * Returns a pointer to the current prompt.
 */
char *data_get_prompt(void);

#ifdef __cplusplus
}
#endif

#endif
