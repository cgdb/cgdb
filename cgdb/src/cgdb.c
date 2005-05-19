/* cgdb.c:
 * -------
 */
#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif /* HAVE_FCNTL_H */

#if HAVE_UNISTD_H
#include <unistd.h>
#endif  /* HAVE_UNISTD_H */

#if HAVE_STDLIB_H 
#include <stdlib.h>
#endif  /* HAVE_STDLIB_H */

#if HAVE_STDIO_H
#include <stdio.h> 
#endif /* HAVE_STDIO_H */

#if HAVE_CURSES_H
#include <curses.h>
#endif /* HAVE_CURSES_H */

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif  /* HAVE_SYS_SELECT_H */

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#if HAVE_ERRNO_H
#include <errno.h>
#endif /* HAVE_ERRNO_H */

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif 

/* Local Includes */
#include "cgdb.h"
#include "logger.h"
#include "interface.h"
#include "scroller.h"
#include "sources.h"
#include "tgdb.h"
#include "helptext.h"
#include "kui.h"
#include "kui_term.h"
#include "fs_util.h"
#include "cgdbrc.h"
#include "io.h"
#include "tgdb_list.h"
#include "terminal.h"

/* --------------- */
/* Local Variables */
/* --------------- */

struct tgdb *tgdb; 			  /* The main TGDB context */

char cgdb_home_dir[MAXLINE];  /* Path to home directory with trailing slash */
char cgdb_help_file[MAXLINE]; /* Path to home directory with trailing slash */

static int gdb_fd = -1;       /* File descriptor for GDB communication */
static int tty_fd = -1;       /* File descriptor for process being debugged */
static int tgdb_readline_fd = -1; /* tgdb returns the 'at prompt' data */

static char *debugger_path = NULL;  /* Path to debugger to use */

struct kui_manager *kui_ctx = NULL; /* The key input package */

int resize_pipe[2] = { -1, -1 };

char last_relative_file[MAX_LINE];

/* ------------------------ */
/* Initialization functions */
/* ------------------------ */

void usage(void) {
fprintf(stderr, 
"CGDB Usage:\n"      
"   cgdb [cgdb options] [gdb options]\n"
"\n"
"CGDB Options:\n"
#ifdef HAVE_GETOPT_H
"   --version   Print version information and then exit.\n"
#else
"   -v          Print version information and then exit.\n"
#endif
#ifdef HAVE_GETOPT_H
"   --help      Print help (this message) and then exit.\n"
#else
"   -h          Print help (this message) and then exit.\n"
#endif
"   -d          Set debugger to use.\n"
    );
    exit(1);
}

/* version_info: Returns version information about cgdb.
 * ------------- 
 *
 * Return:  A pointer to a static buffer, containing version info.
 */
static char *version_info(void) {
    static char buf[MAXLINE];
    sprintf(buf, "%s %s\r\n%s", "CGDB", VERSION, 
            "Copyright 2002-2005 Bob Rossi and Mike Mueller.\n"
            "CGDB is free software, covered by the GNU General Public License, and you are\n"
            "welcome to change it and/or distribute copies of it under certain conditions.\n"
            "There is absolutely no warranty for CGDB.\n");
    return buf;
}

static void parse_long_options(int *argc, char ***argv) {
    int c, option_index = 0, n = 1;
    const char *args = "d:hv";
     
#ifdef HAVE_GETOPT_H
    static struct option long_options[] = {
        { "version", 0, 0, 0 },
        { "help", 0, 0, 0 },
		{ 0, 0, 0, 0 }
    };
#endif

	while ( 1 ) {
    	opterr = 0;
#ifdef HAVE_GETOPT_H
        c = getopt_long(*argc, *argv, args, long_options, &option_index);
#else
        c = getopt(*argc, *argv, args);
#endif
   		if ( c == -1 )
			break;

        if ( ((char)c) == '?' )
            break;

        switch (c) {
            case 0:
                switch ( option_index ) {
                    case 0:
                        printf("%s",version_info());
                        exit(0);
                    case 1:
                        usage();
                        exit(0);
                    default:
                        break;
                }
                break;
            case 'v':
                printf("%s",version_info());
                exit(0);
            case 'd':
                debugger_path = strdup ( optarg );
                n+=2;
                break;
            case 'h':
                usage();
                exit(0);
            default:
                break;
        }
    }

    *argc -= n;
    *argv += n;
}

