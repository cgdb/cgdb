#include "cgdb_clog.h"

std::map<int, std::string> loggerNameMap;

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
            loggerNameMap[id] = filename;
            return 0;
        }
    }

    return -1;
}

const char *clog_filename(int id) {
    const char *result = NULL;

    std::map<int, std::string>::iterator iter =
        loggerNameMap.find(id);
    if (iter != loggerNameMap.end()) {
        result = iter->second.c_str();
    }
        
    return result;
}
