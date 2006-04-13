/* interface.c:
* ------------
 * 
 * Provides the routines for displaying the interface, and interacting with
 * the user via keystrokes.
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

/* System Includes */
#if HAVE_CURSES_H
#include <curses.h>
#endif /* HAVE_CURSES_H */

#if HAVE_SIGNAL_H
#include <signal.h>
#endif

#if HAVE_STDARG_H
#include <stdarg.h>
#endif /* HAVE_STDARG_H */

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif /* HAVE_STDLIB_H */

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#if HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif /* HAVE_SYS_IOCTL_H */

#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */

#if HAVE_TERMIOS_H
#include <termios.h>
#endif /* HAVE_TERMIOS_H */

#if HAVE_CTYPE_H
#include <ctype.h>
#endif

/* Local Includes */
#include "cgdb.h"
#include "config.h"
#include "logger.h"
#include "interface.h"
#include "kui_term.h"
#include "scroller.h"
#include "sources.h"
#include "tgdb.h"
#include "filedlg.h"
#include "cgdbrc.h"
#include "highlight.h"
#include "highlight_groups.h"
#include "fs_util.h"

/* ----------- */
/* Prototypes  */
/* ----------- */

/* ------------ */
/* Declarations */
/* ------------ */

extern struct tgdb *tgdb;

/* ----------- */
/* Definitions */
/* ----------- */

/* This determines the minimum number of rows wants to close a window too 
 * A window should never become smaller than this size */
static int interface_winminheight = 0;

/* The offset that determines allows gdb/sources window to grow or shrink */
static int window_height_shift;

/* This is for the tty I/O window */
#define TTY_WIN_DEFAULT_HEIGHT ((interface_winminheight>4)?interface_winminheight:4)
static int tty_win_height_shift = 0;
#define TTY_WIN_OFFSET ( TTY_WIN_DEFAULT_HEIGHT + tty_win_height_shift )

/* Height and width of the terminal */
#define HEIGHT      (screen_size.ws_row)
#define WIDTH       (screen_size.ws_col)

/* Current window split state */
WIN_SPLIT_TYPE cur_win_split = WIN_SPLIT_EVEN;

/* --------------- */
/* Data Structures */
/* --------------- */

enum Command_Type
{
  CMD_LINE_NUMBER,		/* :123 line number */
  CMD_QUIT,			/* :q   command */
  CMD_QUIT_FORCE,		/* :q!  command */
  CMD_SET,			/* :set command */
  CMD_HELP,			/* :help command */
  CMD_UNKNOWN			/* Unknown command */
};

/* --------------- */
/* Local Variables */
/* --------------- */
static int curses_initialized = 0;	/* Flag: Curses has been initialized */
static int curses_colors = 0;	/* Flag: Terminal supports colors */
static struct scroller *gdb_win = NULL;	/* The GDB input/output window */
static struct scroller *tty_win = NULL;	/* The tty input/output window */
static int tty_win_on = 0;	/* Flag: tty window being shown */
static struct sviewer *src_win = NULL;	/* The source viewer window */
static WINDOW *status_win = NULL;	/* The status line */
static WINDOW *tty_status_win = NULL;	/* The tty status line */
static enum Focus focus = GDB;	/* Which pane is currently focused */
static struct winsize screen_size;	/* Screen size */

struct filedlg *fd;		/* The file dialog structure */

static char regex_line[MAX_LINE];	/* The regex the user enters */
static int regex_line_pos;	/* The index into the current regex */
static int regex_search;	/* Currently searching text ? */
static int regex_direction;	/* Direction to search */
/*static*/ int regex_icase = 0; /* Case insensitive (0), sensitive */
/*static*/ int shortcut_option = 0; /* Flag: Shortcut options enabled */
int config_wrapscan = 1;	/* Wrap around searching */

static char last_key_pressed = 0;	/* Last key user entered in cgdb mode */

static char cur_com_line[MAX_LINE];	/* The line number the user entered */
static int cur_com_pos;		/* The index into the current line */
static int display_command;	/* Currently going to a line number */

/* --------------- */
/* Local Functions */
/* --------------- */

/* init_curses: Initializes curses and sets up the terminal properly.
 * ------------
 *
 * Return Value: Zero on success, non-zero on failure.
 */
static int
init_curses ()
{
  if (putenv ("ESCDELAY=0") == -1)
    fprintf (stderr, "(%s:%d) putenv failed\r\n", __FILE__, __LINE__);

  initscr ();			/* Start curses mode */

  if ((curses_colors = has_colors ()))
    {
      start_color ();
#ifdef NCURSES_VERSION
      use_default_colors ();
#else
      bkgdset (0);
      bkgd (COLOR_WHITE);
#endif
    }

  refresh ();			/* Refresh the initial window once */
  curses_initialized = 1;

  return 0;
}

/* --------------------------------------
 * Theses get the position of each window
 * -------------------------------------- */

/* These are for the source window */
static int
get_src_row (void)
{
  return 0;
}

static int
get_src_col (void)
{
  return 0;
}

static int
get_src_height (void)
{
  return ((int) (((screen_size.ws_row + 0.5) / 2) + window_height_shift));
}

static int
get_src_width (void)
{
  return (screen_size.ws_col);
}

