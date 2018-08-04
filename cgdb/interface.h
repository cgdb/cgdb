/* interface.h:
 * ------------
 * 
 * Provides the routines for displaying the interface, and interacting with
 * the user via keystrokes.
 */

#ifndef _INTERFACE_H_
#define _INTERFACE_H_

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif /* HAVE_SYS_IOCTL_H */

/* Local Includes */
#include "cgdb.h"
#include "cgdbrc.h"
#include "sources.h"

/* --------- */
/* Functions */
/* --------- */

/* if_init: Initializes the interface.
 * --------
 *
 * Return Value: Zero on success, or...
 *               1: Initializing curses failed
 *               2: Signal handler install failed
 *               3: hl groups couldn't be setup
 *               4: Can't create new GDB scroller
 *               5: Can not init the file dialog
 */
int if_init(void);

/* if_input: Handles special input keys from user.
 * ---------
 *
 *   key:  Key code that was received.
 *
 * Return Value:  0 if internal key was used, 
 *                1 if input to gdb,
 *                2 if input to tty or ...
 *                -1        : Quit cgdb
 */

/* if_resize_term: Resizes the application to the current term size.
 * --------------
 *
 * Return Value: -1 on error, 0 on success
 */
int if_resize_term(void);

int if_input(int key);

/* if_print: Prints data to the GDB input/output window.
 * ---------
 *
 *   buf:  NULL-terminated buffer to display.
 */
void if_print(const char *buf);

/* if_print_message: Prints data to the GDB input/output window.
 * -----------------
 *
 * fmt:     The message to display
 */
void if_print_message(const char *fmt, ...) ATTRIBUTE_PRINTF( 1, 2 );

/* if_tty_print: Prints data to the tty input/output window.
 * -------------
 *
 *   buf:  NULL-terminated buffer to display.
 */
void if_tty_print(const char *buf);

/**
 * Print a readline prompt change.
 *
 * @param buf
 * The buffer that readline has put out to edit the prompt.
 */
void if_rl_print(const char *buf);

/**
 * Print a debugger command run by CGDB when showdebugcommands is enabled.
 *
 * @param command
 * The command that CGDB is about to issue to GDB.
 */
void if_sdc_print(const char *command);

/* if_show_file: Displays the requested file in the source display window.
 * -------------
 *
 *   path:  Full path to the file to display
 *   sel_line:  Current line of the file being selected (0 to leave unchanged)
 *   exe_line:  Current line of the file being executed (0 to leave unchanged)
 */
void if_show_file(char *path, int sel_line, int exe_line);

/* if_get_sview: Return a pointer to the source viewer object.
 * -------------
 */
struct sviewer *if_get_sview();

/* if_display_message: Displays a message on the source window status bar.
 * -------------------
 *
 * msg:     A message to the user. This will not be truncated unless the width
 *          of the terminal is smaller than this message. 
 *          msg should never be NULL. use "" instead.
 *          
 * width:   The width of the status bar to use up before truncating. 
 *          If width is 0 then if_display_message will assume the whole width
 *          of the terminal.
 *
 * fmt:     The message to display
 */
void if_display_message(const char *msg, enum win_refresh dorefresh,
                        int width, const char *fmt, ...) ATTRIBUTE_PRINTF( 4, 5 );

/* if_clear_filedlg: Clears all the files the file dialog has to show the user.
 * -----------------
 */
void if_clear_filedlg(void);

/* if_add_fildlg_choice: adds the file filename to the choices the user gets.
 * ---------------------
 *
 *  filename: a file the user can choose to open.
 */
void if_add_filedlg_choice(const char *filename);

/* if_filedlg_display_message: Displays a message on the filedlg window status bar.
 * ---------------------------
 *
 * message: The message to display
 */
void if_filedlg_display_message(char *message);

/* if_shutdown: Cleans up, and restores the terminal (shuts off curses).
 * ------------
 */
void if_shutdown(void);

