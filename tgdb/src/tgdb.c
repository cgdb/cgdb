#include "tgdb.h"
#include "a2-tgdb.h"

int tgdb_init(void) {
    tgdb_start                          = a2_tgdb_init;
    tgdb_send                           = a2_tgdb_send;
    tgdb_tty_send                       = a2_tgdb_tty_send;
    tgdb_recv                           = a2_tgdb_recv;
    tgdb_tty_recv                       = a2_tgdb_tty_recv;
    tgdb_new_tty                        = a2_tgdb_new_tty;
    tgdb_tty_name                       = a2_tgdb_tty_name;
    tgdb_run_command                    = a2_tgdb_run_command;
    tgdb_get_sources                    = a2_tgdb_get_sources;
    tgdb_get_source_absolute_filename   = a2_tgdb_get_source_absolute_filename;
    tgdb_err_msg                        = a2_tgdb_err_msg;
    tgdb_shutdown                       = a2_tgdb_shutdown;
    return 0;
}
