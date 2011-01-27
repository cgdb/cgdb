#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_CURSES_H
#include <curses.h>
#elif HAVE_NCURSES_CURSES_H
#include <ncurses/curses.h>
#endif /* HAVE_CURSES_H */

#if HAVE_STDLIB_H 
#include <stdlib.h>
#endif  /* HAVE_STDLIB_H */

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#if HAVE_MATH_H
#include <math.h>
#endif /* HAVE_MATH_H */

#include "filedlg.h"
#include "cgdb.h"
#include "highlight.h"
#include "kui_term.h"
#include "highlight_groups.h"

struct file_buffer {
   int length;                     /* Number of files in program */
   char **files;                   /* Array containing file */
   char *cur_line;                 /* cur line may have unique color */
   int    max_width;                  /* Width of longest line in file */

   int               sel_line;     /* Current line selected in file dialog */
   int               sel_col;      /* Current column selected in file dialog */

   int               sel_col_rbeg; /* Current beg column matched in regex */
   int               sel_col_rend; /* Current end column matched in regex */
   int               sel_rline;    /* Current line used by regex */
};

struct filedlg{
   struct file_buffer    *buf;     /* All of the widget's data ( files ) */
   WINDOW *win;                    /* Curses window */
};

static char regex_line[MAX_LINE];       /* The regex the user enters */
static int regex_line_pos;              /* The index into the current regex */
static int regex_search;                /* Currently searching text ? */
static int regex_direction;             /* Direction to search */

/* print_in_middle: Prints the message 'string' centered at line in win 
 * ----------------
 *
 *  win:    Curses window
 *  line:   The line to print the message at
 *  width:  The width of the window
 *  string: The message to print
 */
static void print_in_middle(WINDOW *win, int line, int width, char *string) {  
   int  x, y;
   int j;
   int length = strlen(string);

   getyx(win, y, x);

   x = (int)((width-length)/2);
   
   wmove(win, line, 0);
   for (j = 0; j < x; j++)
       waddch(win, ' ');

   mvwprintw(win, line, x, "%s", string);

    for (j = x + length; j < width; j++)
        waddch(win, ' ');
}

struct filedlg *filedlg_new(int pos_r, int pos_c, int height, int width) {
   struct filedlg *fd;

    /* Allocate a new structure */
    if ((fd = malloc(sizeof(struct filedlg))) == NULL)
        return NULL;

    /* Initialize the structure */
    fd->win = newwin(height, width, pos_r, pos_c);
    keypad(fd->win, TRUE);

    /* Initialize the buffer */
    if ( ( fd->buf = malloc(sizeof(struct file_buffer))) == NULL)
        return NULL;

    fd->buf->length             = 0;
    fd->buf->files              = NULL;
    fd->buf->cur_line           = NULL;
    fd->buf->max_width          = 0;
    fd->buf->sel_line           = 0;
    fd->buf->sel_col            = 0;
    fd->buf->sel_col_rbeg       = 0;
    fd->buf->sel_col_rend       = 0;
    fd->buf->sel_rline          = 0;

    return fd;
}

void filedlg_free(struct filedlg *fdlg) {
   filedlg_clear(fdlg);
   delwin(fdlg->win);
   free(fdlg->buf);
   free(fdlg);
}

