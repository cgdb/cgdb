#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "kui.h"
#include "error.h"
#include "sys_util.h"
#include "io.h"
#include "kui_term.h"

/**
 * A kui map structure.
 *
 * This is simply a key/value pair as far as the outside
 * world is concerned. 
 */
struct kui_map {

	/**
	 * The key is the key strokes the user must type for this map
	 * to work. However, there can be sequences of char's encoded in this 
	 * string that represent another value. For example <ESC> would 
	 * represent CGDB_KEY_ESC.
	 */
	char *original_key;

	/**
	 * This is the literal list of keys the user must type for this map to
	 * be activated. It is a list of int's because it can contain things like
	 * CGDB_KEY_ESC, ..
	 */
	int *literal_key;

	/**
	 * The value is the substitution data, if the key is typed.
	 */
	char *original_value;

	/**
	 *
	 */
	int *literal_value;
};

struct kui_map *kui_map_create (
		const char *key_data, 
		const char *value_data ) {

	struct kui_map *map;
	char *key, *value;

	if ( !key_data || !value_data )
		return NULL;

	key = strdup ( key_data );

	if ( !key )
		return NULL;
	
	value = strdup ( value_data );

	if ( !value ) {
		free ( key );
		key = NULL;
		return NULL;
	}

	map = (struct kui_map *)malloc ( sizeof ( struct kui_map ) );

	if ( !map ) {
		free ( key );
		key = NULL;
		free ( value );
		value = NULL;
		return NULL;
	}

	map->original_key = key;
	map->original_value = value;

	if ( kui_term_string_to_cgdb_key_array ( map->original_key, &map->literal_key ) == -1 )
		return NULL;

	if ( kui_term_string_to_cgdb_key_array ( map->original_value, &map->literal_value ) == -1 )
		return NULL;
	
	return map;
}

int kui_map_destroy ( struct kui_map *map ) {

	if ( !map)
		return -1;

	free ( map->original_key );
	map->original_key = NULL;

	free ( map->original_value );
	map->original_value = NULL;

	free (map);
	map = NULL;

	return 0;
}

static int kui_map_destroy_callback ( void *param ) {
	struct kui_map *map = (struct kui_map *)param;
	return kui_map_destroy ( map );
}

int kui_map_get_key ( struct kui_map *map, char **key ) { 

	if ( !map )
		return -1;

	*key = map->original_key;

	return 0;
}

int kui_map_get_value ( struct kui_map *map, char **value ) {

	if ( !map )
		return -1;

	*value = map->original_value;

	return 0;
}

int kui_map_print_cgdb_key_array ( struct kui_map *map ) {
	if ( !map )
		return -1;

	if ( kui_term_print_cgdb_key_array ( map->literal_value ) == -1 )
		return -1;

	return 0;

}

/* Kui map set */

enum kui_map_state {
	KUI_MAP_FOUND,
	KUI_MAP_STILL_LOOKING,
	KUI_MAP_NOT_FOUND,
	KUI_MAP_ERROR
};

/** 
 * This maintains a list of maps.
 * Basically, a key/value pair list.
 */
struct kui_map_set {
	/**
	 * The list of maps available.
	 */
	std_list map_list;

	/**
	 * The iterator, pointing to the current list item matched.
	 */
	std_list_iterator map_iter;

	/**
	 * The state of this current map_set.
	 */
	enum kui_map_state map_state;

	/**
	 * If a map was found at any point, this flag get's set to 1.
	 * Otherwise it is 0.
	 * This helps solve the case where you have
	 * map a d
	 * map abc d
	 *
	 * Now, if 'a' is typed, the map_state is set to KUI_MAP_FOUND, however,
	 * if the user keeps typeing, say maybe 'b', then the map_state will be
	 * KUI_MAP_STILL_LOOKING because it is trying to complete the longer 
	 * mapping. If this is the case, is_found should be 1, to tell the outside
	 * world the a mapping was found.
	 */
	int is_found;

