/* cgdb.c:
 * -------
 */
#include "config.h"

/* System Includes */
#include <curses.h>
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

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

/* --------------- */
/* Local Variables */
/* --------------- */

char cgdb_home_dir[MAXLINE];  /* Path to home directory with trailing slash */
char cgdb_help_file[MAXLINE]; /* Path to home directory with trailing slash */

static char *my_name = NULL;  /* Name of this application (argv[0]) */
static int gdb_fd = -1;       /* File descriptor for GDB communication */
static int tty_fd = -1;       /* File descriptor for process being debugged */

static char *debugger_path = NULL;  /* Path to debugger to use */

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
   char cgdbConfigDir[MAXLINE];
   char homeDir[MAXLINE];
   char *env = getenv("HOME");
   struct stat st;

#ifdef HAVE_CYGWIN
   char win32_path[MAXLINE];
   char win32_homedir[MAXLINE];
   extern void cygwin_conv_to_full_win32_path(const char *path, char *win32_path);
#endif

   if(env == NULL)
      err_quit("%s:%d -> $HOME is not set", __FILE__, __LINE__);

   sprintf( cgdbConfigDir, "%s/.cgdb", env );

#ifdef HAVE_CYGWIN
   cygwin_conv_to_full_win32_path(cgdbConfigDir, win32_path);
   strncpy( cgdbConfigDir, win32_path, strlen(win32_path));
   cygwin_conv_to_full_win32_path(homeDir, win32_homedir);
   strncpy( homeDir, env, strlen(env));
#else 
   strncpy( homeDir, env, strlen(env));
#endif

   /* Check to see if already exists, if does not exist continue */
   if ( stat( cgdbConfigDir, &st ) == -1 && errno == ENOENT ) {
       /* Create home config directory if does not exist */
       if ( access( env, R_OK | W_OK ) == -1 )
           return -1;

       if ( mkdir( cgdbConfigDir, 0755 ) == -1 )
           return -1;
   }

#ifdef HAVE_CYGWIN
   sprintf( cgdb_home_dir, "%s\\", cgdbConfigDir );
#else
   sprintf( cgdb_home_dir, "%s/", cgdbConfigDir );
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

static int start_gdb(int argc, char *argv[]) {
    if ( tgdb_init(debugger_path, argc, argv, &gdb_fd, &tty_fd) == -1 )
        return 1;

    return 0;
}

static void user_input() {
    char *result;
    int key;

    /* Grab a character from the user */
    while ((key = getch()) != ERR){
        /* Check for special keys */
        switch (if_input(key)){
            case 1:
               /* Normal key, send it to GDB */
               if ( CGDB_BACKSPACE_KEY(key) )
                   key = '\b';

               if ( key == KEY_DC )
                   key = 127;

               /* Normal key, send it to GDB */
               if ((result = tgdb_send((char) key)))
                   if_print(result);
               else
                   err_quit("%s: Unable to send data to GDB\n", my_name);

               break;
            case 2: /* Send to child */
               /* Normal key, send it to GDB */
               if ( CGDB_BACKSPACE_KEY(key) )
                   key = '\b';

               if ( key == KEY_DC )
                   key = 127;

               /* Normal key, send it to TTY */
               if ((result = tgdb_tty_send((char) key)))
                   if_tty_print(result);
               else
                   err_quit("%s: Unable to send data to GDB\n", my_name);
               break;
                
            case -1:
                err_quit("%s: Unreasonable terminal size\n", my_name);
                break;
            case -2:
                cleanup();
                exit(0);
                break;
                
            /* Translate curses arrow keys to something more normal */
            case KEY_UP:
                tgdb_send((char)033);
                tgdb_send((char)0133);
                tgdb_send((char)0101);
                break;
            case KEY_DOWN:
                tgdb_send((char)033);
                tgdb_send((char)0133);
                tgdb_send((char)0102);
                break;
            case KEY_LEFT:
                tgdb_send((char)033);
                tgdb_send((char)0133);
                tgdb_send((char)0104);
                break;
            case KEY_RIGHT:
                tgdb_send((char)033);
                tgdb_send((char)0133);
                tgdb_send((char)0103);
                break;

            /* Translate curses arrow keys to something more normal */
            case KEY_UP + KEY_UP:
                tgdb_tty_send((char)033);
                tgdb_tty_send((char)0133);
                tgdb_tty_send((char)0101);
                break;
            case KEY_DOWN + KEY_DOWN:
                tgdb_tty_send((char)033);
                tgdb_tty_send((char)0133);
                tgdb_tty_send((char)0102);
                break;
            case KEY_LEFT + KEY_LEFT:
                tgdb_tty_send((char)033);
                tgdb_tty_send((char)0133);
                tgdb_tty_send((char)0104);
                break;
            case KEY_RIGHT + KEY_RIGHT:
                tgdb_tty_send((char)033);
                tgdb_tty_send((char)0133);
                tgdb_tty_send((char)0103);
                break;
        }
    }
}

