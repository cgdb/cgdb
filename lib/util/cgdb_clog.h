#ifndef __CGDB_CLOG_H__
#define __CGDB_CLOG_H__

const int TGDB_LOGGER = 2;

#include "fs_util.h"
#include "clog.h"

/**
 * Attempt to open the tgdb log file.
 *
 * @param id
 * The clog id
 *
 * @param fmt
 * The filename format
 *
 * @param config_dir
 * The directory to open the file in
 *
 * @return
 * 0 on success, -1 on failure
 */
inline int clog_open(int id, const char *fmt, const char *config_dir)
{
    int i;

    /* Try to open a log file exclusively. This allows us to run
     * several instances of cgdb without the logfiles getting borked. */
    for (i = 1; i < 100; i++)
    {
        char filename[FSUTIL_PATH_MAX];

        /* Initialize the debug file that a2_tgdb writes to */
        snprintf(filename, sizeof(filename), fmt, config_dir, i);

        if (clog_init_path(id, filename) == 0)
            return 0;
    }

    return -1;
}


#endif
