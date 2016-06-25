/* state_machine:
 * -------------
 *
 * This unit is responsible for parsing annotations from the output of gdb.
 * It can distinguish between output that is an annotation and output that 
 * is not an annotation. It sends all output that is not an annotation to 
 * the data package and all output that is an annotation to the annotation 
 * package. It does not interpret the data in any way.
 * All output from gdb must be filtered through this function.
 */

#ifndef __STATE_MACHINE_H__
#define __STATE_MACHINE_H__

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */

#include "tgdb_types.h"
#include "a2-tgdb.h"

struct state_machine;

struct state_machine *state_machine_initialize(void);
void state_machine_shutdown(struct state_machine *sm);

enum internal_state {
    VOID,                   /* not interesting */
    AT_PROMPT,              /* the prompt is being displayed */
    USER_AT_PROMPT,         /* the user is typing at prompt */
    POST_PROMPT,            /* the user is done at the prompt */
    USER_COMMAND,           /* this is a command issued by the user */
    GUI_COMMAND,            /* this is a command issued to gdb by tgdb (not
    the user) */
    INTERNAL_COMMAND        /* This is a command issued by tgdb */
};

/* data_set_state:   Sets the state of the data package. This should usually be called
 *                   after an annotation has been read.
 */
void data_set_state(struct annotate_two *a2, enum internal_state state,
        struct tgdb_list *list);

/* data_get_state:   Gets the state of the data package
 * Returns:          The current state.
 */
enum internal_state data_get_state(struct state_machine *d);

/**
 * This receives all of the output from the debugger. It is all routed
 * through this function.
 *
 * \param input_data
 * This is the stdout from the debugger. This is the data that parse_io
 * will parse.
 *
 * \param input_data_size
 * This is the size of input_data.
 *
 * \param debugger_output
 * This is an out variable. It contains data that has been determined to
 * be the output of the debugger that the user should see.
 *
 * \param debugger_output_size
 * This is the size of debugger_output
 *
 * \param command_list
 * If a command was generated from an annotation, its put in here.
 */
void a2_parse_io(struct annotate_two *a2,
    const char *input_data, const size_t input_data_size,
    char *debugger_output, size_t *debugger_output_size,
    struct tgdb_list *command_list);

/* This unit holds global data to tgdb. It helps keep track of obscure states */

int sm_is_misc_prompt(struct state_machine *sm);

#endif