/* init_home_dir: Attempts to create a config directory in the user's home
 * -------------  directory.
 *
 * Return:  0 on sucess or -1 on error
 */

static int init_home_dir(void) {
    /* Get the home directory */
    char *home_dir = getenv("HOME");
    const char *cgdb_dir = ".cgdb";

    /* Create the config directory */
    if ( ! fs_util_create_dir_in_base ( home_dir, cgdb_dir ) ) {
        logger_write_pos ( logger, __FILE__, __LINE__, "fs_util_create_dir_in_base error");
        return -1; 
    }

    fs_util_get_path ( home_dir, cgdb_dir, cgdb_home_dir );

    return 0;
}

/* create_help_file: Creates the file help.txt in $HOME/.cgdb
 *
 * Return:  0 on sucess or -1 on error
 */
static int create_help_file(void) {
   int i, length = 0;
   static FILE *fd = NULL;
   char *dashes = NULL;

   fs_util_get_path ( cgdb_home_dir, "help.txt", cgdb_help_file );
   
   if ( (fd = fopen(cgdb_help_file, "w")) == NULL)
       return -1;

   /* Write cgdb version number */
   length = 5 + strlen(VERSION);
   dashes = (char *)malloc(sizeof(char) * (length+1));
   memset(dashes, '-', length);
   dashes[length] = 0;
   fprintf(fd, "CGDB %s\n%s\n\n", VERSION, dashes);
   free(dashes);

   /* Write help file */
   for( i = 0; cgdb_help_text[i] != NULL; i++)
       fprintf(fd, "%s\n", cgdb_help_text[i]); 

   if ( fclose(fd) == -1 )
       return -1;

   return 0;
}

/* start_gdb: Starts up libtgdb
 *  Returns:  -1 on error, 0 on success
 */
static int start_gdb(int argc, char *argv[]) {

    if ( ( tgdb = tgdb_initialize(debugger_path, argc, argv, &gdb_fd, &tty_fd, &tgdb_readline_fd)) == NULL )
        return -1;

    return 0;
}

static void send_key(int focus, int key) {
    if ( focus == 1 ) {
        if ( key == '\r' )
            tgdb_send_debugger_char (tgdb, '\n');
        else
            tgdb_send_debugger_char (tgdb, key);
    } else if ( focus == 2 ) {
        tgdb_send_inferior_char (tgdb, key);
    }
}

/* user_input: This function will get a key from the user and process it.
 *
 *  Returns:  -1 on error, 0 on success
 */
static int user_input(void) {
    static int key, val;
    
	key = kui_manager_getkey ( kui_ctx );
    if (key == -1) {
        logger_write_pos ( logger, __FILE__, __LINE__, "kui_manager_getkey error");
        return -1;
    }

    /* Process the key */
    switch ( ( val = if_input(key)) ) {
        case 1:
        case 2:
            if ( kui_term_is_cgdb_key ( key ) ) {
				char *seqbuf = (char*)kui_manager_get_raw_data( kui_ctx );

				if ( seqbuf == NULL ) {
					logger_write_pos ( logger, __FILE__, __LINE__, "input_get_last_seq error");
					return -1;
				} else {
					int length = strlen(seqbuf), i;
					for ( i = 0; i < length; i++ )
						send_key(val, seqbuf[i]);
				}
            } else
                send_key(val, key);
    
            break;
        case -1:
            return -1;
        default:
            break;
    }

    return 0;
}

