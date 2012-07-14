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
#endif /* HAVE_STDLIB_H */

#include "state_machine.h"
#include "annotate.h"
#include "logger.h"
#include "data.h"
#include "globals.h"
#include "annotate_two.h"
#include "sys_util.h"
#include "ibuf.h"

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

enum state {
    DATA,                       /* data from debugger */
    NEW_LINE,                   /* got '\n' */
    CONTROL_Z,                  /* got first ^Z '\032' */
    ANNOTATION,                 /* got second ^Z '\032' */
    NL_DATA                     /* got a nl at the end of annotation */
};

/**
 * The data needed to parse the output of GDB.
 */
struct state_machine {

    /**
	 * Annotations will be stored here.
	 */
    struct ibuf *tgdb_buffer;

    /**
	 * The state of the annotation parser ( current context ).
	 */
    enum state tgdb_state;
};

struct state_machine *state_machine_initialize(void)
{
    struct state_machine *sm =
            (struct state_machine *) cgdb_malloc(sizeof (struct state_machine));

    sm->tgdb_buffer = ibuf_init();
    sm->tgdb_state = DATA;

    return sm;
}

void state_machine_shutdown(struct state_machine *sm)
{
    ibuf_free(sm->tgdb_buffer);
    free(sm);
    sm = NULL;
}

int a2_handle_data(struct annotate_two *a2, struct state_machine *sm,
        const char *data, const size_t size,
        char *gui_data, size_t * gui_size, struct tgdb_list *command_list)
{
    int i, counter = 0;

    /* track state to find next file and line number */
    for (i = 0; i < size; ++i) {
        switch (data[i]) {
                /* Ignore all car returns outputted by gdb */
            case '\r':
                break;
            case '\n':
                switch (sm->tgdb_state) {
                    case DATA:
                        sm->tgdb_state = NEW_LINE;
                        break;
                    case NEW_LINE:
                        sm->tgdb_state = NEW_LINE;
                        data_process(a2, '\n', gui_data, &counter,
                                command_list);
                        break;
                    case CONTROL_Z:
                        sm->tgdb_state = DATA;
                        data_process(a2, '\n', gui_data, &counter,
                                command_list);
                        data_process(a2, '\032', gui_data, &counter,
                                command_list);
                        break;
                    case ANNOTATION:   /* Found an annotation */
                        sm->tgdb_state = NL_DATA;
                        tgdb_parse_annotation(a2, ibuf_get(sm->tgdb_buffer),
                                ibuf_length(sm->tgdb_buffer), command_list);
                        ibuf_clear(sm->tgdb_buffer);
                        break;
                    case NL_DATA:
                        sm->tgdb_state = NEW_LINE;
                        break;
                    default:
                        logger_write_pos(logger, __FILE__, __LINE__,
                                "Bad state transition");
                        break;
                }               /* end switch */
                break;
            case '\032':
                switch (sm->tgdb_state) {
                    case DATA:
                        sm->tgdb_state = DATA;
                        data_process(a2, '\032', gui_data, &counter,
                                command_list);
                        break;
                    case NEW_LINE:
                        sm->tgdb_state = CONTROL_Z;
                        break;
                    case NL_DATA:
                        sm->tgdb_state = CONTROL_Z;
                        break;
                    case CONTROL_Z:
                        sm->tgdb_state = ANNOTATION;
                        break;
                    case ANNOTATION:
                        ibuf_addchar(sm->tgdb_buffer, data[i]);
                        break;
                    default:
                        logger_write_pos(logger, __FILE__, __LINE__,
                                "Bad state transition");
                        break;
                }               /* end switch */
                break;
            default:
                switch (sm->tgdb_state) {
                    case DATA:
                        data_process(a2, data[i], gui_data, &counter,
                                command_list);
                        break;
                    case NL_DATA:
                        sm->tgdb_state = DATA;
                        data_process(a2, data[i], gui_data, &counter,
                                command_list);
                        break;
                    case NEW_LINE:
                        sm->tgdb_state = DATA;
                        data_process(a2, '\n', gui_data, &counter,
                                command_list);
                        data_process(a2, data[i], gui_data, &counter,
                                command_list);
                        break;
                    case CONTROL_Z:
                        sm->tgdb_state = DATA;
                        data_process(a2, '\n', gui_data, &counter,
                                command_list);
                        data_process(a2, '\032', gui_data, &counter,
                                command_list);
                        data_process(a2, data[i], gui_data, &counter,
                                command_list);
                        break;
                    case ANNOTATION:
                        ibuf_addchar(sm->tgdb_buffer, data[i]);
                        break;
                    default:
                        logger_write_pos(logger, __FILE__, __LINE__,
                                "Bad state transition");
                        break;
                }               /* end switch */
                break;
        }                       /* end switch */
    }                           /* end for */

    gui_data[counter] = '\0';
    *gui_size = counter;
    return 0;
}