	/**
	 * An iterator pointing to the found mapping. Read above.
	 */
	std_list_iterator map_iter_found;
};

struct kui_map_set *kui_ms_create ( void ) {

	struct kui_map_set *map;

	map = (struct kui_map_set *)malloc(sizeof(struct kui_map_set));

	if ( !map )
		return NULL;

	map->map_list = std_list_create ( kui_map_destroy_callback );

	if ( !map->map_list ) {
		free ( map );
		map = NULL;
		return NULL;
	}

	return map;
}

int kui_ms_destroy ( struct kui_map_set *kui_ms ) {
	if ( !kui_ms )
		return -1;

	if ( std_list_destroy ( kui_ms->map_list ) == -1 )
		return -1;

	kui_ms->map_list = NULL;

	free (kui_ms);
	kui_ms = NULL;

	return 0;
}

static int kui_map_compare_callback ( 
		const void *a,
		const void *b ) {
	struct kui_map *one = (struct kui_map *)a;
	struct kui_map *two = (struct kui_map *)b;

	return strcasecmp ( one->original_key, two->original_key );
}

int kui_ms_register_map ( 
		struct kui_map_set *kui_ms,
		const char *key_data, 
		const char *value_data ) {
	struct kui_map *map;
	std_list_iterator iter;

	if ( !kui_ms )
		return -1;

	map = kui_map_create ( key_data, value_data );

	if ( !map )
		return -1;

	/* Find the old map */
	if ( !kui_ms->map_list )
		return -1;

	iter = std_list_find ( kui_ms->map_list, map, kui_map_compare_callback );

	if ( !iter )
		return -1;

	/* the key was found, remove it */
	if ( iter != std_list_end ( kui_ms->map_list ) ) {
		iter = std_list_remove ( kui_ms->map_list, iter );

		if ( !iter )
			return -1;
	}

	if ( std_list_insert_sorted ( kui_ms->map_list, map, kui_map_compare_callback ) == -1 )
		return -1;

	return 0;
}

int kui_ms_deregister_map (
		struct kui_map_set *kui_ms,
		const char *key ) {
	std_list_iterator iter;

	if ( !kui_ms )
		return -1;

	if ( !kui_ms->map_list )
		return -1;

	iter = std_list_find ( kui_ms->map_list, key, kui_map_compare_callback );

	if ( !iter )
		return -1;

	/* The key could not be found */
	if ( iter == std_list_begin ( kui_ms->map_list ) )
		return -2;

	iter = std_list_remove ( kui_ms->map_list, iter );

	if ( !iter )
		return -1;

	return 0;
}

std_list kui_ms_get_maps ( struct kui_map_set *kui_ms ) {

	if ( !kui_ms )
		return NULL;

	return kui_ms->map_list;
}

/**
 * Reset the list of maps to it's original state.
 * This is as if no character was passed to kui_ms_update_state.
 *
 * \param kui_ms
 * The map set to reset
 *
 * @return
 * 0 on success, or -1 on error.
 *
 */
static int kui_ms_reset_state ( struct kui_map_set *kui_ms ) {
	if ( !kui_ms )
		return -1;

	kui_ms->map_iter = std_list_begin ( kui_ms->map_list );

	if ( !kui_ms )
		return -1;

	kui_ms->map_state = KUI_MAP_STILL_LOOKING;

	kui_ms->is_found = 0;

	kui_ms->map_iter_found = NULL;

	return 0;
}

/**
 * Get's the state of the current map set.
 *
 * \param kui_ms
 * The map set to get the state of.
 *
 * \param map_state
 * The map set's map state.
 *
 * @return
 * 0 on success, or -1 on error.
 */
static int kui_ms_get_state ( 
		struct kui_map_set *kui_ms, 
		enum kui_map_state *map_state ) {
	if ( !kui_ms )
		return -1;

	*map_state = kui_ms->map_state;

	return 0;
}

