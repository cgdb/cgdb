#ifndef __GDBMI_PARSE_H__
#define __GDBMI_PARSE_H__

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */

#include "types.h"

int gdbmi_parse(char *data, size_t size, char *gui_data, size_t gui_size, struct queue *q);

#endif /* __GDBMI_PARSE_H__ */