/* This is for the source window status bar */
static int
get_src_status_row (void)
{
  /* Usually would be 'get_src_row() + get_src_height()' but
   * the row is 0 */
  return get_src_height ();
}

static int
get_src_status_col (void)
{
  return 0;
}

static int
get_src_status_height (void)
{
  return 1;
}

static int
get_src_status_width (void)
{
  return (screen_size.ws_col);
}

/* This is for the tty I/O window */
static int
get_tty_row (void)
{
  return get_src_status_row () + get_src_status_height ();
}

static int
get_tty_col (void)
{
  return 0;
}

static int
get_tty_height (void)
{
  return TTY_WIN_OFFSET;
}

static int
get_tty_width (void)
{
  return (screen_size.ws_col);
}

/* This is for the tty I/O status bar line */
static int
get_tty_status_row (void)
{
  return get_tty_row () + get_tty_height ();
}

static int
get_tty_status_col (void)
{
  return 0;
}

static int
get_tty_status_height (void)
{
  return 1;
}

static int
get_tty_status_width (void)
{
  return (screen_size.ws_col);
}

/* This is for the debugger window */
static int
get_gdb_row (void)
{
  if (tty_win_on)
    return get_tty_status_row () + get_tty_status_height ();

  return get_src_status_row () + get_src_status_height ();
}

static int
get_gdb_col (void)
{
  return 0;
}

int
get_gdb_height (void)
{
  int window_size = ((screen_size.ws_row / 2) - window_height_shift - 1);
  int odd_screen_size = (screen_size.ws_row % 2);

  if (tty_win_on)
    return window_size - TTY_WIN_OFFSET + odd_screen_size - 1;

  return window_size + odd_screen_size;
}

static int
get_gdb_width (void)
{
  return (screen_size.ws_col);
}

/* ---------------------------------------
 * Below is the core body of the interface
 * --------------------------------------- */

/* Updates the status bar */
static void
update_status_win (void)
{
  int pos;
  char filename[FSUTIL_PATH_MAX];
  int attr;

  if (hl_groups_get_attr (hl_groups_instance, HLG_STATUS_BAR, &attr) == -1)
    return;

  /* Update the tty status bar */
  if (tty_win_on)
    {
      wattron (tty_status_win, attr);
      for (pos = 0; pos < WIDTH; pos++)
	mvwprintw (tty_status_win, 0, pos, " ");

      mvwprintw (tty_status_win, 0, 0, (char *) tgdb_tty_name (tgdb));
      wattroff (tty_status_win, attr);
    }

  /* Print white background */
  wattron (status_win, attr);
  for (pos = 0; pos < WIDTH; pos++)
    mvwprintw (status_win, 0, pos, " ");
  if (tty_win_on)
    wattron (tty_status_win, attr);
  /* Show the user which window is focused */
  if (focus == GDB)
    mvwprintw (status_win, 0, WIDTH - 1, "*");
  else if (focus == TTY && tty_win_on)
    mvwprintw (tty_status_win, 0, WIDTH - 1, "*");
  else if (focus == CGDB)
    mvwprintw (status_win, 0, WIDTH - 1, " ");
  wattroff (status_win, attr);
  if (tty_win_on)
    wattroff (tty_status_win, attr);

  /* Print the regex that the user is looking for Forward */
  if (regex_search && regex_direction)
    {
      if_display_message ("/", WIDTH - 1, "%s", regex_line);
      curs_set (1);
    }
  /* Regex backwards */
  else if (regex_search)
    {
      if_display_message ("?", WIDTH - 1, "%s", regex_line);
      curs_set (1);
    }
  /* Line number search */
  else if (display_command)
    {
      if_display_message (":", WIDTH - 1, "%s", cur_com_line);
      curs_set (1);
    }
  /* Default: Current Filename */
  else
    {
      /* Print filename */
      if (src_win != NULL && source_current_file (src_win, filename) != NULL)
	if_display_message ("", WIDTH - 1, "%s", filename);
    }

  wrefresh (status_win);
}

void
if_display_message (const char *msg, int width, const char *fmt, ...)
{
  va_list ap;
  char va_buf[MAXLINE];
  char buf_display[MAXLINE];
  int pos, error_length, length;
  int attr;

  if (hl_groups_get_attr (hl_groups_instance, HLG_STATUS_BAR, &attr) == -1)
    return;

  curs_set (0);

  if (!width)
    width = WIDTH;

  /* Get the buffer with format */
  va_start (ap, fmt);
#ifdef   HAVE_VSNPRINTF
  vsnprintf (va_buf, sizeof (va_buf), fmt, ap);	/* this is safe */
#else
  vsprintf (va_buf, fmt, ap);	/* this is not safe */
#endif
  va_end (ap);

  error_length = strlen (msg);
  length = strlen (va_buf);

  if (error_length > width)
    strcat (strncpy (buf_display, msg, width - 1), ">");
  else if (error_length + length > width)
    sprintf (buf_display, "%s>%s", msg,
	     va_buf + (length - (width - error_length) + 1));
  else
    sprintf (buf_display, "%s%s", msg, va_buf);

  /* Print white background */
  wattron (status_win, attr);
  for (pos = 0; pos < WIDTH; pos++)
    mvwprintw (status_win, 0, pos, " ");

  mvwprintw (status_win, 0, 0, "%s", buf_display);
  wattroff (status_win, attr);
  wrefresh (status_win);
}

