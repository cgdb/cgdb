#ifndef __GDBMI_TGDB_H__
#define __GDBMI_TGDB_H__

#include <sys/types.h>
#include "types.h"

int gdbmi_tgdb_init(char *debugger, int argc, char **argv, int *gdb, int *child, int *readline);
int gdbmi_tgdb_shutdown(void);
int gdbmi_tgdb_run_command(char *com);
int gdbmi_tgdb_get_source_absolute_filename(char *file);
int gdbmi_tgdb_get_sources(void);
size_t gdbmi_tgdb_recv(char *buf, size_t n, struct Command ***com);
char *gdbmi_tgdb_send(char *c);
int gdbmi_tgdb_send_input(char c);
int gdbmi_tgdb_recv_input(char *buf);
char *gdbmi_tgdb_tty_send(char c);
size_t gdbmi_tgdb_tty_recv(char *buf, size_t n);
int gdbmi_tgdb_new_tty(void);
char *gdbmi_tgdb_tty_name(void);
char *gdbmi_tgdb_err_msg(void);
char *gdbmi_tgdb_get_prompt(void);

#endif /* __GDBMI_TGDB_H__ */
