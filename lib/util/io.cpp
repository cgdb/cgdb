#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_STDIO_H
#include <stdio.h>
#endif /* HAVE_STDIO_H */

#if HAVE_STDARG_H
#include <stdarg.h>
#endif /* HAVE_STDARG_H */

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif /* HAVE_STDLIB_H */

#if HAVE_ERRNO_H
#include <errno.h>
#endif /* HAVE_ERRNO_H */

#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif /* HAVE_FCNTL_H */

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif /* HAVE_SYS_SELECT_H */

#if HAVE_ERRNO_H
#include <errno.h>
#endif /* HAVE_ERRNO_H */

#include "sys_util.h"
#include "cgdb_clog.h"
#include "io.h"

static void process_error(void)
{
    if (errno == EINTR)
        clog_error(CLOG_CGDB, "ERRNO = EINTR");
    else if (errno == EAGAIN)
        clog_error(CLOG_CGDB, "ERRNO = EAGAIN");
    else if (errno == EIO)
        clog_error(CLOG_CGDB, "ERRNO = EIO");
    else if (errno == EISDIR)
        clog_error(CLOG_CGDB, "ERRNO = EISDIR");
    else if (errno == EBADF)
        clog_error(CLOG_CGDB, "ERRNO = EBADF");
    else if (errno == EINVAL)
        clog_error(CLOG_CGDB, "ERRNO = EINVAL");
    else if (errno == EFAULT)
        clog_error(CLOG_CGDB, "ERRNO = EFAULT");
}

int io_read_byte(char *c, int source)
{
    int ret_val = 0;

    if ((ret_val = read(source, c, 1)) == 0) {
        return -1;
    } else if (ret_val == -1) {
        clog_error(CLOG_CGDB, "I/O error");
        process_error();
        return -1;
    }

    return 0;
}

int io_write_byte(int dest, char c)
{
    if (write(dest, &c, 1) != 1)
        return -1;

    return 0;
}

int io_rw_byte(int source, int dest)
{
    char c;

    if (read(source, &c, 1) != 1) {
        clog_error(CLOG_CGDB, "I/O error");
        process_error();
        return -1;
    }

    if (write(dest, &c, 1) != 1) {
        clog_error(CLOG_CGDB, "I/O error");
        return -1;
    }

    return 0;
}

ssize_t io_read(int fd, void *buf, size_t count)
{
    ssize_t amountRead;

  tgdb_read:

    if ((amountRead = read(fd, buf, count)) == -1) {    /* error */
        if (errno == EINTR)
            goto tgdb_read;
        else if (errno != EIO) {
            clog_error(CLOG_CGDB, "error reading from fd");
            return -1;
        } else {
            return 0;           /* Happens on EOF for some reason */
        }

    } else if (amountRead == 0) {   /* EOF */
        return 0;
    } else {
        char *str = sys_quote_nonprintables((char *)buf, amountRead);
        clog_debug(CLOG_GDBIO, "%s", str);
        sbfree(str);
    }

    return amountRead;
}

ssize_t io_writen(int fd, const void *vptr, size_t n)
{
    ssize_t nwritten;
    size_t nleft = n;
    const char *ptr = (const char *) vptr;

    while (nleft > 0) {
        if ((nwritten = write(fd, ptr, nleft)) <= 0) {
            if (errno == EINTR)
                nwritten = 0;
            else
                return -1;
        }
        nleft -= nwritten;
        ptr += nwritten;
    }

    return (n);
}

void io_display_char(FILE * fd, char c)
{
    if (c == '\r')
        fprintf(fd, "(%s)", "\\r");
    else if (c == '\n')
        fprintf(fd, "(%s)\n", "\\n");
    else if (c == '\032')
        fprintf(fd, "(%s)", "\\032");
    else if (c == '\b')
        fprintf(fd, "(%s)", "\\b");
    else
        fprintf(fd, "(%c)", c);

    fflush(fd);
}

int io_data_ready(int fd, int ms)
{
    int ret;

#if defined(HAVE_SYS_SELECT_H)
    fd_set readfds, exceptfds;
    struct timeval timeout;
    struct timeval *timeout_ptr = &timeout;

    FD_ZERO(&readfds);
    FD_ZERO(&exceptfds);
    FD_SET(fd, &readfds);
    FD_SET(fd, &exceptfds);

    timeout.tv_sec = ms / 1000;
    timeout.tv_usec = (ms % 1000) * 1000;

    /* Enforce blocking semantics if the user requested it */
    if (ms == -1)
        timeout_ptr = NULL;

    ret = select(fd + 1, &readfds, (fd_set *) NULL, &exceptfds, timeout_ptr);
    if (ret == -1) {
        clog_error(CLOG_CGDB, "Errno(%d)\n", errno);
        return -1;
    }

    if (ret <= 0)
        return 0;               /* Nothing to read. */
    else
        return 1;
#endif
}

int io_getchar(int fd, unsigned int ms, int *key)
{
    char c;
    int ret;
    int flag = 0;
    int val;

    if (!key)
        return -1;

    val = io_data_ready(fd, ms);
    if (val == -1) {
        clog_error(CLOG_CGDB, "Errno(%d)\n", errno);
        return -1;
    }

    if (val == 0)
        return 0;               /* Nothing to read. */

    /* Set nonblocking */
    flag = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flag | O_NONBLOCK);

  read_again:

    /* Read a char */
    ret = read(fd, &c, 1);

    if (ret == -1 && errno == EAGAIN)
        c = 0;                  /* No data available */
    else if (ret == -1 && errno == EINTR)
        goto read_again;
    else if (ret == -1) {
        c = 0;
        clog_error(CLOG_CGDB, "Errno(%d)\n", errno);
    } else if (ret == 0) {
        c = 0;
        ret = -1;
        clog_error(CLOG_CGDB, "Read returned nothing\n");
    }

    /* Set to original state */
    fcntl(fd, F_SETFL, flag);

    if (ret == -1)
        return -1;

    *key = c;
    return 1;
}
