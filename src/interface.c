/* interface.c:
 * ------------
 * 
 * Provides the routines for displaying the interface, and interacting with
 * the user via keystrokes.
 */

/* System Includes */
#include <curses.h>
#include <limits.h>
#include <signal.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>
#include <ctype.h>

/* Local Includes */
#include "cgdb.h"
#include "config.h"
#include "error.h"
#include "interface.h"
#include "scroller.h"
#include "sources.h"
#include "tgdb.h"
#include "filedlg.h"
#include "commands.h"

/* ----------- */
/* Prototypes  */
/* ----------- */

/* ----------- */
/* Definitions */
/* ----------- */

/* The offset that determines allows gdb/sources window to grow or shrink */
static int window_height_shift = 0;

/* This is for the tty I/O window */
static const int TTY_WIN_DEFAULT_HEIGHT = 4;
static int tty_win_height_shift = 0;
#define TTY_WIN_OFFSET ( TTY_WIN_DEFAULT_HEIGHT + tty_win_height_shift )

/* Height and width of the terminal */
#define HEIGHT      (screen_size.ws_row)
#define WIDTH       (screen_size.ws_col)

/* If the system doesn't offer KEY_RESIZE, just set it to a decent value
 * so that we can pass it around internally when SIGWINCH arrives. */
#ifndef KEY_RESIZE
#define KEY_RESIZE  KEY_MAX
#endif

/* --------------- */
/* Data Structures */
/* --------------- */

#if 0 /* This is defined in interface.h now. */
enum Focus {
    GDB,                 /* Input goes to gdb */
    TTY,                 /* Input goes to tty I/O window */
    CGDB                 /* Input goes to the source window ( CGDB ) */
};
#endif

enum Command_Type {
    CMD_LINE_NUMBER,     /* :123 line number */
    CMD_QUIT,            /* :q   command */
    CMD_QUIT_FORCE,      /* :q!  command */
    CMD_SET,             /* :set command */
    CMD_HELP,            /* :help command */
    CMD_UNKNOWN          /* Unknown command */
};

/* --------------- */
/* Local Variables */
/* --------------- */
static int curses_initialized = 0;      /* Flag: Curses has been initialized */
static int curses_colors      = 0;      /* Flag: Terminal supports colors */
static int resize             = 0;      /* Flag: Resize event occurred */
static struct scroller *gdb_win = NULL; /* The GDB input/output window */
static struct scroller  *tty_win = NULL;/* The tty input/output window */
static int tty_win_on = 0;              /* Flag: tty window being shown */
static struct sviewer  *src_win = NULL; /* The source viewer window */
static WINDOW *status_win = NULL;       /* The status line */
static WINDOW *tty_status_win = NULL;   /* The tty status line */
static enum Focus focus = CGDB;         /* Which pane is currently focused */
static struct winsize screen_size;      /* Screen size */

struct filedlg *fd;                     /* The file dialog structure */
static char regex_line[MAX_LINE];       /* The regex the user enters */
static int regex_line_pos;              /* The index into the current regex */
static int regex_search;                /* Currently searching text ? */
static int regex_direction;             /* Direction to search */
/*static*/ int regex_icase = 0;             /* Case insensitive (0), sensitive */
/*static*/ int shortcut_option = 0;         /* Flag: Shortcut options enabled */
static char last_key_pressed = 0;       /* Last key user entered in cgdb mode */

static char cur_com_line[MAX_LINE];     /* The line number the user entered */
static int cur_com_pos;                 /* The index into the current line */
static int display_command;             /* Currently going to a line number */

/* --------------- */
/* Local Functions */
/* --------------- */

/* init_curses: Initializes curses and sets up the terminal properly.
 * ------------
 *
 * Return Value: Zero on success, non-zero on failure.
 */
