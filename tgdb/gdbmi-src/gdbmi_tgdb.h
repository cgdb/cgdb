#ifndef __A2_TGDB_H__
#define __A2_TGDB_H__

#include <sys/types.h>
#include "types.h"

int gdb_mi_tgdb_init(char *debugger, int argc, char **argv, int *gdb, int *child);
int gdb_mi_tgdb_shutdown(void);
int gdb_mi_tgdb_run_command(char *com);
int gdb_mi_tgdb_get_source_absolute_filename(char *file);
int gdb_mi_tgdb_get_sources(void);
size_t gdb_mi_tgdb_recv(char *buf, size_t n, struct Command ***com);
char *gdb_mi_tgdb_send(char c);
char *gdb_mi_tgdb_tty_send(char c);
size_t gdb_mi_tgdb_tty_recv(char *buf, size_t n);
int gdb_mi_tgdb_new_tty(void);
char *gdb_mi_tgdb_tty_name(void);
char *gdb_mi_tgdb_err_msg(void);

#endif /* __A2_TGDB_H__ */