/* if_draw: Draws the interface on the screen.
 * --------
 */
void
if_draw (void)
{
  /* Only redisplay the filedlg if it is up */
  if (focus == FILE_DLG)
    {
      filedlg_display (fd);
      return;
    }

  update_status_win ();

  if (get_src_height () != 0 && get_gdb_height () != 0)
    wrefresh (status_win);

  if (tty_win_on)
    wrefresh (tty_status_win);

  if (get_src_height () > 0)
    source_display (src_win, focus == CGDB);

  if (tty_win_on && get_tty_height () > 0)
    scr_refresh (tty_win, focus == TTY);

  if (get_gdb_height () > 0)
    scr_refresh (gdb_win, focus == GDB);

  /* This check is here so that the cursor goes to the 
   * cgdb window. The cursor would stay in the gdb window 
   * on cygwin */
  if (get_src_height () > 0 && focus == CGDB)
    wrefresh (src_win->win);
}

/* validate_window_sizes:
 * ----------------------
 *
 * This will make sure that the gdb_window, status_bar and source window
 * have appropriate sizes. Each of the windows will not be able to grow
 * smaller than INTERFACE_WINMINHEIGHT in size. It will also restrict the
 * size of windows to being within the size of the terminal.
 */
static void
validate_window_sizes (void)
{
  int tty_window_offset = (tty_win_on) ? TTY_WIN_OFFSET + 1 : 0;
  int odd_height = (HEIGHT + 1) % 2;
  int max_window_height_shift = (HEIGHT / 2) - tty_window_offset - odd_height;
  int min_window_height_shift = -(HEIGHT / 2);

  /* update max and min based off of users winminheight request */
  min_window_height_shift += interface_winminheight;
  max_window_height_shift -= interface_winminheight;

  /* Make sure that the windows offset is within its bounds: 
   * This checks the window offset.
   * */
  if (window_height_shift > max_window_height_shift)
    window_height_shift = max_window_height_shift;
  else if (window_height_shift < min_window_height_shift)
    window_height_shift = min_window_height_shift;
}

/* if_layout: Update the layout of the screen based on current terminal size.
 * ----------
 *
 * Return Value: Zero on success, non-zero on failure.
 */
static int
if_layout ()
{
  /* Verify the window size is reasonable */
  validate_window_sizes ();

  /* Initialize the GDB I/O window */
  if (gdb_win == NULL)
    {
      gdb_win =
	scr_new (get_gdb_row (), get_gdb_col (), get_gdb_height (),
		 get_gdb_width ());
      if (gdb_win == NULL)
	return 2;
    }
  else
    {				/* Resize the GDB I/O window */
      if (get_gdb_height () > 0)
	scr_move (gdb_win, get_gdb_row (), get_gdb_col (), get_gdb_height (),
		  get_gdb_width ());
    }

  /* Initialize TTY I/O window */
  if (tty_win == NULL)
    {
      tty_win =
	scr_new (get_tty_row (), get_tty_col (), get_tty_height (),
		 get_tty_width ());
      if (tty_win == NULL)
	return 2;
    }
  else
    {				/* Resize the GDB I/O window */
      if (get_tty_height () > 0)
	scr_move (tty_win, get_tty_row (), get_tty_col (), get_tty_height (),
		  get_tty_width ());
    }

  /* Initialize the source viewer window */
  if (src_win == NULL)
    {
      src_win =
	source_new (get_src_row (), get_src_col (), get_src_height (),
		    get_src_width ());
      if (src_win == NULL)
	return 3;
    }
  else
    {				/* Resize the source viewer window */
      if (get_src_height () > 0)
	source_move (src_win, get_src_row (), get_src_col (),
		     get_src_height (), get_src_width ());
    }

  /* Initialize the status bar window */
  status_win = newwin (get_src_status_height (), get_src_status_width (),
		       get_src_status_row (), get_src_status_col ());

  /* Initialize the tty status bar window */
  if (tty_win_on)
    tty_status_win =
      newwin (get_tty_status_height (), get_tty_status_width (),
	      get_tty_status_row (), get_tty_status_col ());

  if_draw ();

  return 0;
}

/* if_resize: Checks if a resize event occurred, and updates display if so.
 * ----------
 *
 * Return Value:  Zero on success, non-zero on failure.
 */
void rl_resize (int rows, int cols);
static int
if_resize ()
{
  if (ioctl (fileno (stdout), TIOCGWINSZ, &screen_size) != -1)
    {
#ifdef NCURSES_VERSION
      if (screen_size.ws_row != LINES || screen_size.ws_col != COLS)
	{
	  resizeterm (screen_size.ws_row, screen_size.ws_col);
	  refresh ();
	  rl_resize (screen_size.ws_row, screen_size.ws_col);
	  return if_layout ();
	}
#else
      /* Stupid way to resize - should work on most systems */
      endwin ();
      LINES = screen_size.ws_row;
      COLS = screen_size.ws_col;
      refresh ();
      source_hscroll (src_win, 0);
#endif
      rl_resize (screen_size.ws_row, screen_size.ws_col);
      return if_layout ();

    }

  return 0;
}