/**
 * This should be called when you are no longer going to call
 * kui_ms_update_state on a kui_ms. It allows the kui map set to finalize some
 * of it's internal state data.
 *
 * \param kui_ms
 * The kui map set to finalize
 *
 * @return
 * 0 on success, or -1 on error.
 */
static int kui_ms_finalize_state ( struct kui_map_set *kui_ms ) {
	if ( !kui_ms )
		return -1;
	
	if ( kui_ms->is_found ) {
		kui_ms->map_state = KUI_MAP_FOUND;
		kui_ms->map_iter = kui_ms->map_iter_found;
	}

	return 0;
}

/**
 * This updates the state of a map set.
 *
 * Basically, the map set receives a character. It also receives the position
 * of that character in the mapping. The map set is then responsible for 
 * checking to see if any of the maps match the particular mapping.
 *
 * \param kui_ms
 * The map set to update
 *
 * \param c
 * The character to match
 *
 * \param position
 * The position of character in the mapping
 *
 * @return
 * 0 on success, or -1 on error.
 */
static int kui_ms_update_state ( 
		struct kui_map_set *kui_ms, 
		char c,
	    int position ) {
	char *matched, *cur;
	int strncmp_return_value;
	int cur_length;
	std_list_iterator iter;
	
	/* Verify parameters */
	if ( !kui_ms )
		return -1;

	if ( position < 0 )
		return -1;

	if ( c < 0 )
		return -1;

	/* Assertion: Should only try to update the state if still looking */
	if ( kui_ms->map_state != KUI_MAP_STILL_LOOKING )
		return -1;

	/* Get the original value */
	{
		struct kui_map *map;
		void *data;

		if ( std_list_get_data ( kui_ms->map_iter, &data ) == -1 )
			return -1;

		map = (struct kui_map *)data;

		if ( kui_map_get_key ( map, &matched ) == -1 )
			return -1;
	}

	/* Start the searching */
	for ( ; 
	 	  kui_ms->map_iter != std_list_end ( kui_ms->map_list );
	   	  kui_ms->map_iter = std_list_next ( kui_ms->map_iter ) ) {
		struct kui_map *map;
		void *data;

		if ( std_list_get_data ( kui_ms->map_iter, &data ) == -1 )
			return -1;

		map = (struct kui_map *)data;

		if ( kui_map_get_key ( map, &cur ) == -1 )
			return -1;

		strncmp_return_value = strncmp ( matched, cur, position );

		/* Once the value is passed, stop looking. */
		if ( ( strncmp_return_value != 0 ) ||
			 ( strncmp_return_value == 0 && cur[position] > c ) ) {
			kui_ms->map_state = KUI_MAP_NOT_FOUND;
			break;
		}
		
		/* A successful find */
		if ( strncmp_return_value == 0 && cur[position] == c ) {
			kui_ms->map_state = KUI_MAP_STILL_LOOKING;
			break;
		}
	}

	/* It was discovered that this map is not found during the loop */
	if ( kui_ms->map_state == KUI_MAP_NOT_FOUND )
		return 0;

	/* Every item has been checked, the map is not in the list */
	if ( kui_ms->map_iter == std_list_end ( kui_ms->map_list ) ) {
		kui_ms->map_state = KUI_MAP_NOT_FOUND;
		return 0;
	}

	/* At this point, the iter points to the valid spot in the list. 
	 * Decide if the correct state is STILL_LOOKING or FOUND.
	 *
	 * The only way to now if you are at FOUND is to do 2 things.
	 * 1. make sure that the lenght of the position is the length
	 * of the current map value. If they are not the same length,
	 * you are STILL_LOOKING
	 * 2. Make sure the next item in the list, doesn't begin with
	 * the current value. If you are at the last spot in the list,
	 * and the first rule holds, it is definatly FOUND.
	 */
	cur_length = strlen ( cur );

	if ( cur_length != position + 1) {
	   return 0; /* STILL_LOOKING */
	} else {
		kui_ms->is_found = 1;
		kui_ms->map_iter_found = kui_ms->map_iter;
	}

	/* If still here, rule 1 passed, check rule 2 */
	iter = std_list_next ( kui_ms->map_iter );

	if ( !iter )
		return -1;

	/* Special case, return FOUND */
	if ( iter == std_list_end ( kui_ms->map_list ) ) {
		kui_ms->map_state = KUI_MAP_FOUND;
		return 0;
	}

	/* Get value and see if it begins with same value */
	{
		struct kui_map *map;
		void *data;

		if ( std_list_get_data ( iter, &data ) == -1 )
			return -1;

		map = (struct kui_map *)data;

		if ( kui_map_get_key ( map, &matched ) == -1 )
			return -1;

	}

	/* The value is not the same, found. */
	if ( strncmp ( matched, cur, position+1 ) != 0 ) {
		kui_ms->map_state = KUI_MAP_FOUND;
		return 0;
	}

	return 0;
}

