#ifndef __ANNOTATE_H__
#define __ANNOTATE_H__

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */

#include "tgdb_types.h"
#include "annotate_two.h"

int tgdb_parse_annotation(struct annotate_two *a2,
        char *data, size_t size, struct tgdb_list *list);

#endif
