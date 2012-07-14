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

#include "io.h"
#include "logger.h"

#define MAXLINE 4096

static FILE *dfd = NULL;
static int debug_on = 0;

static const char *debug_begin = "";
static const char *debug_end = "";

static void process_error(void)
{
    if (errno == EINTR)
        logger_write_pos(logger, __FILE__, __LINE__, "ERRNO = EINTR");
    else if (errno == EAGAIN)
        logger_write_pos(logger, __FILE__, __LINE__, "ERRNO = EAGAIN");
    else if (errno == EIO)
        logger_write_pos(logger, __FILE__, __LINE__, "ERRNO = EIO");
    else if (errno == EISDIR)
        logger_write_pos(logger, __FILE__, __LINE__, "ERRNO = EISDIR");
    else if (errno == EBADF)
        logger_write_pos(logger, __FILE__, __LINE__, "ERRNO = EBADF");
    else if (errno == EINVAL)
        logger_write_pos(logger, __FILE__, __LINE__, "ERRNO = EINVAL");
    else if (errno == EFAULT)
        logger_write_pos(logger, __FILE__, __LINE__, "ERRNO = EFAULT");
}

int io_debug_init(const char *filename)
{
    char config_dir[MAXLINE];

    if (filename == NULL)
        return -1;

    strcpy(config_dir, filename);

    if ((dfd = fopen(config_dir, "w")) == NULL) {
        logger_write_pos(logger, __FILE__, __LINE__,
                "could not open debug file");
        return -1;
    }

    /* A nifty trick to have debug dump to the terminal
     * dfd = stderr;*/

    debug_on = 1;

    return 0;
}

void io_debug_write(const char *write)
{
    fprintf(dfd, "%s%s%s", debug_begin, write, debug_end);
    fflush(dfd);
}

void io_debug_write_fmt(const char *fmt, ...)
{
    va_list ap;
    char va_buf[MAXLINE];

    va_start(ap, fmt);
#ifdef   HAVE_VSNPRINTF
    vsnprintf(va_buf, sizeof (va_buf), fmt, ap);    /* this is safe */
#else
    vsprintf(va_buf, fmt, ap);  /* this is not safe */
#endif
    va_end(ap);

    fprintf(dfd, "%s", va_buf);
    fflush(dfd);
}

int io_read_byte(char *c, int source)
{
    int ret_val = 0;

    if ((ret_val = read(source, c, 1)) == 0) {
        return -1;
    } else if (ret_val == -1) {
        logger_write_pos(logger, __FILE__, __LINE__, "I/O error");
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
        logger_write_pos(logger, __FILE__, __LINE__, "I/O error");
        process_error();
        return -1;
    }

    if (write(dest, &c, 1) != 1) {
        logger_write_pos(logger, __FILE__, __LINE__, "I/O error");
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
            logger_write_pos(logger, __FILE__, __LINE__,
                    "error reading from fd");
            return -1;
        } else {
            return 0;           /* Happens on EOF for some reason */
        }

    } else if (amountRead == 0) {   /* EOF */
        return 0;
    } else {
        char *tmp = (char *) buf;

        tmp[amountRead] = '\0';
        if (debug_on == 1) {
            int i;

            fprintf(dfd, "%s", debug_begin);
            for (i = 0; i < amountRead; ++i) {
                if (((char *) buf)[i] == '\r')
                    fprintf(dfd, "(%s)", "\\r");
                else if (((char *) buf)[i] == '\n')
                    fprintf(dfd, "(%s)\n", "\\n");
                else if (((char *) buf)[i] == '\032')
                    fprintf(dfd, "(%s)", "\\032");
                else if (((char *) buf)[i] == '\b')
                    fprintf(dfd, "(%s)", "\\b");
                else
                    fprintf(dfd, "%c", ((char *) buf)[i]);
            }
            fprintf(dfd, "%s", debug_end);
            fflush(dfd);
        }
        return amountRead;

    }
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
        logger_write_pos(logger, __FILE__, __LINE__, "Errno(%d)\n", errno);
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
        logger_write_pos(logger, __FILE__, __LINE__, "Errno(%d)\n", errno);
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
        logger_write_pos(logger, __FILE__, __LINE__, "Errno(%d)\n", errno);
    } else if (ret == 0) {
        c = 0;
        ret = -1;
        logger_write_pos(logger, __FILE__, __LINE__, "Read returned nothing\n");
    }

    /* Set to original state */
    fcntl(fd, F_SETFL, flag);

    if (ret == -1)
        return -1;

    *key = c;
    return 1;
}
