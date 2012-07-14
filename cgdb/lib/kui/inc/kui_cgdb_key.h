#ifndef __KUI_CGDB_KEY_H__
#define __KUI_CGDB_KEY_H__

/* enum cgdb_key {{{ */

/**
 * The high level keys.
 *
 * This represents key's that can not be represented in 1 ascii key.
 */

/**
 * notation        	meaning
 * --------         -------
 *
 * <Nul>			zero
 * <BS>				backspace
 * <Tab>			tab
 * <NL>				linefeed
 * <FF>				formfeed
 * <CR>				carriage return
 * <Return>			same as <CR>
 * <Enter>			same as <CR>
 * <Esc>			escape
 * <Space>			space
 * <lt>				less-than
 * <Bslash>			backslash
 * <Bar>			vertical bar
 * <Del>			delete
 * <Up>				cursor-up
 * <Down>			cursor-down
 * <Left>			cursor-left
 * <Right>			cursor-right
 * <S-Up>			shift-cursor-up
 * <S-Down>			shift-cursor-down
 * <S-Left>			shift-cursor-left
 * <S-Right>		shift-cursor-right
 * <C-Left>			control-cursor-left
 * <C-Right>		control-cursor-right
 * <F1> - <F12>		function keys 1 to 12
 * <Help>			help key
 * <Undo>			undo key
 * <Insert>			insert key
 * <Home>			home
 * <End>			end
 * <PageUp>			page-up
 * <PageDown>		page-down
 * <S-...>			shift-key
 * <C-...>			control-key
 * <A-...>			alt-key
 */

enum cgdb_key {
    CGDB_KEY_ESC = 10000,
    CGDB_KEY_UP,
    CGDB_KEY_DOWN,
    CGDB_KEY_LEFT,
    CGDB_KEY_RIGHT,
    CGDB_KEY_HOME,
    CGDB_KEY_END,
    CGDB_KEY_PPAGE,
    CGDB_KEY_NPAGE,
    CGDB_KEY_DC,
    CGDB_KEY_IC,

    /* Function Keys */
    CGDB_KEY_F1,
    CGDB_KEY_F2,
    CGDB_KEY_F3,
    CGDB_KEY_F4,
    CGDB_KEY_F5,
    CGDB_KEY_F6,
    CGDB_KEY_F7,
    CGDB_KEY_F8,
    CGDB_KEY_F9,
    CGDB_KEY_F10,
    CGDB_KEY_F11,
    CGDB_KEY_F12,

    /* Ctrl Keys */
    CGDB_KEY_CTRL_A,
    CGDB_KEY_CTRL_B,
    CGDB_KEY_CTRL_C,
    CGDB_KEY_CTRL_D,
    CGDB_KEY_CTRL_E,
    CGDB_KEY_CTRL_F,
    CGDB_KEY_CTRL_G,
    CGDB_KEY_CTRL_H,
    CGDB_KEY_CTRL_I,
    CGDB_KEY_CTRL_J,
    CGDB_KEY_CTRL_K,
    CGDB_KEY_CTRL_L,
    CGDB_KEY_CTRL_M,
    CGDB_KEY_CTRL_N,
    CGDB_KEY_CTRL_O,
    CGDB_KEY_CTRL_P,
    CGDB_KEY_CTRL_Q,
    CGDB_KEY_CTRL_R,
    CGDB_KEY_CTRL_S,
    CGDB_KEY_CTRL_T,
    CGDB_KEY_CTRL_U,
    CGDB_KEY_CTRL_V,
    CGDB_KEY_CTRL_W,
    CGDB_KEY_CTRL_X,
    CGDB_KEY_CTRL_Y,
    CGDB_KEY_CTRL_Z,

    CGDB_KEY_ERROR
};

/* }}} */

#endif
