#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_CURSES_H
#include <curses.h>
#endif

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

/* term.h prototypes */
extern int tgetent();
extern int tgetflag();
extern int tgetnum();
extern char *tgetstr();
extern int tputs();
extern char *tgoto();

#if HAVE_UNISTD_H
#include <unistd.h>
#endif  /* HAVE_UNISTD_H */

#include "input.h"

void main_loop(struct input *i) {
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
            fprintf(stderr, "%s:%d select failed\n", __FILE__, __LINE__);

        if(FD_ISSET(STDIN_FILENO, &rfds)) {
            int c = input_getkey(i);

            if ( c == 'q' )
                return;
            else if ( c >= CGDB_KEY_ESC && c < CGDB_KEY_ERROR ) {
                char *buf = input_get_last_seq(i);
                int length = strlen(buf), counter;
                fprintf(stderr, "Found %s", input_get_last_seq_name(i));
                fprintf(stderr, "[");
                for(counter = 0; counter < length; counter++)
                    fprintf(stderr, "(%d)", buf[counter]);
                fprintf(stderr, "]\r\n");
            } else {
                fprintf(stderr, "(%c:%d)\r\n", c, c);
            }
        }
    }
}

int main(int argc, char **argv){
    struct input *i;
    /* Initalize curses */
    initscr();
    noecho();
    raw();
    refresh();

    i = input_init(STDIN_FILENO);

    main_loop(i);

    /* Shutdown curses */
    echo();
    endwin();
    return 0;
}
