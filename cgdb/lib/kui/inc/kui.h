#ifndef __KUI_H__
#define __KUI_H__

/* includes {{{ */
#include "std_list.h"
/* }}} */

/* Doxygen headers {{{ */
/*! 
 * \file
 * kui.h
 *
 * \brief
 * This interface is intended to be the abstraction layer between an
 * application wanting key input from a user and the user themselves.
 */
/* }}} */

/* struct kui_map {{{ */
/******************************************************************************/
/**
 * @name Creating and Destroying a kui_map.
 * These functions are for createing and destroying a kui map.
 *
 * A kui map is basically a key value pair. As far as the map is concerned both
 * the key and the value are of type 'char*'. This is because, the user needs to
 * type the map in at the keyboard. This may seem obvious at first, but things 
 * like ESCAPE and HOME have to be typed in also.
 */
/******************************************************************************/

//@{

struct kui_map;

/**
 * Create a kui map.
 *
 * \param key_data
 * The map's key data
 *
 * \param value_data
 * The map's value data
 *
 * @return
 * A new instance on success, or NULL on error. 
 */
struct kui_map *kui_map_create (
		const char *key_data, 
		const char *value_data );

/**
 * Destroy a kui map.
 *
 * \param map
 * The map to destroy.
 *
 * @return
 * 0 on success, -1 on error
 */
int kui_map_destroy ( struct kui_map *map );

//@}

/******************************************************************************/
/**
 * @name General operations on a kui map.
 * These function's are the basic functions used to operate on a kui map context
 */
/******************************************************************************/

//@{

/**
 * Get's the maps key.
 *
 * \param map
 * A map
 *
 * \param key
 * Get's the key as a null terminated string.
 * The key should not be modified upon return. Also, it is invalid if the map
 * is destroyed.
 *
 * @return
 * 0 on success, -1 on error
 */
int kui_map_get_key ( struct kui_map *map, char **key );

/**
 * Get's the maps value.
 *
 * \param map
 * A map
 *
 * \param value
 * Get's the value as a null terminated string.
 * The value should not be modified upon return. Also, it is invalid if the map
 * is destroyed.
 *
 * @return
 * 0 on success, -1 on error
 */
int kui_map_get_value ( struct kui_map *map, char **value );

/**
 * Used for debugging.
 * Print's the translated value.
 *
 * \param map
 * The map context
 *
 * @return
 * 0 on success, -1 on error
 */
int kui_map_print_cgdb_key_array ( struct kui_map *map );

//@}

/* }}} */

/* struct kui_map_set {{{ */

/******************************************************************************/
/**
 * @name Creating and Destroying a kui_map_set.
 * These functions are for createing and destroying a kui map set.
 *
 * A kui map set is simply a grouping of maps that the client associated 
 * together. At this level, a map is defined as a key/value pair. The key and 
 * the value are simply character arrays.
 *
 * A Kui map set is an easy way to swap out what the application wishes
 * to have libkui look for when the user is typing input. For instance,
 * if the application wants libkui to look for certain sequences when the 
 * user is doing task A, and look for other sequences the user is doing task
 * B, the application coud simply create 2 kui map sets and swap them 
 * out using TODO.
 */
/******************************************************************************/

//@{

struct kui_map_set;

/**
 * Create a kui map set.
 *
 * @return
 * A new instance on success, or NULL on error. 
 */
struct kui_map_set *kui_ms_create ( void );

/**
 * Destroys a kui map set.
 *
 * \param kui_ms
 * The kui map set to destroy.
 *
 * @return
 * 0 on success or -1 on error
 */
int kui_ms_destroy ( struct kui_map_set *kui_ms );

//@}

/******************************************************************************/
/**
 * @name General operations on a kui map set context.
 * These function's are the basic functions used to operate on a kui map 
 * set context
 */
/******************************************************************************/

//@{

/**
 * Add a map to the map set.
 *
 * \param kui_ms
 * The kui map set to add to.
 *
 * \param key
 * A key. Should be null terminated.
 *
 * \param value
 * A value. Should be null terminated.
 *
 * @return
 * 0 on success, or -1 on error
 */ 
int kui_ms_register_map ( 
		struct kui_map_set *kui_ms,
		const char *key, 
		const char *value );

/**
 * Remove a map from the map set.
 *
 * \param kui_ms
 * The kui map set to add to.
 *
 * \param key
 * A key. Should be null terminated.
 *
 * @return
 * 0 on success, 
 * or -1 on error 
 * or -2 if map did not exist.
 */
int kui_ms_deregister_map (
		struct kui_map_set *kui_ms,
		const char *key );

/**
 * Get's a list of kui_map's. This way, someone can iterate through
 * the list.
 *
 * \param kui_ms
 * A kui map set.
 *
 * @return
 * The list of maps, or NULL on error.
 * If there are no maps, of course the empty list will be returned.
 */
