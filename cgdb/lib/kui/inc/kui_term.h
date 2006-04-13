#ifndef __INPUT_H__
#define __INPUT_H__

/* includes {{{*/

#include "kui.h"

/* }}}*/

/* Doxygen headers {{{ */
/*! 
 * \file
 * kui_term.h
 *
 * \brief
 * This interface is intended to provide a key interface to an application. It 
 * represents unprintable char's and terminal escape sequences via an enumeration.
 * This enumeration has convertion functions capable of changing an enum into
 * an ascii representation and back.
 */
/* }}} */

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
 * <EOL>			end-of-line
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

    /* Alt Letters */
    CGDB_KEY_ALT_A,
    CGDB_KEY_ALT_B,
    CGDB_KEY_ALT_C,
    CGDB_KEY_ALT_D,
    CGDB_KEY_ALT_E,
    CGDB_KEY_ALT_F,
    CGDB_KEY_ALT_G,
    CGDB_KEY_ALT_H,
    CGDB_KEY_ALT_I,
    CGDB_KEY_ALT_J,
    CGDB_KEY_ALT_K,
    CGDB_KEY_ALT_L,
    CGDB_KEY_ALT_M,
    CGDB_KEY_ALT_N,
    CGDB_KEY_ALT_O,
    CGDB_KEY_ALT_P,
    CGDB_KEY_ALT_Q,
    CGDB_KEY_ALT_R,
    CGDB_KEY_ALT_S,
    CGDB_KEY_ALT_T,
    CGDB_KEY_ALT_U,
    CGDB_KEY_ALT_V,
    CGDB_KEY_ALT_W,
    CGDB_KEY_ALT_X,
    CGDB_KEY_ALT_Y,
    CGDB_KEY_ALT_Z,

    /* Alt Shifted Letters */
    CGDB_KEY_ALT_SHIFT_A,
    CGDB_KEY_ALT_SHIFT_B,
    CGDB_KEY_ALT_SHIFT_C,
    CGDB_KEY_ALT_SHIFT_D,
    CGDB_KEY_ALT_SHIFT_E,
    CGDB_KEY_ALT_SHIFT_F,
    CGDB_KEY_ALT_SHIFT_G,
    CGDB_KEY_ALT_SHIFT_H,
    CGDB_KEY_ALT_SHIFT_I,
    CGDB_KEY_ALT_SHIFT_J,
    CGDB_KEY_ALT_SHIFT_K,
    CGDB_KEY_ALT_SHIFT_L,
    CGDB_KEY_ALT_SHIFT_M,
    CGDB_KEY_ALT_SHIFT_N,
    CGDB_KEY_ALT_SHIFT_O,
    CGDB_KEY_ALT_SHIFT_P,
    CGDB_KEY_ALT_SHIFT_Q,
    CGDB_KEY_ALT_SHIFT_R,
    CGDB_KEY_ALT_SHIFT_S,
    CGDB_KEY_ALT_SHIFT_T,
    CGDB_KEY_ALT_SHIFT_U,
    CGDB_KEY_ALT_SHIFT_V,
    CGDB_KEY_ALT_SHIFT_W,
    CGDB_KEY_ALT_SHIFT_X,
    CGDB_KEY_ALT_SHIFT_Y,
    CGDB_KEY_ALT_SHIFT_Z,

    /* Alt Numbers */
    CGDB_KEY_ALT_1,
    CGDB_KEY_ALT_2,
    CGDB_KEY_ALT_3,
    CGDB_KEY_ALT_4,
    CGDB_KEY_ALT_5,
    CGDB_KEY_ALT_6,
    CGDB_KEY_ALT_7,
    CGDB_KEY_ALT_8,
    CGDB_KEY_ALT_9,
    CGDB_KEY_ALT_0,

    /* Alt Shifted Numbers */
    CGDB_KEY_ALT_SHIFT_1,
    CGDB_KEY_ALT_SHIFT_2,
    CGDB_KEY_ALT_SHIFT_3,
    CGDB_KEY_ALT_SHIFT_4,
    CGDB_KEY_ALT_SHIFT_5,
    CGDB_KEY_ALT_SHIFT_6,
    CGDB_KEY_ALT_SHIFT_7,
    CGDB_KEY_ALT_SHIFT_8,
    CGDB_KEY_ALT_SHIFT_9,
    CGDB_KEY_ALT_SHIFT_0,

    /* Alt Special */
    CGDB_KEY_ALT_MINUS,
    CGDB_KEY_ALT_EQUAL,
    CGDB_KEY_ALT_LEFT_BRACKET,
    CGDB_KEY_ALT_RIGHT_BRACKET,
    CGDB_KEY_ALT_BACKSLASH,
    CGDB_KEY_ALT_SEMICOLON,
    CGDB_KEY_ALT_APOSTROPHE,
    CGDB_KEY_ALT_COMMA,
    CGDB_KEY_ALT_PERIOD,
    CGDB_KEY_ALT_DIVIDE,
    CGDB_KEY_ALT_ACCENT_MARK,

    /* Alt Shift Special */
    CGDB_KEY_ALT_SHIFT_UNDERSCORE,
    CGDB_KEY_ALT_SHIFT_PLUS,
    CGDB_KEY_ALT_SHIFT_LEFT_CURLY_BRACKET,
    CGDB_KEY_ALT_SHIFT_RIGHT_CURLY_BRACKET,
    CGDB_KEY_ALT_SHIFT_PIPE,
    CGDB_KEY_ALT_SHIFT_COLON,
    CGDB_KEY_ALT_SHIFT_QUOTE,
    CGDB_KEY_ALT_SHIFT_LESS_THAN,
    CGDB_KEY_ALT_SHIFT_GREATER_THAN,
    CGDB_KEY_ALT_SHIFT_QUESTION_MARK,
    CGDB_KEY_ALT_SHIFT_TILDA,
     
    CGDB_KEY_ERROR
};

