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

#if HAVE_SIGNAL_H
#include <signal.h>
#endif

/* Local includes */
#include "tgdb.h"
#include "error.h"
#include "io.h"
#include "terminal.h"

#define MAXLINE 4096

struct tgdb *tgdb;

static void signal_handler(int signo) {
	if ( signo == SIGINT || signo == SIGTERM || signo == SIGQUIT )
		tgdb_signal_notification ( tgdb, signo );
}

/* Sets up the signal handler for SIGWINCH
 * Returns -1 on error. Or 0 on success */
static int set_up_signal(void) {
	struct sigaction action;

	action.sa_handler = signal_handler;      
	sigemptyset(&action.sa_mask);   
	action.sa_flags = 0;

    if(sigaction(SIGINT, &action, NULL) < 0) {
        err_ret("%s:%d -> sigaction failed ", __FILE__, __LINE__);
		return -1;
	}	

    if(sigaction(SIGTERM, &action, NULL) < 0) {
        err_ret("%s:%d -> sigaction failed ", __FILE__, __LINE__);
		return -1;
	}

    if(sigaction(SIGQUIT, &action, NULL) < 0) {
        err_ret("%s:%d -> sigaction failed ", __FILE__, __LINE__);
		return -1;
	}

	return 0;
}

static int tgdb_readline_input(void){
    char buf[MAXLINE];
    if ( tgdb_recv_readline_data(tgdb, buf, MAXLINE) == -1 ) {
        err_msg("%s:%d tgdb_recv_input error", __FILE__, __LINE__);
        return -1;
    } 
    fprintf(stderr, "%s", buf);
    return 0;
}

static int gdb_input(void) {
    char buf[MAXLINE];
    size_t size;
    size_t i;
    struct tgdb_command *item;

    if( (size = tgdb_recv_debugger_data (tgdb, buf, MAXLINE)) == -1){
        err_msg("%s:%d -> file descriptor closed\n", __FILE__, __LINE__);
        return -1;
    } /* end if */

    for(i = 0; i < size; ++i)
        if(write(STDOUT_FILENO, &(buf[i]), 1) != 1 ){
           err_msg("%s:%d -> could not write byte\n", __FILE__, __LINE__);
           return -1;
        }

    /*tgdb_traverse_commands ( tgdb );*/

    while ( (item = tgdb_get_command(tgdb)) != NULL ) {

        if( item->header == TGDB_QUIT) {
           fprintf ( stderr, "%s:%d TGDB_QUIT\n", __FILE__, __LINE__);
           return -1;
		}
    }

    return 0;
}

static void tty_input(void) {
    char buf[MAXLINE];
    size_t size;
    size_t i;

    if( (size = tgdb_recv_inferior_data(tgdb, buf, MAXLINE)) == -1){
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
    ssize_t size;
    int i;

    if( ( size = io_read(STDIN_FILENO, &command, MAXLINE)) < 0 ){
        err_msg("%s:%d -> could not read byte\n", __FILE__, __LINE__);
        return;
    } /* end if */

    for ( i = 0; i < size; i++ ) {
        /* For testing only */
        /*if ( command[i] == '8' )  {
			tgdb_modify_breakpoint ( tgdb, "test.c", 21, TGDB_BREAKPOINT_ADD );
            continue;
        } 
        if ( command[i] == '9' )  {
			tgdb_modify_breakpoint ( tgdb, "test.c", 21, TGDB_BREAKPOINT_DELETE );
            continue;
        } 
        
        if ( command[i] == '0' )  {
			tgdb_get_inferiors_source_files ( tgdb );
            continue;
        }*/
        tgdb_send_debugger_char ( tgdb, command[i]);
    }
}

void main_loop(int masterfd, int childfd, int readlinefd){
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
       * stdin          (user or gui) 
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
         err_dump("%s:%d select failed\n", __FILE__, __LINE__);

      /* stdin -> tgdb */
      if(FD_ISSET(STDIN_FILENO, &rfds))
          stdin_input(STDIN_FILENO);

      /* readline's output -> stdout */
      if(FD_ISSET(readlinefd, &rfds))
          if ( tgdb_readline_input() == -1 )
              return;

      /* child's output -> stdout */
      if(FD_ISSET(childfd, &rfds)) {
          tty_input();
          continue;
      }

      /* gdb's output -> stdout  */
      if(FD_ISSET(masterfd, &rfds))
          if ( gdb_input() == -1 )
              return;
   }
}

int main(int argc, char **argv){
   
    int gdb_fd, child_fd, tgdb_rlctx;

#if 0
   int c;
   read(0, &c ,1);
#endif

    if ( tty_cbreak(STDIN_FILENO) == -1 )
        err_msg("%s:%d tty_cbreak error\n", __FILE__, __LINE__);

    if ( (tgdb = tgdb_initialize(NULL, argc-1, argv+1, &gdb_fd, &child_fd, &tgdb_rlctx)) == NULL ) {
        err_msg("%s:%d tgdb_start error\n", __FILE__, __LINE__);
        goto driver_end;
    }

	
	/* Ask TGDB to print error messages */
	if ( tgdb_set_verbose_gui_command_output ( tgdb, 1 ) != 1 ) {
		err_msg("%s:%d driver error\n", __FILE__, __LINE__);
		goto driver_end;
	}

	set_up_signal();

    main_loop(gdb_fd, child_fd, tgdb_rlctx);

    if(tgdb_shutdown( tgdb ) == -1)
        err_msg("%s:%d -> could not shutdown\n", __FILE__, __LINE__);

driver_end:

    if ( tty_reset(STDIN_FILENO) == -1 )
        err_msg("%s:%d tty_reset error\n", __FILE__, __LINE__);

    return 0;
}