std_list kui_ms_get_maps ( struct kui_map_set *kui_ms );

//@}

/* }}} */

/* struct kuictx {{{ */

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
 * The callback function that libkui will call when it needs a character 
 * to be read.
 *
 * \param fd
 * The descriptor to read in from. ( if needed )
 *
 * \param ms
 * The The amount of time in milliseconds to wait for input.
 * Pass 0, if you do not want to wait.
 *
 * \param obj
 * A piece of state data to pass along
 *
 * Must return 
 * -1 on error.
 *  0 if no data is ready.
 *  the char read.
 */
typedef int (*kui_getkey_callback)( 
		const int fd, 
		const unsigned int ms,
		const void *obj );

/**
 * Initializes the Key User Interface unit
 *
 * \param stdinfd 
 * The descriptor to read from when looking for the next char
 *
 * \param callback
 * The function that libkui calls to have 1 char read.
 *
 * \param ms
 * The number of milliseconds that this context should block while 
 * attempting to match a mapping by waiting for the user to type
 * the next key.
 *
 * \param state_data
 * This is a piece of data that is not looked at by this context. It
 * is passed to the callback.
 *
 * @return
 * A new instance on success, or NULL on error. 
 */
struct kuictx *kui_create(
		int stdinfd, 
		kui_getkey_callback callback,
		int ms,
	    void *state_data	);

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
 * @name General operations on a kui context.
 * These function's are the basic functions used to operate on a kui context
 */
/******************************************************************************/

//@{

/**
 * Get's the current map set for the kui context.
 *
 * \param kctx
 * The kui context to get the sequence set of
 *
 * @return
 * The list of map sets, or NULL on error.
 * If there are no map sets, of course the empty list will be returned.
 */
std_list kui_get_map_sets ( struct kuictx *kctx );

/**
 * Add's a kui map set to the kui context.
 *
 * \param kctx
 * The kui context to add the map set of
 *
 * \param kui_ms
 * The new kui map set to use.
 *
 * @return
 * 0 on success, or -1 on error.
 */
int kui_add_map_set ( 
		struct kuictx *kctx, 
		struct kui_map_set *kui_ms );

/**
 * Determine's if libkui has data ready to read. It has already been
 * read by the file descriptor, or it was buffered through some other
 * means.
 *
 * \param kctx
 * The kui context.
 *
 * @return
 * -1 on error, otherwise 1 if can get a key, or 0 if nothing available.
 */
int kui_cangetkey ( struct kuictx *kctx );

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

/* }}} */

/* struct kui_managaer {{{ */

/******************************************************************************/
/**
 * @name Creating and Destroying a kui manager.
 * These functions are for createing and destroying a 
 * "Key User Interface" manager
 *
 * This is capable of reading in any type of single/multibyte sequence and 
 * abstracting the details from any higher level. 
 */
/******************************************************************************/

//@{

struct kui_manager;

/**
 * Initializes the Key User Interface manager
 *
 * \param stdinfd 
 * The descriptor to read from when looking for the next char
 *
 * @return
 * A new instance on success, or NULL on error. 
 */
struct kui_manager *kui_manager_create(int stdinfd );

/**
 * Destroys a kui context
 *
 * \param kuim
 * The kui manager
 *
 * @return
 * 0 on success, -1 on error
 */
int kui_manager_destroy ( struct kui_manager *kuim );


//@}

/******************************************************************************/
/**
 * @name General operations on a kui manager context.
 * These function's are the basic functions used to operate on a kui context
 */
/******************************************************************************/

//@{

/**
 * Get's the current map set for the kui context.
 *
 * \param kuim
 * The kui manager context to get the sequence set of
 *
 * @return
 * The list of map sets, or NULL on error.
 * If there are no map sets, of course the empty list will be returned.
 */
std_list kui_manager_get_map_sets ( struct kui_manager *kuim );

/**
 * Add's a kui map set to the kui context.
 *
 * \param kuim
 * The kui context to add the map set of
 *
 * \param kui_ms
 * The new kui map set to use.
 *
 * @return
 * 0 on success, or -1 on error.
 */
int kui_manager_add_map_set ( 
		struct kui_manager *kuim, 
		struct kui_map_set *kui_ms );

/**
 * Determine's if libkui has data ready to read. It has already been
 * read by the file descriptor, or it was buffered through some other
 * means.
 *
 * \param kuim
 * The kui context.
 *
 * @return
 * -1 on error, otherwise 1 if can get a key, or 0 if nothing available.
 */
int kui_manager_cangetkey ( struct kui_manager *kuim );

/**
 * Get's the next key for the application to process.
 *
 * \param kuim
 * The kui context.
 *
 * @return
 * -1 on error, otherwise, a valid key.
 *  A key can either be a normal ascii key, or a CGDB_KEY_* value.
 */

int kui_manager_getkey ( struct kui_manager *kuim );

//@}

/* }}} */

#endif /* __KUI_H__ */
