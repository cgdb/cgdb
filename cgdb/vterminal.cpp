#include "vterminal.h"
#include "vterm.h"
#include "sys_util.h"
#include "scroller.h"

#include "sys_win.h"
#include "highlight_groups.h"

typedef struct {
    size_t cols;
    VTermScreenCell cells[];
} ScrollbackLine;

struct VTerminal
{
    VTerminal(VTerminalOptions options);
    ~VTerminal();

    void push_bytes(const char *bytes, size_t len);

    int damage(VTermRect rect);
    int moverect(VTermRect dest, VTermRect src);
    int movecursor(VTermPos newp, VTermPos oldp, int visible);
    int settermprop(VTermProp prop, VTermValue *val);
    int bell();
    int sb_pushline(int cols, const VTermScreenCell *cells);
    int sb_popline(int cols, VTermScreenCell *cells);

    // Convert VTermScreen cell arrays into utf8 strings
    // Currently it stores the string in textbuf, however, I suggest it may
    // be better to return a std::string
    // Also: Error handling?
    void fetch_row(int row, int start_col, int end_col, int &attr);
    // Fetch a single cell
    bool fetch_cell(int row, int col, VTermScreenCell *cell);

    void scroll_delta(int delta);
    void scroll_get_delta(int &delta);
    void scroll_set_delta(int delta);
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

    // The number of lines scrolled
    // Initialized to zero, no scroll offset
    // 
    int scroll_offset;
    // libvterm scrollback buffer data data
    ScrollbackLine **sb_buffer;  // Scrollback buffer storage
    size_t sb_current;           // number of rows pushed to sb_buffer
                                 // This does not include rows in vterm
                                 // currently
    size_t sb_size;              // sb_buffer size
    // "virtual index" that points to the first sb_buffer row that we need to
    // push to the terminal buffer when refreshing the scrollback. When
    // negative, it actually points to entries that are no longer in sb_buffer
    // (because the window height has increased) and must be deleted from the
    // terminal buffer
    int sb_pending;