/**
 * A Key User Interface context.
 */
struct kuictx {
	/**
	 * The list of kui_map_set structures.
	 *
	 * The kui context will use this list when looking for maps.
	 */
	std_list kui_map_set_list;

	/**
	 * A list of characters, used as a buffer for stdin.
	 */
	std_list buffer;

	/**
	 * The file descriptor to read from.
	 */
	int fd;
};

static int kui_ms_destroy_callback ( void *param ) {
	std_list list = (std_list)param;

	if ( !list )
		return -1;

	if ( std_list_destroy ( list ) == -1 )
		return -1;

	list = NULL;

	return 0;
}

static int kui_ms_destroy_char_callback ( void *param ) {
	char *c = (char*)param;

	if ( !c )
		return -1;

	free ( c );
	c = NULL;

	return 0;
}

struct kuictx *kui_create(int stdinfd) {
	struct kuictx *kctx; 
	
	kctx = (struct kuictx *)malloc(sizeof(struct kuictx));

	if ( !kctx )
		return NULL;

	kctx->kui_map_set_list = std_list_create ( kui_ms_destroy_callback );

	if ( !kctx->kui_map_set_list ) {
		/* Free the kctx */
		free ( kctx );
		kctx = NULL;
		return NULL;
	}

	kctx->fd = stdinfd;

	kctx->buffer = std_list_create ( kui_ms_destroy_char_callback );

	return kctx;
}

int kui_destroy ( struct kuictx *kctx ) {
	int ret = 0;

	if ( !kctx )
		return -1;

	if ( std_list_destroy ( kctx->kui_map_set_list ) == -1 )
		ret = -1;

	free (kctx);
	kctx = NULL;

	return ret;
}

std_list kui_get_map_sets ( struct kuictx *kctx ) {
	if ( !kctx )
		return NULL;

	return kctx->kui_map_set_list;
}

int kui_add_map_set ( 
		struct kuictx *kctx, 
		struct kui_map_set *kui_ms ) {
	if ( !kctx )
		return -1;

	if ( !kui_ms )
		return -1;

	if ( std_list_append ( kctx->kui_map_set_list, kui_ms ) == -1 )
		return -1;

	return 0;
}

/**
 * This basically get's a char from the internal buffer within the kui context
 * or it get's a charachter from the standard input file descriptor.
 *
 * It only blocks for a limited amount of time, waiting for user input.
 *
 * \param kctx
 * The kui context to operate on.
 *
 * @return
 * 0 on success, or -1 on error.
 */
static char kui_findchar ( struct kuictx *kctx ) {
	char c;
	int length;
	void *data;
	std_list_iterator iter;

	/* Use the buffer first. */
	length = std_list_length ( kctx->buffer );

	if ( length == -1 )
		return -1;

	if ( length > 0 ) {
		/* Take the first char in the list */
		iter = std_list_begin ( kctx->buffer );

		if ( !iter )
			return -1; 
		
		if ( std_list_get_data ( iter, &data ) == -1 )
			return -1;

		/* Get the char */
		c = *(char*)data;

		/* Delete the item */
		if ( std_list_remove ( kctx->buffer, iter ) == NULL )
			return -1;
		
	} else {
		/* otherwise, look to read in a char */
		c = io_getchar ( kctx->fd, 1000 );
	}

	return c;
}

