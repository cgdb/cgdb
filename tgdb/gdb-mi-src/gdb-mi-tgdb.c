#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include <signal.h>

#include "gdb-mi-tgdb.h"

int gdb_mi_tgdb_init(char *debugger, int argc, char **argv, int *gdb, int *child){
    return 0;
}

int gdb_mi_tgdb_shutdown(void){
    return 0;
}

int gdb_mi_tgdb_run_command(char *com){
    return 0;
}

int gdb_mi_tgdb_get_source_absolute_filename(char *file){
    return val;
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
