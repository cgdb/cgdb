#ifndef _FILEDLG_H_
#define _FILEDLG_H_

#include <curses.h>

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

/* filedlg_new: Create a new file dialog.
 * ____________
 *
 *   pos_r:   position of the viewer (row)
 *   pos_c:   position of the viewer (column)
 *   height:  height (in lines) of the viewer
 *   width:   width (in columns) of the viewer
 *
 * return value:  a new filedlg object on success, null on failure.
 */
struct filedlg *filedlg_new(int pos_r, int pos_c, int height, int width);

/* filedlg_free:  Release the memory associated with a file dialog.
 * ------------
 *
 *   fdlg:  The file dialog to free.
 */
void filedlg_free(struct filedlg *fdlg);

/* filedlg_add_file_choice:  Add a file to the list of source files.
 * -----------
 *
 * file_choice: A path to a file that the user will be able to choose from.
 *
 * Return Value:  Zero on success, non-zero on error.
 */
int filedlg_add_file_choice(struct filedlg *fd, const char *file_choice);

/* filedlg_clear: Clears all the file_choice's in the dialog.
 * ______________
 */
void filedlg_clear(struct filedlg *fd);

/* filedlg_choose: Allows user to choose a file.
 * _______________
 *
 * fd: The file dialog to use.
 *
 * file: Will be returned as the file name the user picked.
 *       Should be long enough to have a path put into it.
 *
 * Return Value: Zero on success, non-zero on error.
 */
int filedlg_choose(struct filedlg *fd, char *file);

/* filedlg_display_message: Displays a message on the filedlg window status bar.
 * ------------------------
 *
 * fd:      The file dialog to use.
 * message: The message to display
 */
void filedlg_display_message(struct filedlg *fd, char *message);

#endif /* _FILEDLG_H_ */