int filedlg_add_file_choice(struct  filedlg *fd, const char *file_choice) {
    int length;
    int index, i;
    int equal = 1;      /* Not set to 0, because 0 *is* equal */

    if ( file_choice == NULL || *file_choice == '\0' )
        return -1;

    /* find index to insert by comparing:
     * Absolute paths go to the end
     * Relative paths go before the absolute paths 
     */ 
    for ( i = 0; i < fd->buf->length; i++) {
        /* Don't add duplicate entry's ... gdb outputs duplicates */
        if ( ( equal = strcmp( fd->buf->files[i], file_choice)) == 0)
            return -3;
        else if ( equal < 0 ) {
            /* Inserting filename, stop before relative path */
            if ( (file_choice[0] != '.' && file_choice[0] != '/' ) && fd->buf->files[i][0] == '.' )
                break;

            /* Inserting filename, stop before absolute path */
            if ( file_choice[0] != '/' && fd->buf->files[i][0] == '/' )
                break;

        } else if ( equal > 0 ) {/* Found ( file_choice is greater ) */
            /* Inserting Absolute path, it goes to the end */
            if ( file_choice[0] == '/' && fd->buf->files[i][0] != '/' )
                continue;

            /* Inserting relative path, continue until before absolute or relative path */
            if ( file_choice[0] == '.' && (fd->buf->files[i][0] != '.' && fd->buf->files[i][0] != '/'))
                continue;

            break;
        }
    }

    index = i;

    fd->buf->length = fd->buf->length + 1;
    fd->buf->files  = realloc(fd->buf->files, sizeof(char *) * fd->buf->length);

    /* shift everything down and then insert into index */
    for ( i = fd->buf->length - 1; i > index ; i-- )
        fd->buf->files[i] = fd->buf->files[i-1];
    
    if ( (fd->buf->files[index] = strdup(file_choice)) == NULL )
        return -2;

    if ( ( length = strlen(file_choice)) > fd->buf->max_width )
        fd->buf->max_width = length;


    return 0;
}

void filedlg_clear(struct filedlg *fd) {
   int i;

   for ( i = 0; i < fd->buf->length; i++ )
       free(fd->buf->files[i]);

   free(fd->buf->files);
   fd->buf->files       = NULL;
   
   free(fd->buf->cur_line);
   fd->buf->cur_line    = NULL;

   fd->buf->max_width   = 0;
   fd->buf->length      = 0;
   fd->buf->sel_line           = 0;
   fd->buf->sel_col            = 0;
   fd->buf->sel_col_rbeg       = 0;
   fd->buf->sel_col_rend       = 0;
   fd->buf->sel_rline          = 0;
}

static void filedlg_vscroll(struct filedlg *fd, int offset) {
    if (fd->buf){
        fd->buf->sel_line += offset;
        if (fd->buf->sel_line < 0)
            fd->buf->sel_line = 0;
        /* The display message and status bar takes a line */
        if (fd->buf->sel_line >= fd->buf->length )
            fd->buf->sel_line = fd->buf->length - 1;
    }
}

static void filedlg_hscroll(struct filedlg *fd, int offset) {
    int lwidth;
    int max_width;

    if (fd->buf){
        lwidth    = (int) log10(fd->buf->length)+1;
        max_width = fd->buf->max_width - COLS + lwidth + 6;
        
        fd->buf->sel_col += offset;
        if (fd->buf->sel_col > max_width)
            fd->buf->sel_col = max_width;
        if (fd->buf->sel_col < 0)
            fd->buf->sel_col = 0;
    }
}

static void filedlg_search_regex_init(struct filedlg *fd) {
    if ( fd == NULL || fd->buf == NULL)
        return;

    /* Start from beggining of line if not at same line */
    if ( fd->buf->sel_rline != fd->buf->sel_line ) {
        fd->buf->sel_col_rend = 0;
        fd->buf->sel_col_rbeg = 0;
    }

    /* Start searching at the beggining of the selected line */
    fd->buf->sel_rline    = fd->buf->sel_line;
}

static int filedlg_search_regex(struct filedlg *fd, const char *regex,
                                int opt, int direction, int icase) {
   if ( fd == NULL    || fd->buf == NULL || 
        regex == NULL || strlen(regex) == 0)
      return -1;

   return hl_regex(regex, 
                  (const char **)fd->buf->files,
                  (const char **)fd->buf->files,
                  fd->buf->length,
                  &fd->buf->cur_line, &fd->buf->sel_line,
                  &fd->buf->sel_rline, &fd->buf->sel_col_rbeg,
                  &fd->buf->sel_col_rend, opt, direction, icase);
}

