#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "io.h"
#include "error.h"
#include "globals.h"
#include "types.h"
#include "macro.h"

static FILE *dfd = NULL;
static int debug_on = 0;

static const char *debug_begin = "";
static const char *debug_end   = "";

static void process_error(void) {
   if(errno == EINTR)
      err_ret("%s:%d -> ERRNO = EINTR", __FILE__, __LINE__);
   else if(errno == EAGAIN)
      err_ret("%s:%d -> ERRNO = EAGAIN", __FILE__, __LINE__);
   else if(errno == EIO)
      err_ret("%s:%d -> ERRNO = EIO", __FILE__, __LINE__);
   else if(errno == EISDIR)
      err_ret("%s:%d -> ERRNO = EISDIR", __FILE__, __LINE__);
   else if(errno == EBADF)
      err_ret("%s:%d -> ERRNO = EBADF", __FILE__, __LINE__);
   else if(errno == EINVAL)
      err_ret("%s:%d -> ERRNO = EINVAL", __FILE__, __LINE__);
   else if(errno == EFAULT)
      err_ret("%s:%d -> ERRNO = EFAULT", __FILE__, __LINE__);
}

int io_debug_init(const char *filename){
   char config_dir[MAXLINE];
   if ( filename == NULL )
      global_get_config_gdb_debug_file(config_dir);
   else
      strcpy( config_dir, filename );
   
   
   if( (dfd = fopen(config_dir, "w")) == NULL) {
      err_msg("%s:%d -> could not open debug file", __FILE__, __LINE__);
      return -1;
   }

   debug_on = 1;

   return 0;
}

void io_debug_write(const char *write){
   fprintf(dfd, "%s%s%s", debug_begin, write, debug_end);
   fflush(dfd);
} 

void io_debug_write_fmt(const char *fmt, ...) {
    va_list ap;
    char va_buf[MAXLINE];

    va_start(ap, fmt);
#ifdef   HAVE_VSNPRINTF
    vsnprintf(va_buf, sizeof(va_buf), fmt, ap);  /* this is safe */
#else
    vsprintf(va_buf, fmt, ap);             /* this is not safe */
#endif
    va_end(ap);

    fprintf(dfd, "%s", va_buf);
    fflush(dfd);
}

int io_read_byte(char *c, int source){
   int ret_val = 0;
   
   if( ( ret_val = read(source, c, 1)) == 0) {
      return -1;
   } else if ( ret_val == -1 ) {
      err_msg("%s:%d -> I/O error", __FILE__, __LINE__); process_error();
      return -1;
   } 

   return 0;
}

int io_write_byte(int dest, char c){
    macro_write_char(c);
   
   if(write(dest, &c, 1) != 1)
      return -1;

   return 0;   
}

int io_rw_byte(int source, int dest){
   char c;
   
   if(read(source, &c, 1) != 1){
      err_msg("%s:%d -> I/O error", __FILE__, __LINE__);
      process_error();
      err_ret("source bad\n");
      return -1;
   }
      
   if(write(dest, &c, 1) != 1){
      err_msg("dest bad\n");
      return -1;
   }

   return 0;      
}

ssize_t io_read(int fd, void *buf, size_t count){
   ssize_t amountRead;
   tgdb_read:
   
   if( (amountRead = read(fd, buf, count)) == -1){ /* error */
      if(errno == EINTR)
         goto tgdb_read;
      else if ( errno != EIO ) {
         err_ret("%s:%d -> error reading from fd", __FILE__, __LINE__);
         return -1;
      } else {
         return 0; /* Happens on EOF for some reason */
      }

   } else if(amountRead == 0){ /* EOF */
      return 0;
   } else {
      char *tmp = (char *)buf;
      tmp[amountRead] = '\0';
      if(debug_on == 1){
         int i;
         fprintf(dfd, "%s", debug_begin);
         for(i =0 ; i < amountRead; ++i) {
            if(((char *)buf)[i] == '\r')
               fprintf(dfd, "(%s)", "\\r");
            else if(((char *)buf)[i] == '\n')
               fprintf(dfd, "(%s)\n", "\\n");
            else if(((char *)buf)[i] == '\032')
               fprintf(dfd, "(%s)", "\\032");
            else if(((char *)buf)[i] == '\b')
               fprintf(dfd, "(%s)", "\\b");
            else
               fprintf(dfd, "%c", ((char *)buf)[i]);
         }
         fprintf(dfd, "%s", debug_end);
         fflush(dfd);
      }
      return amountRead;

   }
}

ssize_t io_writen(int fd, const void *vptr, size_t n) {
   ssize_t nwritten;
   size_t nleft = n;
   const char *ptr = (const char *)vptr;

   macro_write_str((char *)vptr);

   while(nleft > 0) {
      if( (nwritten = write( fd, ptr, nleft)) <= 0) {
         if(errno == EINTR)
            nwritten = 0;
         else
            return -1;
      } // end if
      nleft -= nwritten;
      ptr += nwritten;
   } // end while

   return (n);
} // end writen

void io_display_char(FILE *fd, char c){
   if(c == '\r')
      fprintf(fd, "(%s)", "\\r");
   else if(c == '\n')
      fprintf(fd, "(%s)\n", "\\n");
   else if(c == '\032')
      fprintf(fd, "(%s)", "\\032");
   else if(c == '\b')
      fprintf(fd, "(%s)", "\\b");
   else
      fprintf(fd, "(%c)", c);

   fflush(fd);
}
