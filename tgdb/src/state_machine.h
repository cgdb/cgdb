#ifndef __STATE_MACHINE_H__
#define __STATE_MACHINE_H__

#include <sys/types.h>
#include "types.h"

int tgdb_handle_data(char *data, size_t size, char *gui_data, size_t gui_size, struct Command ***com);

#endif