int filedlg_display( struct filedlg *fd ) {
    char fmt[5];
    int width, height;
    int lwidth;
    int file;
    int i;
    int attr;
    static char *label = "Select a file or press q to cancel.";

    curs_set(0);

    /* Check that a file is loaded */
    if (fd == NULL || fd->buf == NULL || fd->buf->files == NULL){
        wrefresh(fd->win);
        return 0;
    }

    /* Initialize variables */
    getmaxyx(fd->win, height, width);

    /* The status bar and display line 
     * Fake the display function to think the height is 2 lines less */
    height-=2;

    /* Set starting line number (center source file if it's small enough) */
    if (fd->buf->length < height)
        file = (fd->buf->length - height) / 2;
    else{
        file = fd->buf->sel_line - height / 2;
        if (file > fd->buf->length - height)
            file = fd->buf->length - height;
        else if (file < 0)
            file = 0;
    }

    /* Print 'height' lines of the file, starting at 'file' */
    lwidth = (int) log10(fd->buf->length)+1;
    sprintf(fmt, "%%%dd", lwidth);

    print_in_middle(fd->win, 0, width, label);
    wmove(fd->win, 0, 0);

    for (i = 1; i < height + 1; i++, file++){
        wmove(fd->win, i, 0);
        if (has_colors()){
            /* Outside of filename, just finish drawing the vertical file */
            if (file < 0 || file >= fd->buf->length){
                int j;
                for (j = 1; j < lwidth; j++)
                    waddch(fd->win, ' ');
                waddch(fd->win, '~');
                wattron(fd->win, A_BOLD);
                waddch(fd->win, VERT_LINE);
                wattroff(fd->win, A_BOLD);
                for (j = 2+lwidth; j < width; j++)
                    waddch(fd->win, ' ');
            }
            /* Mark the current file with an arrow */
            else if (file == fd->buf->sel_line){
                wattron(fd->win, A_BOLD);
                wprintw(fd->win, fmt, file+1);
                wattroff(fd->win, A_BOLD);
		if (hl_groups_get_attr (hl_groups_instance, HLG_ARROW, &attr) == -1)
		  return -1;
                wattron(fd->win, attr);
                waddch(fd->win, '-');
                waddch(fd->win, '>');
                wattroff(fd->win, attr);
                if ( fd->buf->cur_line != NULL )
                    hl_wprintw(fd->win, fd->buf->cur_line, width-lwidth-2, fd->buf->sel_col);
                else
                    hl_wprintw(fd->win, fd->buf->files[file], width-lwidth-2, fd->buf->sel_col);
            }
            /* Ordinary file */
            else{
                wprintw(fd->win, fmt, file+1);
                wattron(fd->win, A_BOLD);
                waddch(fd->win, VERT_LINE);
                wattroff(fd->win, A_BOLD);
                waddch(fd->win, ' ');
                
                /* No special file information */
                if ( file == fd->buf->sel_line && fd->buf->cur_line != NULL )
                    hl_wprintw(fd->win, fd->buf->cur_line, width-lwidth-2, fd->buf->sel_col);
                else
                    hl_wprintw(fd->win, fd->buf->files[file], width-lwidth-2, fd->buf->sel_col);
            }
        }
        else{
            wprintw(fd->win, "%s\n", fd->buf->files[file]);
        }
    }

    /* Add the 2 lines back in so the status bar can be drawn */
    height += 2;

    /* Update status bar */
    wmove(fd->win, height , 0);
    
    /* Print white background */
    if (hl_groups_get_attr (hl_groups_instance, HLG_STATUS_BAR, &attr) == -1)
      return -1;
    wattron(fd->win, attr);

    for ( i = 0; i < width; i++)
       mvwprintw(fd->win, height - 1, i, " ");

    if ( regex_search && regex_direction)
        mvwprintw(fd->win, height - 1, 0, "Search:%s" , regex_line);
    else if ( regex_search )
        mvwprintw(fd->win, height - 1, 0, "RSearch:%s" , regex_line);

    wattroff(fd->win, attr);
    
    wmove(fd->win, height - (file - fd->buf->sel_line) - 1, lwidth+2);
    wrefresh(fd->win);

    return 0;
}

void filedlg_display_message(struct filedlg *fd, char *message) {
    int height, width, i;
    int attr;

    if (hl_groups_get_attr (hl_groups_instance, HLG_STATUS_BAR, &attr) == -1)
      return;

    getmaxyx(fd->win, height, width);
    
    /* Print white background */
    wattron(fd->win, attr);

    for ( i = 0; i < width; i++)
       mvwprintw(fd->win, height - 1, i, " ");

    mvwprintw(fd->win, height - 1, 0, "%s" , message);
    wattroff(fd->win, attr);
    wrefresh(fd->win);
}

