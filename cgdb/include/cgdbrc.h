#ifndef __CGDBRC_H__
#define __CGDBRC_H__

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_STDIO_H
#include <stdio.h> 
#endif /* HAVE_STDIO_H */

/* config_parse_string: parse a string of command data and execute the commands
 * that it represents.
 * --------------
 *  buffer: a line of command data to parse (representing a single command).
 */
int command_parse_string( const char *buffer );

int command_parse_file( FILE *fp );

enum ArrowStyle {
    ARROWSTYLE_SHORT,
    ARROWSTYLE_LONG,
    ARROWSTYLE_HIGHLIGHT
};

#endif /* __CGDBRC_H__ */
