#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include <signal.h>

#include "gdbmi_tgdb.h"
#include "gdbmi_init.h"
#include "tgdb_util.h"
#include "util.h"
#include "pseudo.h"
#include "error.h"

static int gdb_stdin = 0; /* Writing to this writes to gdb's stdin */
static pid_t gdb_pid = 0;

static int master_tty_fd = -1, slave_tty_fd = -1;
static char child_tty_name[SLAVE_SIZE];  /* the name of the slave psuedo-termainl */

int gdb_mi_tgdb_init(char *debugger, int argc, char **argv, int *gdb, int *child){
    if(( gdb_pid = invoke_debugger(debugger, argc, argv, &gdb_stdin, gdb)) == -1 ) {
        err_msg("(%s:%d) invoke_debugger failed", __FILE__, __LINE__);
        return -1;
    }

   if ( tgdb_util_new_tty(&master_tty_fd, &slave_tty_fd, child_tty_name) == -1){
      err_msg("%s:%d tgdb_util_new_tty error", __FILE__, __LINE__);
      return -1;
   }

   *child   = master_tty_fd;

   return 0;
}

int gdb_mi_tgdb_shutdown(void){
   /* tty for gdb child */
   xclose(master_tty_fd);
   xclose(slave_tty_fd);
   if ( pty_release(child_tty_name) == -1 ) {
      err_msg("%s:%d pty_release error", __FILE__, __LINE__);
      return -1;
   }

   return 0;
}

int gdb_mi_tgdb_run_command(char *com){
    return 0;
}

int gdb_mi_tgdb_get_source_absolute_filename(char *file){
    return 0;
}

int gdb_mi_tgdb_get_sources(void){
   return 0;
}

size_t gdb_mi_tgdb_recv(char *buf, size_t n, struct Command ***com){
    return 0;
}

char *gdb_mi_tgdb_send(char c){
    return (char*)0;
}

char *gdb_mi_tgdb_tty_send(char c){
    return (char *)0;
}

size_t gdb_mi_tgdb_tty_recv(char *buf, size_t n){
   return 0; 
}

int gdb_mi_tgdb_new_tty(void) {
   return 0;
}

char *gdb_mi_tgdb_tty_name(void) {
    return (char *)0;
}

char *gdb_mi_tgdb_err_msg(void) {
   return (char *)0;
}