/**
 * Updates the state data for each map set in the kui
 *
 * \param kctx
 * The kui context to operate on.
 *
 * @return
 * 0 on success, or -1 on error.
 */
static int kui_reset_state_data ( struct kuictx *kctx ) {
    std_list_iterator iter;
    struct kui_map_set *map_set;
    void *data;

	for ( iter = std_list_begin ( kctx->kui_map_set_list );
		  iter != std_list_end ( kctx->kui_map_set_list );
		  iter = std_list_next ( iter ) ) {

		if ( std_list_get_data ( iter, &data ) == -1 )
			return -1;

		map_set = (struct kui_map_set *)data;

		if ( kui_ms_reset_state ( map_set ) == -1 )
			return -1;
	}

	return 0;
}

/**
 * Updates each list in the kui context.
 *
 * \param kctx
 * The kui context to operate on.
 *
 * \param c
 * The character to match
 *
 * \param position
 * The position of character in the mapping
 *
 * @return
 * 0 on success, or -1 on error.
 */
static int kui_update_each_list ( 
		struct kuictx *kctx, 
		char c,
	    int position	) {
    std_list_iterator iter;
	struct kui_map_set *map_set;
	void *data;
	enum kui_map_state map_state;

	/* For each active map list (doesn't return KUI_MAP_NOT_FOUND), 
	 * give the char c to let the list update it's internal state. 
	 */
	for ( iter = std_list_begin ( kctx->kui_map_set_list );
		  iter != std_list_end ( kctx->kui_map_set_list );
		  iter = std_list_next ( iter ) ) {

		if ( std_list_get_data ( iter, &data ) == -1 )
			return -1;

		map_set = (struct kui_map_set *)data;

		if ( kui_ms_get_state ( map_set, &map_state ) == -1 )
			return -1;

		if ( map_state != KUI_MAP_NOT_FOUND ) {
			if ( kui_ms_update_state ( map_set, c, position ) == -1 )
				return -1;
		}
	}

	return 0;
}

/**
 * Checks to see if any of the map set's are matching a map.
 * If they are, then the kui context should keep trying to match. Otherwise
 * it should stop trying to match.
 *
 * \param kctx
 * The kui context to operate on.
 *
 * \param should_continue
 * outbound as 1 if the kui context should keep looking, otherwise 0.
 *
 * @return
 * 0 on success, or -1 on error.
 */
static int kui_should_continue_looking (
		struct kuictx *kctx,
		int *should_continue ) {
	std_list_iterator iter;
	struct kui_map_set *map_set;
	void *data;
	enum kui_map_state map_state;

	if ( !kctx )
		return -1;

	if ( !should_continue )
		return -1;

	*should_continue = 0;

	/* Continue if at least 1 of the lists still says 
	 * KUI_MAP_STILL_LOOKING. If none of the lists is at this state, then
	 * there is no need to keep looking
	 */
	for ( iter = std_list_begin ( kctx->kui_map_set_list );
		  iter != std_list_end ( kctx->kui_map_set_list );
		  iter = std_list_next ( iter ) ) {

		if ( std_list_get_data ( iter, &data ) == -1 )
			return -1;

		map_set = (struct kui_map_set *)data;

		if ( kui_ms_get_state ( map_set, &map_state ) == -1 )
			return -1;

		if ( map_state == KUI_MAP_STILL_LOOKING )
			*should_continue = 1;
	}

	return 0;
}

/**
 * Update each map list's state.
 *
 * \param kctx
 * The kui context to operate on.
 *
 * @return
 * 0 on success, or -1 on error.
 */
