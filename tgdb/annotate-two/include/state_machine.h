/* state_machine:
 * -------------
 *
 * This unit is responsible for parsing annotations from the output of gdb.
 * It can distinguish between output that is an annotation and output that 
 * is not an annotation. It sends all output that is not an annotation to 
 * the data package and all output that is an annotation to the annotation 
 * package. It does not interpret the data in any way.
 * All ouput from gdb must be filtered through this function.
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

/**
 * \param data
 * The buffer to parse.
 *
 * \param size
 * The size of the buffer data.
 *
 * \param gui_data
 * This is the information in DATA that was not an annotation.
 *
 * \param gui_size
 * The size of the buffer gui_data.
 *
 * \param command_list
 * If a command was generated from an annotation, its put in here.
 */
int a2_handle_data(struct annotate_two *a2,
        struct state_machine *sm,
        const char *data, const size_t size,
        char *gui_data, size_t * gui_size, struct tgdb_list *command_list);

#endif