static void process_commands(struct tgdb *tgdb)
{
    struct tgdb_command *item;

    while ( ( item = tgdb_get_command (tgdb )) != NULL ) {
        
        switch (item -> header){
            /* This updates all the breakpoints */
            case TGDB_UPDATE_BREAKPOINTS:
            {
                struct sviewer *sview = if_get_sview();
                char *file; 
				struct tgdb_list *list = (struct tgdb_list *)item->data;
				tgdb_list_iterator *iterator;
				struct tgdb_breakpoint *tb;

                source_clear_breaks(if_get_sview());

				iterator = tgdb_list_get_first ( list );

            	while ( iterator ) {
					/* For each breakpoint */
                	tb = (struct tgdb_breakpoint *)tgdb_list_get_item ( iterator );

                    file = strdup ( tb->file );

                    if ( tb->enabled )
                        source_enable_break(sview, file, tb->line);
                    else
                        source_disable_break(sview, file, tb->line);

					iterator = tgdb_list_next ( iterator );
                }

                if_show_file(NULL, 0);
                break;
            }

            /* This means a source file or line number changed */
            case TGDB_UPDATE_FILE_POSITION:
            {
				struct tgdb_file_position *tfp;

				tfp = (struct tgdb_file_position *) item->data;

				/* Update the file */
				source_reload ( if_get_sview(), tfp->absolute_path, 0 );

                if_show_file ( 
                    tfp->absolute_path, 
                    tfp->line_number);

                source_set_relative_path ( 
                    if_get_sview(), 
                    tfp->absolute_path, 
                    tfp->relative_path);

                break;
            }
                
            /* This is a list of all the source files */
            case TGDB_UPDATE_SOURCE_FILES:
            {
				struct tgdb_list *list =
					(struct tgdb_list *) item->data;
				tgdb_list_iterator *i = tgdb_list_get_first ( list );
                char *s;

                if_clear_filedlg();

                while ( i ) {
					s = tgdb_list_get_item ( i );

                    if_add_filedlg_choice( s );

					i = tgdb_list_next ( i );
                }

                if_set_focus(FILE_DLG);
                break;
            }
                
            /* The user is trying to get a list of source files that make up
             * the debugged program but libtgdb is claiming that gdb knows
             * none. */
            case TGDB_SOURCES_DENIED:
                if_display_message("Error:", 0,  
                       " No sources available! Was the program compiled with debug?");
                break;

            /* This is the absolute path to the last file the user requested */
            case TGDB_ABSOLUTE_SOURCE_ACCEPTED:
             {
				struct tgdb_source_file *file = 
						( struct tgdb_source_file *) item->data;
                if_show_file( file->absolute_path, 1);
                source_set_relative_path(if_get_sview(), file->absolute_path, last_relative_file);
                break;
             }

            /* The source file requested does not exist */
            case TGDB_ABSOLUTE_SOURCE_DENIED:
             {
				struct tgdb_source_file *file = 
						( struct tgdb_source_file *) item->data;
                if_show_file(NULL, 0 );
                /* com can be NULL when tgdb orig requests main file */
                if ( file->absolute_path != NULL )
                    if_display_message("No such file:", 0, " %s", file->absolute_path);

                break;
             }
			case TGDB_INFERIOR_EXITED:
			 {
				 /*
				  * int *status = item->data;
				  * This could eventually go here, but for now, the update breakpoint 
				  * display function makes the status bar go back to the name of the file.
				  *
				  * if_display_message ( "Program exited with value", 0, " %d", *status );
				  */

				 /* Clear the cache */
				 break;
			 }
            case TGDB_QUIT:
                cleanup();            
                exit(0);
                break;
            /* Default */
            default:
                break;
        }
    }
}

/* gdb_input: Recieves data from tgdb:
 *
 *  Returns:  -1 on error, 0 on success
 */
static int gdb_input()
{
    char *buf = malloc(GDB_MAXBUF+1);
    int size;

    /* Read from GDB */
    size = tgdb_recv_debugger_data( tgdb, buf, GDB_MAXBUF);
    if (size == -1){
        logger_write_pos ( logger, __FILE__, __LINE__, "tgdb_recv_debugger_data error");
		free( buf );
		buf = NULL;
        return -1;
    }

    buf[size] = 0;

    process_commands(tgdb);

    /* Display GDB output 
     * The strlen check is here so that if_print does not get called
     * when displaying the filedlg. If it does get called, then the 
     * gdb window gets displayed when the filedlg is up
     */
    if ( strlen( buf ) > 0 )
        if_print(buf);

	free( buf );
	buf = NULL;
    return 0;
}

/* child_input: Recieves data from the child application:
 *
 *  Returns:  -1 on error, 0 on success
 */
