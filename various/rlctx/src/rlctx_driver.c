#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_STDIO_H
#include <stdio.h> 
#endif /* HAVE_STDIO_H */

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

/* Library includes */
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#if HAVE_ERRNO_H
#include <errno.h>
#endif /* HAVE_ERRNO_H */

/* Local includes */
#include "error.h"
#include "io.h"
#include "terminal.h"
#include "rlctx.h"

#define MAXLINE 4096

struct rlctx *rl;

static int tgdb_command_callback(const char *line) {
    fprintf ( stderr, "COMMAND(%s)\n", line );
    return 0;
}

/*static int tgdb_change_prompt ( const char *prompt ) {
    fprintf ( stderr, "CHANGE_PROMPT(%s)\n", prompt );
    return 0;
}*/

static int tgdb_completion_callback ( const char *line ) {
    fprintf ( stderr, "TAB_COMPLETION(%s)\n", line );

    return 0;
}

static int tgdb_init_readline ( char *config_dir, int *fd ) {
    /* Initialize readline */
    if ( (rl = rlctx_init((const char *)config_dir, "rlctx_driver")) == NULL ) {
        err_msg("(%s:%d) rlctx_init failed", __FILE__, __LINE__);
        return -1;
    }

    /* Register callback for each command recieved at readline */
    if ( rlctx_register_command_callback(rl, &tgdb_command_callback) == -1 ) {
        err_msg("(%s:%d) rlctx_register_callback failed", __FILE__, __LINE__);
        return -1;
    }

    /* Register callback for tab completion */
    if ( rlctx_register_completion_callback(rl, &tgdb_completion_callback) == -1 ) {
        err_msg("(%s:%d) rlctx_register_callback failed", __FILE__, __LINE__);
        return -1;
    }

    /* Let the GUI check this for reading, 
     * if it finds data, it should call tgdb_recv_input */
    if ( (*fd = rlctx_get_fd(rl)) == -1 ) {
        err_msg("%s:%d rlctx_get_fd error", __FILE__, __LINE__);
        return -1;
    }

    return 0;
}

int tgdb_rl_send ( char c ) {
    if ( rlctx_send_char ( rl, c ) == -1 ) {
        err_msg("(%s:%d) rlctx_send_char failed", __FILE__, __LINE__);
        return -1;
    }

    return 0;
}


static int tgdb_readline_input(void){
    char buf[MAXLINE];

    if ( rlctx_recv ( rl, buf, MAXLINE ) == -1 ) {
        err_msg("%s:%d rlctx_recv error", __FILE__, __LINE__);
        return -1;
    }
    fprintf(stderr, "%s", buf);
    return 0;
}

static void stdin_input(int fd) {
    static char command[MAXLINE + 1];
    ssize_t size;
    int i;

    if( ( size = io_read(STDIN_FILENO, &command, MAXLINE)) < 0 ){
        err_msg("%s:%d -> could not read byte\n", __FILE__, __LINE__);
        return;
    } /* end if */

    for ( i = 0; i < size; i++ ) {
        /* For testing only */
        if ( command[i] == '8' )  {
            continue;
        } 
        if ( command[i] == '9' )  {
            continue;
        } 
        
        if ( command[i] == '0' )  {
            continue;
        } 

        if ( rlctx_send_char ( rl, command[i] ) == -1 ) {
            err_msg("(%s:%d) rlctx_send_char failed", __FILE__, __LINE__);
            return;
        }
    }
}

void main_loop(int readlinefd){
  int    max;
  fd_set rfds;
  int result;

   /* get max fd  for select loop */
   max = (readlinefd > STDIN_FILENO) ? readlinefd : STDIN_FILENO;
//   max = (max > readlinefd) ? max : readlinefd;
   
   while(1){
      /* Clear the set and 
       *
       * READ FROM:
       * stdin          (user or gui) 
       * master         (gdb's stdout)
       * gui_stdout     (gui's stdout sending new info)
       *
       */
      FD_ZERO(&rfds);
      
      FD_SET(STDIN_FILENO, &rfds);
      FD_SET(readlinefd, &rfds);
      
      result = select(max + 1, &rfds, NULL, NULL, NULL);
      
      /* if the signal interuppted system call keep going */
      if(result == -1 && errno == EINTR)
         continue;
      else if(result == -1) /* on error ... must die -> stupid OS */
         err_dump("%s:%d select failed\n", __FILE__, __LINE__);

      /* stdin -> tgdb */
      if(FD_ISSET(STDIN_FILENO, &rfds))
          stdin_input(STDIN_FILENO);

      /* readline's output -> stdout */
      if(FD_ISSET(readlinefd, &rfds))
          if ( tgdb_readline_input() == -1 )
              return;
   }
}

int main(int argc, char **argv){
   
    int tgdb_rlctx;

#if 0
   int c;
   read(0, &c ,1);
#endif

    if ( tty_cbreak(STDIN_FILENO) == -1 )
        err_msg("%s:%d tty_cbreak error\n", __FILE__, __LINE__);

    if ( tgdb_init_readline ( ".", &tgdb_rlctx ) == -1 ) {
        err_msg("%s:%d tty_cbreak error\n", __FILE__, __LINE__);
        goto reset_driver;
    }

    main_loop(tgdb_rlctx);

reset_driver:

    if ( tty_reset(STDIN_FILENO) == -1 )
        err_msg("%s:%d tty_reset error\n", __FILE__, __LINE__);

    return 0;
}
