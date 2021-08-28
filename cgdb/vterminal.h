#ifndef VTERMINAL_H
#define VTERMINAL_H

#include <string>
#include <stddef.h>

// A virtual terminal based on vterm
struct VTerminal;

struct VTerminalOptions
{
    void *data;
    int width, height; 
    void (*terminal_write_cb)(char *buffer, size_t size, void *data);
    void (*terminal_resize_cb)(int width, int height, void *data);
    void (*terminal_close_cb)(void *data);
};

VTerminal *vterminal_new(VTerminalOptions options);
void vterminal_free(VTerminal *vterminal);

void vterminal_push_bytes(VTerminal *terminal, const char *bytes, size_t len);
void vterminal_get_height_width(VTerminal *terminal, int &height, int &width);

// Fetch the text and attributes for a row and column
// 
// @param terminal
// The terminal to fetch the row text from
//
// @param row
// The row to fetch at
//
// @param col
// The column to fetch at
//
// @param utf8text
// Will return the text set at the row/col
//
// @param attr
// Will return the attributes set at the row/col
//
// @param width
// The width of the row/col.
// An ascii character is size 1, width == 1
// A wide unicode character is size 2, width == 2
// The next column you access should be col + width
void vterminal_fetch_row_col(VTerminal *terminal, int row,
        int col, std::string &utf8text, int &attr, int &width);

void vterminal_fetch_row(VTerminal *terminal, int row,
        int start_col, int end_col, std::string &utf8text);
void vterminal_get_cursor_pos(VTerminal *terminal, int &row, int &col);
// Get the number of rows in the scrollback buffer
// This does not include rows in vterm currently
void vterminal_get_sb_num_rows(VTerminal *terminal, int &num);
void vterminal_scroll_delta(VTerminal *terminal, int delta);
void vterminal_scroll_get_delta(VTerminal *terminal, int &delta);
void vterminal_scroll_set_delta(VTerminal *terminal, int delta);
void vterminal_push_screen_to_scrollback(VTerminal *terminal);

#endif
