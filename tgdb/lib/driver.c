/* From configure */
#include "config.h"

/* This include must stay above readline, some readline's don't include it */
#include <stdio.h> 

#ifdef HAVE_LIBREADLINE
#  if defined(HAVE_READLINE_READLINE_H)
#    include <readline/readline.h>
#  elif defined(HAVE_READLINE_H)
#    include <readline.h>
#  else /* !defined(HAVE_READLINE_H) */
extern char *readline ();
#  endif /* !defined(HAVE_READLINE_H) */
char *cmdline = NULL;
#else /* !defined(HAVE_READLINE_READLINE_H) */
 /* no readline */
#endif /* HAVE_LIBREADLINE */

#ifdef HAVE_READLINE_HISTORY
#  if defined(HAVE_READLINE_HISTORY_H)
#    include <readline/history.h>
#  elif defined(HAVE_HISTORY_H)
#    include <history.h>
#  else /* !defined(HAVE_HISTORY_H) */
extern void add_history ();
extern int write_history ();
extern int read_history ();
#  endif /* defined(HAVE_READLINE_HISTORY_H) */
 /* no history */
#endif /* HAVE_READLINE_HISTORY */


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
#include <string.h>
#include <errno.h>

/* Local includes */
#include "tgdb.h"
#include "error.h"
#include "io.h"
#include "terminal.h"

static void tgdb_send_user_command(char *line) {
    char buf[strlen(line) + 2];
    char *ret;
    add_history(line);
    sprintf(buf, "%s\n", line);
    if ( (ret = tgdb_send(buf)) != NULL )
        fprintf(stderr, "%s", ret);
}

static int readline_fd[2] = { -1, -1 }; /* readline write to this */
static int init_readline(void){
    FILE *n;

    if ( pipe(readline_fd) == -1 ) {
       err_msg("(%s:%d) pipe failed", __FILE__, __LINE__);
       return -1;
    }

    if ( (n = fdopen(readline_fd[1], "w")) == NULL ) {
       err_msg("(%s:%d) fdopen failed", __FILE__, __LINE__);
       return -1;
    }

    /* Set readline's output stream */
    rl_outstream = n;
    
    /* Initalize gdb */
    rl_callback_handler_install(tgdb_get_prompt(), tgdb_send_user_command);

    if ( rl_reset_terminal("dumb") == -1 ) {
        err_msg("%s:%d rl_reset_terminal\n", __FILE__, __LINE__); 
        return -1;
    }
    
    return 0;
}

static void tgdb_readline_input(void){
    char buf[MAXLINE];
    tgdb_recv_input(buf);
    fprintf(stderr, "%s", buf);
}

/* This is the output of readline, it should be displayed in the gdb window 
 * if the gdb is ready to accept a command */
static void readline_input(int fd){
    char c;
    
    if ( read(fd, &c, 1) != 1 ) 
        err_quit("%s:%d read error\n", __FILE__, __LINE__);
    /*fprintf(stderr, "%c", c);*/

    tgdb_send_input(c);
}

static int gdb_input(void) {
    char buf[MAXLINE];
    struct Command **com;
    size_t size;
    size_t i;

    if( (size = tgdb_recv(buf, MAXLINE, &com)) == -1){
    err_msg("%s:%d -> file descriptor closed\n", __FILE__, __LINE__);
    return;
    } /* end if */

    for(i = 0; i < size; ++i)
    if(write(STDOUT_FILENO, &(buf[i]), 1) != 1 ){
       err_msg("%s:%d -> could not write byte\n", __FILE__, __LINE__);
       return;
    }

//    tgdb_traverse_command(stderr, &com);
//
    { 
    size_t j;
    for(j = 0; com != NULL && com[j] != NULL ; ++j)
       if((*com)[j].header == QUIT)
           return -1;
    } 

    tgdb_delete_command(&com);
    return 0;
}

static void tty_input(void) {
    char buf[MAXLINE];
    size_t size;
    size_t i;

    if( (size = tgdb_tty_recv(buf, MAXLINE)) == -1){
    err_msg("%s:%d -> file descriptor closed\n", __FILE__, __LINE__);
    return;
    } /* end if */

    for(i = 0; i < size; ++i)
    if(write(STDOUT_FILENO, &(buf[i]), 1) != 1 ){
       err_msg("%s:%d -> could not write byte\n", __FILE__, __LINE__);
       return;
    }
}

static void stdin_input(int fd) {
    static char command[MAXLINE + 1];
    static char *result;
    ssize_t size;
    int i;

    if( ( size = io_read(STDIN_FILENO, &command, MAXLINE)) < 0 ){
    err_msg("%s:%d -> could not read byte\n", __FILE__, __LINE__);
    return;
    } /* end if */

    for ( i = 0; i < size; i++ ) {
     if ( rl_stuff_char(command[i]) != 1 ) {
        err_quit("%s:%d rl_stuff_char error\n", __FILE__, __LINE__);
        return;
     }
    }
   rl_callback_read_char();
}

void mainLoop(int masterfd, int childfd, int readlinefd){
  int    max;
  fd_set rfds;
  int result;

   /* get max fd  for select loop */
   max = (masterfd > STDIN_FILENO) ? masterfd : STDIN_FILENO;
   max = (max > childfd) ? max : childfd;
   max = (max > readline_fd[0]) ? max : readline_fd[0];
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
      FD_SET(readline_fd[0], &rfds);
      FD_SET(readlinefd, &rfds);
      
      result = select(max + 1, &rfds, NULL, NULL, NULL);
      
      /* if the signal interuppted system call keep going */
      if(result == -1 && errno == EINTR)
         continue;
      else if(result == -1) /* on error ... must die -> stupid OS */
         err_dump("main.c:mainLoop - select failed\n");

      if(FD_ISSET(readline_fd[0], &rfds))
          readline_input(readline_fd[0]);

      /* stdin -> gdb's input */
      if(FD_ISSET(STDIN_FILENO, &rfds))
          stdin_input(STDIN_FILENO);
              
      /* gdb's output -> stdout */
      if(FD_ISSET(masterfd, &rfds))
          if ( gdb_input() == -1 )
              return;

      if(FD_ISSET(childfd, &rfds))
          tty_input();

      if(FD_ISSET(readlinefd, &rfds))
          tgdb_readline_input();
   }
}

int main(int argc, char **argv){
   
   int gdb_fd, child_fd, tgdb_readline_fd;

   tgdb_init();
   if ( tgdb_start(NULL, argc-1, argv+1, &gdb_fd, &child_fd, &tgdb_readline_fd) == -1 )
      err_msg("%s:%d -> could not init\n", __FILE__, __LINE__);

   init_readline();

   mainLoop(gdb_fd, child_fd, tgdb_readline_fd);

   rl_callback_handler_remove();

   if(tgdb_shutdown() == -1)
      err_msg("%s:%d -> could not shutdown\n", __FILE__, __LINE__);

   return 0;
}