static int child_input()
{
    char *buf = malloc(GDB_MAXBUF+1);
    int size;

    /* Read from GDB */
    size = tgdb_recv_inferior_data ( tgdb, buf, GDB_MAXBUF);
    if (size == -1){
        logger_write_pos ( logger, __FILE__, __LINE__, "tgdb_recv_inferior_data error ");
		free( buf );
		buf = NULL;
        return -1;
    }
    buf[size] = 0;

    /* Display CHILD output */
    if_tty_print(buf);
	free( buf );
	buf = NULL;
    return 0;
}

static int tgdb_readline_input(void){
    char buf[MAXLINE];
    if ( tgdb_recv_readline_data (tgdb, buf, MAXLINE) == -1 ) {
        logger_write_pos ( logger, __FILE__, __LINE__, "tgdb_recv_readline_data error");
        return -1;
    }
    if_print(buf);
    return 0;
}

static int cgdb_resize_term(int fd) {
    int c, result;
    read(resize_pipe[0], &c, sizeof(int) );

	/* If there is more input in the pipe, that means another resize has
	 * been recieved, and we still have not handled this one. So, skip this
	 * one and only handle the next one.
	 */
	result = io_data_ready( resize_pipe[0], 0 );
    if (result == -1) {
        logger_write_pos ( logger, __FILE__, __LINE__, "io_data_ready");
        return -1;
    }

    if (result) {
		if_print ( "skip signal\n" );
		return 0;
	}

    if ( if_resize_term() == -1 ) {
        logger_write_pos ( logger, __FILE__, __LINE__, "Unreasonable terminal size");
        return -1;
    }
    return 0;
}

static void main_loop(void) {
    fd_set rset;
    int max;
    
    max = (gdb_fd > STDIN_FILENO) ? gdb_fd : STDIN_FILENO;
    max = (max > tty_fd) ? max : tty_fd;
    max = (max > tgdb_readline_fd) ? max : tgdb_readline_fd;
    max = (max > resize_pipe[0]) ? max : resize_pipe[0];

    /* Main (infinite) loop:
     *   Sits and waits for input on either stdin (user input) or the
     *   GDB file descriptor.  When input is received, wrapper functions
     *   are called to process the input, and handle it appropriately.
     *   This will result in calls to the curses interface, typically. */
     
    for (;;){

        /* Reset the fd_set, and watch for input from GDB or stdin */
        FD_ZERO(&rset);
        
        FD_SET(STDIN_FILENO, &rset);
        FD_SET(gdb_fd, &rset);
        FD_SET(tty_fd, &rset);
        FD_SET(tgdb_readline_fd, &rset);
        FD_SET(resize_pipe[0], &rset);

        /* Wait for input */
        if (select(max + 1, &rset, NULL, NULL, NULL) == -1){
            if (errno == EINTR)
                continue;
            else {
                logger_write_pos ( logger, __FILE__, __LINE__, "select failed: %s", strerror(errno));
				return;
			}
        }

        /* Input received:  Handle it */
        if (FD_ISSET(STDIN_FILENO, &rset))
            if ( user_input() == -1 ) {
                logger_write_pos ( logger, __FILE__, __LINE__, "user_input failed");
                return;
            }

        /* gdb's output -> stdout ( triggered from readline input ) */
        if (FD_ISSET(tgdb_readline_fd, &rset))
            if ( tgdb_readline_input() == -1 )
                return;

        /* child's ouptut -> stdout
         * The continue is important I think. It allows all of the child
         * output to get written to stdout before tgdb's next command.
         * This is because sometimes they are both ready.
         */
        if (FD_ISSET(tty_fd, &rset)) {
            if ( child_input() == -1 )
                return;
            continue;
        }

        /* gdb's output -> stdout */
        if (FD_ISSET(gdb_fd, &rset))
            if ( gdb_input() == -1 )
                return;

        /* A resize signal occured */
        if (FD_ISSET(resize_pipe[0], &rset))
            if ( cgdb_resize_term(resize_pipe[0]) == -1)
                return;
    }
}

/* ----------------- */
/* Exposed Functions */
/* ----------------- */

/* cleanup: Invoked by the various err_xxx funtions when dying.
 * -------- */