static void process_commands(struct Command ***commands)
{
    static char filename[PATH_MAX];
    static int updating_breakpts = 0;
    char *com;
    int i;

    for (i = 0; *commands != NULL && (*commands)[i] != NULL; i++){
        com = (*commands)[i]->data;
        switch ((*commands)[i] -> header){
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
                char path[PATH_MAX];
                char enabled;
                int line;
                sscanf(com, "%s %c %d %s", 
                      file, &enabled, &line, file);
                if (realpath(file,  path)){
                    if (enabled == 'y')
                        source_enable_break(sview, path, line);
                    else
                        source_disable_break(sview, path, line);
                }
                break;
            }
                
            /* Show a particular file */
            case SOURCE_FILE_UPDATE:
                strncpy(filename, com, PATH_MAX-1);
                filename[PATH_MAX-1] = 0; 
                break;

            /* Jump to line number */
            case LINE_NUMBER_UPDATE:
                if_show_file(filename, atoi(com));
                break;
                
            /* Clearing the list of choices the user had last */
            case SOURCES_START:
                if_clear_filedlg();
                break;

            /* Send the file the user picked to get the absolute path */
            case SOURCES_END:
                if_show_filedlg(filename);
                tgdb_get_source_absolute_filename(filename);
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
    }

    tgdb_delete_command(commands);
}

static void gdb_input()
{
    char *buf = malloc(GDB_MAXBUF+1);
    struct Command **commands;
    int size;

    /* Read from GDB */
    size = tgdb_recv(buf, GDB_MAXBUF, &commands);
    if (size == -1){
        err_quit("%s: Error while reading from GDB\n", my_name);
        return;
    }
    buf[size] = 0;

    process_commands(&commands);

    /* Display GDB output */
    if_print(buf);
}

static void child_input()
{
    char *buf = malloc(GDB_MAXBUF+1);
    int size;

    /* Read from GDB */
    size = tgdb_tty_recv(buf, GDB_MAXBUF);
    if (size == -1){
        err_quit("%s: Error while reading from GDB\n", my_name);
        return;
    }
    buf[size] = 0;

    /* Display CHILD output */
    if_tty_print(buf);
}

static void main_loop(void)
{
    fd_set set;
    int max = (gdb_fd > STDIN_FILENO) ? gdb_fd : STDIN_FILENO;
    max = ((max > tty_fd) ? max : tty_fd) + 1;

    /* Main (infinite) loop:
     *   Sits and waits for input on either stdin (user input) or the
     *   GDB file descriptor.  When input is received, wrapper functions
     *   are called to process the input, and handle it appropriately.
     *   This will result in calls to the curses interface, typically. */
     
    for (;;){

        /* Reset the fd_set, and watch for input from GDB or stdin */
        FD_ZERO(&set);
        FD_SET(STDIN_FILENO, &set);
        FD_SET(gdb_fd, &set);
        FD_SET(tty_fd, &set);
    
        /* Wait for input */
        if (select(max, &set, NULL, NULL, NULL) == -1){
            if (errno == EINTR){
                user_input();
                continue;
            }
            else
                err_dump("%s: select failed: %s\n", my_name, strerror(errno));
        }

        /* Input received:  Handle it */
        if (FD_ISSET(STDIN_FILENO, &set))
            user_input();
            
        if (FD_ISSET(gdb_fd, &set))
            gdb_input();

        if (FD_ISSET(tty_fd, &set))
            child_input();
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

    /* Shut down interface */
    if_shutdown();
   
    /* Shut down debugger */
    tgdb_shutdown();
}

int main(int argc, char *argv[]) {
    parse_long_options(&argc, &argv);

    /* Set up some data */
    my_name = argv[0];

    /* Create the home directory */
    if ( init_home_dir() == -1 )
        err_quit("%s: Unable to create home dir ~/.cgdb\n", my_name); 

    if ( create_help_file() == -1 )
        err_quit("%s: Unable to create help file\n", my_name); 
    
    /* Start GDB */
    if (start_gdb(argc, argv))
        err_quit("%s: Unable to invoke GDB\n", my_name); 
    
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

    /* Enter main loop */
    main_loop();

    /* Shut down curses and exit */
    cleanup();
    return 0;
}

