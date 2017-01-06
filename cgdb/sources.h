/* sources.h:
 * ----------
 * 
 * Source file management routines for the GUI.  Provides the ability to
 * add files to the list, load files, and display within a curses window.
 * Files are buffered in memory when they are displayed, and held in
 * memory for the duration of execution.  If memory consumption becomes a
 * problem, this can be optimized to unload files which have not been
 * displayed recently, or only load portions of large files at once. (May
 * affect syntax highlighting.)
 *
 */

#ifndef _SOURCES_H_
#define _SOURCES_H_

#include "sys_win.h"

/* ----------- */
/* Definitions */
/* ----------- */

/* Max length of a line */
#define MAX_LINE        4096

/* Count of marks */
#define MARK_COUNT      26

/* --------------- */
/* Data Structures */
/* --------------- */

/* Global mark: source file and line number */
struct sviewer_mark {
    struct list_node *node;
    int line;
};

/* Source viewer object */
struct sviewer {
    struct list_node *list_head;           /* File list */
    struct list_node *cur;                 /* Current node we're displaying */
    sviewer_mark global_marks[MARK_COUNT]; /* Global A-Z marks */
    sviewer_mark jump_back_mark;           /* Location where last jump occurred from */

    SWINDOW *win;                          /* Curses window */
    uint64_t addr_frame;                   /* Current frame address 
                                              Zero if unknown. */
    /**
     * The last regular expression searched for.
     *
     * This is useful with the hlsearch option. In this case,
     * CGDB will display all of the matches to the last regular
     * expression searched for.
     */
    struct hl_regex_info *last_hlregex;

    /**
     * The current regular expression being searched for.
     *
     * This is the active regular expression being searched for. It is
     * interactive, as the user updates the regular expression they
     * would like to use for searching, CGDB highlights the text in
     * the source that represents the next match.
     */
    struct hl_regex_info *hlregex;
};

struct source_line {
    char *line;
    int len;
    struct hl_line_attr *attrs;
};

struct buffer {
    struct source_line *lines;  /* Stretch buffer array with line information */
    uint64_t *addrs;            /* The list of corresponding addresses */
    int max_width;              /* Width of longest line in file */
    char *file_data;            /* Entire file pointer if read in that way */
    int tabstop;                /* Tabstop value used to load file */
    enum tokenizer_language_support language;   /* The language type of this file */
};

struct line_flags {
    unsigned char breakpt : 2;
    unsigned char has_mark : 1;
};

struct list_node {
    char *path;                 /* Full path to source file */
    struct buffer file_buf;     /* File buffer */
    line_flags *lflags;         /* Breakpoints */
    int sel_line;               /* Current line selected in viewer */
    int sel_col;                /* Current column selected in viewer */
    int exe_line;               /* Current line executing, or -1 if not set */
    int sel_rline;              /* Current line used by regex */

    enum tokenizer_language_support language;   /* The language type of this file */

    time_t last_modification;   /* timestamp of last modification */

    int local_marks[MARK_COUNT];/* Line numbers for local (a..z) marks */

    uint64_t addr_start;        /* Disassembly start address */
    uint64_t addr_end;          /* Disassembly end address */

    struct list_node *next;     /* Pointer to next link in list */
};

/* --------- */
/* Functions */
/* --------- */

/* source_new:  Create a new source viewer object.
 * -----------
 *
 * Return Value:  A new sviewer object on success.
 */
struct sviewer *source_new(SWINDOW *window);

/* source_add:  Add a file to the list of source files.
 * -----------
 *
 *   sview:  Source viewer object
 *   path:   Full path to the source file (this is considered to be a
 *           unique identifier -- no duplicate paths in the list!)
 *
 * Return Value:  pointer to your brand new node.
 */
struct list_node *source_add(struct sviewer *sview, const char *path);

void source_add_disasm_line(struct list_node *node, const char *line);

int source_highlight(struct list_node *node);

struct list_node *source_get_node(struct sviewer *sview, const char *path);

/* source_del:  Remove a file from the list of source files.
 * -----------
 *
 *   sview:  Source viewer object
 *   path:   Full path to the source file to remove from the list.  If the
 *           file is buffered in memory, it'll be freed.
 *
 * Return Value:  Zero on success, non-zero on error.
 */
int source_del(struct sviewer *sview, const char *path);

/* source_length:  Get the length of a source file.  If the source file hasn't
 * --------------  been buffered already, it will be loaded into memory.
 *
 *   sview:  Source viewer object
 *   path:   Full path to the source file
 *
 * Return Value:  Length of file on success, -1 on error.
 */
int source_length(struct sviewer *sview, const char *path);

