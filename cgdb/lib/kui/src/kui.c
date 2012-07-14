/* includes {{{ */

#include <string.h>             /* strdup */
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "kui.h"
#include "sys_util.h"
#include "io.h"
#include "kui_term.h"
#include "kui_tree.h"

/* }}} */

/* struct kui_map {{{ */

/**
 * A kui map structure.
 *
 * This is simply a key/value pair as far as the outside
 * world is concerned. 
 */
struct kui_map {

    /**
	 * The user wants this map to be activated when he/she types this sequence.
	 * This data representation is entirly in ascii, since the user has to 
	 * be able to type the sequence on the keyboard.
	 */
    char *original_key;

    /**
	 * This is the list of keys that must be matched, in order for this map
	 * to be activated. The value of each item is either an ascii char typed 
	 * by the user, or it is a high level terminal escape sequence.
	 *
	 * This is a NULL terminated list.
	 */
    int *literal_key;

    /**
	 * The value is the substitution data, if the literal_key is typed.
	 */
    char *original_value;

    /**
	 * This data is passed in place of the key, if the user types the 
	 * literal_key.
	 *
	 * This is a NULL terminated list.
	 */
    int *literal_value;
};

struct kui_map *kui_map_create(const char *key_data, const char *value_data)
{

    struct kui_map *map;
    char *key, *value;

    /* Validify parameters */
    if (!key_data || !value_data)
        return NULL;

    map = (struct kui_map *) malloc(sizeof (struct kui_map));
    if (!map)
        return NULL;

    /* Initialize all fields */
    map->original_key = NULL;
    map->literal_key = NULL;
    map->original_value = NULL;
    map->literal_value = NULL;

    key = strdup(key_data);

    if (!key) {
        kui_map_destroy(map);
        return NULL;
    }

    value = strdup(value_data);

    if (!value) {
        kui_map_destroy(map);
        return NULL;
    }

    map->original_key = key;
    map->original_value = value;

    if (kui_term_string_to_key_array(map->original_key,
                    &map->literal_key) == -1) {
        kui_map_destroy(map);
        return NULL;
    }

    if (kui_term_string_to_key_array(map->original_value,
                    &map->literal_value) == -1) {
        kui_map_destroy(map);
        return NULL;
    }

    return map;
}

int kui_map_destroy(struct kui_map *map)
{

    if (!map)
        return -1;

    if (map->original_key) {
        free(map->original_key);
        map->original_key = NULL;
    }

    if (map->original_value) {
        free(map->original_value);
        map->original_value = NULL;
    }

    if (map->literal_key) {
        free(map->literal_key);
        map->literal_key = NULL;
    }

    if (map->literal_value) {
        free(map->literal_value);
        map->literal_value = NULL;
    }

    free(map);
    map = NULL;

    return 0;
}

int kui_map_get_key(struct kui_map *map, char **key)
{

    if (!map)
        return -1;

    *key = map->original_key;

    return 0;
}

int kui_map_get_literal_key(struct kui_map *map, int **key)
{

    if (!map)
        return -1;

    *key = map->literal_key;

    return 0;
}

int kui_map_get_value(struct kui_map *map, char **value)
{

    if (!map)
        return -1;

    *value = map->original_value;

    return 0;
}

int kui_map_get_literal_value(struct kui_map *map, int **value)
{

    if (!map)
        return -1;

    *value = map->literal_value;

    return 0;
}

int kui_map_print_cgdb_key_array(struct kui_map *map)
{
    if (!map)
        return -1;

    if (kui_term_print_key_array(map->literal_value) == -1)
        return -1;

    return 0;

}

/* }}} */

/* struct kui_map_set {{{ */

/* Kui map set */

/** 
 * This maintains a list of maps. It also is capable of tracking if a user is 
 * matching any map in this set with the current key strokes being typed.
 */
