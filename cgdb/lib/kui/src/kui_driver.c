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

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif /* HAVE_STDLIB_H */

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

#include "kui.h"

void main_loop(struct kuictx *i) {
    int    max;
    fd_set rfds;
    int result;

	/* Kuictx return values */
	kui_sequence_key key;
	int kui_input_key;

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
            int c = kui_getkey(i, &key, &kui_input_key);

            if ( c == 'q' ) {
                return;
			} else {
                fprintf(stderr, "(%c:%d)\r\n", c, c);
            }
        }
    }
}

static void usage ( void ) {
	fprintf ( stderr, "kui_driver\r\n" );
}

int main(int argc, char **argv){
    struct kuictx *i;
    /* Initalize curses */
    initscr();
    noecho();
    raw();
    refresh();
	usage ();

    i = kui_create ( STDIN_FILENO );

    main_loop(i);

    /* Shutdown curses */
    echo();
    endwin();
    return 0;
}