/* source_current_file: Get the name of the current file being displayed.
 * --------------------
 *
 *  path: The path to the current file being displayed.
 *  
 *  Return Value: NULL if no file is being displayed, otherwise a pointer to
 *                the current path of the file being displayed.
 */
char *source_current_file(struct sviewer *sview);

/* source_display:  Display a portion of a file in a curses window.
 * ---------------
 *
 *   sview:  Source viewer object
 *   focus:  If the window should have focus
 *
 * Return Value:  Zero on success, non-zero on error.
 */
int source_display(struct sviewer *sview, int focus, enum win_refresh dorefresh);

/* Relocate the source window.
 *
 * @param sview
 * Source viewer object
 *
 * @param win
 * The new window
 */
void source_move(struct sviewer *sview, SWINDOW *win);

/* source_vscroll:  Change current position in source file.
 * --------------
 * 
 *   sview:   Source viewer object
 *   offset:  Plus or minus number of lines to move
 */
void source_vscroll(struct sviewer *sview, int offset);

/* source_hscroll:  Scroll the source file right or left in the window.
 * ---------------
 *
 *   sview:   Source viewer object
 *   offset:  Plus or minus number of lines to move
 */
void source_hscroll(struct sviewer *sview, int offset);

/* source_set_sel_line:  Set current user-selected line
 * --------------------
 *
 *   sview:  Source viewer object
 *   line:   Current line number, or -1 for the last line
 *
 */
void source_set_sel_line(struct sviewer *sview, int line);

/* source_set_exec_line:  Set currently selected line and executing line
 * ---------------
 *
 *   sview:     Source viewer object
 *   path:      Full path to the source file (may be NULL to leave unchanged)
 *   sel_line:  Current selected line number (0 to leave unchanged)
 *   exe_line:  Current executing line number (0 to leave unchanged)
 *
 * Return Value: Zero on success, non-zero on failure.
 *               5 -> file does not exist
 */
int source_set_exec_line(struct sviewer *sview, const char *path, int sel_line, int exe_line);

int source_set_exec_addr(struct sviewer *sview, uint64_t addr);

/**
 * Initialize a regular expression search in the source viewer.
 *
 * This function should be called during the start of a regular expression
 * search and before source_search_regex is called. 
 *
 * @param sview
 * The source viewer object
 */
void source_search_regex_init(struct sviewer *sview);

/**
 * Allows the user to search for a regular expression.
 *
 * @param sview
 * The source viewer object
 *
 * @param regex
 * The regular expression to search for. If NULL, then no regex will be
 * tried, but the state can still put back to its old self!
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
 * Zero on match and non-zero on failure.
 */
int source_search_regex(struct sviewer *sview, const char *regex, int opt,
        int direction, int icase);

/* source_free:  Release the memory associated with a source viewer.
 * ------------
 *
 *   sview:  The source viewer to free.
 */
void source_free(struct sviewer *sview);

/* ----------- */
/* Breakpoints */
/* ----------- */

/* source_enable_break:  Enable a given breakpoint.
 * --------------------
 *
 *   sview:    The source viewer object
 *   path:     The path to the source file, absolute if available
 *   line:     Line number of breakpoint
 *   enabled:  0 for disabled, otherwise enabled
 */
void source_enable_break(struct sviewer *sview, const char *path,
        int line, int enabled);

/**
 * Replace all existing breakpoints with the breakpoints provided.
 *
 * @param sview
 * The source viewer object
 * 
 * @param breakpoints
 * The new breakpoints to set
 */
void source_set_breakpoints(struct sviewer *sview,
        struct tgdb_breakpoint *breakpoints);

/**
 * Check's to see if the current source file has changed. If it has it loads
 * the new source file up.
 *
 * \param sview
 * The source viewer object
 *
 * \param path
 * The path to the file to reload into memory
 *
 * \param force
 * Force the file to be reloaded, even if autosourcereload option is off.
 *
 * \return
 * 0 on success or -1 on error
 */
int source_reload(struct sviewer *sview, const char *path, int force);

/* ----- */
/* Marks */
/* ----- */

/* source_set_mark:  Set mark at current selected line.
 * --------------------
 *
 *   sview:  The source viewer object
 *   key: local mark char: a..z or global mark: A..Z
 *   return: 1 if a mark was set, 0 otherwise
 */
int source_set_mark(struct sviewer *sview, int key);

/* source_goto_mark:  Goto mark specified at key.
 * --------------------
 *
 *   sview:  The source viewer object
 *   key: local mark char: a..z or global mark: A..Z
 *   return: 1 if a jump was successful, 0 otherwise
 */
int source_goto_mark(struct sviewer *sview, int key);

#endif