static int kui_update_list_state ( struct kuictx *kctx ) {
	std_list_iterator iter;
	struct kui_map_set *map_set;
	void *data;

	if ( !kctx )
		return -1;

	/* If a macro was found, change the state of the map_list */
	for ( iter = std_list_begin ( kctx->kui_map_set_list );
		  iter != std_list_end ( kctx->kui_map_set_list );
		  iter = std_list_next ( iter ) ) {

		if ( std_list_get_data ( iter, &data ) == -1 )
			return -1;

		map_set = (struct kui_map_set *)data;

		if ( kui_ms_finalize_state ( map_set ) == -1 )
			return -1;
	}

	return 0;
}

/**
 * Checks to see if a kui map has been found.
 *
 * \param kctx
 * The kui context to operate on.
 *
 * \param was_map_found
 * outbound as 1 if a map was found , otherwise 0.
 *
 * \param the_map_found
 * If was_map_found returns as 1, then this is the map that was found.
 *
 * @return
 * 0 on success, or -1 on error.
 */
static int kui_was_map_found ( 
		struct kuictx *kctx,
		int *was_map_found,
		struct kui_map **the_map_found ) {
	std_list_iterator iter;
	struct kui_map_set *map_set;
	void *data;
	enum kui_map_state map_state;

	if ( !was_map_found )
		return -1;

	if ( !the_map_found )
		return -1;

	*was_map_found = 0;

	/* At this point, the loop exited for one of several reasons.
	 *
	 * Each list has a correct state value. If one of the lists has the value
	 * KUI_MAP_FOUND, then a map was found. This should be the value used.
	 *
	 * If none of the lists has the value kui_map_found, then no map was found.
	 * What a shame. Why did I write all of this code ?!?
	 */
	for ( iter = std_list_begin ( kctx->kui_map_set_list );
		  iter != std_list_end ( kctx->kui_map_set_list );
		  iter = std_list_next ( iter ) ) {

		if ( std_list_get_data ( iter, &data ) == -1 )
			return -1;

		map_set = (struct kui_map_set *)data;

		if ( kui_ms_get_state ( map_set, &map_state ) == -1 )
			return -1;

		if ( map_state == KUI_MAP_FOUND ) {
			void *data;

			if ( std_list_get_data ( map_set->map_iter, &data ) == -1 )
				return -1;

			*the_map_found = ( struct kui_map *)data;
			fprintf ( stderr, "MAP FOUND(%s)\r\n", (*the_map_found)->original_value );
			*was_map_found = 1;
		}
	}


	return 0;
}

/**
 * Updates the kui context buffer.
 *
 * \param kctx
 * The kui context to operate on.
 *
 * \param the_map_found
 * The map that was found, valid if map_was_found is 1
 *
 * \param map_was_found
 * 1 if a map was found, otherwise 0
 *
 * \param position
 * ???
 *
 * \param bufmax
 * ???
 *
 * \param c
 * ???
 *
 * @return
 * 0 on success, or -1 on error.
 *
 * Here is a simple example to help illustrate this algorithm.
 *
 * map ab xyz
 * map abcdf do_not_reach
 *
 * the buffer contained, abcdefgh
 *
 * abcde is read in coming into this function.
 *
 * so, ab matches a map, cde is read in to continue matching
 * but never does. that means fgh is left in the buffer.
 *
 * ab changes to xyz.
 * cde needs to be put back.
 *
 * so you end up with 
 * 	xyzcdefgh
 *
 * which means that you put back the extra char's first.
 * Then you put back the map.
 *
 * The only 2 things to know are, what are the extra chars
 * and what is the value of the map.
 */
