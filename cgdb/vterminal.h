#ifndef VTERMINAL_H
#define VTERMINAL_H

#include <string>
#include <stddef.h>

// A virtual terminal based on vterm
struct VTerminal;

struct VTerminalOptions
{
    // An opaque pointer for callbacks
    void *data;

    // The height of the virtual terminal
    int height; 

    // The width of the virtual terminal
    int width;

    // The size (number of rows) of the scrollback buffer
    int scrollback_buffer_size;

    // A function to ring the bell
    void (*ring_bell)(void *data);
};

// Create a new virtual terminal
//
// @param options
// The options to use when creating the virtual terminal
VTerminal *vterminal_new(VTerminalOptions options);

// Free a virtual terminal
//
// @param terminal
// The terminal to operate on
void vterminal_free(VTerminal *terminal);

// Write data to the virtual terminal
//
// @param terminal
// The terminal to operate on
//
// @param data
// The data to write to the virtual terminal
//
// @param len
// The number of characters in data to write
void vterminal_write(VTerminal *terminal, const char *data, size_t len);

// Get the height and width of the terminal
//
// @param terminal
// The terminal to operate on
//
// @param height
// The height of the virtual terminal
//
// @param width
// The width of the virtual terminal
void vterminal_get_height_width(VTerminal *terminal, int &height, int &width);

// Fetch the text and attributes for a row and column
// 
// @param terminal
// The terminal to operate on
//
// @param row
// The row to fetch at
//
// @param col
// The column to fetch at
//
// @param utf8text
// Will return the text at the row/col
//
// @param attr
// Will return the attributes at the row/col
//
// @param width
// Will return the width of the row/col.
// An ascii character is size 1, width == 1
// A wide unicode character is size 2, width == 2
// The next column you access should be col + width
void vterminal_fetch_row_col(VTerminal *terminal, int row,
        int col, std::string &utf8text, int &attr, int &width);

// Fetch the text for a row and column range
// 
// @param terminal
// The terminal to operate on
//
// @param row
// The row to fetch at
//
// @param col
// The starting column to fetch at
//
// @param end_col
// The ending column to fetch up to (end_col itself is not included)
//
// @param utf8text
// Will return the text between the start and end column
void vterminal_fetch_row(VTerminal *terminal, int row,
        int start_col, int end_col, std::string &utf8text);

// Get the position of the cursor in the terminal
// 
// @param terminal
// The terminal to operate on
//
// @param row
// The row the cursor is on
//
// @param col
// The column the cursor is on
void vterminal_get_cursor_pos(VTerminal *terminal, int &row, int &col);

// Get the number of rows in the scrollback buffer
//
// @param terminal
// The terminal to operate on
//
// @param num
// The number of rows in the scrollback buffer
void vterminal_scrollback_num_rows(VTerminal *terminal, int &num);

// Adjust the scrollback buffer position
//
// @param terminal
// The terminal to operate on
//
// @param delta
// The amount to adjust the scrollback buffer
// A positives number scrolls "up", you can see more of the scrollback buffer.
// A negative number scrolls "down", you see more of the active terminal.
// No matter the delta passed, this constraint will be followed internally:
//    0 >= active delta <= vterminal_scrollback_num_rows()
void vterminal_scroll_delta(VTerminal *terminal, int delta);

// Get the current scrollback delta
//
// @param terminal
// The terminal to operate on
//
// @param delta
// The current scrollback delta in the terminal
// The delta return value will be constrained as follows:
//    0 >= delta <= vterminal_scrollback_num_rows()
void vterminal_scroll_get_delta(VTerminal *terminal, int &delta);

// Set the current scrollback delta
//
// @param terminal
// The terminal to operate on
//
// @param delta
// The scrollback delta
// When delta is 0, the terminal is not scrolling.
// When delta is 1, the terminal scrolls back 1 line.
// When delta is 2, the terminal scrolls back 2 lines.
// The terminal can be scrolled back to vterminal_scrollback_num_rows().
void vterminal_scroll_set_delta(VTerminal *terminal, int delta);

// Push the terminal screen contents to the scrollback buffer
//
// This is generally useful when the user types ctrl-l to clear the screen.
// Traditionally vterm would clear the screen, but not push the contents
// of the screen to the scrollback buffer. This allows the terminal contents
// to be pushed to the scrollback buffer before clearing it.
//
// @param terminal
// The terminal to operate on
void vterminal_push_screen_to_scrollback(VTerminal *terminal);

#endif
