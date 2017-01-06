#ifndef __HIGHLIGHT_GROUPS_H__
#define __HIGHLIGHT_GROUPS_H__

/*!
 * \file
 * highlight_group.h
 *
 * \brief
 * This file is dedicated to abstracting away the attributes and colors that 
 * are used to draw particular characters on the terminal. Each group has its
 * own characteristics, and can be configured by the user from the cgdbrc file.
 *
 * Basically, manages the colors and attributes (bold, reverse) etc. of text on
 * the screen.
 */

/* hl_group_kind {{{*/

/**
 * This is all of the different syntax highlighting color possibilities.
 * Each enumeration value represents a highlighting group. The rest of CGDB
 * can use these group names to represent which attributes the user wants to
 * use when drawing this particular group. Each highlighting group has 
 * default values that CGDB assigns to it. However, each of them are also
 * configurable.
 *
 * If you modify this enumeration, update the manual!
 */
enum hl_group_kind {
    HLG_KEYWORD = 1,
    HLG_TYPE,
    HLG_LITERAL,
    HLG_COMMENT,
    HLG_DIRECTIVE,
    HLG_TEXT,
    HLG_INCSEARCH,
    HLG_SEARCH,
    HLG_STATUS_BAR,
    HLG_EXECUTING_LINE_ARROW,
    HLG_SELECTED_LINE_ARROW,
    HLG_EXECUTING_LINE_HIGHLIGHT,
    HLG_SELECTED_LINE_HIGHLIGHT,
    HLG_EXECUTING_LINE_BLOCK,
    HLG_SELECTED_LINE_BLOCK,
    HLG_ENABLED_BREAKPOINT,
    HLG_DISABLED_BREAKPOINT,
    HLG_SELECTED_LINE_NUMBER,
    HLG_EXECUTING_LINE_NUMBER,
    HLG_SCROLL_MODE_STATUS,
    HLG_LOGO,
    HLG_MARK,
    HLG_LAST,

    /* Straight colors - not configurable */
    HLG_BLACK,
    HLG_RED,
    HLG_GREEN,
    HLG_YELLOW,
    HLG_BLUE,
    HLG_MAGENTA,
    HLG_CYAN,
    HLG_WHITE,
    HLG_BOLD_BLACK,
    HLG_BOLD_RED,
    HLG_BOLD_GREEN,
    HLG_BOLD_YELLOW,
    HLG_BOLD_BLUE,
    HLG_BOLD_MAGENTA,
    HLG_BOLD_CYAN,
    HLG_BOLD_WHITE,
};

/* }}}*/

/* Creating and Destroying a hl_groups context. {{{*/
/******************************************************************************/
/**
 * @name Createing and Destroying a hl_groups context.
 * These functions are for createing and destroying a hl_groups context.
 */
/******************************************************************************/

/*@{*/

/**
 *  This struct is a reference to a hl_groups instance.
 */
struct hl_groups;
typedef struct hl_groups *hl_groups_ptr;

/** 
 * Currently, there is only a single instance. This is used (init/destroyed)
 * externally to this file. The entire application can use this to represent 
 * the current highlighting groups. In the future, it's possible each window 
 * could have there own * instance.
 */
extern hl_groups_ptr hl_groups_instance;

/**
 * This initializes an hl_groups instance.
 *
 * The client must call this function before any other function in the 
 * hl_groups library.
 *
 * @return
 * NULL on error, a valid context on success.
 */
hl_groups_ptr hl_groups_initialize(void);

/**
 * This will terminate a hl_groups session. No functions should be called on
 * the hl_groups context passed into this function after this call.
 *
 * \param hl_groups
 * An instance of hl_groups to operate on.
 *
 * @return
 * 0 on success or -1 on error
 */
int hl_groups_shutdown(hl_groups_ptr hl_groups);

/*@}*/
/* }}}*/

/* Functional commands {{{*/
/******************************************************************************/
/**
 * @name Functional commands
 * These functinos are used to ask the hl_groups context to perform a task.
 */
/******************************************************************************/

/*@{*/

/**
 * Setup the highlighting group to have all of the default values.
 *
 * \param hl_groups
 * An instance of hl_groups to operate on.
 *
 * \return
 * 0 on success or -1 on error
 */
int hl_groups_setup(hl_groups_ptr hl_groups);

/**
 * Get the attributes that may be passed to swin_wattron to tell the curses library
 * how to print this particular group.
 *
 * Note, this function can not fail.
 *
 * \param hl_groups
 * An instance of hl_groups to operate on.
 *
 * \param kind
 * The particular group to get the attributes for.
 *
 * \return
 * The attributes associated with the highlight group and the kind.
 *
 * If hl_groups is invalid than A_NORMAL will be returned or A_BOLD when
 * kind is HLG_EXECUTING_LINE_HIGHLIGHT.
 */
int hl_groups_get_attr(hl_groups_ptr hl_groups, enum hl_group_kind kind);

/**
 * Parse a particular command. This may move into the cgdbrc file later on.
 *
 * \param hl_groups
 * An instance of hl_groups to operate on.
 *
 * \return
 * 0 on success or -1 on error
 */
int hl_groups_parse_config(hl_groups_ptr hl_groups);

/**
 * Parse an ansi SGR (Select Graphic Rendition) escape sequence and return the
 * attributes you can use with ncurses.
 *
 * \param hl_groups
 * An instance of hl_groups to operate on.
 *
 * \param buf
 * String with escape sequence to parse
 *
 * \param attr
 * Ncurses attribute calculated from escape sequence
 *
 * \return
 * Number of characters in escape sequence
 */
int hl_ansi_get_color_attrs(hl_groups_ptr hl_groups,
    const char *buf, int *attr);

enum hl_group_kind hl_get_color_group(const char *color);

/**
 * Given a set of attributes and the column they start at, print the line.
 */
struct hl_line_attr {
    int col;
    int attr;
};

/**
 * Print a line with highlighting.
 *
 * @param win
 * The window to write to.
 *
 * @param line
 * The line to write.
 *
 * @param line_len
 * The length of the line to write.
 *
 * @param attrs
 * The attributes to write.
 *
 * @param x
 * The x position to write to, -1 for current position.
 *
 * @param y
 * The y position to write to, -1 for current position.
 *
 * @param col
 * The column to write to.
 *
 * @param width
 */
void hl_printline(SWINDOW *win, const char *line, int line_len,
        const hl_line_attr *attrs, int x, int y, int col, int width);

/**
 * Print a line with highlighting.
 *
 * This differs from hl_printline by only printing the text with attributes.
 * This is useful if you want to first print the line with syntax
 * highlighting and then do another pass with regex highlighting (or some
 * other attributes to highlight on top of the syntax).
 *
 * @param win
 * The window to write to.
 *
 * @param line
 * The line to write.
 *
 * @param line_len
 * The length of the line to write.
 *
 * @param attrs
 * The attributes to write.
 *
 * @param x
 * The x position to write to, -1 for current position.
 *
 * @param y
 * The y position to write to, -1 for current position.
 *
 * @param col
 * The column to write to.
 *
 * @param width
 */
void hl_printline_highlight(SWINDOW *win, const char *line, int line_len,
        const hl_line_attr *attrs, int x, int y, int col, int width);

/*@}*/
/* }}}*/

#endif /* __HIGHLIGHT_GROUPS_H__ */