static int kui_update_buffer ( 
		struct kuictx *kctx,
		struct kui_map *the_map_found,
	    int map_was_found,
	    int position,
		char *bufmax,
		char *c) {
	int i;
	int j;


	/* Find extra chars */
	if ( map_was_found )
		/* For the example, this would be 'ab' 2 */
		i = strlen ( the_map_found->original_key );
	else {
		i = 1; /* bufmax[0] will be returned to the user */
		*c = bufmax[0];
	}

	/* for the example, position would be 5 
	 * Assertion: bufmax[position] is valid 
	 */
	for ( j = position ; j >= i; --j ) {
		char *val = malloc ( sizeof ( char ) );
		if ( !val )
			return -1;

		*val = bufmax[j];

		if ( std_list_prepend ( kctx->buffer, val ) == -1 )
			return -1;
	}

	/* Add the map back */
	if ( map_was_found ) {
		int length;

		/* Add the value onto the buffer */
		length = strlen ( the_map_found->original_value );
		for ( i = length-1; i >= 0; --i ) {
			char *val = malloc ( sizeof ( char ) );
			if ( !val )
				return -1;

			*val = the_map_found->original_value[i];

			if ( std_list_prepend ( kctx->buffer, val ) == -1 )
				return -1;
		}
	} 

	return 0;
}

/**
 * Get's the next char.
 *
 * \param map_found
 * returns as 0 if no map was found. In this case, the return value is valid.
 * returns as 1 if a mapping was found. In this case the return value is not 
 * valid.
 *
 * @return
 * -1 on error
 * The key on success ( valid if map_found == 0 )
 */
static int kui_findkey ( 
		struct kuictx *kctx,
	    int *was_map_found ) {

	char c;
	int position;
	int should_continue;
	char bufmax[1024]; /* This constant limits this function */
	struct kui_map *the_map_found;

	/* Validate parameters */
	if ( !kctx )
		return -1;

	if ( !was_map_found )
		return -1;

	if ( !kctx->kui_map_set_list )
		return -1;

	/* Initialize variables on stack */
	c = -1;
	position = -1;
	*was_map_found = 0;
	should_continue = 0;

	/* Reset the state data for all of the lists */
	if ( kui_reset_state_data ( kctx ) == -1 )
		return -1;

	/* Start the main loop */
	while ( 1 ) {
		c = kui_findchar ( kctx );

		/* If there is no more data ready, stop. */
		if ( c == 0 )
			break; 

		++position;

		bufmax[position] = c;

		/* Update each list, with the character read, and the position. */
		if ( kui_update_each_list ( kctx, c, position ) == -1 )
			return -1;

		/* Check to see if at least a single map is being matched */
		if ( kui_should_continue_looking ( kctx, &should_continue ) == -1 )
			return -1;

		if ( !should_continue )
			break;
	}

	c = 0; /* This should no longer be used. Enforcing that. */

	/* All done looking for chars, let lists that matched a mapping
	 * be known. ex KUI_MAP_STILL_LOOKING => KUI_MAP_FOUND. This 
	 * happens when 
	 *    map abc   xyz
	 *    map abcde xyz
	 *
     * If the user types abcd, the list will still be looking,
     * even though it already found a mapping.
	 */
	if ( kui_update_list_state ( kctx ) == -1 )
		return -1;

	/* Check to see if a map was found.
	 * If it was, get the map also.
	 */
	if ( kui_was_map_found ( kctx, was_map_found, &the_map_found ) == -1 )
		return -1;

	/* Update the buffer and get the final char. */
	if ( kui_update_buffer ( kctx, the_map_found, *was_map_found, position, bufmax, &c ) == -1 )
		return -1;

	return c;
}

int kui_getkey ( struct kuictx *kctx ) {
	int map_found; 
	int c;

	/* If a map was found, restart the algorithm. */
	do {
		c = kui_findkey ( kctx, &map_found );

		if ( c == -1 )
			return -1;

	} while ( map_found == 1 );

	return c;
}

int kui_cangetkey ( struct kuictx *kctx ) {
	int length;

	/* Use the buffer first. */
	length = std_list_length ( kctx->buffer );

	if ( length == -1 )
		return -1;

	if ( length > 0 )
		return 1;

	return 0;
}
