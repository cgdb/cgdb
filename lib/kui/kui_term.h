#ifndef __INPUT_H__
#define __INPUT_H__

/* includes {{{*/

#include "kui_map_set.h"
#include "kui_cgdb_key.h"

/* }}}*/

/* Doxygen headers {{{ */
/*! 
 * \file
 * kui_term.h
 *
 * \brief
 * This interface is intended to provide a key interface to an application. It 
 * represents unprintable char's and terminal escape sequences via an enumeration.
 * This enumeration has conversion functions capable of changing an enum into
 * an ascii representation and back.
 */
/* }}} */

/**
 * Fill a kui_map_set with the terminal mappings.
 *
 * This basically abstracts away the keyboard. A map will be created for
 * each abstract key the user has on there keyboard (HOME,END,PGUP,ESC).
 *
 * \param ms The map set to fill 
 *
 * @return
 * True on succes, or false on error.
 */
bool kui_term_get_terminal_mappings(kui_map_set& ms);

/**
 * Determine if a key is a cgdb_key. This is true if the key passed in is in
 * the bounds of the cgdb_key enumeration.
 *
 * \param key
 * The key to check
 *
 * @return
 * true if the key is a cgdb_key, otherwise false
 *
 */
bool kui_term_is_cgdb_key(int key);

/**
 * This get's the ascii character representation of the key.
 *
 * \param key
 * The cgdb_key to get the ascii character sequence of.
 *
 * \return
 * The ascii character sequence on success. If key is not a cgdb_key
 * or if this function fails, NULL will return.
 */
const char *kui_term_get_ascii_char_sequence_from_key(int key);

/**
 * Get's the cgdb_key from the keycode.
 *
 * \param keycode
 * The code to get the corresponding key from
 *
 * @return
 * The new key, or -1 on error.
 */
int kui_term_get_cgdb_key_from_keycode(const char *keycode);

/**
 * Get's the key in string form.
 *
 * \param key
 * The key to get in string form.
 *
 * @return
 * The string form. or NULL on error.
 */
const char *kui_term_get_string_from_key(int key);

/**
 * Get's the keycode associated with this key.
 *
 * \param key
 * The key to get the keycode of.
 *
 * @return
 * The keycode. or NULL on error.
 */
const char *kui_term_get_keycode_from_cgdb_key(int key);

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
int kui_term_string_to_key_array(const char *string, int **key_array);

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
int kui_term_print_key_array(const int *key_array);

#endif /* __INPUT_H__ */