static int init_curses()
{
    if ( putenv("ESCDELAY=0") == -1 )
       fprintf(stderr, "(%s:%d) putenv failed\r\n", __FILE__, __LINE__);

    initscr();                       /* Start curses mode */
    cbreak();                        /* Line buffering disabled */
    noecho();                        /* Do not echo characters typed by user */
    keypad(stdscr, TRUE);            /* Translate arrow keys, Fn keys, etc. */
    timeout(0);                      /* Use non-blocking I/O */
    refresh();                       /* Refresh the initial window once */

    if ((curses_colors = has_colors())){
        start_color();
        init_pair(CGDB_COLOR_GREEN, COLOR_GREEN, COLOR_BLACK);
        init_pair(CGDB_COLOR_RED, COLOR_RED, COLOR_BLACK);
        init_pair(CGDB_COLOR_CYAN, COLOR_CYAN, COLOR_BLACK);
        init_pair(CGDB_COLOR_WHITE, COLOR_WHITE, COLOR_BLACK);
        init_pair(CGDB_COLOR_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(CGDB_COLOR_BLUE, COLOR_BLUE, COLOR_BLACK);
        init_pair(CGDB_COLOR_YELLOW, COLOR_YELLOW, COLOR_BLACK);
        init_pair(CGDB_COLOR_INVERSE_GREEN, COLOR_BLACK, COLOR_GREEN );
        init_pair(CGDB_COLOR_INVERSE_RED, COLOR_BLACK, COLOR_RED);
        init_pair(CGDB_COLOR_INVERSE_CYAN, COLOR_BLACK, COLOR_CYAN);
        init_pair(CGDB_COLOR_INVERSE_WHITE, COLOR_BLACK, COLOR_WHITE);
        init_pair(CGDB_COLOR_INVERSE_MAGENTA, COLOR_BLACK, COLOR_MAGENTA);
        init_pair(CGDB_COLOR_INVERSE_BLUE, COLOR_BLACK, COLOR_BLUE);
        init_pair(CGDB_COLOR_INVERSE_YELLOW, COLOR_BLACK, COLOR_YELLOW);
        init_pair(CGDB_COLOR_STATUS_BAR, COLOR_BLACK, COLOR_WHITE);
    }

    curses_initialized = 1;
    return 0;
}

/* --------------------------------------
 * Theses get the position of each window
 * -------------------------------------- */

/* These are for the source window */
static int get_src_row(void) {
    return 0;
}

static int get_src_col(void) {
    return 0;
}

static int get_src_height(void) {
    return ((int)(((screen_size.ws_row+0.5)/2) - window_height_shift));
}

static int get_src_width(void) {
    return (screen_size.ws_col);
}

/* This is for the source window status bar */
static int get_src_status_row(void) {
    /* Usually would be 'get_src_row() + get_src_height()' but
     * the row is 0 */
    return get_src_height();
}

static int get_src_status_col(void) {
    return 0;
}

static int get_src_status_height(void) {
    return 1;
}

static int get_src_status_width(void) {
    return (screen_size.ws_col);
}

/* This is for the tty I/O window */
static int get_tty_row(void) {
    return get_src_status_row() + get_src_status_height();
}

static int get_tty_col(void) {
    return 0;
}

static int get_tty_height(void) {
    return TTY_WIN_OFFSET;
}

static int get_tty_width(void) {
    return (screen_size.ws_col);
}

/* This is for the tty I/O status bar line */
static int get_tty_status_row(void) {
    return get_tty_row() + get_tty_height();
}

static int get_tty_status_col(void) {
    return 0;
}

static int get_tty_status_height(void) {
    return 1;
}

static int get_tty_status_width(void) {
    return (screen_size.ws_col);
}

/* This is for the debugger window */
static int get_gdb_row(void) {
    if ( tty_win_on )
        return get_tty_status_row() + get_tty_status_height();

    return get_src_status_row() + get_src_status_height();
}

static int get_gdb_col(void) {
    return 0;
}

static int get_gdb_height(void) {
    int window_size = ((screen_size.ws_row/2) + window_height_shift - 1);
    int odd_screen_size = (screen_size.ws_row%2);

    if ( tty_win_on ) 
        return window_size - TTY_WIN_OFFSET + odd_screen_size - 1;

    return window_size + odd_screen_size;
}

static int get_gdb_width(void) {
    return (screen_size.ws_col);
}

/* ---------------------------------------
 * Below is the core body of the interface
 * --------------------------------------- */

/* Updates the status bar */
static void update_status_win(void) {
    int pos;
    char filename[PATH_MAX];

    /* Update the tty status bar */
    if ( tty_win_on ) {
        wattron(tty_status_win, COLOR_PAIR(CGDB_COLOR_STATUS_BAR));
        for ( pos = 0; pos < WIDTH; pos++)
           mvwprintw(tty_status_win, 0, pos, " ");

        mvwprintw(tty_status_win, 0, 0, tgdb_tty_name());
        wattroff(tty_status_win, COLOR_PAIR(CGDB_COLOR_STATUS_BAR));
    }

    /* Print white background */
    wattron(status_win, COLOR_PAIR(CGDB_COLOR_STATUS_BAR));
    for ( pos = 0; pos < WIDTH; pos++)
       mvwprintw(status_win, 0, pos, " ");
    if ( tty_win_on )
       wattron(tty_status_win, COLOR_PAIR(CGDB_COLOR_STATUS_BAR));
    /* Show the user which window is focused */
    if ( focus == GDB )
       mvwprintw(status_win, 0, WIDTH - 1, "*");
    else if ( focus == TTY && tty_win_on )
       mvwprintw(tty_status_win, 0, WIDTH - 1, "*");
    else if ( focus == CGDB )
       mvwprintw(status_win, 0, WIDTH - 1, " ");
    wattroff(status_win, COLOR_PAIR(CGDB_COLOR_STATUS_BAR));
    if ( tty_win_on )
       wattroff(tty_status_win, COLOR_PAIR(CGDB_COLOR_STATUS_BAR));
    
    /* Print the regex that the user is looking for Forward */
    if ( regex_search && regex_direction){
        if_display_message("/", WIDTH- 1, "%s", regex_line);
        curs_set(1);
    }
    /* Regex backwards */
    else if ( regex_search ){
        if_display_message("?", WIDTH - 1, "%s", regex_line);
        curs_set(1);
    }
    /* Line number search */
    else if ( display_command ){
        if_display_message(":", WIDTH - 1, "%s", cur_com_line);
        curs_set(1);
    }
    /* Default: Current Filename */
    else {
        /* Print filename */
        if (src_win != NULL && source_current_file(src_win, filename) != NULL)
            if_display_message("", WIDTH - 1, "%s", filename);
    }
    
    wrefresh(status_win);
}

void if_display_message(const char *msg, int width, const char *fmt, ...) {
    va_list ap;
    char va_buf[MAXLINE];
    char buf_display[MAXLINE];
    int pos, error_length, length;
    curs_set(0);

    if ( !width )
        width = WIDTH;

    /* Get the buffer with format */
    va_start(ap, fmt);
#ifdef   HAVE_VSNPRINTF
    vsnprintf(va_buf, sizeof(va_buf), fmt, ap);  /* this is safe */
#else
    vsprintf(va_buf, fmt, ap);             /* this is not safe */
#endif
    va_end(ap);

    error_length = strlen(msg);
    length = strlen(va_buf);

    if ( error_length > width )
        strcat(strncpy(buf_display, msg , width - 1), ">");
    else if ( error_length + length > width )
        sprintf(buf_display, "%s>%s", msg, va_buf + (length - (width - error_length) + 1));
    else 
        sprintf(buf_display, "%s%s", msg, va_buf);

    /* Print white background */
    wattron(status_win, COLOR_PAIR(CGDB_COLOR_STATUS_BAR));
    for ( pos = 0; pos < WIDTH; pos++)
       mvwprintw(status_win, 0, pos, " ");

    mvwprintw(status_win, 0, 0, "%s" , buf_display);
    wattroff(status_win, COLOR_PAIR(CGDB_COLOR_STATUS_BAR));
    wrefresh(status_win);
}

/* if_draw: Draws the interface on the screen.
 * --------
 */
void if_draw( void )
{
    update_status_win();

    if ( get_src_height() != 0 && get_gdb_height() != 0 )
        wrefresh(status_win);    

    if ( tty_win_on )
        wrefresh(tty_status_win);

    if ( get_src_height() > 0 )
        source_display(src_win, focus == CGDB);

    if ( tty_win_on && get_tty_height() > 0 )
        scr_refresh(tty_win, focus == TTY);

    if ( get_gdb_height() > 0 )
        scr_refresh(gdb_win, focus == GDB);

    if ( get_src_height() == 0 || get_gdb_height() == 0 ) {
        curs_set(0);
        wrefresh(status_win);
    }
}

/* if_layout: Update the layout of the screen based on current terminal size.
 * ----------
 *
 * Return Value: Zero on success, non-zero on failure.
 */
static int if_layout()
{
    /* Verify the window size is reasonable */
    if (COLS < 20 || LINES < 10)
        return 1;

    /* Make sure that the windows offset is within its bounds: 
     * This checks the window offset.
     * */
    if ( window_height_shift >= HEIGHT/2 )
       window_height_shift = HEIGHT/2;
    else if ( window_height_shift <=  -(HEIGHT/2 - ((HEIGHT+1)%2)))
       window_height_shift = -(HEIGHT/2 - ((HEIGHT+1)%2));

    /* Initialize the GDB I/O window */
    if (gdb_win == NULL){
        gdb_win = scr_new(get_gdb_row(), get_gdb_col(), get_gdb_height(), get_gdb_width());
        if (gdb_win == NULL)
            return 2;
    } else{     /* Resize the GDB I/O window */
       if ( get_gdb_height() > 0 )
          scr_move(gdb_win, get_gdb_row(), get_gdb_col(), get_gdb_height(), get_gdb_width());
    }

    /* Initialize TTY I/O window */
    if (tty_win == NULL){
        tty_win = scr_new(get_tty_row(), get_tty_col(), get_tty_height(), get_tty_width());
        if (tty_win == NULL)
            return 2;
    } else{     /* Resize the GDB I/O window */
       if ( get_tty_height() > 0 )
          scr_move(tty_win, get_tty_row(), get_tty_col(), get_tty_height(), get_tty_width());
    }

    /* Initialize the source viewer window */
    if (src_win == NULL){
        src_win = source_new(get_src_row(), get_src_col(), get_src_height(), get_src_width());
        if (src_win == NULL)
            return 3;
    } else{ /* Resize the source viewer window */
       if ( get_src_height() > 0 )
          source_move(src_win, get_src_row(), get_src_col(), get_src_height(), get_src_width());
    }

    /* Initialize the status bar window */
    status_win = newwin(get_src_status_height(), get_src_status_width(),
                        get_src_status_row(), get_src_status_col());

    /* Initialize the tty status bar window */
    if ( tty_win_on )
        tty_status_win = newwin(get_tty_status_height(), get_tty_status_width(),
                                get_tty_status_row(), get_tty_status_col());

    if_draw();
    
    return 0;
}

/* if_resize: Checks if a resize event occurred, and updates display if so.
 * ----------
 *
 * Return Value:  Zero on success, non-zero on failure.
 */
static int if_resize()
{
    if (ioctl(fileno(stdout), TIOCGWINSZ, &screen_size) != -1){
#ifdef HAVE_NCURSES
        if (screen_size.ws_row != LINES || screen_size.ws_col != COLS){
            resizeterm(screen_size.ws_row, screen_size.ws_col);
            refresh();
            return if_layout();
        }
#else
        /* Stupid way to resize - should work on most systems */
        endwin();
        LINES = screen_size.ws_row;
        COLS  = screen_size.ws_col;
        refresh();
        source_hscroll(src_win, 0);
#endif
        return if_layout();
    }

    return 0;
}

/* increase_low_win: Makes gdb source window larger and source window smaller 
 * -----------------
 */ 
static void increase_low_win(void){
   int height = HEIGHT/2;

   if ( window_height_shift < height) {
      window_height_shift++;
      if_layout();
   }
}

/* decrease_low_win: Makes gdb source window smaller and source window larger 
 * -----------------
 */ 
static void decrease_low_win(void){
    int height = (HEIGHT/2) - ((tty_win_on)?TTY_WIN_OFFSET + 1:0);
    int odd_height = ((HEIGHT + 1)%2);
    
    if ( window_height_shift > -(height - odd_height)) {
        window_height_shift--;
        if_layout();
    }
}

/* increase_tty_win: Makes tty I/O window larger and source window smaller 
 * -----------------
 */ 
static void increase_tty_win(void){
   int height = get_gdb_height() + get_tty_height();

   /* Do nothing unless tty I/O window is displayed */
   if ( !tty_win_on )
       return;

   if ( tty_win_height_shift + TTY_WIN_DEFAULT_HEIGHT < height) {
      tty_win_height_shift++;
      if_layout();
   }
}

/* decrease_tty_win: Makes tty I/O window smaller and source window larger 
 * -----------------
 */ 
static void decrease_tty_win(void){
   /* Do nothing unless tty I/O window is displayed */
   if ( !tty_win_on )
       return;
    
    if ( tty_win_height_shift > -TTY_WIN_DEFAULT_HEIGHT) {
        tty_win_height_shift--;
        if_layout();
    }
}

/* signal_handler: Handles the WINCH signal (xterm resize).
 * ---------------
 */
static void signal_handler(int signo)
{
    if (signo == SIGWINCH){
        if (resize == 0)
            ungetch(KEY_RESIZE);
        resize = 1;
    }
}

/* if_get_command: Gets a command from the user
 * -----------------
 */ 
static void if_get_command(struct sviewer *sview) {
   int c;
   cur_com_pos = 0;
   cur_com_line[cur_com_pos] = '\0';
   display_command = 1;
   if_draw();

   while ( ( c = wgetch(sview->win) ) != ERR ) {
      if ( cur_com_pos == (MAX_LINE - 1) && !(c == 27 || c == 8 || c == 127 ))
          continue;
      
      /* Quit the search if the user hit escape */
      if ( c == 27 ) {
         cur_com_pos = 0;
         cur_com_line[cur_com_pos] = '\0';
         display_command = 0;
         if_draw();
         return;
      }

      /* If the user hit enter, then a successful regex has been recieved */
      if ( c == KEY_ENTER  || c == '\r' || c == '\n') {
         cur_com_line[cur_com_pos] = '\0';
         display_command = 0;
         return;
      }

      /* If the user hit backspace or delete remove a char */
      if ( CGDB_BACKSPACE_KEY(c)) {
         if (cur_com_pos > 0)
            --cur_com_pos;

         cur_com_line[cur_com_pos] = '\0';
         if_draw();

         update_status_win();
         wrefresh(status_win);
         continue;
      }

      /* Add a char, search and draw */
      cur_com_line[cur_com_pos++] = c;
      cur_com_line[cur_com_pos] = '\0';
      if_draw();
      update_status_win();
      wrefresh(status_win);
   }

   /* Finished */
   display_command = 0;
   if_draw();
}


#if 0 /* removed */
static enum Command_Type command_type(void)
{
    int start = 0;

    if (cur_com_pos == 0)
        return CMD_UNKNOWN;

    /* Check for a quit command */
    if (cur_com_pos == 1 && cur_com_line[0] == 'q')
        return CMD_QUIT;

    /* Check for a quit force (:q!) command */
    if (cur_com_pos == 2 && strncmp(cur_com_line, "q!", 2) == 0)
        return CMD_QUIT_FORCE;

    /* Check for a set command */
    if (cur_com_pos >= 4 && strncmp(cur_com_line, "set ", 4) == 0)
        return CMD_SET;

    /* Check for a help command */
    if (cur_com_pos >= 4 && strncmp(cur_com_line, "help", 4) == 0)
        return CMD_HELP;

    /* Check for a line number command */
    if ( cur_com_line[0] == '+' || cur_com_line[0] == '-' )
        start++;

    for ( ; start < cur_com_pos; start++ )
        if (!isdigit(cur_com_line[start]))
            return CMD_UNKNOWN;
    
    return CMD_LINE_NUMBER;
}
#endif

static void if_run_command(struct sviewer *sview) {
    /* Get a command and then try to process it */
    if_get_command(sview);

    /* refresh and return if the user entered no data */
    if ( cur_com_pos == 0 ) {
        if_draw();
        return;
    }

    if ( command_parse_string( cur_com_line ) ) {
        if_display_message("Unknown command: ", 0, "%s", cur_com_line);
    } else {
        update_status_win();
    }
    
#if 0
    switch (command_type()){
        case CMD_LINE_NUMBER:
            /* The user entered a line number */
            if ( cur_com_line[0] == '+' )
                source_vscroll(sview, atoi(cur_com_line + 1));
            else if ( cur_com_line[0] == '-' )
                source_vscroll(sview, -atoi(cur_com_line + 1));
            else 
                source_set_sel_line(sview, atoi(cur_com_line));
            if_draw();
            break;

        case CMD_QUIT:
            /* TODO: Test to see if debugged program is running */
            cleanup();
            exit(0);
            break;
        
        case CMD_QUIT_FORCE:
            cleanup();
            exit(0);
            break;

        case CMD_SET:
            set_com = cur_com_line + 4;
            /* Add 4 to size of string ( 'set ' ) */
            if ( cur_com_pos == 6 && strncmp(set_com, "ic", 2) == 0 ) 
                regex_icase = 1;
            else if ( cur_com_pos == 8 && strncmp(set_com, "noic", 4) == 0 ) 
                regex_icase = 0;
            else if ( cur_com_pos == 6 && strncmp(set_com, "sc", 2) == 0 ) 
                shortcut_option = 1;
            else if ( cur_com_pos == 8 && strncmp(set_com, "nosc", 4) == 0 ) 
                shortcut_option = 0;
            else
                goto error;
            update_status_win();
            break;
        case CMD_HELP:
            if_display_help();
            break;
        case CMD_UNKNOWN:
        error:
            if_display_message("Unknown command: ", 0, "%s", cur_com_line);
            break;
    }
#endif
}

/* capture_regex: Captures a regular expression from the user.
 * ---------------
 *
 *   sview:  Source viewer object
 *
 *  Side Effect: 
 *
 *  regex_line: The regex the user has entered.
 *  regex_line_pos: The next available index into regex_line.
 *
 * Return Value: 0 if user gave a regex, otherwise 1.
 */
static int capture_regex(struct sviewer *sview) {
   int c;
   
   /* Initialize the function for finding a regex and tell user */
   regex_search = 1;
   regex_line_pos = 0;
   regex_line[regex_line_pos] = '\0';
   if_draw();

   while ( ( c = wgetch(sview->win) ) != ERR ) {
      if ( regex_line_pos == (MAX_LINE - 1) && !(c == 27 || c == 8 || c == 127 ))
          continue;
      
      /* Quit the search if the user hit escape */
      if ( c == 27 ) {
         regex_line_pos = 0;
         regex_line[regex_line_pos] = '\0';
         regex_search = 0;
         source_search_regex(sview, regex_line, 2, regex_direction, regex_icase);
         if_draw();
         return 1;
      }

      /* If the user hit enter, then a successful regex has been recieved */
      if ( c == KEY_ENTER  || c == '\r' || c == '\n') {
         regex_line[regex_line_pos] = '\0';
         regex_search = 0;
         source_search_regex(sview, regex_line, 2, regex_direction, regex_icase);
         if_draw();
         return 0;
      }

      /* If the user hit backspace or delete remove a char */
      if ( CGDB_BACKSPACE_KEY(c)) {
         if (regex_line_pos > 0)
            --regex_line_pos;

         regex_line[regex_line_pos] = '\0';
         source_search_regex(sview, regex_line, 1, regex_direction, regex_icase);
         if_draw();

         update_status_win();
         continue;
      }

      /* Add a char, search and draw */
      regex_line[regex_line_pos++] = c;
      regex_line[regex_line_pos] = '\0';
      source_search_regex(sview, regex_line, 1, regex_direction, regex_icase);
      if_draw();
      update_status_win();
   }

   /* Finished */
   regex_search = 0;
   return 0;
}

/* tty_input: Handles user input to the tty I/O window.
 * ----------
 *
 *   key:  Keystroke received.
 *
 * Return Value:    0 if internal key was used, 
 *                  2 if input to tty,
 *                  -1        : Error resizing terminal -- terminal too small
 *                  KEY_UP    : Send up arrow key to GDB
 *                  KEY_DOWN  : Send down arrow key to GDB
 *                  KEY_LEFT  : Send left arrow key to GDB
 *                  KEY_RIGHT : Send right arrow key to GDB
 */
static int tty_input(int key)
{
    static enum { NORMAL, ESCAPE, SARROW } state = NORMAL;

    /* Handle special keys */
    switch (key){
        /* Special keys to pass back to GDB */
        case KEY_UP:
        case KEY_DOWN:
        case KEY_LEFT:
        case KEY_RIGHT:
            return key + key;
        case KEY_PPAGE:
            scr_up(tty_win, get_tty_height()-1);
            break;
        case KEY_NPAGE:
            scr_down(tty_win, get_tty_height()-1);
            break;
        case KEY_F(11):
            scr_home(tty_win);
            break;
        case KEY_F(12):
            scr_end(tty_win);
            break;
        /* Shift-arrow key support -- portable? */
        case 27:
            state = ESCAPE;
            break;
        case 91:
            if (state != ESCAPE)
                return 2;
            state++;
            break;
        case 97:
            if (state != SARROW)
                return 2;
            scr_up(tty_win, 1);
            state = NORMAL;
            break;
        case 98:
            if (state != SARROW)
                return 2;
            scr_down(tty_win, 1);
            state = NORMAL;
            break;        
        default:
            return 2;    
    }

    if_draw();

    return 0;
}

/* gdb_input: Handles user input to the GDB window.
 * ----------
 *
 *   key:  Keystroke received.
 *
 * Return Value:    0 if internal key was used, 
 *                  1 if input to gdb or ...
 *                  -1        : Error resizing terminal -- terminal too small
 *                  KEY_UP    : Send up arrow key to GDB
 *                  KEY_DOWN  : Send down arrow key to GDB
 *                  KEY_LEFT  : Send left arrow key to GDB
 *                  KEY_RIGHT : Send right arrow key to GDB
 */
static int gdb_input(int key)
{
    static enum { NORMAL, ESCAPE, SARROW } state = NORMAL;

    /* Handle special keys */
    switch (key){
        /* Special keys to pass back to GDB */
        case KEY_UP:
        case KEY_DOWN:
        case KEY_LEFT:
        case KEY_RIGHT:
            return key;
        case KEY_PPAGE:
            scr_up(gdb_win, get_gdb_height()-1);
            break;
        case KEY_NPAGE:
            scr_down(gdb_win, get_gdb_height()-1);
            break;
        case KEY_F(11):
            scr_home(gdb_win);
            break;
        case KEY_F(12):
            scr_end(gdb_win);
            break;
        /* Shift-arrow key support -- portable? */
        case 27:
            state = ESCAPE;
            break;
        case 91:
            if (state != ESCAPE)
                return 1;
            state++;
            break;
        case 97:
            if (state != SARROW)
                return 1;
            scr_up(gdb_win, 1);
            state = NORMAL;
            break;
        case 98:
            if (state != SARROW)
                return 1;
            scr_down(gdb_win, 1);
            state = NORMAL;
            break;        
        default:
            return 1;    
    }

    if_draw();

    return 0;
}

/* source_input: Handles user input to the source window.
 * -------------
 *
 *   sview:     Source viewer object
 *   key:       Keystroke received.
 */
static void source_input(struct sviewer *sview, int key)
{
    static enum { NORMAL, ESCAPE, ESCAPE2 } state = NORMAL;
    
    switch (key){
        case KEY_UP:
        case 'k':                            /* VI-style up-arrow */
            source_vscroll(sview, -1);
            break;
        case KEY_DOWN:
        case 'j':                            /* VI-style down-arrow */
            source_vscroll(sview, 1);
            break;
        case KEY_LEFT:
        case 'h':
            source_hscroll(sview, -1);
            break;
        case KEY_RIGHT:
        case 'l':
            source_hscroll(sview, 1);
            break;
        case KEY_PPAGE:
        case 'K':
            source_vscroll(sview, -(get_src_height() - 1));
            break;
        case KEY_NPAGE:
        case 'J':
            source_vscroll(sview, get_src_height() - 1);
            break;
        case 'g': /* beggining of file */
            if ( last_key_pressed == 'g' )
                source_set_sel_line(sview, 1);
            break;
        case 'G': /* end of file */
            source_set_sel_line(sview, 10000000);
            break;
        case 'f':
            source_next(sview);
            break;
        case 'b':
            source_previous(sview);
            break;
        case '=':
           /* Makes gdb source window larger and source window smaller */ 
           increase_low_win();
           break;
        case '-':
           /* Makes gdb source window smaller and source window larger */ 
           decrease_low_win();
           break;
        case '+':
           decrease_tty_win();
           break;
        case '_':
           increase_tty_win();
           break;
//        case '=':
//           /* Makes the windows the same size */
//           window_height_shift = 0;
//           if_layout();
//           break;
        case 'o':
           /* Causes file dialog to be opened */
           tgdb_get_sources();
           break;
        case ':':
           /* Allows user to go to a line number */ 
           if_run_command(sview);
           return;
        /* More potentially non-portable escape code bull: HOME and END */
        case 27:
            state = ESCAPE;
            break;
        case 91:
            if (state == ESCAPE)
                state++;
            break;
        case 55:
            if (state == ESCAPE2){
                state = NORMAL;
                if (sview->cur && sview->cur->buf.tlines)
                    source_vscroll(sview, -sview->cur->sel_line);
            }
            break;
        case 56:
            if (state == ESCAPE2){
                state = NORMAL;
                if (sview->cur && sview->cur->buf.tlines)
                    source_vscroll(sview, sview->cur->buf.length
                                            - sview->cur->sel_line - 1);
            }
            break;
        case ' ':
            {
                char *path;
                int   line = sview->cur->sel_line;
                char *command;

                if (!sview->cur || !sview->cur->path || !line)
                    return;

                /* Get filename (strip path off -- GDB is dumb) */
                path = strrchr(sview->cur->path, '/') + 1;
                if ((int) path == 1)
                    path = sview->cur->path;
                    
                command = (char *) malloc(strlen(path) + 20);
                sprintf(command, "%s %s:%d", 
                        sview->cur->buf.breakpts[line] ? "clear" : "break",
                        path, line+1);
                tgdb_run_command(command);
                free(command);
            }
            break;
        default:
            break;
    }

    /* Some extended features that are set by :set sc */
    if ( shortcut_option ) {
        switch ( key ) {
            case 'r': tgdb_run_command("run");      break;
            case 'n': tgdb_run_command("next");     break;
            case 's': tgdb_run_command("step");     break;
            case 'c': tgdb_run_command("continue"); break;
            case 'f': tgdb_run_command("finish");   break;
            case 'u': tgdb_run_command("up");       break;
            case 'd': tgdb_run_command("down");     break;
            default:                                break;
        }
    }

    if_draw();
}

/* Sets up the signal handler for SIGWINCH
 * Returns -1 on error. Or 0 on success */
static int set_up_signal(void) {
   struct sigaction action;

   action.sa_handler = signal_handler;      
   sigemptyset(&action.sa_mask);   
   action.sa_flags = 0;

   if(sigaction(SIGWINCH, &action, NULL) < 0) {
      err_ret("%s:%d -> sigaction failed ", __FILE__, __LINE__);
      return -1;
   }

   return 0;
}

/* ----------------- */
/* Exposed Functions */
/* ----------------- */

/* See interface.h for function descriptions. */
int if_init(void)
{
    if (init_curses())
        return 1;

    /* Set up the signal handler to catch SIGWINCH */
    if ( set_up_signal() == -1 )
        return 2;

    if (ioctl(fileno(stdout), TIOCGWINSZ, &screen_size) == -1){
        screen_size.ws_row = LINES;
        screen_size.ws_col = COLS;
    }

    /* Create the file dialog object */
    if ( ( fd = filedlg_new(0, 0, HEIGHT, WIDTH)) == NULL)
       return 5;
   
    switch (if_layout()){
        case 1:
            return 3;
        case 2:
            return 4;
    }

    return 0;    
}

int internal_if_input(int key) {
    /* The ESC Key, puts the debugger into command mode */
    if ( focus != CGDB && key == 27) {
       focus = CGDB;
       if_draw();
       return 0;
    } else if ( key == 27 ) 
        return 0;
   
    /* This happens no matter what the focus */
    if ( key == KEY_RESIZE ) {
        if (if_resize())
            return -1;
        resize = 0;
        return 0;
    }

    /* Check for global keystrokes */
    switch ( focus ) {
        case CGDB:
            switch(key){
                case 0: 
                    /* What is this? It happens when you hit CTRL+SPACE */
                    return 0;
                case 'i':
                    focus = GDB;
                    if_draw();
                    return 0;
                case 't':
                    if ( tty_win_on ) {
                       focus = TTY;
                       if_draw();
                    }
                    return 0;
                case 'm':
                    macro_start();
                    return 0;
                case 'M':
                    macro_load("macro_text.txt");
                    return 0;
                case 'S':
                    macro_save("macro_text.txt");
                    return 0;
                case '/':
                case '?':
                    regex_direction = ('/' == key);
                    
                    /* Capturing regular expressions */
                    source_search_regex_init(src_win);
                    capture_regex(src_win);
                    return 0;
                case 'n':
                     source_search_regex(src_win, regex_line, 2, regex_direction, regex_icase);
                     if_draw();
                     break;
                case 'N':
                     source_search_regex(src_win, regex_line, 2, !regex_direction, regex_icase);
                     if_draw();
                     break;
                case 'T':
                     if ( tty_win_on ) {
                         tty_win_on = 0;
                         focus = CGDB;
                     } else {
                         tty_win_on = 1;
                         focus = TTY;
                     }

                     if_layout();

                     break;
                case CONTROL_T:
                     if ( tgdb_new_tty() == -1 ) { 
                         /* Error */
                     } else {
                         scr_free(tty_win);
                         tty_win = NULL;
                         if_layout();
                     }

                     break;
                case KEY_F(1):
                     if_display_help();
                     return 0;
                case KEY_F(5):
                    /* Issue GDB run command */
                    tgdb_run_command("run");
                    return 0;
                case KEY_F(6):
                    /* Issue GDB continue command */
                    tgdb_run_command("continue");
                    return 0;
                case KEY_F(7):
                    /* Issue GDB finish command */
                    tgdb_run_command("finish");
                    return 0;
                case KEY_F(8):
                    /* Issue GDB next command */
                    tgdb_run_command("next");
                    return 0;
                case KEY_F(10):
                    /* Issue GDB step command */
                    tgdb_run_command("step");
                    return 0;
            }
            source_input(src_win, key);
            return 0;
            break;
        case TTY:
            return tty_input(key);
        case GDB:
            return gdb_input(key);
        default:
            /* Focus is screwed up, fix it */
            fprintf(stderr, "KEYBOARD ERROR\r\n");
            return 0;
    }
    
    /* Never gets here */
    return 0;
}

int if_input(int key) {
    int result= internal_if_input(key);
    last_key_pressed = key;
    return result;
}

void if_tty_print(const char *buf) {
    /* If the tty I/O window is not open send output to gdb window */
    if ( !tty_win_on )
        if_print(buf);

    /* Print it to the scroller */
    scr_add(tty_win, buf);

    /* Only need to redraw if tty_win is being displayed */
    if ( tty_win_on && get_gdb_height() > 0 ) {
        scr_refresh(tty_win, focus == TTY);

        /* Make sure cursor reappears in source window if focus is there */
        if (focus == CGDB)
           wrefresh(src_win->win);
    }
}
        
void if_print(const char *buf)
{
    /* Print it to the scroller */
    scr_add(gdb_win, buf);

    if ( get_gdb_height() > 0 ) {
        scr_refresh(gdb_win, focus == GDB);

        /* Make sure cursor reappears in source window if focus is there */
        if (focus == CGDB)
           wrefresh(src_win->win);
    }
}

void if_show_file(char *path, int line)
{
    if (source_set_exec_line(src_win, path, line) == 0)
       if_draw();
}

void if_display_help(void) {
    extern char cgdb_help_file[MAXLINE];
    int ret_val = 0;
    if (cgdb_help_file[0] && 
        (ret_val = source_set_exec_line(src_win, cgdb_help_file , 1)) == 0)
       if_draw();
    else if ( ret_val ==  5 )/* File does not exist */
        if_display_message("No such file: %s", 0, cgdb_help_file);
}


struct sviewer *if_get_sview()
{
    return src_win;
}

void if_clear_filedlg(void) {
   filedlg_clear(fd);
}

void if_add_filedlg_choice(const char *filename) {
   filedlg_add_file_choice(fd, filename);
}

void if_show_filedlg(char *filename) {
   filedlg_choose(fd, filename);
}

void if_filedlg_display_message(char *message) {
    filedlg_display_message(fd, message);
}

void if_shutdown(void)
{
    /* Shut down curses cleanly */
    if (curses_initialized){
        /*nocbreak(); */
        keypad(stdscr, FALSE); 
        echo();
        endwin();
    }

    if ( status_win != NULL )
        delwin(status_win);

    if ( tty_status_win != NULL )
        delwin(tty_status_win);

    if (gdb_win != NULL)
        scr_free(gdb_win);

    if (tty_win != NULL)
        scr_free(tty_win);

    if (src_win != NULL)
        source_free(src_win);
}

void if_set_focus( Focus f )
{
    switch ( f ) {
    case GDB:
        focus = f;
        if_draw();
        break;
    case TTY:
        if ( tty_win_on ) {
            focus = f;
            if_draw();
        }
        break;
    case CGDB:
        focus = f;
        if_draw();
        break;
    default:
        return;
    }
}

void if_tty_toggle( void )
{
    tty_win_on = !tty_win_on;
    if ( tty_win_on ) {
        if_set_focus( CGDB );
    } else {
        if_set_focus( TTY );
    }

    if_layout();
}

void if_search_next( void )
{
    source_search_regex(src_win, regex_line, 2, regex_direction, regex_icase);
    if_draw();
}
