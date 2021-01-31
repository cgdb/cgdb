#ifndef __CGDB_CLOG_H__
#define __CGDB_CLOG_H__

#include <map>
#include <string>

const int CLOG_CGDB_ID = 1;
#define CLOG_CGDB CLOG(CLOG_CGDB_ID)

const int CLOG_GDBIO_ID = 2;
#define CLOG_GDBIO CLOG(CLOG_GDBIO_ID)

const int CLOG_GDBMIIO_ID = 3;
#define CLOG_GDBMIIO CLOG(CLOG_GDBMIIO_ID)

#include "config.h"
#include "fs_util.h"
#include "clog.h"

#define CGDB_CLOG_FORMAT "%d %t %f:%n(%F) %l:%m\n\n"

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
int clog_open(int id, const char *fmt, const char *config_dir);

/**
 * Determine if an error log message was sent to the logger.
 *
 * @return
 * True if an error log message was written to a log file, False otherwise.
 */
bool clog_did_error_occur();

#endif
