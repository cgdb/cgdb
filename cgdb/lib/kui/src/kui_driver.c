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
#include "kui_term.h"

struct kui_map_set *terminal_map;

void main_loop(struct kuictx *i) {
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
			while ( 1 ) {
				int c = kui_getkey(i);

				if ( c == -1 ) {
					fprintf ( stderr, "kui_getkey failed\n" );
					return;
				}

				if ( c == 'q' ) {
					fprintf ( stderr, "User aborted\n" );
					return;
				} else {
					fprintf(stderr, "(%c:%d)\r\n", c, c);
				}

				if ( kui_cangetkey ( i ) == 1 )
					continue;
				else
					break;
			}
        }
    }
}

static void usage ( void ) {
	fprintf ( stderr, "kui_driver\r\n" );
}

static int create_terminal_mappings ( struct kuictx *i ) {
	std_list list;
    std_list_iterator current;
    void *current_data;
	struct kui_map *map;
	char *key, *value;
	int j, length;
	
	/* Create the terminal kui map */
	terminal_map = kui_term_get_terminal_mappings ();

	if ( !terminal_map )
		return -1;

	
	list = kui_ms_get_maps ( terminal_map );

	if ( !list )
		return -1;

    for (current  = std_list_begin(list); 
         current != std_list_end(list);
         current  = std_list_next(current)) {

        if (std_list_get_data(current, &current_data) != 0)
            return -1;

		map = ( struct kui_map *)current_data;

		if ( kui_map_get_value ( map, &value ) == -1 )
			return -1;

		if ( kui_map_get_key ( map, &key ) == -1 )
			return -1;

		length = strlen ( key );

		fprintf ( stderr, "MAP->" ); 

		fprintf ( stderr, "KEY[");
		for ( j = 0; j < length; ++j )
			fprintf ( stderr, "(%d)", (int)key[j]);
	    fprintf ( stderr, "]" );
		
		fprintf ( stderr, "VALUE(%s)", value );

		if ( kui_map_print_cgdb_key_array ( map ) == -1 )
			return -1;

	}

	if ( kui_add_map_set ( i, terminal_map ) == -1 )
		return -1;

	return 0;
}


int main(int argc, char **argv){

#if 0
	/* Test translating mappings to values */
	int *a;

	if ( kui_term_string_to_cgdb_key_array ( argv[1], &a ) == -1 )
		return -1;

	if ( kui_term_print_cgdb_key_array ( a ) == -1 )
		return -1;

	free ( a );
	a = NULL;
	return 0;
#endif

	
#if 1
    struct kuictx *i;
    /* Initalize curses */
    initscr();
    noecho();
    raw();
    refresh();
	usage ();

    i = kui_create ( STDIN_FILENO );

	if ( create_terminal_mappings ( i ) == -1 )
		fprintf ( stderr, "terminal mappings failed\r\n" );

    main_loop(i);

    /* Shutdown curses */
    echo();
    endwin();
#endif
    return 0;
}
