#include "gdbmi_parse.h"

int gdbmi_parse(char *data, size_t size, char *gui_data, size_t gui_size, struct queue *q) {
    int i;
    for(i = 0; i < size; ++i)
        gui_data[i] = data[i];
    return size;
}
