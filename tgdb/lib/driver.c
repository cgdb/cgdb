/* From configure */
#include "config.h"

/* Library includes */
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <sys/ioctl.h>

/* Local includes */
#include "tgdb.h"
#include "error.h"
#include "io.h"
#include "terminal.h"

void mainLoop(int masterfd, int childfd, int readlinefd){
  int    max;
  fd_set rfds;
  int result;

   /* get max fd  for select loop */
   max = (masterfd > STDIN_FILENO) ? masterfd : STDIN_FILENO;
   max = (max > childfd) ? max : childfd;
   max = (max > readlinefd) ? max : readlinefd;
   
   while(1){
      /* Clear the set and 
       *
       * READ FROM:
       * stdin          (user or gui ... who is the user anyway 
       * master         (gdb's stdout)
       * gui_stdout     (gui's stdout sending new info)
       *
       */
      FD_ZERO(&rfds);
      
      FD_SET(STDIN_FILENO, &rfds);
      FD_SET(masterfd, &rfds);
      FD_SET(childfd, &rfds);
      FD_SET(readlinefd, &rfds);
      
      result = select(max + 1, &rfds, NULL, NULL, NULL);
      
      /* if the signal interuppted system call keep going */
      if(result == -1 && errno == EINTR)
         continue;
      else if(result == -1) /* on error ... must die -> stupid OS */
         err_dump("main.c:mainLoop - select failed\n");

      /* stdin -> gdb's input */
      if(STDIN_FILENO != -1 && FD_ISSET(STDIN_FILENO, &rfds)){
         static char cur_com[MAXLINE];
         static int cur_com_pos = 0;

         static char command[MAXLINE + 1];
         static char *result;
         ssize_t size;
         int i;

         if( ( size = io_read(STDIN_FILENO, &command, MAXLINE)) < 0 ){
            err_msg("%s:%d -> could not read byte\n", __FILE__, __LINE__);
            return;
         } /* end if */
            
         for ( i = 0; i < size; i++ ) {
            if ( command[i] == '\n' ) {
                 cur_com[cur_com_pos] = '\0';
                 if( (result = tgdb_send(cur_com)) == NULL){
                     err_msg("%s:%d -> file descriptor closed\n", __FILE__, __LINE__);
                     return;
                 } else
                     fprintf(stderr, "%s", result);
                 cur_com_pos = 0;
            } else {
                 cur_com[cur_com_pos++] = command[i];
                 if ( tgdb_send_input(command[i])  == -1 ) {
                     err_msg("%s:%d -> tgdb_send_input error\n", __FILE__, __LINE__);
                     return;
                 }
            }
         }
      } /* end if */
              
      /* gdb's output -> stdout */
      if(masterfd != -1 && FD_ISSET(masterfd, &rfds)){
         char buf[MAXLINE];
         struct Command **com;
         size_t size;
         size_t i;
         
         if( (size = tgdb_recv(buf, MAXLINE, &com)) == -1){
            err_msg("%s:%d -> file descriptor closed\n", __FILE__, __LINE__);
            return;
         } /* end if */

         for(i = 0; i < size; ++i)
            /*fprintf(stderr, "[%c]", buf[i]);*/
            if(write(STDOUT_FILENO, &(buf[i]), 1) != 1 ){
               err_msg("%s:%d -> could not write byte\n", __FILE__, __LINE__);
               return;
            }

         tgdb_traverse_command(stderr, &com);

         { 
            size_t j;
            for(j = 0; com != NULL && com[j] != NULL ; ++j)
               if((*com)[j].header == QUIT)
                  return;
         } 

         tgdb_delete_command(&com);
      } /* end if */

      if(childfd != -1 && FD_ISSET(childfd, &rfds)){
         char buf[MAXLINE];
         size_t size;
         size_t i;
         
         if( (size = tgdb_tty_recv(buf, MAXLINE)) == -1){
            err_msg("%s:%d -> file descriptor closed\n", __FILE__, __LINE__);
            return;
         } /* end if */

         for(i = 0; i < size; ++i)
            /*fprintf(stderr, "<%c>", buf[i]);*/
            if(write(STDOUT_FILENO, &(buf[i]), 1) != 1 ){
               err_msg("%s:%d -> could not write byte\n", __FILE__, __LINE__);
               return;
            }
      } /* end if */

      if(readlinefd != -1 && FD_ISSET(readlinefd, &rfds)){
         char buf[MAXLINE];
         size_t size;
         size_t i;
         
         if( (size = tgdb_tty_recv(buf, MAXLINE)) == -1){
            err_msg("%s:%d -> file descriptor closed\n", __FILE__, __LINE__);
            return;
         } /* end if */

         for(i = 0; i < size; ++i)
            /*fprintf(stderr, "<%c>", buf[i]);*/
            if(write(STDOUT_FILENO, &(buf[i]), 1) != 1 ){
               err_msg("%s:%d -> could not write byte\n", __FILE__, __LINE__);
               return;
            }
      } /* end if */
   }
}

int main(int argc, char **argv){
   
   int gdb_fd, child_fd, readline_fd;
   
   if(tty_cbreak(STDIN_FILENO) < 0)
      fprintf(stderr, "tty_cbreak error\n");   
   
   tgdb_init();
   if ( tgdb_start(NULL, argc-1, argv+1, &gdb_fd, &child_fd, &readline_fd) == -1 )
      err_msg("%s:%d -> could not init\n", __FILE__, __LINE__);

   mainLoop(gdb_fd, child_fd, readline_fd);

   if(tgdb_shutdown() == -1)
      err_msg("%s:%d -> could not shutdown\n", __FILE__, __LINE__);

   /* reset the terminal to how it was */
   if(tty_reset(STDIN_FILENO) == -1)
      err_msg("Terminal attributes could not be reset ...\n 
               try typeing 'reset' at the prompt\n");

   return 0;
}
