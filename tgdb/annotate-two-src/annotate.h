#ifndef __ANNOTATE_H__
#define __ANNOTATE_H__

#include <sys/types.h>
#include "types.h"

int tgdb_parse_annotation(char *data, size_t size, struct Command ***com);

#endif
