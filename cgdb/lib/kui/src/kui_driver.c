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

#if HAVE_UNISTD_H
#include <unistd.h>
#endif  /* HAVE_UNISTD_H */

#include "kui.h"
#include "kui_term.h"

void main_loop(struct kui_manager *i) {
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
				int c = kui_manager_getkey(i);

				if ( c == -1 ) {
					fprintf ( stderr, "kui_manager_getkey failed\n" );
					return;
				}

				if ( c == 'q' ) {
					fprintf ( stderr, "User aborted\n" );
					return;
				} else {
					if ( kui_term_is_cgdb_key ( c ) ) {
						char *val;	

						val = (char*)kui_term_get_string_from_cgdb_key ( c );

						fprintf ( stderr, "%s\r\n", val );
					} else
						fprintf(stderr, "%c\r\n", c);
				}

				if ( kui_manager_cangetkey ( i ) == 1 )
					continue;
				else
					break;
			}
        }
    }
}

static int create_mappings ( struct kui_manager *kuim ) {

	struct kui_map_set *map;
	map = kui_ms_create ();
	if ( !map )
		return -1;

#if 1

	if ( kui_ms_register_map ( map, "abc", "xyz" ) == -1 ) {
		/* TODO: Free map and return */
		return -1;
	}

	if ( kui_ms_deregister_map ( map, "abc" ) == -1 ) {
		/* TODO: Free map and return */
		return -1;
	}
#endif

#if 0
	if ( kui_ms_register_map ( map, "abc", "xyz" ) == -1 ) {
		/* TODO: Free map and return */
		return -1;
	}

	if ( kui_ms_register_map ( map, "abc", "xyp" ) == -1 ) {
		/* TODO: Free map and return */
		return -1;
	}

	if ( kui_ms_register_map ( map, "xyzd", "<F4>" ) == -1 ) {
		/* TODO: Free map and return */
		return -1;
	}

	if ( kui_ms_register_map ( map, "xyzd", "<F4>" ) == -1 ) {
		/* TODO: Free map and return */
		return -1;
	}

	if ( kui_ms_register_map ( map, "a<F1>", "xyz" ) == -1 ) {
		/* TODO: Free map and return */
		return -1;
	}

	if ( kui_ms_register_map ( map, "a<F1><F1>", "xxx" ) == -1 ) {
		/* TODO: Free map and return */
		return -1;
	}

	if ( kui_ms_register_map ( map, "a<F1><F1>", "xxx" ) == -1 ) {
		/* TODO: Free map and return */
		return -1;
	}

	if ( kui_ms_register_map ( map, "<Left><Right><F1><F1>", "<F2>" ) == -1 ) {
		/* TODO: Free map and return */
		return -1;
	}

#endif
	if ( kui_manager_add_map_set ( kuim, map ) == -1 )
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
    struct kui_manager *i;
    /* Initalize curses */
    initscr();
    noecho();
    raw();
    refresh();

    i = kui_manager_create ( STDIN_FILENO );

	create_mappings ( i );

    main_loop(i);

	kui_manager_destroy ( i );

    /* Shutdown curses */
    echo();
    endwin();
#endif
    return 0;
}