int
if_resize_term (void)
{
  if (if_resize ())
    return -1;

  return 0;
}


/*
 * increase_win_height: Increase size of source or tty window
 * ____________________
 *
 * Param jump_or_tty - if 0, increase source window by 1
 *                     if 1, if tty window is visible, increase it by 1
 *                           else jump source window to next biggest quarter 
 *
 */
static void
increase_win_height (int jump_or_tty)
{
  int height = (HEIGHT / 2) - ((tty_win_on) ? TTY_WIN_OFFSET + 1 : 0);
  int old_window_height_shift = window_height_shift;
  int old_tty_win_height_shift = tty_win_height_shift;

  if (jump_or_tty)
    {
      /* user input: '+' */
      if (tty_win_on)
	{
	  /* tty window is visible*/
	  height = get_gdb_height () + get_tty_height ();

	  if (tty_win_height_shift + TTY_WIN_DEFAULT_HEIGHT <
	      height - interface_winminheight)
	    {
	      /* increase tty window size by 1*/
	      tty_win_height_shift++;
	    }
	}
      else
	{
	  /* no tty window*/
	  if (cur_win_split == WIN_SPLIT_FREE)
	    {
	      /* cur position is not on mark, find nearest mark*/
	      cur_win_split = (int) (2 * window_height_shift) / height;

	      /* handle rounding on either side of mid-way mark*/
	      if (window_height_shift > 0)
		{
		  cur_win_split++;
		}
	    }
	  else
	    {
	      /* increase to next mark*/
	      cur_win_split++;
	    }

	  /* check split bounds*/
	  if (cur_win_split > WIN_SPLIT_TOP_FULL)
	    {
	      cur_win_split = WIN_SPLIT_TOP_FULL;
	    }

	  /* set window height to specified quarter mark*/
	  window_height_shift = (int) (height * (cur_win_split / 2.0));
	}
    }
  else
    {
      /* user input: '='*/
      cur_win_split = WIN_SPLIT_FREE;	/* cur split is not on a mark*/
      window_height_shift++;	/* increase src window size by 1*/

    }

  /* reduce flicker by avoiding unnecessary redraws*/
  if (window_height_shift != old_window_height_shift ||
      tty_win_height_shift != old_tty_win_height_shift)
    {
      if_layout ();
    }
}

/*
 * decrease_win_height: Decrease size of source or tty window
 * ____________________
 *
 * Param jump_or_tty - if 0, decrease source window by 1
 *                     if 1, if tty window is visible, decrease it by 1
 *                           else jump source window to next smallest quarter 
 *
 */
static void
decrease_win_height (int jump_or_tty)
{
  int height = HEIGHT / 2;
  int old_window_height_shift = window_height_shift;
  int old_tty_win_height_shift = tty_win_height_shift;

  if (jump_or_tty)
    {
      /* user input: '_'*/
      if (tty_win_on)
	{
	  /* tty window is visible*/
	  if (tty_win_height_shift >
	      -(TTY_WIN_DEFAULT_HEIGHT - interface_winminheight))
	    {
	      /* decrease tty window size by 1*/
	      tty_win_height_shift--;
	    }
	}
      else
	{
	  /* no tty window*/
	  if (cur_win_split == WIN_SPLIT_FREE)
	    {
	      /* cur position is not on mark, find nearest mark*/
	      cur_win_split = (int) (2 * window_height_shift) / height;

	      /* handle rounding on either side of mid-way mark*/
	      if (window_height_shift < 0)
		{
		  cur_win_split--;
		}
	    }
	  else
	    {
	      /* decrease to next mark*/
	      cur_win_split--;
	    }

	  /* check split bounds*/
	  if (cur_win_split < WIN_SPLIT_BOTTOM_FULL)
	    {
	      cur_win_split = WIN_SPLIT_BOTTOM_FULL;
	    }

	  /* set window height to specified quarter mark*/
	  window_height_shift = (int) (height * (cur_win_split / 2.0));
	}
    }
  else
    {
      /* user input: '-'*/
      cur_win_split = WIN_SPLIT_FREE;	/* cur split is not on a mark*/
      window_height_shift--;	/* decrease src window size by 1*/

    }

  /* reduce flicker by avoiding unnecessary redraws*/
  if (window_height_shift != old_window_height_shift ||
      tty_win_height_shift != old_tty_win_height_shift)
    {
      if_layout ();
    }
}

/* signal_handler: Handles the WINCH signal (xterm resize).
 * ---------------
 */

void rl_sigint_recved (void);
static void
signal_handler (int signo)
{
  extern int resize_pipe[2];
  if (signo == SIGWINCH)
    {
      int c = CGDB_KEY_RESIZE;
      write (resize_pipe[1], &c, sizeof (int));
    }
  else if (signo == SIGINT || signo == SIGTERM ||
	   signo == SIGQUIT || signo == SIGCHLD)
    {
      rl_sigint_recved ();
      tgdb_signal_notification (tgdb, signo);
    }
}

/* if_get_command: Gets a command from the user
 * -----------------
 */
