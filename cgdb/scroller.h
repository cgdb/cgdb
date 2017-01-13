/* scroller.h:
 * -----------
 *
 * A scrolling buffer utility.  Able to add and subtract to the buffer.
 * All routines that would require a screen update will automatically redraw
 * the scroller.  There is no "draw" function.
 */

#ifndef _SCROLLER_H_
#define _SCROLLER_H_

/* Count of marks */
#define MARK_COUNT 26

/* --------------- */
/* Data Structures */
/* --------------- */

/** The possible sources of input for the scroller */
enum ScrInputKind {
    SCR_INPUT_DEBUGGER,   /* Input from the debugger (GDB) */
    SCR_INPUT_INFERIOR,   /* Input from the program being debugged */
    SCR_INPUT_READLINE    /* Input from readline (the prompt) */
};

struct scroller_line {
    char *line;
    int line_len;
    enum ScrInputKind kind;
    struct hl_line_attr *attrs;
};

struct scroller_mark
{
    int r;
    int c;
};

struct scroller {
    struct scroller_line *lines;

    char *last_inferior_line;
    int last_inferior_attr;

    int in_scroll_mode;         /* Currently in scroll mode? */

    /**
     * The number of lines to display in the scroller window.
     *
     * This starts at zero, and only the prompt is displayed in the
     * scroller. Run cgdb -q to see this.
     *
     * Every time another line is added to the scroller, this is increased,
     * so that more lines are dispalyed. It will ultimatley hit the height
     * of the scroller and max out.
     *
     * This field is used to allow the scroller to be cleared (Ctrl-l).
     *
     * The tricky part here is that long lines wrap in the scroller. When
     * this field is 1, indicating to display a single line, that line may
     * take up many rows in the scroller. The function number_rows_to_display
     * helps here.
     */
    int lines_to_display;

    struct {
        int r;                  /* Current line (row) number */
        int c;                  /* Current column number */
        int pos;                /* Cursor position in last line */
    } current;

    /** If in search mode. 1 if searching, 0 otherwise */
    int in_search_mode;
    /** The last regex searched for */
    struct hl_regex_info *last_hlregex;
    /** The current regex if in_search_mode is true */
    struct hl_regex_info *hlregex;

    /** 
     * The original row, or last selected row, when searching.
     */
    int search_r;
    SWINDOW *win; /* The scoller's own window */

    scroller_mark marks[MARK_COUNT]; /* Local a-z marks */
    scroller_mark jump_back_mark;    /* Location where last jump occurred from */
};

/* --------- */
/* Functions */
/* --------- */

/* scr_new: Creates and initializes a new scroller
 * --------
 *
 * Return Value: A pointer to a new scroller.
 */
struct scroller *scr_new(SWINDOW *window);

/* scr_free: Releases the memory allocated by a scroller
 * ---------
 *
 *   scr:  Pointer to the scroller object
 */
void scr_free(struct scroller *scr);

/* scr_up: Move up a number of lines
 * -------
 *
 *   scr:    Pointer to the scroller object
 *   nlines: Number of lines to scroll back; will not scroll past beginning
 */
void scr_up(struct scroller *scr, int nlines);

/* scr_down: Move down a number of lines
 * ---------
 *
 *   scr:    Pointer to the scroller object
 *   nlines: Number of lines to scroll down; will not scroll past end
 */
void scr_down(struct scroller *scr, int nlines);

/* scr_home: Jump to the top line of the buffer
 * ---------
 *
 *   scr:  Pointer to the scroller object
 */
void scr_home(struct scroller *scr);

/* scr_end: Jump to the bottom line of the buffer
 * --------
 *
 *   scr:  Pointer to the scroller object
 */
void scr_end(struct scroller *scr);

/* scr_add:  Append a string to the buffer.
 * --------
 *
 *   scr:  Pointer to the scroller object
 *   buf:  Buffer to append -- \b characters will be treated as backspace!
 */
void scr_add(struct scroller *scr, const char *buf, enum ScrInputKind kind);

/* Reposition the buffer on the screen
 *
 * @param scr
 * Pointer to the scroller object
 *
 * @param win
 * The new window
 */
void scr_move(struct scroller *scr, SWINDOW *win);

/**
 * Clear the scroller.
 *
 * When the scroller is cleared, only the prompt will be displayed
 * at the top of the gdb scroller window.
 *
 * @param scr
 * The scroller to clear.
 */
void scr_clear(struct scroller *scr);

/* scr_refresh: Refreshes the scroller on the screen, in case the caller
 * ------------ damages the screen area where the scroller is written (or,
 *              perhaps the terminal size has changed, and you wish to redraw).
 *
 *   scr:    Pointer to the scroller object
 *   focus:  If the window has focus
 */
void scr_refresh(struct scroller *scr, int focus, enum win_refresh dorefresh);

/**
 * Should be called before scr_search_regex
 *
 * This function initializes scr before it can search for a regex
 *
 * @param scr
 * Pointer to the scroller object
 */
void scr_search_regex_init(struct scroller *scr);

/**
 * Searches for regex in current scroller and displays line.
 *
 * @param scr
 * Pointer to the scroller object
 *
 * @param regex
 * The regular expression to search for. If NULL, then no regex will be tried.
 *
 * @param opt
 * If 1, Then the search is temporary ( User has not hit enter )
 * If 2, The search is perminant
 *
 * @param direction
 * If 0 then forward, else reverse
 *
 * @param icase
 * If 0 ignore case.
 *
 * @return
 * Zero on match, 
 * -1 if sview->cur is NULL
 * -2 if regex is NULL
 * -3 if regcomp fails
 * non-zero on failure.
 */
int scr_search_regex(struct scroller *scr, const char *regex, int opt,
    int direction, int icase);

int scr_set_mark(struct scroller *scr, int key);
int scr_goto_mark(struct scroller *scr, int key);

#endif
