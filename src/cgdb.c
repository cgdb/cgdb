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

/* This is for PATH_MAX */
#if HAVE_LIMITS_H
#include <limits.h>
#endif /* HAVE_LIMITS_H */

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif 

/* Local Includes */
#include "cgdb.h"
#include "error.h"
#include "interface.h"
#include "scroller.h"
#include "sources.h"
#include "tgdb.h"
#include "helptext.h"
#include "queue.h"
#include "input.h"

/* --------------- */
/* Local Variables */
/* --------------- */

char cgdb_home_dir[MAXLINE];  /* Path to home directory with trailing slash */
char cgdb_help_file[MAXLINE]; /* Path to home directory with trailing slash */

static char *my_name = NULL;  /* Name of this application (argv[0]) */
static int gdb_fd = -1;       /* File descriptor for GDB communication */
static int tty_fd = -1;       /* File descriptor for process being debugged */
static int tgdb_readline_fd = -1; /* tgdb returns the 'at prompt' data */

static char *debugger_path = NULL;  /* Path to debugger to use */
struct queue *commandq;

struct input *input = NULL;         /* Initialize the input package */

int resize_pipe[2] = { -1, -1 };

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
            "Copyright 2002-2003 Robert Rossi and Mike Mueller.\n"
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
        { "help", 0, 0, 0 }
    };

    opterr = 0;
    while ((c = getopt_long(*argc, *argv, args, long_options, &option_index)) != EOF) {
#else
    opterr = 0;
    while ((c = getopt(*argc, *argv, args)) != EOF) {
#endif
        switch (c) {
            case 0:
                switch ( option_index ) {
                    case 0:
                        printf("%s",version_info());
                        exit(0);
                    case 1:
                        usage();
                        break;
                    default:
                        break;
                }
                break;
            case 'v':
                printf("%s",version_info());
                exit(0);
            case 'd':
                debugger_path = optarg;
                n+=2;
                break;
            case 'h':
                usage();
                break;
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
    char cgdb_config_dir_unix_path[MAXLINE];
    char homeDir[MAXLINE];
    char *env = getenv("HOME");
    struct stat st;

#ifdef HAVE_CYGWIN
   char cgdb_config_dir_win_path[MAXLINE];
   char win32_homedir[MAXLINE];
   extern void cygwin_conv_to_full_win32_path(const char *path, char *win32_path);
#endif

   if(env == NULL)
      err_quit("%s:%d -> $HOME is not set", __FILE__, __LINE__);

   sprintf( cgdb_config_dir_unix_path, "%s/.cgdb", env );

#ifdef HAVE_CYGWIN
   cygwin_conv_to_full_win32_path(cgdb_config_dir_unix_path, cgdb_config_dir_win_path);
   strncpy( cgdb_config_dir_unix_path, cgdb_config_dir_win_path, strlen(cgdb_config_dir_win_path) + 1);
   cygwin_conv_to_full_win32_path(env, win32_homedir);
   strncpy( homeDir, win32_homedir, strlen(win32_homedir) + 1);
#else 
   strncpy( homeDir, env, strlen(env) + 1);
#endif

   /* Check to see if already exists, if does not exist continue */
   if ( stat( cgdb_config_dir_unix_path, &st ) == -1 && errno == ENOENT ) {
       /* Create home config directory if does not exist */
       if ( access( env, R_OK | W_OK ) == -1 )
           return -1;

       if ( mkdir( cgdb_config_dir_unix_path, 0755 ) == -1 )
           return -1;
   }

#ifdef HAVE_CYGWIN
   sprintf( cgdb_home_dir, "%s\\", cgdb_config_dir_unix_path );
#else
   sprintf( cgdb_home_dir, "%s/", cgdb_config_dir_unix_path );
#endif

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


   sprintf(cgdb_help_file, "%shelp.txt", cgdb_home_dir);
   
   if ( (fd = fopen(cgdb_help_file, "w")) == NULL)
       return -1;

   /* Write cgdb version number */
   length = 5 + strlen(VERSION);
   dashes = (char *)malloc(sizeof(char) * (length));
   memset(dashes, '-', length);
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
    if ( tgdb_init() == -1 ) {
        err_msg("%s:%d tgdb_init error\n", __FILE__, __LINE__);
        return -1;
    }

    if ( tgdb_start(debugger_path, argc, argv, &gdb_fd, &tty_fd, &tgdb_readline_fd) == -1 )
        return -1;

    return 0;
}

static void send_key(int focus, int key) {
    if ( focus == 1 ) {
        if ( key == '\r' )
            tgdb_send_input('\n');
        else
            tgdb_send_input(key);
    } else if ( focus == 2 ) {
        tgdb_tty_send(key);
    }
}

/* user_input: This function will get a key from the user and process it.
 *
 *  Returns:  -1 on error, 0 on success
 */
static int user_input(void) {
    static int key, val;
    
    key = input_getkey(input);

    /* Process the key */
    switch ( ( val = if_input(key)) ) {
        case 1:
        case 2:
            if ( key >= CGDB_KEY_ESC && key < CGDB_KEY_ERROR ) {
                    char *seqbuf = input_get_last_seq(input);

                    if ( seqbuf == NULL ) {
                        err_msg("%s:%d input_get_last_seq error\n", __FILE__, __LINE__);
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

static void process_commands(struct queue *q)
{
    static char filename[PATH_MAX];
    static int line_num;
    static int updating_breakpts = 0;
    char *com;
    struct Command *item;

    while ( queue_size(q) > 0 ) {
        item = queue_pop(q);
        com = item->data;
        switch (item -> header){
            case BREAKPOINTS_BEGIN:
                if (!updating_breakpts){
                    updating_breakpts = 1;
                    source_clear_breaks(if_get_sview());
                }
                break;
                
            case BREAKPOINTS_END:
                updating_breakpts = 0;
                if_show_file(NULL, 0);
                break;
                
            case BREAKPOINT:
            {
                struct sviewer *sview = if_get_sview();
                char *file = malloc(strlen(com)+1);
                char enabled;
                int line;
                sscanf(com, "%s %c %d %s", 
                      file, &enabled, &line, file);
                if (enabled == 'y')
                    source_enable_break(sview, file, line);
                else
                    source_disable_break(sview, file, line);
                break;
            }
                
            /* Show a particular file */
            case SOURCE_FILE_UPDATE:
                strncpy(filename, com, PATH_MAX-1);
                filename[PATH_MAX-1] = 0; 
                break;

            /* Jump to line number */
            case LINE_NUMBER_UPDATE:
                line_num = atoi(com);
                break;

            case CURRENT_FILE_UPDATE:
                if_show_file(filename, line_num);
                source_set_relative_path(if_get_sview(), filename, com);
                break;
                
            /* Clearing the list of choices the user had last */
            case SOURCES_START:
                if_clear_filedlg();
                break;

            /* Send the file the user picked to get the absolute path */
            case SOURCES_END:
                if_set_focus(FILE_DLG);
                break;

            /* The user is trying to get a list of source files that make up
             * the debugged program but libtgdb is claiming that gdb knows
             * none. */
            case SOURCES_DENIED:
                if_display_message("Error:", 0,  
                       " No sources available! Was program compiled with debug?");
                break;

            /* Add the file to the user's choice */
            case SOURCE_FILE:
                if_add_filedlg_choice(com);
                break;

            /* This is the absolute path to the last file the user requested */
            case ABSOLUTE_SOURCE_ACCEPTED:
                if_show_file(com, 1);
                break;

            /* The source file requested does not exist */
            case ABSOLUTE_SOURCE_DENIED:
                if_show_file(NULL, 0 );
                /* com can be NULL when tgdb orig requests main file */
                if ( com[0] )
                    if_display_message("No such file:", 0, " %s", com);
                break;

            case QUIT:
                cleanup();            
                exit(0);
                break;
            /* Default */
            default:
                break;
        }
        tgdb_delete_command(item);
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
    size = tgdb_recv(buf, GDB_MAXBUF, commandq);
    if (size == -1){
        err_msg("%s:%d tgdb_recv error \n", __FILE__, __LINE__);
        return -1;
    }

    buf[size] = 0;

    process_commands(commandq);

    /* Display GDB output 
     * The strlen check is here so that if_print does not get called
     * when displaying the filedlg. If it does get called, then the 
     * gdb window gets displayed when the filedlg is up
     */
    if ( strlen( buf ) > 0 )
        if_print(buf);

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
    size = tgdb_tty_recv(buf, GDB_MAXBUF);
    if (size == -1){
        err_msg("%s:%d tgdb_tty_recv error \n", __FILE__, __LINE__);
        return -1;
    }
    buf[size] = 0;

    /* Display CHILD output */
    if_tty_print(buf);
    return 0;
}

static int tgdb_readline_input(void){
    char buf[MAXLINE];
    if ( tgdb_recv_input(buf) == -1 ) {
        err_msg("%s:%d tgdb_recv_input error\n", __FILE__, __LINE__);
        return -1;
    }
    if_print(buf);
    return 0;
}

static int cgdb_resize_term(int fd) {
    int c;
    read(resize_pipe[0], &c, sizeof(int) );

    if ( if_resize_term() == -1 ) {
        err_msg("%s:%d %s: Unreasonable terminal size\n", __FILE__, __LINE__, my_name);
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
            else
                err_dump("%s: select failed: %s\n", my_name, strerror(errno));
        }

        /* Input received:  Handle it */
        if (FD_ISSET(STDIN_FILENO, &rset))
            if ( user_input() == -1 ) {
                err_msg("%s:%d user_input failed\n", __FILE__, __LINE__);
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
   
    /* Shut down debugger */
    tgdb_shutdown();
}

int init_resize_pipe(void) {
    if ( pipe(resize_pipe) == -1 ) {
        err_msg("%s:%d pipe error\n", __FILE__, __LINE__);
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

    /* Set up some data */
    my_name = argv[0];

    /* Create the home directory */
    if ( init_home_dir() == -1 )
        err_quit("%s: Unable to create home dir ~/.cgdb\n", my_name); 

    if ( create_help_file() == -1 )
        err_quit("%s: Unable to create help file\n", my_name); 

    /* Start GDB */
    if (start_gdb(argc, argv) == -1)
        err_quit("%s: Unable to invoke GDB\n", my_name); 

    commandq = queue_init();

    if ( (input = input_init(STDIN_FILENO) ) == NULL )
        err_quit("%s: Unable to initialize input library\n", my_name); 

    /* Initialize the display */
    switch (if_init()){
        case 1:
            err_quit("%s: Unable to initialize the curses library\n", my_name);
        case 2:
            err_quit("%s: Unable to handle signal: SIGWINCH\n", my_name);
        case 3:
            err_quit("%s: Unreasonable terminal size -- too small\n", my_name);
        case 4:
            err_quit("%s: New GDB window failed -- out of memory?\n", my_name);
    }

    /* Initialize the pipe that is used for resize */
    if( init_resize_pipe() == -1 )
        err_quit("%s: init_resize_pipe error\n", my_name); 

    /* Enter main loop */
    main_loop();

    /* Shut down curses and exit */
    cleanup();
    return 0;
}

