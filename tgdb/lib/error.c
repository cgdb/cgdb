#include <string.h>
#include <stdio.h>
#include <stdarg.h>     /* ANSI C header file */
#include <stdlib.h>     /* exit ... */
#include <errno.h>      /* errno */
#include "error.h"

#define MAXLINE 4096

static char err_buf[MAXLINE], err_buf_internal[MAXLINE];

static void err_doit(int errnoflag, const char *fmt, va_list ap);

/* Nonfatal error related to a system call.
 * Print a message and return. */
void err_ret(const char *fmt, ...) {
   va_list     ap;

   va_start(ap, fmt);
   err_doit(1, fmt, ap);
   va_end(ap);
   return;
}

/* Fatal error related to a system call.
 * Print a message and terminate. */
void err_sys(const char *fmt, ...) {
   va_list     ap;

   va_start(ap, fmt);
   err_doit(1, fmt, ap);
   va_end(ap);
   exit(1);
}

/* Fatal error related to a system call.
 * Print a message, dump core, and terminate. */
void err_dump(const char *fmt, ...) {
   va_list     ap;

   va_start(ap, fmt);
   err_doit(1, fmt, ap);
   va_end(ap);
   abort();    /* dump core and terminate */
   exit(1);    /* shouldn't get here */
}

/* Nonfatal error unrelated to a system call.
 * Print a message and return. */
void err_msg(const char *fmt, ...) {
   va_list     ap;

   va_start(ap, fmt);
   err_doit(0, fmt, ap);
   va_end(ap);
   return;
}

/* Fatal error unrelated to a system call.
 * Print a message and terminate. */
void err_quit(const char *fmt, ...) {
   va_list     ap;

   va_start(ap, fmt);
   err_doit(0, fmt, ap);
   va_end(ap);
   exit(1);
}

/* Print a message and return to caller.
 * Caller specifies "errnoflag" and "level". */
static void err_doit(int errnoflag, const char *fmt, va_list ap) {
   int      errno_save, n;
   char  buf[MAXLINE];

   errno_save = errno;     /* value caller might want printed */
#ifdef   HAVE_VSNPRINTF
   vsnprintf(buf, sizeof(buf), fmt, ap);  /* this is safe */
#else
   vsprintf(buf, fmt, ap);             /* this is not safe */
#endif
   n = strlen(buf);
   if (errnoflag)
      snprintf(buf+n, sizeof(buf)-n, ": %s", strerror(errno_save));
   strcat(buf, "\n");

   if ( strlen(err_buf_internal) + n < MAXLINE )
      strcat(err_buf_internal, buf);
   
   /*************************
    * Change to see on stdout
    *************************/
    /* in case stdout and stderr are the same */
   fflush(stdout);      
   fputs(buf, stderr);
   fflush(stderr);

   return;
}

char *err_get(void) {
   /* Return a pointer to a buffer, and clear the internal buffer */
   strncpy(err_buf, err_buf_internal, strlen(err_buf_internal));
   err_buf_internal[0] = '\0';
   return err_buf;
}
