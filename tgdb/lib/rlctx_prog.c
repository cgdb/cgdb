#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif /* HAVE_FCNTL_H */

#if HAVE_UNISTD_H
#include <unistd.h>
#endif  /* HAVE_UNISTD_H */

#if HAVE_ERRNO_H
#include <errno.h>
#endif /* HAVE_ERRNO_H */

#if HAVE_STDIO_H
#include <stdio.h> 
#endif /* HAVE_STDIO_H */

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

#if HAVE_SIGNAL_H
#include <signal.h>
#endif /* HAVE_SIGNAL_H */

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

#include "util.h"

static struct string *command;
static char rl_history_file[4096];

static void rl_write_history(char *file) {
    write_history(file); 
}

static void rl_read_history(char *file) {
    using_history();
    read_history(file); 
    history_set_pos(history_length);
    strcpy(rl_history_file, file);
}

static void redisplay(void) {
    /* Blast the prompt */
    fprintf(stderr, "\r");
    rl_forced_update_display();
}

static void change_prompt(char *prompt) {
    rl_set_prompt(prompt); 

    /* Tell the update functions that we have moved onto a new line, with 
     * rl_prompt already displayed. This could be used by applications that 
     * want to output the prompt string themselves, but still need Readline 
     * to know the prompt string length for redisplay. It should be used 
     * after setting rl_already_prompted.
     *
     * The text from above is from the readline manual. It seems to be exactly
     * what libtgdb wants and it seems to work.
     */
    rl_on_new_line_with_prompt();
}

static int read_command( int fd ) {
    static enum rlctx_parse_state { DATA, COMMAND } rlctx_state = DATA;
    static enum rlctx_com_type { REDISPLAY, CHANGE_PROMPT, READ_HISTORY, WRITE_HISTORY } rlctx_com;
    static unsigned short state = 0;
    static unsigned short command_byte = 0;
    int i;
    const int size = 128;
    char buf[size];
    int result;

    if ( ( result = read(fd, buf, size - 1)) == -1 ){ 
        if ( errno == EBADF ) 
            err_msg("%s:%d write error1", __FILE__, __LINE__);
        else if ( errno == EINVAL )
            err_msg("%s:%d write error2", __FILE__, __LINE__);
        else if ( errno == EAGAIN )
            err_msg("%s:%d write error5", __FILE__, __LINE__);
        else if ( errno == EINTR )
            err_msg("%s:%d write error6", __FILE__, __LINE__);
        else if ( errno == EIO ) 
            err_msg("%s:%d write error8", __FILE__, __LINE__);
        else
            err_msg("%s:%d write error9", __FILE__, __LINE__);
        return -1;
    } else if ( result == 0 )   /* EOF */
        return -2;

    /* NOTE: This is a total hack. 
     * There should be a common interface to send and recieve messages.
     * Add it before modifing this code anymore. The other place this 
     * code is in is rlctx.c.
     * This is only here because it was a fast proof of concept.
     */

    for ( i = 0; i < result; i++) {
        /* Change states */
        if ( buf[i] == '\032' ) {
            if ( rlctx_state == DATA ) {
                rlctx_state = COMMAND;
                command_byte = 1;
            } else { /* Found a full command, call callback */
                rlctx_state = DATA;
                if ( rlctx_com == CHANGE_PROMPT ) {
                    change_prompt(string_get(command));
                    string_clear(command);
                } else if ( rlctx_com == REDISPLAY )
                    redisplay();
                else if ( rlctx_com == READ_HISTORY ) {
                    rl_read_history(string_get(command));
                    string_clear(command);
                } else if ( rlctx_com == WRITE_HISTORY ) {
                    rl_write_history(string_get(command));
                    string_clear(command);
                }

                fprintf(stderr, "\031");
            }
            /* Don't want to add \032 to command */
            continue;
        }

        /* This determines the type of command sent from rlctx_prog */
        if ( command_byte ) {
            if ( buf[i] == 'P' )
                rlctx_com = CHANGE_PROMPT;
            else if ( buf[i] == 'T' )
                rlctx_com = REDISPLAY;
            else if ( buf[i] == 'R' )
                rlctx_com = READ_HISTORY;
            else if ( buf[i] == 'W' )
                rlctx_com = WRITE_HISTORY;
            else {
                err_msg("%s:%d Communication error", __FILE__, __LINE__);
                return;
            }
            command_byte = 0;
            continue;
        }

        if ( rlctx_com == CHANGE_PROMPT || 
             rlctx_com == READ_HISTORY ||
             rlctx_com == WRITE_HISTORY)
            string_addchar(command, buf[i]);
    }
}

