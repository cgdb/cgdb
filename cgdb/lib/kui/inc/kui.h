#ifndef __KUI_H__
#define __KUI_H__

#include "input.h"

/*! 
 * \file
 * kui.h
 *
 * \brief
 * This interface is intended to be the abstraction layer between an
 * application wanting key input from a user and the user themselves.
 */

struct kuictx;

/******************************************************************************/
/**
 * @name Createing and Destroying a kui context.
 * These functions are for createing and destroying a 
 * "Key User Interface" Context
 *
 * This is capable of reading in any type of multibyte sequence and 
 * abstracting the details from any higher level. Of course it is also
 * capable of reading in any single byte sequence.
 *
 * Basically, instead of making an application read in a multibyte sequence, 
 * the application can register a multibyte sequence with libkui and libkui 
 * will assign it a kui_sequence_key. This way, when the application wants 
 * input, it can just ask libkui for input. libkui can return to the 
 * application either an actual key that was typed, or a kui_sequence_key.
 *
 * If the application get's back a kui_sequence_key, it can ask libkui what
 * the sequence was that made this key get returned. This is primarily
 * used for debugging purposes.
 *
 * Note, this library is not used for reading in terminal specific escape 
 * codes. Please do not confuse the term multibyte sequence with something
 * related to the TERM environment variable or termcap/terminfo.
 * All of those details are handled at a lower level (libinput).
 */
/******************************************************************************/

//@{

/**
 * Initializes the Key User Interface unit
 *
 * \param stdinfd 
 * The descriptor to read from when looking for the next char
 *
 * @return
 * A new instance on success, or NULL on error. 
 */
struct kuictx *kui_create(int stdinfd);

/**
 * Destroys a kui context
 *
 * \param kctx
 * The kui context
 *
 * @return
 * 0 on success, -1 on error
 */
int kui_destroy ( struct kuictx *kctx );

//@}

/******************************************************************************/
/**
 * @name Createing and Destroying a kui_sequence_set.
 * These functions are for createing and destroying a kui sequence set.
 *
 * A Kui sequence set is an easy way to swap out what the application wishes
 * to have libkui look for when the user is typeing input. For instance,
 * if the application wants libkui to look for certain sequences when the 
 * user is doing task A, and look for other sequences the user is doing task
 * B, the application coud simply create 2 kui_sequence sets and swap them 
 * out using kui_set_current_sequence_set.
 *
 * The application can ask the kui_sequence_set to register a particular 
 * byte/multibyte sequence. When it does, if the kui_sequence_set allows the 
 * operation, then the application will recieve a key the relates to the 
 * particular sequence requested.
 */
/******************************************************************************/

//@{

struct kui_sequence_set;
typedef int kui_sequence_key;

/**
 * Create a kui sequence set.
 *
 * @return
 * A new instance on success, or NULL on error. 
 */
struct kui_sequence_set *kui_ss_create ( void );

/**
 * Destroys a kui sequence set.
 *
 * \param kui_ss
 * The kui sequence set to destroy.
 *
 * @return
 * 0 on success or -1 on error
 */
int kui_ss_destroy ( struct kui_sequence_set *kui_ss );

/**
 * Add a sequence to the sequence set.
 *
 * \param kui_ss
 * The kui sequence set to add to.
 *
 * \param sequence
 * The sequence of bytes that should be read in.
 *
 * \param size
 * The size of sequence
 *
 * \param key
 * The new key associated with this key sequence
 *
 * @return
 * 0 on success, or -1 on error
 */ 
int kui_ss_register_sequence ( 
		struct kui_sequence_set *kui_ss,
		const int sequence[], 
		const int size,
	    kui_sequence_key *key );

/**
 * Remove a sequence from the sequence set.
 *
 * \param kui_ss
 * The kui sequence set to add to.
 *
 * \param key
 * The sequence to remove
 *
 * @return
 * 0 on success, or -1 on error
 */
int kui_ss_deregister_sequence (
		struct kui_sequence_set *kui_ss,
		kui_sequence_key *key );

//@}

/******************************************************************************/
/**
 * @name General operations on a kui context.
 * These function's are the basic functions used to operate on a kui context
 */
/******************************************************************************/

//@{

/**
 * Get's the current sequence set for the kui context.
 *
 * \param kctx
 * The kui context to get the sequence set of
 *
 * @return
 * A new instance on success, or NULL on error. 
 */
struct kui_sequence_set *kui_get_sequence_set ( struct kuictx *kctx );

/**
 * Set's the current kui context to use the kui_ss set.
 *
 * \param kctx
 * The kui context to set the sequence set of
 *
 * \param kui_ss
 * The new kui sequence set to use.
 *
 * @return
 * 0 on success, or -1 on error.
 */
int kui_set_sequence_set ( 
		struct kuictx *kctx, 
		struct kui_sequence_set *kui_ss );

/**
 * Read's a registered key sequence.
 *
 * This is capable of reading in either 
 * 1. a sequence that has been registered.
 * 2. a single key
 * 3. a single special key
 *
 * Currently, since I don't know how this interface should look the
 * return value of this function is,
 *
 * -1 on error.
 *  	Strictly an error.
 * 0 on success and key is a valid sequence read
 * 		Strictly a sequence.
 * 1 on success and input_key is a valid libinput key read.
 * 		Strictly a key read from libinput
 */
int kui_getkey ( 
		struct kuictx *kctx,
	    kui_sequence_key *key,
	    int *input_key	);

//@}

#endif /* __KUI_H__ */