struct kui_map_set {
    /* The ktree used in determining if a map has been reached or is being
     * reached. Of course, nothing could match also. This structure is efficient
     * in doing the work, it looks only at the current key read.  */
    struct kui_tree *ktree;

    /* A linked list of the maps being checked for. */
    std_list maps;
};

static int kui_map_destroy_callback(void *data)
{
    struct kui_map *map;

    if (!data)
        return -1;

    map = (struct kui_map *) data;

    return kui_map_destroy(map);
}

struct kui_map_set *kui_ms_create(void)
{
    struct kui_map_set *map;

    map = (struct kui_map_set *) malloc(sizeof (struct kui_map_set));

    if (!map)
        return NULL;

    map->maps = std_list_create(kui_map_destroy_callback);

    if (!map->maps) {
        kui_ms_destroy(map);
        return NULL;
    }

    map->ktree = kui_tree_create();

    if (!map->ktree) {
        kui_ms_destroy(map);
        return NULL;
    }

    return map;
}

int kui_ms_destroy(struct kui_map_set *kui_ms)
{
    int retval = 0;

    if (!kui_ms)
        return -1;

    if (kui_ms->ktree) {
        if (kui_tree_destroy(kui_ms->ktree) == -1)
            retval = -1;
    }

    if (kui_ms->maps) {
        if (std_list_destroy(kui_ms->maps) == -1)
            retval = -1;
        kui_ms->maps = NULL;
    }

    free(kui_ms);
    kui_ms = NULL;

    return retval;
}

static int kui_map_compare_callback(const void *a, const void *b)
{
    struct kui_map *one = (struct kui_map *) a;
    struct kui_map *two = (struct kui_map *) b;

    return strcmp(one->original_key, two->original_key);
}

static int kui_map_compare_key_callback(const void *a, const void *b)
{
    struct kui_map *one = (struct kui_map *) a;
    char *two = (char *) b;

    return strcmp(one->original_key, two);
}

int kui_ms_register_map(struct kui_map_set *kui_ms,
        const char *key_data, const char *value_data)
{
    struct kui_map *map;

    if (!kui_ms)
        return -1;

    map = kui_map_create(key_data, value_data);

    if (!map)
        return -1;

    if (std_list_insert_sorted(kui_ms->maps, map,
                    kui_map_compare_callback) == -1)
        return -1;

    if (kui_tree_insert(kui_ms->ktree, map->literal_key, map) == -1)
        return -1;

    return 0;
}

int kui_ms_deregister_map(struct kui_map_set *kui_ms, const char *key)
{
    std_list_iterator iter;
    struct kui_map *map;
    void *data;

    if (!kui_ms)
        return -1;

    iter = std_list_find(kui_ms->maps, key, kui_map_compare_key_callback);

    if (!iter)
        return -1;

    if (std_list_get_data(iter, &data) == -1)
        return -1;

    /* If the mapping exists, remove it. */
    if (data) {
        map = (struct kui_map *) data;

        /* Delete from the tree */
        if (kui_tree_delete(kui_ms->ktree, map->literal_key) == -1)
            return -1;

        if (std_list_remove(kui_ms->maps, iter) == NULL)
            return -1;
    }

    return 0;
}

/* }}} */

/* struct kuictx {{{ */

/**
 * A Key User Interface context.
 */
struct kuictx {
    /**
	 * The list of kui_map_set structures.
	 */
    std_list kui_map_set_list;

    /**
	 * A list of characters, used as a buffer for stdin.
	 */
    std_list buffer;

    /**
	 * A volitale buffer. This is reset upon every call to kui_getkey.
	 */
    std_list volatile_buffer;

    /**
	 * The callback function used to get data read in.
	 */
    kui_getkey_callback callback;

    /**
	 * Milliseconds to block on a read.
	 */
    int ms;

    /**
	 * state data
	 */
    void *state_data;

    /**
	 * The file descriptor to read from.
	 */
    int fd;
};

