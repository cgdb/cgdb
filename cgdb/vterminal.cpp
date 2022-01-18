#include "vterminal.h"
// To use fill_utf8 
#include "utf8.h"
#include "vterm.h"
#include "sys_util.h"
#include "scroller.h"

#include "sys_win.h"
#include "highlight_groups.h"

// The scrollback buffer data structure
typedef struct {
    // The number of cells in the list below
    size_t cols;
    // A list of cells
    VTermScreenCell cells[];
} ScrollbackLine;

struct VTerminal
{
    VTerminal(VTerminalOptions options);
    ~VTerminal();

    // Write data to vterm
    //
    // @param data
    // The data to write to vterm
    //
    // @param len
    // The number of characters in data to write
    void write(const char *data, size_t len);

    // Move the cursor to the new location
    //
    // @param newp
    // The new location of the cursor
    //
    // @param oldp
    // The old location of the cursor
    //
    // @param visible
    // The cursor is not visible when zero, otherwise visible
    void movecursor(VTermPos newp, VTermPos oldp, int visible);

    // Set some terminal properties
    // 
    // @param prop
    // The property to set
    //
    // @param val
    // The value to set the property to
    //
    // @return
    // 1 on success, otherwise 0
    int settermprop(VTermProp prop, VTermValue *val);


    // Ring the bell
    void bell();

    // Push a line onto the scrollback buffer
    //
    // @param cols
    // The number of columns in the row to push to the scrollback buffer
    //
    // @param cells
    // The cells in each row to push to the scrollback buffer
    //
    // @return
    // On success will return 1, otherwise 0
    int sb_pushline(int cols, const VTermScreenCell *cells);
    
    // Pop a line off the scrollback buffer
    //
    // @return
    // On success will return 1, otherwise 0
    int sb_popline(int cols, VTermScreenCell *cells);

    // Convert VTermScreen cell arrays into utf8 strings
    // Currently it stores the string in textbuf, however, I suggest it may
    // be better to return a std::string
    // Also: Error handling?
    void fetch_row(int row, int start_col, int end_col, int &attr, int &width);
    // Fetch a single cell
    bool fetch_cell(int row, int col, VTermScreenCell *cell);

    // Adjust the scrollback buffer position
    //
    // See vterminal_scroll_delta for comments
    void scroll_delta(int delta);

    // Get the current scrollback delta
    //
    // See vterminal_scroll_get_delta for comments
    void scroll_get_delta(int &delta);

    // Set the current scrollback delta
    //
    // See vterminal_scroll_set_delta for comments
    void scroll_set_delta(int delta);

    // Push the terminal screen contents to the scrollback buffer
    //
    // See vterminal_push_screen_to_scrollback for comments
    void push_screen_to_scrollback();

    // options passed to this terminal instance
    VTerminalOptions options;

    // libvterm terminal and screen instance
    VTerm *vt;
    VTermScreen *vts;

    // buffer used to:
    //  - convert VTermScreen cell arrays into utf8 strings
    //  - MAYBE: receive data from libvterm as a result of key presses.
    char textbuf[0x1fff];

    // The number of lines scrolled back, initialized to zero
    int scroll_offset;

    // Scrollback buffer storage
    ScrollbackLine **sb_buffer;

    // Number of rows pushed to sb_buffer.
    // Does not include rows in vterm currently.
    size_t sb_current;

    // The scrollback buffer size (sb_buffer)
    size_t sb_size;

    // True if the cursor is visible, otherwise false
    bool cursor_visible;

    // The position of the cursor
    VTermPos cursorpos;
};

static int vterminal_damage(VTermRect rect, void *data);
static int vterminal_moverect(VTermRect dest, VTermRect src, void *data);
static int vterminal_movecursor(VTermPos newp, VTermPos oldp, int visible,
    void *data);
static int vterminal_settermprop(VTermProp prop, VTermValue *val, void *data);
static int vterminal_bell(void *data);
static int vterminal_resize(int rows, int cols, void *data);
static int vterminal_sb_pushline(int cols, const VTermScreenCell *cells,
    void *data);
static int vterminal_sb_popline(int cols, VTermScreenCell *cells, void *data);

static VTermScreenCallbacks vterm_screen_callbacks = {
  .damage      = vterminal_damage,
  .moverect    = vterminal_moverect,
  .movecursor  = vterminal_movecursor,
  .settermprop = vterminal_settermprop,
  .bell        = vterminal_bell,
  .resize      = vterminal_resize,
  .sb_pushline = vterminal_sb_pushline,
  .sb_popline  = vterminal_sb_popline,
};

