#ifndef COMMANDS_H
#define COMMANDS_H

/* config_parse_string: parse a string of command data and execute the commands
 * that it represents.
 * --------------
 *  buffer: a line of command data to parse (representing a single command).
 */
int command_parse_string( const char *buffer );

#endif
