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

/******************************************************************************/
/**
 * @name Creating and Destroying a kui context.
 * These functions are for createing and destroying a 
 * "Key User Interface" Context
 *
 * This is capable of reading in any type of single/multibyte sequence and 
 * abstracting the details from any higher level. 
 */
/******************************************************************************/

//@{

struct kuictx;

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
 * @name Creating and Destroying a kui_sequence_set.
 * These functions are for createing and destroying a kui sequence set.
 *
 * A Kui sequence set is an easy way to swap out what the application wishes
 * to have libkui look for when the user is typing input. For instance,
 * if the application wants libkui to look for certain sequences when the 
 * user is doing task A, and look for other sequences the user is doing task
 * B, the application coud simply create 2 kui_sequence sets and swap them 
 * out using kui_set_current_sequence_set.
 *
 * The application can ask the kui_sequence_set to register a particular 
 * byte/multibyte sequence. When it does, if the kui_sequence_set allows the 
 * operation, then this kui sequence set uses the new sequence when calling
 * getkey.
 */
/******************************************************************************/

//@{

struct kui_sequence_set;

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
 * \param data
 * The substitution text associated with the sequence.
 *
 * \param data_size
 * The size of data
 *
 * @return
 * 0 on success, or -1 on error
 */ 
int kui_ss_register_sequence ( 
		struct kui_sequence_set *kui_ss,
		const int sequence[], 
		const int size,
		const int data[],
		const int data_size );

/**
 * Remove a sequence from the sequence set.
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
 * @return
 * 0 on success, or -1 on error
 */
int kui_ss_deregister_sequence (
		struct kui_sequence_set *kui_ss,
		const int sequence[],
	    const int size	);

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
 *
 * TODO: Change the parameter to be a std_list.
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
 *
 * TODO: Change the parameter to be a std_list.
 */
int kui_set_sequence_set ( 
		struct kuictx *kctx, 
		struct kui_sequence_set *kui_ss );

/**
 * Get's the next key for the application to process.
 *
 * \param kctx
 * The kui context.
 *
 * @return
 * -1 on error, otherwise, a valid key.
 *  A key can either be a normal ascii key, or a CGDB_KEY_* value.
 */

int kui_getkey ( struct kuictx *kctx );

//@}

#endif /* __KUI_H__ */
