#ifndef __A2_TGDB_H__
#define __A2_TGDB_H__

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */

#include "types.h"

int a2_tgdb_init(char *debugger, int argc, char **argv, int *gdb, int *child, int *readline);
int a2_tgdb_shutdown(void);
int a2_tgdb_get_source_absolute_filename(char *file);
int a2_tgdb_get_sources(void);
size_t a2_tgdb_recv(char *buf, size_t n, struct queue *q);
char *a2_tgdb_send(char *command, int out_type);
int a2_tgdb_send_input(char c);
int a2_tgdb_recv_input(char *buf);
char *a2_tgdb_tty_send(char c);
size_t a2_tgdb_tty_recv(char *buf, size_t n);
int a2_tgdb_new_tty(void);
char *a2_tgdb_tty_name(void);
char *a2_tgdb_err_msg(void);

/* These are called from within annotate-two-src for now 
 * A better approach will be given in the future.
 */
int a2_tgdb_change_prompt(char *prompt);

int tgdb_setup_buffer_command_to_run(
      struct queue *q,
      char *com, 
      enum buffer_command_type com_type,        /* Who is running this command */
      enum buffer_output_type out_type,         /* Where should the output go */
      enum buffer_command_to_run com_to_run);   /* What command to run */

#endif /* __A2_TGDB_H__ */