    bool cursor_visible;
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
    sb_size = 10000; // TODO: Make a cgdb option
    sb_pending = 0;
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
VTerminal::push_bytes(const char *bytes, size_t len)
{
    vterm_input_write(vt, bytes, len);
    vterm_screen_flush_damage(vts);
}

void scr_add_buf(struct scroller *scr, const char *buf);
int
VTerminal::damage(VTermRect rect)
{
    clog_info(CLOG_CGDB, "vterm damage start_row(%d) end_row(%d)",
            rect.start_row, rect.end_row);

    VTermPos pos;
    scroller *scr = (scroller*)options.data;
    for (pos.row = rect.start_row; pos.row < rect.end_row; ++pos.row) {
        for (pos.col = rect.start_col; pos.col < rect.end_col; ++pos.col) {
            VTermScreenCell cell;
            vterm_screen_get_cell(vts, pos, &cell);

            int cursor_here = pos.row ==
                cursorpos.row && pos.col == cursorpos.col;
            int cursor_visible = false; // TODO: Determine from scroller when
            int draw_cursor = cursor_visible && cursor_here;

            // chpen(&cell, pt,
            //    draw_cursor &&
            //    pt->cursor_shape == VTERM_PROP_CURSORSHAPE_BLOCK);

            if(cell.chars[0] == 0) {
                // put_erase(pt, cell.width, pos);
                // clog_info(CLOG_CGDB, "vterm char empty");
            } else {
                // put_glyph(pt, cell.chars, cell.width, pos);
                // clog_info(CLOG_CGDB, "vterm char %c", cell.chars[0]);
            }

#if 0
            char utf8[6*VTERM_MAX_CHARS_PER_CELL]; size_t len = 0;
            if (cell.chars[0]) {
                for (int i = 0; cell.chars[i]; ++i) {
                    len += utf_char2bytes((int)cell.chars[i],
                                (uint8_t *)ptr + cell_len);
                }
            } else {
                *ptr = ' ';
                len = 1;
            }
            scr_add_buf(scr, &cell.chars[0]);
            scr_end(scr);
#endif

            if (draw_cursor) {
                // TODO: Draw down the road using ncurses
            }

            pos.col += cell.width;
        }
    }
   
    // invalidate_terminal(rect.start_row, rect.end_row);
    return 1;
}

int
VTerminal::moverect(VTermRect dest, VTermRect src)
{
    clog_info(CLOG_CGDB, "vterm moverect"
            " dest.start_row(%d) dest.end_row(%d)"
            " src.start_row(%d) src.end_row(%d)",
            dest.start_row, dest.end_row,
            src.start_row, src.end_row);
    // invalidate_terminal(data, MIN(dest.start_row, src.start_row),
    //      MAX(dest.end_row, src.end_row));
    return 1;
}

int
VTerminal::movecursor(VTermPos newp, VTermPos oldp, int visible)
{
    clog_info(CLOG_CGDB, "vterm movecursor"
            " new.row(%d) new.col(%d)"
            " old.row(%d) old.col(%d)",
            newp.row, newp.col,
            oldp.row, oldp.col);

    cursorpos.row = newp.row;
    cursorpos.col = newp.col;

    // invalidate_terminal(term, oldp.row, oldp.row + 1);
    // invalidate_terminal(term, newp.row, newp.row + 1);
    return 1;
}

int
VTerminal::settermprop(VTermProp prop, VTermValue *val)
{
    clog_info(CLOG_CGDB, "vterm settermprop");

    switch (prop) {
        case VTERM_PROP_ALTSCREEN:
            break;

        case VTERM_PROP_CURSORVISIBLE:
            cursor_visible = val->boolean;
            // invalidate_terminal(term, cursorpos.row, cursorpos.row + 1);
            break;

        case VTERM_PROP_TITLE: {
            // buf_T *buf = handle_get_buffer(term->buf_handle);
            // buf_set_term_title(buf, val->string);
            break;
        }

        case VTERM_PROP_MOUSE:
            //term->forward_mouse = (bool)val->number;
            break;

        default:
            return 0;
    }

    return 1;
}

int
VTerminal::bell()
{
    clog_info(CLOG_CGDB, "vterm bell");
    // ui_call_bell();
    return 1;
}

int
VTerminal::sb_pushline(int cols, const VTermScreenCell *cells)
{
    clog_info(CLOG_CGDB, "vterm sb_pushline cols(%d)", cols);

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

    if (sb_pending < (int)sb_size) {
        sb_pending++;
    }

    memcpy(sbrow->cells, cells, sizeof(cells[0]) * c);
    //pmap_put(ptr_t)(invalidated_terminals, term, NULL);

    return 1;
}

static inline unsigned int utf8_seqlen(long codepoint)
{
  if(codepoint < 0x0000080) return 1;
  if(codepoint < 0x0000800) return 2;
  if(codepoint < 0x0010000) return 3;
  if(codepoint < 0x0200000) return 4;
  if(codepoint < 0x4000000) return 5;
  return 6;
}   
    
/* Does NOT NUL-terminate the buffer */
static int fill_utf8(long codepoint, char *str)
{
  int nbytes = utf8_seqlen(codepoint);

  // This is easier done backwards
  int b = nbytes;
  while(b > 1) { 
    b--;
    str[b] = 0x80 | (codepoint & 0x3f);
    codepoint >>= 6;
  }   

  switch(nbytes) {
    case 1: str[0] =        (codepoint & 0x7f); break;
    case 2: str[0] = 0xc0 | (codepoint & 0x1f); break;
    case 3: str[0] = 0xe0 | (codepoint & 0x0f); break;
    case 4: str[0] = 0xf0 | (codepoint & 0x07); break;
    case 5: str[0] = 0xf8 | (codepoint & 0x03); break;
    case 6: str[0] = 0xfc | (codepoint & 0x01); break;
  }

  return nbytes;
} 



int
VTerminal::sb_popline(int cols, VTermScreenCell *cells)
{
    clog_info(CLOG_CGDB, "vterm sb_popline cols(%d)", cols);
    if (!sb_current) {
        return 0;
    }

    if (sb_pending) {
        sb_pending--;
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

    // pmap_put(ptr_t)(invalidated_terminals, term, NULL);

    return 1;
}

static int get_ncurses_color_index(VTermColor &color)
{
    int index = -1;
    if (VTERM_COLOR_IS_DEFAULT_FG(&color)) {
        index = -1;
    } else if (VTERM_COLOR_IS_DEFAULT_BG(&color)) {
        index = -1;
    } else if (VTERM_COLOR_IS_INDEXED(&color)) {
        if (color.indexed.idx >= 0 && color.indexed.idx < 16) {
            index = color.indexed.idx;
        }
    } else if (VTERM_COLOR_IS_RGB(&color)) {
        // ?
    }
    return index;
}

void
VTerminal::fetch_row(int row, int start_col, int end_col, int &attr)
{
  int fg_index, bg_index;
  int col = start_col;
  size_t line_len = 0;
  char *ptr = textbuf;

 row = row - scroll_offset;

  while (col < end_col) {
    VTermScreenCell cell;
    fetch_cell(row, col, &cell);
    
    // set fb and bg colors
    //vterm_screen_convert_color_to_rgb(vts, &cell.fg);
    //vterm_screen_convert_color_to_rgb(vts, &cell.bg);
    //hl_get_color_attr(
    //        cell.fg.rgb.red, cell.fg.rgb.green, cell.fg.rgb.blue,
    //        cell.bg.rgb.red, cell.bg.rgb.green, cell.bg.rgb.blue, attr);

    int fg_index = get_ncurses_color_index(cell.fg);
    int bg_index = get_ncurses_color_index(cell.bg);
    hl_get_color_attr_from_index(fg_index, bg_index, attr);

    // set attributes
    if (cell.attrs.bold) {
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
}

bool
VTerminal::fetch_cell(int row, int col, VTermScreenCell *cell)
{
  if (row < 0) {
    if(-row > sb_current) {
      // fprintf(stderr, "ARGH! Attempt to fetch scrollback beyond"
              // buffer at line %d\n", -pos.row);
      return false; // ?
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
    int attr;
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
vterminal_free(VTerminal *vterminal)
{
    delete vterminal;
}

void vterminal_push_bytes(VTerminal *terminal, const char *bytes, size_t len)
{
    terminal->push_bytes(bytes, len);
}

void vterminal_get_height_width(VTerminal *terminal, int &height, int &width)
{
    vterm_get_size(terminal->vt, &height, &width);
}

void vterminal_fetch_row(VTerminal *terminal, int row,
    int start_col, int end_col, std::string &utf8text)
{
    int attr;
    terminal->fetch_row(row, start_col, end_col, attr);
    if (terminal->textbuf) {
        utf8text = terminal->textbuf;
    } else {
        utf8text = "";
    }
}

void vterminal_fetch_row_col(VTerminal *terminal, int row,
        int col, std::string &utf8text, int &attr)
{
    terminal->fetch_row(row, col, col+1, attr);
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

void vterminal_get_sb_num_rows(VTerminal *terminal, int &num)
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
    VTerminal *terminal = (VTerminal*)data;
    return terminal->damage(rect);
}

static int vterminal_moverect(VTermRect dest, VTermRect src, void *data)
{
    VTerminal *terminal = (VTerminal*)data;
    return terminal->moverect(dest, src);
}

static int vterminal_movecursor(VTermPos newp, VTermPos oldp, int visible,
    void *data)
{
    VTerminal *terminal = (VTerminal*)data;
    return terminal->movecursor(newp, oldp, visible);
}

static int vterminal_settermprop(VTermProp prop, VTermValue *val, void *data)
{
    VTerminal *terminal = (VTerminal*)data;
    return terminal->settermprop(prop, val);
}

static int vterminal_bell(void *data)
{
    VTerminal *terminal = (VTerminal*)data;
    return terminal->bell();
}

static int vterminal_resize(int rows, int cols, void *data)
{
    return 1;
}

// Scrollback push handler (from pangoterm).
static int
vterminal_sb_pushline(int cols, const VTermScreenCell *cells, void *data)
{
    VTerminal *terminal = (VTerminal*)data;
    return terminal->sb_pushline(cols, cells);
}

/// Scrollback pop handler (from pangoterm).
///
/// @param cols
/// @param cells  VTerm state to update.
/// @param data   Terminal
static int
vterminal_sb_popline(int cols, VTermScreenCell *cells, void *data)
{
    VTerminal *terminal = (VTerminal*)data;
    return terminal->sb_popline(cols, cells);
}

// }}}