/* }}} */

/**
 * A new kui_map_set get's created each time this function is returned.
 *
 * This basically abstracts away the keyboard. A map will be created for
 * each abstract key the user has on there keyboard (HOME,END,PGUP,ESC).
 *
 * @return
 * The new kui_map, or NULL on error.
 */
struct kui_map_set *kui_term_get_terminal_mappings ( void );

/**
 * Determine if a key is a cgdb_key. This is true if the key passed in is in
 * the bounds of the cgdb_key enumeration.
 *
 * \param key
 * The key to check
 *
 * @return
 * 1 if the key is a cgdb_key, otherwise 0
 *
 */
int kui_term_is_cgdb_key ( int key );

/**
 * Get's the cgdb_key from the keycode.
 *
 * \param keycode
 * The code to get the corresponding cgdb_key from
 *
 * @return
 * The new key, or -1 on error.
 * CGDB_KEY_ERROR is returned if no cgdb_key matches the keycode.
 */
int kui_term_get_cgdb_key_from_keycode ( const char *keycode );

/**
 * Get's the key in string form.
 *
 * \param key
 * The key to get in string form.
 *
 * @return
 * The string form. or NULL on error.
 * if CGDB_KEY_ERROR is passed in, "CGDB_KEY_ERROR" will be returned.
 */
const char *kui_term_get_string_from_cgdb_key ( int key );

/**
 * Get's the keycode associated with this key.
 *
 * \param key
 * The key to get the keycode of.
 *
 * @return
 * The keycode. or NULL on error.
 * if CGDB_KEY_ERROR is passed in, "CGDB_KEY_ERROR" will be returned.
 */
const char *kui_term_get_keycode_from_cgdb_key ( int key );

/*
 * Parses the original buffer, and if successful
 * will create an output buffer. The output buffer is allocated for you.
 * When you are done with it, you should free it.
 *
 * Basically, this function translates strings like
 * 'ab<esc><Home>de<PageUp>' into an int array containing
 *
 * a
 * b
 * CGDB_KEY_ESC
 * CGDB_KEY_HOME
 * d
 * e
 * CGDB_KEY_PPAGE
 * NULL
 *
 * \param string
 * The value to translate
 *
 * \param literal
 * An outbound parameter, which is a null-terminated
 * int pointer, containing all valid int's.
 *
 * @return
 * 0 on success, or -1 on error.
 */
int kui_term_string_to_cgdb_key_array (
		const char *string,
		int **cgdb_key_array );

/**
 * Print's a cgdb_key_array in human readable form. This is mostly used for
 * debugging purposes.
 *
 * \param cgdb_key_array
 * The key array to print.
 *
 * @return
 * 0 on success, or -1 on error.
 */
int kui_term_print_cgdb_key_array ( int *cgdb_key_array );

#endif /* __INPUT_H__ */
