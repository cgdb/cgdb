#include "cgdb_clog.h"

/**
 * Modified clog's clog_error function to set this variable.
 */
bool clog_cgdb_error_occurred = false;

int clog_open(int id, const char *fmt, const char *config_dir)
{
    int i;

    /* Try to open a log file exclusively. This allows us to run
     * several instances of cgdb without the logfiles getting borked. */
    for (i = 1; i < 100; i++)
    {
        char filename[FSUTIL_PATH_MAX];

        /* Initialize the debug file that a2_tgdb writes to */
        snprintf(filename, sizeof(filename), fmt, config_dir, i);

        if (clog_init_path(id, filename) == 0) {
            return 0;
        }
    }

    return -1;
}

bool clog_did_error_occur()
{
    return clog_cgdb_error_occurred;
}
