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
 * This get's the ascii character representation of the key.
 *
 * \param key
 * The cgdb_key to get the ascii character sequence of.
 *
 * \return
 * The ascii charachter sequence on success. If key is not a cgdb_key
 * or if this function fails, NULL will return.
 */
char *kui_term_get_ascii_char_sequence_from_key (int key);

/**
 * Get's the cgdb_key from the keycode.
 *
 * \param keycode
 * The code to get the corresponding key from
 *
 * @return
 * The new key, or -1 on error.
 * CGDB_KEY_ERROR is returned if no key matches the keycode.
 */
int kui_term_get_cgdb_key_from_keycode (const char *keycode);

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
const char *kui_term_get_string_from_key (int key);

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
int kui_term_string_to_key_array (const char *string, int **key_array);

/**
 * Print's a key_array in human readable form. This is mostly used for
 * debugging purposes.
 *
 * \param key_array
 * The key array to print.
 *
 * @return
 * 0 on success, or -1 on error.
 */
int kui_term_print_key_array (int *key_array);

#endif /* __INPUT_H__ */
