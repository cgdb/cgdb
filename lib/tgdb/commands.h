#ifndef __COMMANDS_H__
#define __COMMANDS_H__

#include "tgdb_types.h"

struct commands;

/**  
 * This should probably be moved out of a2-tgdb.h
 */

/* commands_initialize: Initialize the commands unit */
struct commands *commands_initialize(struct tgdb *tgdb);
void commands_shutdown(struct commands *c);

/**
 * This function receives the output from gdb when gdb
 * is running a command on behalf of this package.
 *
 * @param c
 * The commands instance
 *
 * @param str
 * The string to process
 */
void commands_process(struct commands *c, const std::string &str);

/**
 * Called when a command produces an error.
 */
void commands_process_error(struct commands *c);

/**
 * Return if the debugger supports the /s flag for the disassemble command.
 *
 * @return
 * 0 if /s is not supported, 1 if it is supported.
 */
int commands_disassemble_supports_s_mode(struct commands *c);

/**
 * Set the active request type being processed through gdb.
 *
 * @param c
 * The commands instance
 *
 * @param type
 * The current request type
 */
void commands_set_current_request_type(struct commands *c,
        enum tgdb_request_type type);

/**
 * Get the current request type being processed through gdb.
 *
 * @param c
 * The commands instance
 *
 * @return
 * The current request type
 */
enum tgdb_request_type commands_get_current_request_type(struct commands *c);

#endif /* __COMMANDS_H__ */
