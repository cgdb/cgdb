#include "tgdb.h"
#include "a2-tgdb.h"
#include "gdbmi_tgdb.h"

int tgdb_init(void) {
    if ( 1 ) {
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
        tgdb_get_prompt                     = a2_tgdb_get_prompt;
    } else {
        tgdb_start                          = gdbmi_tgdb_init;
        tgdb_send                           = gdbmi_tgdb_send;
        tgdb_tty_send                       = gdbmi_tgdb_tty_send;
        tgdb_recv                           = gdbmi_tgdb_recv;
        tgdb_tty_recv                       = gdbmi_tgdb_tty_recv;
        tgdb_new_tty                        = gdbmi_tgdb_new_tty;
        tgdb_tty_name                       = gdbmi_tgdb_tty_name;
        tgdb_run_command                    = gdbmi_tgdb_run_command;
        tgdb_get_sources                    = gdbmi_tgdb_get_sources;
        tgdb_get_source_absolute_filename   = gdbmi_tgdb_get_source_absolute_filename;
        tgdb_err_msg                        = gdbmi_tgdb_err_msg;
        tgdb_shutdown                       = gdbmi_tgdb_shutdown;
        tgdb_get_prompt                     = gdbmi_tgdb_get_prompt;
    }
    return 0;
}
