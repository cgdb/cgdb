#ifndef __A2_TGDB_H__
#define __A2_TGDB_H__

#include <sys/types.h>
#include "types.h"

int a2_tgdb_init(char *debugger, int argc, char **argv, int *gdb, int *child, int *readline);
int a2_tgdb_shutdown(void);
int a2_tgdb_run_command(char *com);
int a2_tgdb_get_source_absolute_filename(char *file);
int a2_tgdb_get_sources(void);
size_t a2_tgdb_recv(char *buf, size_t n, struct Command ***com);
char *a2_tgdb_send(char *line);
int a2_tgdb_send_input(char c);
int a2_tgdb_recv_input(char *buf);
char *a2_tgdb_tty_send(char c);
size_t a2_tgdb_tty_recv(char *buf, size_t n);
int a2_tgdb_new_tty(void);
char *a2_tgdb_tty_name(void);
char *a2_tgdb_err_msg(void);
char *a2_tgdb_get_prompt(void);

#endif /* __A2_TGDB_H__ */
