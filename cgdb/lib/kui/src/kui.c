#include "kui.h"
#include "error.h"
#include "sys_util.h"

/* kui map set */

struct kui_map {
	char *key_data;
	char *value_data;
};

struct kui_map *kui_map_create (
		const char *key_data, 
		const int key_size,
		const char *value_data,
		const int value_size ) {

	struct kui_map *map = (struct kui_map *)
		xmalloc(sizeof(struct kui_map));

	if ( !map )
		return NULL;

	map->key_data = NULL;
	map->value_data = NULL;

	return map;
}

int kui_map_destroy ( struct kui_map *map ) {
	if ( !map)
		return -1;

	free (map);
	map = NULL;

	return 0;

}

int kui_map_get_key ( struct kui_map *map, char **key ) { 
	if ( !map )
		return -1;

	*key = map->key_data;

	return 0;
}

int kui_map_get_value ( struct kui_map *map, char **value ) {
	
	if ( !map )
		return -1;

	*value = map->value_data;

	return 0;
}

/* Kui map set */

/** 
 * This maintains a list of maps.
 * Basically, a key/value pair list.
 */
struct kui_map_set {
	/**
	 * The list of maps available.
	 */
	std_list map_list;
};

struct kui_map_set *kui_ms_create ( void ) {
	struct kui_map_set *kui_ms = 
		(struct kui_map_set *)xmalloc(sizeof(struct kui_map_set));

	if ( !kui_ms )
		return NULL;

	kui_ms->map_list = std_list_create ( NULL );

	return kui_ms;
}

int kui_ms_destroy ( struct kui_map_set *kui_ms ) {
	if ( !kui_ms )
		return -1;

	if ( std_list_destroy ( kui_ms->map_list ) == -1 )
		return -1;

	free (kui_ms);
	kui_ms = NULL;

	return 0;
}

int kui_ms_register_map ( 
		struct kui_map_set *kui_ms,
		const char *key_data, 
		const int key_size,
		const char *value_data,
		const int value_size ) {

	if ( !kui_ms )
		return -1;

	/* Not implemented */
	return -1;
}

int kui_ms_deregister_map (
		struct kui_map_set *kui_ms,
		const char *key_data,
	    const int size	) {

	if ( !kui_ms )
		return -1;
	/* Not implemented */
	return -1;
}

std_list kui_ms_get_maps ( struct kui_map_set *kui_ms ) {

	if ( !kui_ms )
		return NULL;

	return kui_ms->map_list;
}

/* kui */

/** Needs doco */
struct kuictx {
	int stdinfd;
	struct input *ictx;
	std_list kui_map_set_list;
};

struct kuictx *kui_create(int stdinfd) {
	struct kuictx *kctx = (struct kuictx *)xmalloc(sizeof(struct kuictx));

	return kctx;
}

int kui_destroy ( struct kuictx *kctx ) {
	if ( !kctx )
		return -1;

	free (kctx);
	kctx = NULL;

	return 0;
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

//	kctx->kui_ms = kui_ms;
	return 0;
}

int kui_getkey ( struct kuictx *kctx ) {

	if ( !kctx )
		return -1;

	/* Not implemented */
	return -1;
}
