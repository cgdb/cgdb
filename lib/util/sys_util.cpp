#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_STDIO_H
#include <stdio.h>
#endif /* HAVE_STDIO_H */

#if HAVE_STDARG_H
#include <stdarg.h>
#endif /* HAVE_STDARG_H */

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */

#if HAVE_CTYPE_H
#include <ctype.h>
#endif

#if HAVE_ERRNO_H
#include <errno.h>
#endif /* HAVE_ERRNO_H */

#define CLOG_MAIN
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

int cgdb_string_to_int(const char *str, int *num) {
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

int cgdb_hexstr_to_u64(const char *str, uint64_t *num)
{
    int result = -1;

    if (str && num) {
        uint64_t strtoull_result;
        char *end_ptr;

        errno = 0;
        strtoull_result = strtoull(str, &end_ptr, 16);
        if (errno == 0 && str != end_ptr &&
            (*end_ptr == '\0' || *end_ptr == ' ')) {
            *num = strtoull_result;
            result = 0;
        }
    }

    return result;
}

int cgdb_supports_debugger_attach_detection()
{
#ifdef HAVE_PROC_SELF_STATUS_FILE
    return 1;
#else
    return 0;
#endif
}

int cgdb_is_debugger_attached()
{
    int result;
    if (cgdb_supports_debugger_attach_detection()) {
        int debugger_attached = 0;
        static const char TracerPid[] = "TracerPid:";

        FILE *fp = fopen("/proc/self/status", "r");
        if ( fp ) {
            size_t line_len = 0;
            char *line = NULL;

            while (getline(&line, &line_len, fp) != -1) {
                char *tracer_pid = strstr(line, TracerPid);

                if (tracer_pid) {
                    debugger_attached = !!atoi(tracer_pid + sizeof(TracerPid) - 1);
                    break;
                }
            }

            free(line);
            fclose(fp);
        }

        result = debugger_attached;
    } else {
        /* TODO: Implement for other platforms. */
        result = -1;
    }

    return result;
}

long get_file_size(FILE *file)
{
    if (fseek(file, 0, SEEK_END) != -1) {
        long size;

        size = ftell(file);
        fseek(file, 0, SEEK_SET);

        return size;
    }

    return -1;
}

int log10_uint(unsigned int val)
{
    if (val >= 1000000000u) return 9;
    if (val >= 100000000u) return 8;
    if (val >= 10000000u) return 7;
    if (val >= 1000000u) return 6;
    if (val >= 100000u) return 5;
    if (val >= 10000u) return 4;
    if (val >= 1000u) return 3;
    if (val >= 100u) return 2;
    if (val >= 10u) return 1;
    return 0;
}

char *sys_aprintf(const char *fmt, ...)
{
    int n;
    va_list ap;

    va_start(ap, fmt);
    n = vsnprintf(NULL, 0, fmt, ap) + 1;
    va_end(ap);

    if (n > 0 ) {
        char *str = (char *)cgdb_malloc(n);

        va_start(ap, fmt);
        vsnprintf(str, n, fmt, ap);
        va_end(ap);

        return str;
    }

    return NULL;
}

std::string sys_quote_nonprintables(const char *str, int len)
{
    int i;
    std::string ret;

    if (len == -1)
        len = strlen(str);

    for (i = 0; i < len; ++i)
    {
        const char *ch = NULL;

        if (str[i] == '\r')
            ch = "\\r";
        else if (str[i] == '\n')
            ch = "\\n";
        else if (str[i] == '\032')
            ch = "\\032";
        else if (str[i] == '\033')
            ch = "\\033";
        else if (str[i] == '\b')
            ch = "\\b";
        else if (str[i] == '\t')
            ch = "\\t";

        if (ch) {
            ret.push_back('(');
            ret.append(ch);
            ret.push_back(')');
        } else
            ret.push_back(str[i]);
    }

    return ret;
}
