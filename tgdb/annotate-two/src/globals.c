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
	 * This determines if the char enter has been typed by the
	 * user since the prompt annotation has been sent by gdb
	 */
    unsigned short info_sources_started;

    unsigned short completion_started;

    /** 
	 * Has the 'list' command been started.
	 */
    unsigned short list_started;

    /**
	 * Did the list have an error?
	 */
    unsigned short list_had_error;

    /**
	 * Is a misc prompt command be run.
	 */
    unsigned short misc_prompt_command;
};

struct globals *globals_initialize(void)
{
    struct globals *g = (struct globals *) cgdb_malloc(sizeof (struct globals));

    g->info_sources_started = 0;
    g->completion_started = 0;
    g->list_started = 0;
    g->list_had_error = 0;
    g->misc_prompt_command = 0;

    return g;
}

/* For info_sources_started */
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

void global_set_start_info_sources(struct globals *g)
{
    g->info_sources_started = 1;
}

int global_has_info_sources_started(struct globals *g)
{
    return g->info_sources_started;
}

void global_reset_info_sources_started(struct globals *g)
{
    g->info_sources_started = 0;
}

void global_set_start_completion(struct globals *g)
{
    g->completion_started = 1;
}

int global_has_completion_started(struct globals *g)
{
    return g->completion_started;
}

void global_reset_completion_started(struct globals *g)
{
    g->completion_started = 0;
}

/* For list_started */
void global_set_start_list(struct globals *g)
{
    g->list_started = 1;
    g->list_had_error = 0;
}

int global_has_list_started(struct globals *g)
{
    return g->list_started;
}

void global_list_finished(struct globals *g)
{
    g->list_started = 0;
}

/* For list_had_error */
unsigned short global_list_had_error(struct globals *g)
{
    return g->list_had_error;
}

void global_set_list_error(struct globals *g, unsigned short error)
{
    g->list_had_error = error;
}