void cleanup()
{
	char *log_file, *tmp_log_file;
	int has_recv_data;

    /* Cleanly scroll the screen up for a prompt */
    scrl(1);
    move(LINES-1, 0);
    printf("\n");

    /* The order of these is important. They each must restore the terminal
     * the way they found it. Thus, the order in which curses/readline is 
     * started, is the reverse order in which they should be shutdown 
     */

    /* Shut down interface */
    if_shutdown();

	/* Finally, should display the errors. 
	 * TGDB guarentees the logger to be open at this point.
	 * So, we can get the filename directly from the logger 
	 */
	logger_get_file ( logger, &tmp_log_file );
	log_file = strdup ( tmp_log_file );
	logger_has_recv_data ( logger, &has_recv_data );
   
    /* Shut down debugger */
    tgdb_shutdown( tgdb );

    tty_reset(STDIN_FILENO);

	if ( has_recv_data )
		fprintf ( stderr, "CGDB had unexpected results, see %s for details.\n", log_file );

	free ( log_file );
	log_file = NULL;
}

int init_resize_pipe(void) {
    if ( pipe(resize_pipe) == -1 ) {
        logger_write_pos ( logger, __FILE__, __LINE__, "pipe error");
        return -1;
    }

    return 0;
}

int main(int argc, char *argv[]) {
/* Uncomment to debug and attach */
#if 0
    int c;
    read(0, &c, 1);
#endif

    parse_long_options(&argc, &argv);

	/* First create tgdb, because it has the error log */
    if (start_gdb(argc, argv) == -1) {
        fprintf ( stderr, "%s:%d Unable to invoke GDB", __FILE__, __LINE__); 
        exit(-1);
    }

	/* From here on, the logger is initialized */

    /* Create the home directory */
    if ( init_home_dir() == -1 ) {
        logger_write_pos ( logger, __FILE__, __LINE__, "Unable to create home dir ~/.cgdb"); 
        cleanup();
        exit(-1);
	}

    if ( create_help_file() == -1 ) {
        logger_write_pos ( logger, __FILE__, __LINE__,  "Unable to create help file"); 
        cleanup();
        exit(-1);
	}

    if ( tty_cbreak(STDIN_FILENO) == -1 ) {
        logger_write_pos ( logger, __FILE__, __LINE__, "tty_cbreak error");
        cleanup();
        exit(-1);
	}

	kui_ctx = kui_manager_create ( STDIN_FILENO );
	if ( !kui_ctx  ) {
        logger_write_pos ( logger, __FILE__, __LINE__, "Unable to initialize input library"); 
        cleanup();
        exit(-1);
	}

    /* Initialize the display */
    switch (if_init()){
        case 1:
            logger_write_pos ( logger, __FILE__, __LINE__, "Unable to initialize the curses library");
			cleanup();
			exit(-1);
        case 2:
            logger_write_pos ( logger, __FILE__, __LINE__, "Unable to handle signal: SIGWINCH");
			cleanup();
			exit(-1);
        case 3:
            logger_write_pos ( logger, __FILE__, __LINE__, "Unreasonable terminal size -- too small");
			cleanup();
			exit(-1);
        case 4:
            logger_write_pos ( logger, __FILE__, __LINE__, "New GDB window failed -- out of memory?");
			cleanup();
			exit(-1);
    }

    /* Initialize the pipe that is used for resize */
    if( init_resize_pipe() == -1 ) {
        logger_write_pos ( logger, __FILE__, __LINE__, "init_resize_pipe error"); 
		cleanup();
		exit(-1);
	}

    {
        char config_file[ FSUTIL_PATH_MAX ];
        FILE *config;
        fs_util_get_path( cgdb_home_dir, "cgdbrc", config_file );
        config = fopen( config_file, "r" );
        if( config ) { 
            command_parse_file( config );
            fclose( config );
        }
    }

    /* Enter main loop */
    main_loop();

    /* Shut down curses and exit */
    cleanup();
    return 0;
}

int cgdb_set_esc_sequence_timeout ( int msec ) {
	return kui_manager_set_terminal_escape_sequence_timeout ( kui_ctx, msec );
}