static int kui_ms_destroy_int_callback(void *param)
{
    int *i = (int *) param;

    if (!i)
        return -1;

    free(i);
    i = NULL;

    return 0;
}

struct kuictx *kui_create(int stdinfd,
        kui_getkey_callback callback, int ms, void *state_data)
{
    struct kuictx *kctx;

    kctx = (struct kuictx *) malloc(sizeof (struct kuictx));

    if (!kctx)
        return NULL;

    kctx->callback = callback;
    kctx->state_data = state_data;
    kctx->kui_map_set_list = std_list_create(NULL);
    kctx->ms = ms;

    if (!kctx->kui_map_set_list) {
        kui_destroy(kctx);
        return NULL;
    }

    kctx->fd = stdinfd;

    kctx->buffer = std_list_create(kui_ms_destroy_int_callback);

    if (!kctx->buffer) {
        kui_destroy(kctx);
        return NULL;
    }

    kctx->volatile_buffer = std_list_create(kui_ms_destroy_int_callback);

    if (!kctx->volatile_buffer) {
        kui_destroy(kctx);
        return NULL;
    }

    return kctx;
}

int kui_destroy(struct kuictx *kctx)
{
    int ret = 0;

    if (!kctx)
        return -1;

    if (kctx->kui_map_set_list) {
        if (std_list_destroy(kctx->kui_map_set_list) == -1)
            ret = -1;
        kctx->kui_map_set_list = NULL;
    }

    if (kctx->buffer) {
        if (std_list_destroy(kctx->buffer) == -1)
            ret = -1;
        kctx->buffer = NULL;
    }

    if (kctx->volatile_buffer) {
        if (std_list_destroy(kctx->volatile_buffer) == -1)
            ret = -1;
        kctx->volatile_buffer = NULL;
    }

    free(kctx);
    kctx = NULL;

    return ret;
}

std_list kui_get_map_sets(struct kuictx * kctx)
{
    if (!kctx)
        return NULL;

    return kctx->kui_map_set_list;
}

int kui_clear_map_sets(struct kuictx *kctx)
{
    if (!kctx)
        return -1;

    return std_list_remove_all(kctx->kui_map_set_list);
}

