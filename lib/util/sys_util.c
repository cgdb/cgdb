#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_STDIO_H
#include <stdio.h>
#endif /* HAVE_STDIO_H */

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */

#if HAVE_ERRNO_H
#include <errno.h>
#endif /* HAVE_ERRNO_H */

#include "sys_util.h"

void *cgdb_calloc(size_t nmemb, size_t size)
{
    void *t = calloc(nmemb, size);

    if (t)
        return t;

    exit(-1);
}

void *cgdb_malloc(size_t size)
{
    void *t = malloc(size);

    if (t)
        return t;

    exit(-1);
}

void *cgdb_realloc(void *ptr, size_t size)
{
    void *t = realloc(ptr, size);

    if (t)
        return t;

    exit(-1);
}

char *cgdb_strdup(const char *s)
{
    char *t = strdup(s);

    if (t)
        return t;

    exit(-1);
}

int cgdb_close(int fd)
{
    int ret;

  cgdb_close_start:
    if ((ret = close(fd)) == -1 && errno == EINTR)
        goto cgdb_close_start;
    else if (ret == -1)
        exit(-1);

    return 0;
}

int cgdb_string_to_int(char *str, int *num) {
    int result = -1;

    if (str && num) {
        long int strtol_result;
        char *end_ptr;

        errno = 0;
        strtol_result = strtol(str, &end_ptr, 10);
        if (errno == 0 && str != end_ptr && *end_ptr == '\0') {
            int convert_to_int_result = (int)strtol_result;
            if (convert_to_int_result == strtol_result) {
                *num = convert_to_int_result;
                result = 0;
            }
        }
    }

    return result;
}

int cgdb_is_debugger_attached()
{
#ifdef HAVE_PROC_SELF_STATUS_FILE
    int debugger_attached = 0;
    static const char TracerPid[] = "TracerPid:";

    FILE *fp = fopen("/proc/self/status", "r");
    if ( fp ) {
        ssize_t chars_read;
        size_t line_len = 0;
        char *line = NULL;

        while ((chars_read = getline(&line, &line_len, fp)) != -1) {
            char *tracer_pid = strstr(line, TracerPid);

            if (tracer_pid) {
                debugger_attached = !!atoi(tracer_pid + sizeof(TracerPid) - 1);
                break;
            }
        }

        free(line);
        fclose(fp);
    }

    return debugger_attached;
#else
    /* TODO: Implement for other platforms. */
    return -1;
#endif
}