VTerminal::VTerminal(VTerminalOptions options) : vt(nullptr)
{
    this->options = options;
    cursorpos.row = 0;
    cursorpos.col = 0;
    cursor_visible = true;

    // neovim has a buffer assignment here, can i use our scroller or
    // do i need a new buffer concept?

    // Create vterm
    vt = vterm_new(this->options.height, this->options.width);
    vterm_set_utf8(vt, 1);

    // Setup state
    VTermState *state = vterm_obtain_state(vt);
#if 0
    // TODO: Determine if the colors are set by vterm automatically
    for (int index = 0; index < 16; ++index) {
        VTermColor color;
        vterm_color_rgb(&color,
                        (uint8_t)((color_val >> 16) & 0xFF),
                        (uint8_t)((color_val >> 8) & 0xFF),
                        (uint8_t)((color_val >> 0) & 0xFF));
        vterm_state_set_palette_color(state, index,
    }
#endif

    // Setup screen
    vts = vterm_obtain_screen(vt);
    vterm_screen_set_callbacks(vts, &vterm_screen_callbacks, this);
    vterm_screen_set_damage_merge(vts, VTERM_DAMAGE_SCROLL);
    vterm_screen_reset(vts, 1);

    // Configure the scrollback buffer.
    scroll_offset = 0;
    sb_current = 0;
    sb_size = this->options.scrollback_buffer_size;
    sb_buffer = (ScrollbackLine**)malloc(sizeof(ScrollbackLine *) * sb_size);
}

VTerminal::~VTerminal()
{
    for (size_t i = 0; i < sb_current; i++) {
      free(sb_buffer[i]);
    }
    free(sb_buffer);
    vterm_free(vt);
}

void
VTerminal::write(const char *data, size_t len)
{
    vterm_input_write(vt, data, len);
    vterm_screen_flush_damage(vts);
}

void
VTerminal::movecursor(VTermPos newp, VTermPos oldp, int visible)
{
    cursorpos.row = newp.row;
    cursorpos.col = newp.col;
}

int
VTerminal::settermprop(VTermProp prop, VTermValue *val)
{
    switch (prop) {
        case VTERM_PROP_CURSORVISIBLE:
            cursor_visible = val->boolean;
            break;

        default:
            return 0;
    }

    return 1;
}

void
VTerminal::bell()
{
    if (this->options.ring_bell) {
        this->options.ring_bell(this->options.data);
    }
}

int
VTerminal::sb_pushline(int cols, const VTermScreenCell *cells)
{
    if (!sb_size) {
        return 0;
    }

    // copy vterm cells into sb_buffer
    size_t c = (size_t)cols;
    ScrollbackLine *sbrow = NULL;
    if (sb_current == sb_size) {
        if (sb_buffer[sb_current - 1]->cols == c) {
            // Recycle old row if it's the right size
            sbrow = sb_buffer[sb_current - 1];
        } else {
            free(sb_buffer[sb_current - 1]);
        }

        // Make room at the start by shifting to the right.
        memmove(sb_buffer + 1, sb_buffer,
                sizeof(sb_buffer[0]) * (sb_current - 1));
    } else if (sb_current > 0) {
        // Make room at the start by shifting to the right.
        memmove(sb_buffer + 1, sb_buffer, sizeof(sb_buffer[0]) * sb_current);
    }

    if (!sbrow) {
        sbrow = (ScrollbackLine *)malloc(
            sizeof(ScrollbackLine) + c * sizeof(sbrow->cells[0]));
        sbrow->cols = c;
    }

    // New row is added at the start of the storage buffer.
    sb_buffer[0] = sbrow;
    if (sb_current < sb_size) {
        sb_current++;
    }

    memcpy(sbrow->cells, cells, sizeof(cells[0]) * c);

    return 1;
}

int
VTerminal::sb_popline(int cols, VTermScreenCell *cells)
{
    if (!sb_current) {
        return 0;
    }

    ScrollbackLine *sbrow = sb_buffer[0];
    sb_current--;

    // Forget the "popped" row by shifting the rest onto it.
    memmove(sb_buffer, sb_buffer + 1, sizeof(sb_buffer[0]) * (sb_current));

    size_t cols_to_copy = (size_t)cols;
    if (cols_to_copy > sbrow->cols) {
        cols_to_copy = sbrow->cols;
    }

    // copy to vterm state
    memcpy(cells, sbrow->cells, sizeof(cells[0]) * cols_to_copy);
    for (size_t col = cols_to_copy; col < (size_t)cols; col++) {
        cells[col].chars[0] = 0;
        cells[col].width = 1;
    }

    free(sbrow);

    return 1;
}

int ansi_get_closest_color_value(int r, int g, int b);

static int get_ncurses_color_index(VTermColor &color, bool &bold)
{
    int index = -1;
    bold = false;

    if (VTERM_COLOR_IS_DEFAULT_FG(&color)) {
        index = -1;
    } else if (VTERM_COLOR_IS_DEFAULT_BG(&color)) {
        index = -1;
    } else if (VTERM_COLOR_IS_INDEXED(&color)) {
        if (color.indexed.idx >= 0 && color.indexed.idx < 16) {
            index = color.indexed.idx;
        } else if (color.indexed.idx >=232) {
            int num = color.indexed.idx;
            int gray = 255 * (MIN(num, 255) - 232) / (255 - 232);
            index = ansi_get_closest_color_value( gray, gray, gray );
        } else if (color.indexed.idx >= 16) {
            int num = color.indexed.idx;
            int red = ((num - 16) / 36);
            int green = (((num - 16) - red * 36) / 6);
            int blue = ((num - 16) % 6);
            index = ansi_get_closest_color_value( red * 255 / 6,
                    green * 255 / 6, blue * 255 / 6 );
        }

        // Colors 8 through 15 are high intensity colors
        // To my knowledge, the only way to handle this with ncurses is
        // to bold the corresponding low intensity numbers.
        // https://en.wikipedia.org/wiki/ANSI_escape_code
        if (index >=8 && index < 16) {
            index = index - 8;
            bold = true;
        }
    } else if (VTERM_COLOR_IS_RGB(&color)) {
        // TODO: RGB is currently unsupported
    }
    return index;
}

void
VTerminal::fetch_row(int row, int start_col, int end_col, int &attr, int &width)
{
  int col = start_col;
  size_t line_len = 0;
  char *ptr = textbuf;

 row = row - scroll_offset;

  while (col < end_col) {
    VTermScreenCell cell;
    fetch_cell(row, col, &cell);

    // TODO: What about rgb colors?
    bool fg_bold = false, bg_bold = false;
    int fg_index = get_ncurses_color_index(cell.fg, fg_bold);
    int bg_index = get_ncurses_color_index(cell.bg, bg_bold);
    hl_get_color_attr_from_index(fg_index, bg_index, attr);

    // set attributes
    if (cell.attrs.bold || fg_bold || bg_bold) {
        attr |= SWIN_A_BOLD;
    } else if (cell.attrs.underline) {
        attr |= SWIN_A_UNDERLINE;
    } else if (cell.attrs.blink) {
        attr |= SWIN_A_BLINK;
    } else if (cell.attrs.reverse) {
        attr |= SWIN_A_REVERSE;
    }

    int cell_len = 0;
    if (cell.chars[0]) {
      for (int i = 0; cell.chars[i]; i++) {
        cell_len += fill_utf8(cell.chars[i], ptr);
      }
    } else {
      *ptr = ' ';
      cell_len = 1;
    }
    char c = *ptr;
    ptr += cell_len;
    if (c != ' ') {
      // only increase the line length if the last character is not whitespace
      line_len = (size_t)(ptr - textbuf);
    }
    col += cell.width;
  }

  // trim trailing whitespace
  textbuf[line_len] = 0;
  width = col - start_col;
}

bool
VTerminal::fetch_cell(int row, int col, VTermScreenCell *cell)
{
  if (row < 0) {
    if(-row > sb_current) {
        clog_error(CLOG_CGDB, "Attempt to fetch scrollback beyond"
            " buffer at line %d\n", -row);
      return false;
    }

    /* pos.row == -1 => sb_buffer[0], -2 => [1], etc... */
    ScrollbackLine *sbrow = sb_buffer[-row - 1];
    if ((size_t)col < sbrow->cols) {
      *cell = sbrow->cells[col];
    } else {
      // fill the pointer with an empty cell
      *cell = (VTermScreenCell) {
        .chars = { 0 },
        .width = 1,
      };
      return false;
    }
  } else {
    vterm_screen_get_cell(vts, (VTermPos){.row = row, .col = col},
        cell);
  }
  return true;
}

void
VTerminal::scroll_delta(int delta)
{
    // Ensure you can't scroll past scrolling boundries
    // 0 >= scroll_offset <= sb_current
    if(delta > 0) {
        if(scroll_offset + delta > sb_current)
            delta = sb_current - scroll_offset;
    } else if(delta < 0) {
        if(delta < -scroll_offset)
            delta = -scroll_offset;
    } 
      
    scroll_offset += delta; 
}

void
VTerminal::scroll_get_delta(int &delta)
{
    delta = scroll_offset;
}

void
VTerminal::scroll_set_delta(int delta)
{
    scroll_offset = delta;
}

void
VTerminal::push_screen_to_scrollback()
{
    int height, width;
    vterm_get_size(vt, &height, &width);

    // Take each row that has some content and push to scrollback buffer
    VTermPos pos;
    for (pos.row = 0; pos.row < height; ++pos.row) {
        VTermScreenCell cells[width];
        for (pos.col = 0; pos.col < width; ++pos.col) {
            fetch_cell(pos.row, pos.col, &cells[pos.col]);
        }
        sb_pushline(width, cells);

        if (pos.row == cursorpos.row) {
            break;
        }
    }
}



VTerminal *
vterminal_new(VTerminalOptions options)
{
    return new VTerminal(options);
}

void
vterminal_free(VTerminal *terminal)
{
    delete terminal;
}

void vterminal_write(VTerminal *terminal, const char *data, size_t len)
{
    terminal->write(data, len);
}

void vterminal_get_height_width(VTerminal *terminal, int &height, int &width)
{
    vterm_get_size(terminal->vt, &height, &width);
}

void vterminal_fetch_row(VTerminal *terminal, int row,
    int start_col, int end_col, std::string &utf8text)
{
    int attr;
    int width;
    terminal->fetch_row(row, start_col, end_col, attr, width);
    if (terminal->textbuf) {
        utf8text = terminal->textbuf;
    } else {
        utf8text = "";
    }
}

void vterminal_fetch_row_col(VTerminal *terminal, int row,
        int col, std::string &utf8text, int &attr, int &width)
{
    terminal->fetch_row(row, col, col+1, attr, width);
    if (terminal->textbuf) {
        utf8text = terminal->textbuf;
    } else {
        utf8text = "";
    }
}

void vterminal_get_cursor_pos(VTerminal *terminal, int &row, int &col)
{
    row = terminal->cursorpos.row;
    col = terminal->cursorpos.col;
}

void vterminal_scrollback_num_rows(VTerminal *terminal, int &num)
{
    num = terminal->sb_current;
}

void vterminal_scroll_delta(VTerminal *terminal, int delta)
{
    terminal->scroll_delta(delta);
}

void vterminal_scroll_get_delta(VTerminal *terminal, int &delta)
{
    terminal->scroll_get_delta(delta);
}

void vterminal_scroll_set_delta(VTerminal *terminal, int delta)
{
    terminal->scroll_set_delta(delta);
}

void vterminal_push_screen_to_scrollback(VTerminal *terminal)
{
    terminal->push_screen_to_scrollback();
}

// libvterm callbacks {{{

static int vterminal_damage(VTermRect rect, void *data)
{
    return 1;
}

static int vterminal_moverect(VTermRect dest, VTermRect src, void *data)
{
    return 1;
}

static int vterminal_movecursor(VTermPos newp, VTermPos oldp, int visible,
    void *data)
{
    VTerminal *terminal = (VTerminal*)data;
    terminal->movecursor(newp, oldp, visible);
    return 1;
}

static int vterminal_settermprop(VTermProp prop, VTermValue *val, void *data)
{
    VTerminal *terminal = (VTerminal*)data;
    return terminal->settermprop(prop, val);
}

static int vterminal_bell(void *data)
{
    VTerminal *terminal = (VTerminal*)data;
    terminal->bell();
    return 1;
}

static int vterminal_resize(int rows, int cols, void *data)
{
    return 1;
}

static int
vterminal_sb_pushline(int cols, const VTermScreenCell *cells, void *data)
{
    VTerminal *terminal = (VTerminal*)data;
    return terminal->sb_pushline(cols, cells);
}

static int
vterminal_sb_popline(int cols, VTermScreenCell *cells, void *data)
{
    VTerminal *terminal = (VTerminal*)data;
    return terminal->sb_popline(cols, cells);
}

// }}}