/* enum Focus: An enumeration representing a focus state. 
 * ------------
 *  GDB: focus on the gdb i/o window
 *  CGDB: focus on source window, accepts command input.
 *  CGDB_STATUS_BAR: focus on the status bar, accepts commands.
 *  FILE_DLG: focus on file dialog window
 */
typedef enum Focus { GDB, CGDB, CGDB_STATUS_BAR, FILE_DLG } Focus;

/* if_set_focus: Sets the current input focus to a different window 
 * ------------
 *  f: The region to focus
 */
void if_set_focus(Focus f);

/* if_get_focus: Sets the current input focus to a different window 
 * ------------
 *  Return: The region thats focused
 */
Focus if_get_focus(void);

/* if_display_help: Displays the help on the screen.
 * ------------
 */
void if_display_help(void);

/** 
 * Display a new logo in the source window.
 *
 * @param reset
 * If 0, will not reset the logo, otherwise will.
 */
void if_display_logo(int reset);

/* if_search_next: finds the next match in a given direction.
 * ------------
 */
void if_search_next(void);

/* if_draw:
 * -----------
 */
void if_draw(void);

 /**
  * Set the window split orientation. (vertical or horizontal)
  *
  * @param newOrientation
  * The orientation to switch to.
  */
void if_set_winsplitorientation(WIN_SPLIT_ORIENTATION_TYPE newOrientation);

 /* if_set_winsplit:
  * ________________
  */
void if_set_winsplit(WIN_SPLIT_TYPE newSplit);

/* if_highlight_sviewer:
 * ---------------------
 *
 *  Highlights the current node of the source viewer to be the
 *  new language type.
 *
 *  l 	The new language type to highlight.
 */
void if_highlight_sviewer(enum tokenizer_language_support l);

/* if_change_winminheight:
 * -----------------------
 * 
 * This sets the minimal height of a window. Windows will never become smaller.
 *
 * Returns -1 if value is not acceptable. Otherwise, 0.
 */
int if_change_winminheight(int value);

/* if_change_winminwidth:
 * 
 * This sets the minimal width of a window. Windows will never become smaller.
 *
 * Returns -1 if value is not acceptable. Otherwise, 0.
 */
int if_change_winminwidth(int value);

/**
 * This get's the height size of the GDB window.
 *
 * \return
 * The size of the window.
 */
int get_gdb_height(void);

/**
 * Will clear the last line in the GDB window. This function is slightly slow
 * as it writes spaces all the way, if it's necessary or not.
 *
 * \return
 * 0 on success or -1 on error
 */
int if_clear_line(void);

#ifdef TESTING
int tst_get_src_row();
int tst_get_src_col();
int tst_get_src_height();
int tst_get_src_width();
int tst_get_src_status_row();
int tst_get_src_status_col();
int tst_get_src_status_height();
int tst_get_src_status_width(); 
int tst_get_sep_row();
int tst_get_sep_col();
int tst_get_sep_height();
int tst_get_sep_width();
int tst_get_gdb_row();
int tst_get_gdb_col();
int tst_get_gdb_height();
int tst_get_gdb_width();
int tst_get_height();
int tst_get_width();
int tst_get_interface_winminheight();
int tst_get_interface_winminwidth();
void tst_set_window_shift(int shift);
int tst_get_window_shift();
void tst_reset_window_shift();
void tst_set_cur_split_orientation(WIN_SPLIT_ORIENTATION_TYPE orientation);
WIN_SPLIT_ORIENTATION_TYPE tst_get_cur_split_orientation();
void tst_set_cur_win_split(WIN_SPLIT_TYPE type);
WIN_SPLIT_TYPE tst_get_cur_win_split();
void tst_validate_window_sizes();
Focus tst_get_focus();
void tst_set_focus(enum Focus f);
void tst_set_screen_size(struct winsize scr_size);
void tst_create_swindow(SWINDOW **win, int nlines, int ncols, int begin_y,
                        int begin_x);
SWINDOW* tst_get_vseparator_win();
void tst_separator_display(int draw);
#endif // TESTING

#endif // _INTERFACE_H_