/* capture_regex: Captures a regular expression from the user.
 * ---------------
 *  Side Effect: 
 *
 *  regex_line: The regex the user has entered.
 *  regex_line_pos: The next available index into regex_line.
 *
 * Return Value: 0 if user gave a regex, otherwise 1.
 */
static int capture_regex(struct filedlg *fd) {
   int c;
   extern struct kui_manager *kui_ctx;

   /* Initialize the function for finding a regex and tell user */
   regex_search = 1;
   regex_line_pos = 0;
   regex_line[regex_line_pos] = '\0';
   filedlg_display(fd);

   do {
      c = kui_manager_getkey_blocking (kui_ctx);

      if ( regex_line_pos == (MAX_LINE - 1) && !(c == CGDB_KEY_ESC || c == 8 || c == 127 ))
          continue;

      /* Quit the search if the user hit escape */
      if ( c == CGDB_KEY_ESC ) {
         regex_line_pos = 0;
         regex_line[regex_line_pos] = '\0';
         regex_search = 0;
         filedlg_search_regex(fd, regex_line, 2, regex_direction, 1);
         filedlg_display(fd);
         return 1;
      }

      /* If the user hit enter, then a successful regex has been recieved */
      if ( c == '\r' || c == '\n' || c == CGDB_KEY_CTRL_M) {
         regex_line[regex_line_pos] = '\0';
         break;
      }

      /* If the user hit backspace or delete remove a char */
      if ( CGDB_BACKSPACE_KEY(c) ) {
         if (regex_line_pos > 0)
            --regex_line_pos;

         regex_line[regex_line_pos] = '\0';
         filedlg_search_regex(fd, regex_line, 1, regex_direction, 1);
         filedlg_display(fd);
         continue;
      }

      /* Add a char, search and draw */
      regex_line[regex_line_pos++] = c;
      regex_line[regex_line_pos] = '\0';
      filedlg_search_regex(fd, regex_line, 1, regex_direction, 1);
      filedlg_display(fd);
    } while (1);

   /* Finished */
   regex_search = 0;
   filedlg_search_regex(fd, regex_line, 2, regex_direction, 1);
   filedlg_display(fd);
   return 0;
}


int filedlg_recv_char(struct filedlg *fd, int key, char *file) {
    int height, width;

    /* Initialize size variables */
    getmaxyx(fd->win, height, width);

    filedlg_display(fd); 

    switch ( key ) {
        case 'q':
            return -1;
        /* Vertical scrolling */
        case CGDB_KEY_DOWN:
        case 'j':
            filedlg_vscroll(fd, 1);
            break;
        case CGDB_KEY_NPAGE:
	case CGDB_KEY_CTRL_F:	/* VI-style page down */
            filedlg_vscroll(fd, height - 1);
            break;
        case CGDB_KEY_UP:
        case 'k':
            filedlg_vscroll(fd, -1);
            break;
        case CGDB_KEY_PPAGE:
	case CGDB_KEY_CTRL_B:	/* VI-style page up */
            filedlg_vscroll(fd, -(height - 1));
            break;
        /* Horizontal scrolling */
        case CGDB_KEY_RIGHT:
        case 'l':
            filedlg_hscroll(fd, 1);
            break;
        case CGDB_KEY_LEFT:
        case 'h':
            filedlg_hscroll(fd, -1);
            break;
        case '/':
        case '?':
            regex_direction = ('/' == key);
             
            /* Capturing regular expressions */
            filedlg_search_regex_init(fd);
            capture_regex(fd);
            break;
        case 'n':
            filedlg_search_regex(fd, regex_line, 2, regex_direction, 1); 
            break;
        case 'N':
            filedlg_search_regex(fd, regex_line, 2, !regex_direction, 1);
            break;
        /* User selected a file */
        case '\n':
        case '\r':
		case CGDB_KEY_CTRL_M:
            strcpy(file, fd->buf->files[fd->buf->sel_line]);
            return 1; 
        default:
            break;
    }
  
    filedlg_display(fd); 

    return 0;
}
