#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_CURSES_H
#include <curses.h>
#endif /* HAVE_CURSES_H */

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

int main(int argc, char **argv){
    struct input *i;
    /* Initalize curses */
    initscr();
    noecho();
    raw();
    refresh();

    i = input_init(STDIN_FILENO);

    while ( 1 ) {
        int c = input_getkey(i);
        if ( c == 'q' )
            break;
        else if ( c == 27 )
            fprintf(stderr, "ESC\r\n");
        else if ( c >= CGDB_KEY_UP && c < CGDB_KEY_ERROR ) {
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

    /* Shutdown curses */
    echo();
    endwin();
    return 0;
}
