#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>

#include "gdbmi_tgdb.h"
#include "gdbmi_parse.h"
#include "sys_util.h"
#include "fork_util.h"
#include "fs_util.h"
#include "pseudo.h"
#include "error.h"
#include "io.h"

//static int gdb_stdin = 0;   /* Writing to this writes to gdb's stdin */
//static int gdb_stdout = 0;  /* Reading from this read's from gdb's stdout && stderr */
//static pid_t gdb_pid = 0;

static int master_tty_fd = -1, slave_tty_fd = -1;
static char child_tty_name[SLAVE_SIZE];  /* the name of the slave psuedo-termainl */

int gdbmi_tgdb_init(int *readline){
//    if(( gdb_pid = invoke_debugger(debugger, argc, argv, &gdb_stdin, &gdb_stdout, 1)) == -1 ) {
//        err_msg("(%s:%d) invoke_debugger failed", __FILE__, __LINE__);
//        return -1;
//    }
//
//   if ( util_new_tty(&master_tty_fd, &slave_tty_fd, child_tty_name) == -1){
//      err_msg("%s:%d tgdb_util_new_tty error", __FILE__, __LINE__);
//      return -1;
//   }
//
//   *gdb       = gdb_stdout;
//   *child     = master_tty_fd;

   return 0;
}

int gdbmi_tgdb_shutdown(void){
   /* tty for gdb child */
   xclose(master_tty_fd);
   xclose(slave_tty_fd);
   if ( pty_release(child_tty_name) == -1 ) {
      err_msg("%s:%d pty_release error", __FILE__, __LINE__);
      return -1;
   }

   return 0;
}

int gdbmi_tgdb_run_command(char *com){
    return 0;
}

int gdbmi_tgdb_get_source_absolute_filename(char *file){
    return 0;
}

int gdbmi_tgdb_get_sources(void){
   return 0;
}

size_t gdbmi_tgdb_recv(char *buf, size_t n, struct queue *q){
   return n;
}

char *gdbmi_tgdb_send(char *command, int out_type) {
    static char buf[4];
//    memset(buf, '\0', 4); 
//    buf[0] = c;
//
//    if(io_write_byte(gdb_stdin, c) == -1){
//        err_ret("%s:%d io_write_byte error", __FILE__, __LINE__);
//        return NULL;
//    }
//
//    /* Ask for the file name after every command */
//    if ( c == '\n' ) {
//        char *str = "-interpreter-exec console \"info source\"\n";
//        io_writen(gdb_stdin, strlen(str) , str);
//    }
    
    return buf;   
}

int gdbmi_tgdb_send_input(char c){
    return 0;
}

int gdbmi_tgdb_recv_input(char *buf){
    return 0;
}

char *gdbmi_tgdb_tty_send(char c){
    return (char *)0;
}

size_t gdbmi_tgdb_tty_recv(char *buf, size_t n){
   return 0; 
}

int gdbmi_tgdb_new_tty(void) {
   return 0;
}

char *gdbmi_tgdb_tty_name(void) {
    return (char *)0;
}


char *gdbmi_tgdb_err_msg(void) {
   return (char *)0;
}

char *gdbmi_tgdb_return_client_command ( enum tgdb_command_type c ) {
	return (char *)0;
}

char *gdbmi_tgdb_client_modify_breakpoint ( const char *file, int line, enum tgdb_breakpoint_action b ) {
	return (char *)0;
}