static void
if_get_command (struct sviewer *sview)
{
  int c;
  cur_com_pos = 0;
  cur_com_line[cur_com_pos] = '\0';
  display_command = 1;
  if_draw ();

  while ((c = wgetch (sview->win)) != ERR)
    {
      if (cur_com_pos == (MAX_LINE - 1)
	  && !(c == CGDB_KEY_ESC || c == 8 || c == 127))
	continue;

      /* Quit the search if the user hit escape */
      if (c == 27)
	{
	  cur_com_pos = 0;
	  cur_com_line[cur_com_pos] = '\0';
	  display_command = 0;
	  if_draw ();
	  return;
	}

      /* If the user hit enter, then a successful regex has been recieved */
      if (c == '\r' || c == '\n')
	{
	  cur_com_line[cur_com_pos] = '\0';
	  display_command = 0;
	  return;
	}

      /* If the user hit backspace or delete remove a char */
      if (CGDB_BACKSPACE_KEY (c) || c == 8 || c == 127)
	{
	  if (cur_com_pos > 0)
	    --cur_com_pos;

	  cur_com_line[cur_com_pos] = '\0';
	  if_draw ();

	  update_status_win ();
	  wrefresh (status_win);
	  continue;
	}

      /* Add a char, search and draw */
      cur_com_line[cur_com_pos++] = c;
      cur_com_line[cur_com_pos] = '\0';
      if_draw ();
      update_status_win ();
      wrefresh (status_win);
    }

  /* Finished */
  display_command = 0;
  if_draw ();
}

