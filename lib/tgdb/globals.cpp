#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#include "tgdb_types.h"
#include "globals.h"
#include "sys_util.h"

/**
 * The globals context.
 * This store various amounts of global data for the annotate_two context.
 */
struct globals {

    /**
	 * Is a misc prompt command be run.
	 */
    unsigned short misc_prompt_command;
};

struct globals *globals_initialize(void)
{
    struct globals *g = (struct globals *) cgdb_malloc(sizeof (struct globals));

    g->misc_prompt_command = 0;

    return g;
}

void globals_shutdown(struct globals *g)
{
    free(g);
    g = NULL;
}

/* This turns true if tgdb gets a misc prompt. This is so that we do not 
 * send commands to gdb at this point.
 */
int globals_is_misc_prompt(struct globals *g)
{
    return g->misc_prompt_command;
}

void globals_set_misc_prompt_command(struct globals *g, unsigned short set)
{
    g->misc_prompt_command = set;
}
