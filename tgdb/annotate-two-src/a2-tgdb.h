#ifndef __A2_TGDB_H__
#define __A2_TGDB_H__

#include <sys/types.h>
#include "types.h"

extern int a2_tgdb_init(char *debugger, int argc, char **argv, int *gdb, int *child);
extern int a2_tgdb_shutdown(void);
extern int a2_tgdb_run_command(char *com);
extern int a2_tgdb_get_source_absolute_filename(char *file);
extern int a2_tgdb_get_sources(void);
extern size_t a2_tgdb_recv(char *buf, size_t n, struct Command ***com);
extern char *a2_tgdb_send(char c);
extern char *a2_tgdb_tty_send(char c);
extern size_t a2_tgdb_tty_recv(char *buf, size_t n);
extern int a2_tgdb_new_tty(void);
extern char *a2_tgdb_tty_name(void);
extern char *a2_tgdb_err_msg(void);

#endif /* __A2_TGDB_H__ */