static unsigned short finished = 0;

void main_loop(int infd, int comfd){
    int    max;
    fd_set rfds;
    fd_set efds;
    int result;
  
    max = infd;
    max = ( max > comfd ) ? max : comfd;
   
    while(1){
        FD_ZERO(&rfds);
        FD_ZERO(&efds);
      
        FD_SET(infd, &rfds);
        FD_SET(comfd, &rfds);
        FD_SET(infd, &efds);
        FD_SET(comfd, &rfds);
      
        result = select(max + 1, &rfds, NULL, &efds, NULL);
      
        /* if the signal interuppted system call keep going */
        if(result == -1 && errno == EINTR)
            continue;
        else if(result == -1) {/* on error ... must die -> stupid OS */
            finished = 1;
            err_msg("%s:%d select failed", __FILE__, __LINE__);
            return;
        }

        /* stdin -> readline input */
        if(FD_ISSET(infd, &rfds) || FD_ISSET(infd, &efds))
            rl_callback_read_char();

        if ( finished )
            return;

        /* stdin -> readline input */
        if(FD_ISSET(comfd, &rfds) || FD_ISSET(comfd, &efds))
            if ( read_command ( comfd ) == -1 ) {
                err_msg("%s:%d read_command failed", __FILE__, __LINE__);
                return;
            }
    }
}

static void rlctx_send_user_command(char *line) {
    /* This happens when rl_callback_read_char gets EOF */
    if ( line == NULL ) {
        finished = 1;
        return;
    }
    
    /* Don't add the enter command */
    if ( line && *line != '\0' )
        add_history(line);

    fprintf(stderr, "\032C%s\032", line);
}


static int tab_completion(int a, int b) {
    /* Printing a new line here is cheating. 
     * libtgdb doesn't actually get a prompt when '\t' is hit.
     * So, a \n is never sent to the console.
     * This is a hack to fix that.
     */
    fprintf(stderr, "\n");
    fprintf(stderr, "\032T%s\032", rl_line_buffer);
}

static int init_readline(void){
    /* Tell readline not to put the initial prompt */
    rl_already_prompted = 1;
    
    /* Tell readline what the prompt is if it needs to put it back */
    rl_callback_handler_install("", rlctx_send_user_command);

    /* Set the terminal type to dumb so the output of readline can be
     * understood by tgdb */
    if ( rl_reset_terminal("dumb") == -1 ) {
        err_msg("%s:%d rl_reset_terminal\n", __FILE__, __LINE__); 
        return -1;
    }

    rl_bind_key ('\t', tab_completion);
    
    return 0;
}

static void signal_catcher( int SIGNAL ) {
    if ( SIGNAL != SIGHUP )
        err_msg("%s:%d caught unknown signal: %d", __FILE__, __LINE__, SIGNAL);
}

static int rlctx_setup_signals(void) {
   struct sigaction action;

   action.sa_handler = signal_catcher;
   sigemptyset(&action.sa_mask);
   action.sa_flags = 0;

   if(sigaction(SIGHUP, &action, NULL) < 0) {
      err_ret("%s:%d -> sigaction failed ", __FILE__, __LINE__);
      return -1;
   }

   return 0;
}

int main(int argc, char **argv) {
    int fd = atoi(argv[1]);

    command = string_init();

    if ( rlctx_setup_signals() == -1 ) {
        err_ret("%s:%d rlctx_setup_signals failed ", __FILE__, __LINE__);
        return -1;
    }
    
    init_readline();

    main_loop(STDIN_FILENO, fd);

    rl_write_history(rl_history_file);

    rl_callback_handler_remove();
}
