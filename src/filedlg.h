#ifndef _FILEDLG_H_
#define _FILEDLG_H_

struct filedlg;

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

/* filedlg_recv_char: Sens a character to the filedlg.
 *
 *   fdlg:  The file dialog to free.
 *   key :  The next key of input to process
 *   file:  The file the user selected
 *
 *  returns -1 when aborted by user.
 *  returns 0 when needs more input
 *  returns 1 when done ( file is valid )
 */
int filedlg_recv_char(struct filedlg *fd, int key, char *file);

/* filedlg_display_message: Displays a message on the filedlg window status bar.
 * ------------------------
 *
 * fd:      The file dialog to use.
 * message: The message to display
 */
void filedlg_display_message(struct filedlg *fd, char *message);

/* filedlg_display: Redraws the file dialog.
 *
 * Returns 0 on success or -1 on error
 */
int filedlg_display( struct filedlg *fd );

#endif /* _FILEDLG_H_ */