static void
if_run_command (struct sviewer *sview)
{
  /* Get a command and then try to process it */
  if_get_command (sview);

  /* refresh and return if the user entered no data */
  if (cur_com_pos == 0)
    {
      if_draw ();
      return;
    }

  if (command_parse_string (cur_com_line))
    {
      if_display_message ("Unknown command: ", 0, "%s", cur_com_line);
    }
  else
    {
      update_status_win ();
    }

  if_draw ();
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
static int
capture_regex (struct sviewer *sview)
{
  int c;

  int orig_line = sview->cur->sel_line;
  /* Initialize the function for finding a regex and tell user */
  regex_search = 1;
  regex_line_pos = 0;
  regex_line[regex_line_pos] = '\0';
  if_draw ();

  while ((c = wgetch (sview->win)) != ERR)
    {
      if (regex_line_pos == (MAX_LINE - 1)
	  && !(c == CGDB_KEY_ESC || c == 8 || c == 127))
	continue;

      /* Quit the search if the user hit escape */
      if (c == 27)
	{
	  regex_line_pos = 0;
	  *regex_line = '\0';
	  regex_search = 0;
	  source_search_regex (sview, regex_line, 2, regex_direction,
			       regex_icase);
	  sview->cur->sel_line = orig_line;
	  if_draw ();
	  return 1;
	}

      /* If the user hit enter, then a successful regex has been recieved */
      if (c == '\r' || c == '\n')
	{
	  regex_line[regex_line_pos] = '\0';
	  regex_search = 0;
	  source_search_regex (sview, regex_line, 2, regex_direction,
			       regex_icase);
	  if_draw ();
	  return 0;
	}

      /* If the user hit backspace or delete remove a char */
      if (CGDB_BACKSPACE_KEY (c) || c == 8 || c == 127)
	{
	  if (regex_line_pos > 0)
	    --regex_line_pos;

	  regex_line[regex_line_pos] = '\0';
	  source_search_regex (sview, regex_line, 1, regex_direction,
			       regex_icase);
	  if_draw ();

	  update_status_win ();
	  continue;
	}

      /* Add a char, search and draw */
      regex_line[regex_line_pos++] = c;
      regex_line[regex_line_pos] = '\0';
      source_search_regex (sview, regex_line, 1, regex_direction,
			   regex_icase);
      if_draw ();
      update_status_win ();
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
 */
static int
tty_input (int key)
{
  /* Handle special keys */
  switch (key)
    {
    case CGDB_KEY_PPAGE:
      scr_up (tty_win, get_tty_height () - 1);
      break;
    case CGDB_KEY_NPAGE:
      scr_down (tty_win, get_tty_height () - 1);
      break;
    case CGDB_KEY_F11:
      scr_home (tty_win);
      break;
    case CGDB_KEY_F12:
      scr_end (tty_win);
      break;
    default:
      return 2;
    }

  if_draw ();

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
 */
static int
gdb_input (int key)
{
  /* Handle special keys */
  switch (key)
    {
    case CGDB_KEY_PPAGE:
      scr_up (gdb_win, get_gdb_height () - 1);
      break;
    case CGDB_KEY_NPAGE:
      scr_down (gdb_win, get_gdb_height () - 1);
      break;
    case CGDB_KEY_F11:
      scr_home (gdb_win);
      break;
    case CGDB_KEY_F12:
      scr_end (gdb_win);
      break;
    default:
      return 1;
    }

  if_draw ();

  return 0;
}

/**
 * toggle a breakpoint
 * 
 * \param sview
 * The source view
 *
 * \param t
 * The action to take
 *
 * \return
 * 0 on success, -1 on error.
 */
static int
toggle_breakpoint (struct sviewer *sview, enum tgdb_breakpoint_action t)
{
  char *path;
  int line;
  tgdb_request_ptr request_ptr;

  if (!sview || !sview->cur || !sview->cur->path)
    return 0;

  line = sview->cur->sel_line;

  /* Get filename (strip path off -- GDB is dumb) */
  path = strrchr (sview->cur->path, '/') + 1;
  if ((int) path == 1)
    path = sview->cur->path;

  /* delete an existing breakpoint */
  if (sview->cur->buf.breakpts[line])
    t = TGDB_BREAKPOINT_DELETE;

  request_ptr = tgdb_request_modify_breakpoint (tgdb, path, line + 1, t);
  if (!request_ptr)
    return -1;

  handle_request (tgdb, request_ptr);

  return 0;
}

/* source_input: Handles user input to the source window.
 * -------------
 *
 *   sview:     Source viewer object
 *   key:       Keystroke received.
 */
static void
source_input (struct sviewer *sview, int key)
{
  switch (key)
    {
    case CGDB_KEY_UP:
    case 'k':			/* VI-style up-arrow */
      source_vscroll (sview, -1);
      break;
    case CGDB_KEY_DOWN:
    case 'j':			/* VI-style down-arrow */
      source_vscroll (sview, 1);
      break;
    case CGDB_KEY_LEFT:
    case 'h':
      source_hscroll (sview, -1);
      break;
    case CGDB_KEY_RIGHT:
    case 'l':
      source_hscroll (sview, 1);
      break;
    case CGDB_KEY_CTRL_U:	/* VI-style 1/2 page up */
      source_vscroll (sview, -(get_src_height () / 2));
      break;
    case CGDB_KEY_PPAGE:
    case CGDB_KEY_CTRL_B:	/* VI-style page up */
      source_vscroll (sview, -(get_src_height () - 1));
      break;
    case CGDB_KEY_CTRL_D:	/* VI-style 1/2 page down */
      source_vscroll (sview, (get_src_height () / 2));
      break;
    case CGDB_KEY_NPAGE:
    case CGDB_KEY_CTRL_F:	/* VI-style page down */
      source_vscroll (sview, get_src_height () - 1);
      break;
    case 'g':			/* beggining of file */
      if (last_key_pressed == 'g')
	source_set_sel_line (sview, 1);
      break;
    case 'G':			/* end of file */
      source_set_sel_line (sview, 10000000);
      break;
    case '=':
      /* inc window by 1*/
      increase_win_height (0);
      break;
    case '-':
      /* dec window by 1*/
      decrease_win_height (0);
      break;
    case '+':
      /* inc to jump or inc tty*/
      increase_win_height (1);
      break;
    case '_':
      /* dec to jump or dec tty*/
      decrease_win_height (1);
      break;
    case 'o':
      /* Causes file dialog to be opened */
      {
	tgdb_request_ptr request_ptr;
	request_ptr = tgdb_request_inferiors_source_files (tgdb);
	handle_request (tgdb, request_ptr);
      }
      break;
    case ':':
      /* Allows user to go to a line number */
      if_run_command (sview);
      return;
    case ' ':
      {
	enum tgdb_breakpoint_action t = TGDB_BREAKPOINT_ADD;
	toggle_breakpoint (sview, t);
      }
      break;
    case 't':
      {
	enum tgdb_breakpoint_action t = TGDB_TBREAKPOINT_ADD;
	toggle_breakpoint (sview, t);
      }
      break;
    default:
      break;
    }

  /* Some extended features that are set by :set sc */
  if (shortcut_option)
    {
      tgdb_request_ptr request_ptr = NULL;
      switch (key)
	{
	case 'r':
	  request_ptr = tgdb_request_run_debugger_command (tgdb, TGDB_RUN);
	  break;
	case 'n':
	  request_ptr = tgdb_request_run_debugger_command (tgdb, TGDB_NEXT);
	  break;
	case 's':
	  request_ptr = tgdb_request_run_debugger_command (tgdb, TGDB_STEP);
	  break;
	case 'c':
	  request_ptr =
	    tgdb_request_run_debugger_command (tgdb, TGDB_CONTINUE);
	  break;
	case 'f':
	  request_ptr = tgdb_request_run_debugger_command (tgdb, TGDB_FINISH);
	  break;
	case 'u':
	  request_ptr = tgdb_request_run_debugger_command (tgdb, TGDB_UP);
	  break;
	case 'd':
	  request_ptr = tgdb_request_run_debugger_command (tgdb, TGDB_DOWN);
	  break;
	default:
	  break;
	}
      if (request_ptr)
	handle_request (tgdb, request_ptr);
    }

  if_draw ();
}

/* Sets up the signal handler for SIGWINCH
 * Returns -1 on error. Or 0 on success */
static int
set_up_signal (void)
{
  struct sigaction action;

  action.sa_handler = signal_handler;
  sigemptyset (&action.sa_mask);
  action.sa_flags = 0;

  if (sigaction (SIGWINCH, &action, NULL) < 0)
    {
      logger_write_pos (logger, __FILE__, __LINE__, "sigaction failed ");
      return -1;
    }

  if (sigaction (SIGINT, &action, NULL) < 0)
    {
      logger_write_pos (logger, __FILE__, __LINE__, "sigaction failed ");
      return -1;
    }

  if (sigaction (SIGTERM, &action, NULL) < 0)
    {
      logger_write_pos (logger, __FILE__, __LINE__, "sigaction failed ");
      return -1;
    }

  if (sigaction (SIGQUIT, &action, NULL) < 0)
    {
      logger_write_pos (logger, __FILE__, __LINE__, "sigaction failed ");
      return -1;
    }

  if (sigaction (SIGCHLD, &action, NULL) < 0)
    {
      logger_write_pos (logger, __FILE__, __LINE__, "sigaction failed ");
      return -1;
    }

  return 0;
}

/* ----------------- */
/* Exposed Functions */
/* ----------------- */

/* See interface.h for function descriptions. */
int
if_init (void)
{
  if (init_curses ())
    return 1;

  hl_groups_instance = hl_groups_initialize ();
  if (!hl_groups_instance)
    return 3;

  if (hl_groups_setup (hl_groups_instance) == -1)
    return 3;

  /* Set up the signal handler to catch SIGWINCH */
  if (set_up_signal () == -1)
    return 2;

  if (ioctl (fileno (stdout), TIOCGWINSZ, &screen_size) == -1)
    {
      screen_size.ws_row = LINES;
      screen_size.ws_col = COLS;
    }

  /* Create the file dialog object */
  if ((fd = filedlg_new (0, 0, HEIGHT, WIDTH)) == NULL)
    return 5;

  /* Set up window layout */
  window_height_shift = (int) ((HEIGHT / 2) * (cur_win_split / 2.0));
  switch (if_layout ())
    {
    case 2:
      return 4;
    }

  return 0;
}

int
internal_if_input (int key)
{
  /* The ESC Key, puts the debugger into command mode */
  if (focus != CGDB && key == CGDB_KEY_ESC)
    {
      focus = CGDB;
      if_draw ();
      return 0;
    }
  else if (key == CGDB_KEY_ESC)
    return 0;

  /* Check for global keystrokes */
  switch (focus)
    {
    case CGDB:
      switch (key)
	{
	case 'i':
	  focus = GDB;
	  if_draw ();
	  return 0;
	case 'I':
	  if (tty_win_on)
	    {
	      focus = TTY;
	      if_draw ();
	    }
	  return 0;
	case '/':
	case '?':
	  regex_direction = ('/' == key);

	  /* Capturing regular expressions */
	  source_search_regex_init (src_win);
	  capture_regex (src_win);
	  return 0;
	case 'n':
	  if (!shortcut_option)
	    {
	      source_search_regex (src_win, regex_line, 2, regex_direction,
				   regex_icase);
	      if_draw ();
	    }
	  break;
	case 'N':
	  source_search_regex (src_win, regex_line, 2, !regex_direction,
			       regex_icase);
	  if_draw ();
	  break;
	case 'T':
	  if (tty_win_on)
	    {
	      tty_win_on = 0;
	      focus = CGDB;
	    }
	  else
	    {
	      tty_win_on = 1;
	      focus = TTY;
	    }

	  if_layout ();

	  break;
	case CGDB_KEY_CTRL_T:
	  if (tgdb_tty_new (tgdb) == -1)
	    {
	      /* Error */
	    }
	  else
	    {
	      scr_free (tty_win);
	      tty_win = NULL;
	      if_layout ();
	    }

	  break;
	case CGDB_KEY_F1:
	  if_display_help ();
	  return 0;
	case CGDB_KEY_F5:
	  /* Issue GDB run command */
	  {
	    tgdb_request_ptr request_ptr;
	    request_ptr = tgdb_request_run_debugger_command (tgdb, TGDB_RUN);
	    handle_request (tgdb, request_ptr);
	  }
	  return 0;
	case CGDB_KEY_F6:
	  /* Issue GDB continue command */
	  {
	    tgdb_request_ptr request_ptr;
	    request_ptr =
	      tgdb_request_run_debugger_command (tgdb, TGDB_CONTINUE);
	    handle_request (tgdb, request_ptr);
	  }
	  return 0;
	case CGDB_KEY_F7:
	  /* Issue GDB finish command */
	  {
	    tgdb_request_ptr request_ptr;
	    request_ptr =
	      tgdb_request_run_debugger_command (tgdb, TGDB_FINISH);
	    handle_request (tgdb, request_ptr);
	  }
	  return 0;
	case CGDB_KEY_F8:
	  /* Issue GDB next command */
	  {
	    tgdb_request_ptr request_ptr;
	    request_ptr = tgdb_request_run_debugger_command (tgdb, TGDB_NEXT);
	    handle_request (tgdb, request_ptr);
	  }
	  return 0;
	case CGDB_KEY_F10:
	  /* Issue GDB step command */
	  {
	    tgdb_request_ptr request_ptr;
	    request_ptr = tgdb_request_run_debugger_command (tgdb, TGDB_STEP);
	    handle_request (tgdb, request_ptr);
	  }
	  return 0;
	case CGDB_KEY_CTRL_L:
	  if_layout ();
	  return 0;
	}
      source_input (src_win, key);
      return 0;
      break;
    case TTY:
      return tty_input (key);
    case GDB:
      return gdb_input (key);
    case FILE_DLG:
      {
	static char filedlg_file[MAX_LINE];
	int ret = filedlg_recv_char (fd, key, filedlg_file);
	/* The user cancelled */
	if (ret == -1)
	  {
	    if_set_focus (CGDB);
	    return 0;
	    /* Needs more data */
	  }
	else if (ret == 0)
	  {
	    return 0;
	    /* The user picked a file */
	  }
	else if (ret == 1)
	  {
	    tgdb_request_ptr request_ptr;
	    request_ptr = tgdb_request_filename_pair (tgdb, filedlg_file);
	    handle_request (tgdb, request_ptr);
	    if_set_focus (CGDB);
	    return 0;
	  }
      }
    default:
      /* Focus is screwed up, fix it */
      fprintf (stderr, "KEYBOARD ERROR\r\n");
      return 0;
    }

  /* Never gets here */
  return 0;
}

int
if_input (int key)
{
  int result = internal_if_input (key);
  last_key_pressed = key;
  return result;
}

void
if_tty_print (const char *buf)
{
  /* If the tty I/O window is not open send output to gdb window */
  if (!tty_win_on)
    if_print (buf);

  /* Print it to the scroller */
  scr_add (tty_win, buf);

  /* Only need to redraw if tty_win is being displayed */
  if (tty_win_on && get_gdb_height () > 0)
    {
      scr_refresh (tty_win, focus == TTY);

      /* Make sure cursor reappears in source window if focus is there */
      if (focus == CGDB)
	wrefresh (src_win->win);
    }
}

void
if_print (const char *buf)
{
  /* Print it to the scroller */
  scr_add (gdb_win, buf);

  if (get_gdb_height () > 0)
    {
      scr_refresh (gdb_win, focus == GDB);

      /* Make sure cursor reappears in source window if focus is there */
      if (focus == CGDB)
	wrefresh (src_win->win);
    }
}

void
if_print_message (const char *fmt, ...)
{
  va_list ap;
  char va_buf[MAXLINE];

  /* Get the buffer with format */
  va_start (ap, fmt);
#ifdef   HAVE_VSNPRINTF
  vsnprintf (va_buf, sizeof (va_buf), fmt, ap);	/* this is safe */
#else
  vsprintf (va_buf, fmt, ap);	/* this is not safe */
#endif
  va_end (ap);

  if_print (va_buf);
}

void
if_show_file (char *path, int line)
{
  if (source_set_exec_line (src_win, path, line) == 0)
    if_draw ();
}

void
if_display_help (void)
{
  char cgdb_help_file[MAXLINE];
  int ret_val = 0;

  fs_util_get_path (PKGDATADIR, "cgdb.txt", cgdb_help_file);
  ret_val = source_set_exec_line (src_win, cgdb_help_file, 1);
  if (ret_val == 0)
    if_draw ();
  else if (ret_val == 5)	/* File does not exist */
    if_display_message ("No such file: %s", 0, cgdb_help_file);
}

struct sviewer *
if_get_sview ()
{
  return src_win;
}

void
if_clear_filedlg (void)
{
  filedlg_clear (fd);
}

void
if_add_filedlg_choice (const char *filename)
{
  filedlg_add_file_choice (fd, filename);
}

void
if_filedlg_display_message (char *message)
{
  filedlg_display_message (fd, message);
}

void
if_shutdown (void)
{
  /* Shut down curses cleanly */
  if (curses_initialized)
    endwin ();

  if (status_win != NULL)
    delwin (status_win);

  if (tty_status_win != NULL)
    delwin (tty_status_win);

  if (gdb_win != NULL)
    scr_free (gdb_win);

  if (tty_win != NULL)
    scr_free (tty_win);

  if (src_win != NULL)
    source_free (src_win);
}

void
if_set_focus (Focus f)
{
  switch (f)
    {
    case GDB:
      focus = f;
      if_draw ();
      break;
    case TTY:
      if (tty_win_on)
	{
	  focus = f;
	  if_draw ();
	}
      break;
    case CGDB:
      focus = f;
      if_draw ();
      break;
    case FILE_DLG:
      focus = f;
      if_draw ();
      break;
    default:
      return;
    }
}

Focus
if_get_focus (void)
{
  return focus;
}

void
if_set_winsplit (WIN_SPLIT_TYPE new_split)
{
  cur_win_split = new_split;
  window_height_shift = (int) ((HEIGHT / 2) * (cur_win_split / 2.0));
  if_layout ();
}

void
if_highlight_sviewer (enum tokenizer_language_support l)
{
  /* src_win->cur is NULL when reading cgdbrc */
  if (src_win->cur)
    {
      src_win->cur->language = l;
      highlight (src_win->cur);
      if_draw ();
    }
}

int
if_change_winminheight (int value)
{
  if (value < 0)
    return -1;
  else if (tty_win_on && value > HEIGHT / 3)
    return -1;
  else if (value > HEIGHT / 2)
    return -1;

  interface_winminheight = value;
  if_layout ();

  return 0;
}

int
if_clear_line ()
{
  int width = get_gdb_width ();
  int i;
  char line[width+3];
  line[0] = '\r';
  
  for (i = 1; i <= width; ++i)
    line[i] = ' ';

  line[i] = '\r';
  line[i+1] = '\0';

  if_print (line);

  return 0;
}
