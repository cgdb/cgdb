#ifndef __GDBMI_PARSE_H__
#define __GDBMI_PARSE_H__

#include <sys/types.h>
#include "types.h"

int gdbmi_parse(char *data, size_t size, char *gui_data, size_t gui_size, struct Command ***com);

#endif /* __GDBMI_PARSE_H__ */