int kui_add_map_set(struct kuictx *kctx, struct kui_map_set *kui_ms)
{
    if (!kctx)
        return -1;

    if (!kui_ms)
        return -1;

    if (std_list_append(kctx->kui_map_set_list, kui_ms) == -1)
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
 * \param key
 * The key that was read
 *
 * @return
 * 1 on success,
 * 0 if no more input, 
 * or -1 on error.
 */
static int kui_findchar(struct kuictx *kctx, int *key)
{
    int length;
    void *data;
    std_list_iterator iter;

    if (!key)
        return -1;

    /* Use the buffer first. */
    length = std_list_length(kctx->buffer);

    if (length == -1)
        return -1;

    if (length > 0) {
        /* Take the first char in the list */
        iter = std_list_begin(kctx->buffer);

        if (!iter)
            return -1;

        if (std_list_get_data(iter, &data) == -1)
            return -1;

        /* Get the char */
        *key = *(int *) data;

        /* Delete the item */
        if (std_list_remove(kctx->buffer, iter) == NULL)
            return -1;

    } else {
        /* Otherwise, look to read in a char,
         * This function called returns the same conditions as this function*/
        return kctx->callback(kctx->fd, kctx->ms, kctx->state_data, key);
    }

    return 1;
}

/**
 * Updates the state data for each map set in the kui
 *
 * \param data
 * The kui context to operate on.
 *
 * \param user_data
 * Not used 
 *
 * @return
 * 1 on success, 0 to stop, -1 on error
 */
static int kui_reset_state_data(void *data, void *user_data)
{
    struct kui_map_set *map_set = (struct kui_map_set *) data;

    if (kui_tree_reset_state(map_set->ktree) == -1)
        return -1;

    return 1;
}

/**
 * Updates each list in the kui context.
 *
 * \param data
 * The kui context to operate on.
 *
 * \param user_data
 * The key received.
 *
 * @return
 * 1 on success, 0 to stop, -1 on error
 */
static int kui_update_each_list(struct kuictx *kctx, int key, int *map_found)
{
    std_list_iterator iter;
    struct kui_map_set *map_set;
    void *data;

    if (!kctx)
        return -1;

    *map_found = 0;

    /* Continue if at least 1 of the lists still says 
     * KUI_MAP_STILL_LOOKING. If none of the lists is at this state, then
     * there is no need to keep looking
     */
    for (iter = std_list_begin(kctx->kui_map_set_list);
            iter != std_list_end(kctx->kui_map_set_list);
            iter = std_list_next(iter)) {

        if (std_list_get_data(iter, &data) == -1)
            return -1;

        map_set = (struct kui_map_set *) data;

        if (kui_tree_push_key(map_set->ktree, key, map_found) == -1)
            return -1;

        if (*map_found) {
            /* If a map was found, reset the extra char's read */
            if (std_list_remove_all(kctx->volatile_buffer) == -1)
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
static int kui_should_continue_looking(struct kuictx *kctx,
        int *should_continue)
{
    std_list_iterator iter;
    struct kui_map_set *map_set;
    void *data;
    enum kui_tree_state map_state;

    if (!kctx)
        return -1;

    if (!should_continue)
        return -1;

    *should_continue = 0;

    /* Continue if at least 1 of the lists still says 
     * KUI_MAP_STILL_LOOKING. If none of the lists is at this state, then
     * there is no need to keep looking
     */
    for (iter = std_list_begin(kctx->kui_map_set_list);
            iter != std_list_end(kctx->kui_map_set_list);
            iter = std_list_next(iter)) {

        if (std_list_get_data(iter, &data) == -1)
            return -1;

        map_set = (struct kui_map_set *) data;

        if (kui_tree_get_state(map_set->ktree, &map_state) == -1)
            return -1;

        if (map_state == KUI_TREE_MATCHING)
            *should_continue = 1;
    }

    return 0;
}

/**
 * Update each map list's state.
 *
 * \param data
 * The kui context to operate on.
 *
 * \param user_data
 * Unused
 *
 * @return
 * 1 on success, 0 to stop, -1 on error
 */
static int kui_finalize_state(void *data, void *user_data)
{
    struct kui_map_set *map_set = (struct kui_map_set *) data;

    if (kui_tree_finalize_state(map_set->ktree) == -1)
        return -1;

    return 1;
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
static int kui_was_map_found(struct kuictx *kctx,
        int *was_map_found, struct kui_map **the_map_found)
{
    std_list_iterator iter;
    struct kui_map_set *map_set;
    void *data;
    enum kui_tree_state map_state;

    if (!was_map_found)
        return -1;

    if (!the_map_found)
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
    for (iter = std_list_begin(kctx->kui_map_set_list);
            iter != std_list_end(kctx->kui_map_set_list);
            iter = std_list_next(iter)) {

        if (std_list_get_data(iter, &data) == -1)
            return -1;

        map_set = (struct kui_map_set *) data;

        if (kui_tree_get_state(map_set->ktree, &map_state) == -1)
            return -1;

        if (map_state == KUI_TREE_FOUND) {
            void *data;

            if (kui_tree_get_data(map_set->ktree, &data) == -1)
                return -1;

            *was_map_found = 1;
            *the_map_found = (struct kui_map *) data;
        }
    }

    return 0;
}

static int intlen(const int *val)
{
    int length = 0;

    while (val[length] != 0)
        ++length;

    return length;
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
 * \param key
 * The key the user typed this time around.
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
static int kui_update_buffer(struct kuictx *kctx,
        struct kui_map *the_map_found, int map_was_found, int *key)
{

    int i;
    std_list_iterator iter;

    if (!map_was_found) {
        void *data;

        iter = std_list_end(kctx->volatile_buffer);

        iter = std_list_previous(iter);

        if (!iter || iter == std_list_end(kctx->volatile_buffer))
            return -1;

        if (std_list_get_data(iter, &data) == -1)
            return -1;

        *key = *(int *) data;

        if (std_list_remove(kctx->volatile_buffer, iter) == NULL)
            return -1;
    }

    /* Add the extra char's read */
    for (iter = std_list_begin(kctx->volatile_buffer);
            iter != std_list_end(kctx->volatile_buffer);
            iter = std_list_next(iter)) {

        int *val = malloc(sizeof (int));
        void *data;

        if (!val)
            return -1;

        if (std_list_get_data(iter, &data) == -1)
            return -1;

        *val = *(int *) data;

        if (std_list_prepend(kctx->buffer, val) == -1)
            return -1;
    }

    /* Add the map value */
    if (map_was_found) {
        int length;

        /* Add the value onto the buffer */
        length = intlen(the_map_found->literal_value);

        for (i = length - 1; i >= 0; --i) {
            int *val = malloc(sizeof (int));

            if (!val)
                return -1;

            *val = the_map_found->literal_value[i];

            if (std_list_prepend(kctx->buffer, val) == -1)
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
static int kui_findkey(struct kuictx *kctx, int *was_map_found)
{

    int key, retval;
    int should_continue;
    struct kui_map *the_map_found = NULL;
    int *val;
    int map_found;

    /* Validate parameters */
    if (!kctx)
        return -1;

    if (!was_map_found)
        return -1;

    if (!kctx->kui_map_set_list)
        return -1;

    /* Initialize variables on stack */
    key = -1;
    *was_map_found = 0;
    should_continue = 0;

    if (std_list_remove_all(kctx->volatile_buffer) == -1)
        return -1;

    /* Reset the state data for all of the lists */
    if (std_list_foreach(kctx->kui_map_set_list, kui_reset_state_data,
                    NULL) == -1)
        return -1;

    /* Start the main loop */
    while (1) {
        retval = kui_findchar(kctx, &key);
        if (retval == -1)
            return -1;

        /* If there is no more data ready, stop. */
        if (retval == 0)
            break;

        /* Append to the list */
        val = malloc(sizeof (int));
        if (!val)
            return -1;
        *val = key;
        if (std_list_prepend(kctx->volatile_buffer, val) == -1)
            return -1;

        /* Update each list, with the character read, and the position. */
        if (kui_update_each_list(kctx, key, &map_found) == -1)
            return -1;

        /* Check to see if at least a single map is being matched */
        if (kui_should_continue_looking(kctx, &should_continue) == -1)
            return -1;

        if (!should_continue)
            break;
    }

    key = 0;                    /* This should no longer be used. Enforcing that. */

    /* All done looking for chars, let lists that matched a mapping
     * be known. ex KUI_MAP_STILL_LOOKING => KUI_MAP_FOUND. This 
     * happens when 
     *    map abc   xyz
     *    map abcde xyz
     *
     * If the user types abcd, the list will still be looking,
     * even though it already found a mapping.
     */
    if (std_list_foreach(kctx->kui_map_set_list, kui_finalize_state,
                    NULL) == -1)
        return -1;

    /* Check to see if a map was found.
     * If it was, get the map also.
     */
    if (kui_was_map_found(kctx, was_map_found, &the_map_found) == -1)
        return -1;

    /* Update the buffer and get the final char. */
    if (kui_update_buffer(kctx, the_map_found, *was_map_found, &key) == -1)
        return -1;

    return key;
}

int kui_getkey(struct kuictx *kctx)
{
    int map_found;
    int key;

    /* If a map was found, restart the algorithm. */
    do {
        key = kui_findkey(kctx, &map_found);

        if (key == -1)
            return -1;

    } while (map_found == 1);

    return key;
}

int kui_cangetkey(struct kuictx *kctx)
{
    int length;

    /* Use the buffer first. */
    length = std_list_length(kctx->buffer);

    if (length == -1)
        return -1;

    if (length > 0)
        return 1;

    return 0;
}

int kui_set_blocking_ms(struct kuictx *kctx, unsigned long msec)
{
    if (!kctx)
        return -1;

    kctx->ms = msec;
    return 0;
}

int kui_get_blocking_ms(struct kuictx *kctx, unsigned long *msec)
{
    if (!kctx || !msec)
        return -1;

    *msec = kctx->ms;
    return 0;
}

/* }}} */

/* struct kui_manager {{{ */

/**
 * The main kui context.
 * This context is capable of doing all the work associated with the KUI.
 */
struct kui_manager {
    /* The terminal escape sequence mappings */
    struct kuictx *terminal_keys;
    /* The user defined mappings */
    struct kuictx *normal_keys;
    /* Need a reference to the terminal escape sequence mappings when destroying
     * this context. (a list is populated in the create function)  */
    struct kui_map_set *terminal_key_set;
};

static int create_terminal_mappings(struct kui_manager *kuim, struct kuictx *i)
{
    struct kui_map_set *terminal_map;

    /* Create the terminal kui map */
    terminal_map = kui_term_get_terminal_mappings();

    if (!terminal_map)
        return -1;

    kuim->terminal_key_set = terminal_map;

    if (kui_add_map_set(i, terminal_map) == -1)
        return -1;

    return 0;
}

int char_callback(const int fd,
        const unsigned int ms, const void *obj, int *key)
{

    return io_getchar(fd, ms, key);
}

int kui_callback(const int fd, const unsigned int ms, const void *obj, int *key)
{

    struct kuictx *kctx = (struct kuictx *) obj;
    int result;

    if (!key)
        return -1;

    result = kui_cangetkey(kctx);
    if (result == -1)
        return -1;

    if (result == 1) {
        *key = kui_getkey(kctx);
        if (*key == -1)
            return -1;
    }

    /* If there is no data ready, check the I/O */
    if (result == 0) {
        result = io_data_ready(kctx->fd, ms);
        if (result == -1)
            return -1;

        if (result == 1) {
            *key = kui_getkey(kctx);
            if (*key == -1)
                return -1;
        }

        if (result == 0)
            return 0;
    }

    return 1;
}

struct kui_manager *kui_manager_create(int stdinfd,
        unsigned int keycode_timeout, unsigned int mapping_timeout)
{
    struct kui_manager *man;

    man = (struct kui_manager *) malloc(sizeof (struct kui_manager));

    if (!man)
        return NULL;

    man->normal_keys = NULL;
    man->terminal_keys =
            kui_create(stdinfd, char_callback, keycode_timeout, NULL);

    if (!man->terminal_keys) {
        kui_manager_destroy(man);
        return NULL;
    }

    if (create_terminal_mappings(man, man->terminal_keys) == -1) {
        kui_manager_destroy(man);
        return NULL;
    }

    man->normal_keys =
            kui_create(-1, kui_callback, mapping_timeout, man->terminal_keys);

    if (!man->normal_keys) {
        kui_manager_destroy(man);
        return NULL;
    }

    return man;
}

int kui_manager_destroy(struct kui_manager *kuim)
{
    int ret = 0;

    if (!kuim)
        return 0;

    if (kui_ms_destroy(kuim->terminal_key_set) == -1)
        ret = -1;

    if (kui_destroy(kuim->terminal_keys) == -1)
        ret = -1;

    if (kui_destroy(kuim->normal_keys) == -1)
        ret = -1;

    free(kuim);
    kuim = NULL;

    return ret;
}

std_list kui_manager_get_map_sets(struct kui_manager * kuim)
{
    if (!kuim)
        return NULL;

    return kui_get_map_sets(kuim->normal_keys);
}

int kui_manager_clear_map_sets(struct kui_manager *kuim)
{
    if (!kuim)
        return -1;

    return kui_clear_map_sets(kuim->normal_keys);
}

int kui_manager_add_map_set(struct kui_manager *kuim,
        struct kui_map_set *kui_ms)
{

    if (!kuim)
        return -1;

    return kui_add_map_set(kuim->normal_keys, kui_ms);
}

int kui_manager_cangetkey(struct kui_manager *kuim)
{
    if (!kuim)
        return -1;

    /* I'm not sure if checking the terminal keys here is the best solution.
     *
     * It might make more sense to flow the extra characters read from during
     * the terminal keys processing up to the normal keys buffer after a 
     * mapping is found.
     *
     * For now this seems to work. Essentially, the next read get's the buffered 
     * terminal keys first which is what should happen anyways.
     */
    return kui_cangetkey(kuim->terminal_keys) ||
            kui_cangetkey(kuim->normal_keys);
}

int kui_manager_getkey(struct kui_manager *kuim)
{
    if (!kuim)
        return -1;

    return kui_getkey(kuim->normal_keys);

}

int kui_manager_getkey_blocking(struct kui_manager *kuim)
{
    if (!kuim)
        return -1;

    unsigned long terminal_keys_msec, normal_keys_msec, val;

    /* Get the original values */
    kui_get_blocking_ms(kuim->terminal_keys, &terminal_keys_msec);
    kui_get_blocking_ms(kuim->normal_keys, &normal_keys_msec);

    /* Set the values to be blocking */
    kui_set_blocking_ms(kuim->terminal_keys, -1);
    kui_set_blocking_ms(kuim->normal_keys, -1);

    /* Get the key */
    val = kui_getkey(kuim->normal_keys);

    /* Restore the values */
    kui_set_blocking_ms(kuim->terminal_keys, terminal_keys_msec);
    kui_set_blocking_ms(kuim->normal_keys, normal_keys_msec);

    return val;
}

int kui_manager_set_terminal_escape_sequence_timeout(struct kui_manager *kuim,
        unsigned int msec)
{
    if (!kuim)
        return -1;

    return kui_set_blocking_ms(kuim->terminal_keys, msec);
}

int
kui_manager_set_key_mapping_timeout(struct kui_manager *kuim, unsigned int msec)
{
    if (!kuim)
        return -1;

    return kui_set_blocking_ms(kuim->normal_keys, msec);
}

int
kui_manager_get_terminal_keys_kui_map(struct kui_manager *kuim,
        enum cgdb_key key, std_list kui_map_set)
{
    struct kui_map_set *map_set;
    struct kuictx *terminalkeys;
    std_list map_sets;
    std_list_iterator iter, kui_map_set_iter;
    void *data;
    const char *keycode_str;

    if (!kuim)
        return -1;

    keycode_str = kui_term_get_keycode_from_cgdb_key(key);
    if (keycode_str == NULL)
        return -1;

    /* The first map set in the terminal_keys */
    terminalkeys = kuim->terminal_keys;
    map_sets = kui_get_map_sets(terminalkeys);

    if (std_list_length(map_sets) > 0) {
        iter = std_list_begin(map_sets);
        if (std_list_get_data(iter, &data) == -1) {
            return -1;
        }
        map_set = (struct kui_map_set *) data;
    }

    /* At this point, the kui_map_set is available
     * Add each kui_map_set into it. */
    for (kui_map_set_iter = std_list_begin(kui_map_set);
            kui_map_set_iter != std_list_end(kui_map_set);
            kui_map_set_iter = std_list_next(kui_map_set_iter)) {
        if (std_list_get_data(kui_map_set_iter, &data) == -1)
            return -1;
        kui_ms_register_map(map_set, data, keycode_str);
    }

    return 0;
}

/* }}} */
