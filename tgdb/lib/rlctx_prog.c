#include "config.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

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

void main_loop(void){
    int    max;
    fd_set rfds;
    int result;
  
    max = STDIN_FILENO;
   
    while(1){
        FD_ZERO(&rfds);
      
        FD_SET(STDIN_FILENO, &rfds);
      
        result = select(max + 1, &rfds, NULL, NULL, NULL);
      
        /* if the signal interuppted system call keep going */
        if(result == -1 && errno == EINTR)
            continue;
        else if(result == -1) /* on error ... must die -> stupid OS */
            err_dump("main.c:mainLoop - select failed\n");

        /* stdin -> readline input */
        if(FD_ISSET(STDIN_FILENO, &rfds))
            rl_callback_read_char();
    }
}

static void rlctx_send_user_command(char *line) {
    /* Don't add the enter command */
    if ( line && *line != '\0' )
        add_history(line);

    fprintf(stderr, "\032%s\032", line);
}

static int init_readline(void){
    /* Tell readline not to put the initial prompt */
    rl_already_prompted = 1;
    
    /* Tell readline what the prompt is if it needs to put it back */
    rl_callback_handler_install("(tgdb) ", rlctx_send_user_command);

    /* Set the terminal type to dumb so the output of readline can be
     * understood by tgdb */
    if ( rl_reset_terminal("dumb") == -1 ) {
        err_msg("%s:%d rl_reset_terminal\n", __FILE__, __LINE__); 
        return -1;
    }
    
    return 0;
}

int main(int argc, char **argv) {
    init_readline();

    main_loop();
}
