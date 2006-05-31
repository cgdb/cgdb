/* data:
 * -----
 *
 * This unit recieves all of the data that was not an annotation from gdb.
 * It gets the data 1 character at a time and passes it along to the
 * commands unit when necessary.
 *
 */


#ifndef __DATA_H__
#define __DATA_H__

#ifdef __cplusplus
extern "C" {
#endif 

#include "tgdb_types.h"
#include "tgdb_command.h"
#include "annotate_two.h"

struct data;

enum internal_state {
   VOID,             /* not interesting */
   AT_PROMPT,        /* the prompt is being displayed */
   USER_AT_PROMPT,   /* the user is typing at prompt */
   POST_PROMPT,      /* the user is done at the prompt */
   USER_COMMAND,     /* this is a command issued by the user */
   GUI_COMMAND,      /* this is a command issued to gdb by tgdb (not the user) */
   INTERNAL_COMMAND  /* This is a command issued by tgdb */
};

struct data *data_initialize ( void );

void data_shutdown ( struct data *d );

/* data_set_state:   Sets the state of the data package. This should usually be called
 *                   after an annotation has been read.
 */
void data_set_state ( struct annotate_two *a2, enum internal_state state );

/* data_get_state:   Gets the state of the data package 
 * Returns:          The current state.
 */
enum internal_state data_get_state ( struct data *d );

/* data_process:  This process's every character that is outputted from gdb not 
 *                including annotations.
 *
 *    a     -  A character read from gdb.
 *    buf   -  This is a buffer of information that will get returned to the user
 *             based on whatever the character a was.
 *    n     -  This is the current size of buf.
 */
void data_process ( 
		struct annotate_two *a2, 
		char a, char *buf, int *n, 
		struct tgdb_list *list);

#ifdef __cplusplus
}
#endif

#endif
