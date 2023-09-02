#include "kui_ctx.h"

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
 * \param the_map_found
 * The map that was found, or nullptr
 *
 * \param key
 * The key the user typed this time around.
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
void kuictx::update_buffer(kui_map *the_map_found, int *key)
{
    if (!the_map_found && m_volatile_buffer.size()) {
        *key = m_volatile_buffer.back();
        m_volatile_buffer.pop_back();
    }

    for (auto it : m_volatile_buffer) {
        m_buffer.push_front(it);
    }

    /* Add the map value */
    if (the_map_found) {
        /* Add the value onto the buffer */
        const int *literal_value = the_map_found->get_literal_value();
        int length = intlen(literal_value);

        for (int i = length - 1; i >= 0; --i) {
            m_buffer.push_front(literal_value[i]);
        }
    }
}

/**
 * This basically get's a char from the internal buffer within the kui context
 * or it get's a character from the standard input file descriptor.
 *
 * It only blocks for a limited amount of time, waiting for user input.
 *
 * \param key
 * The key that was read
 *
 * @return
 * 1 on success,
 * 0 if no more input, 
 * or -1 on error.
 */
int kuictx::findchar(int& key)
{
    if (m_buffer.size()) {
        key = m_buffer.front();
        m_buffer.pop_front();
        return 1;
    } else {
        /* Otherwise, look to read in a char,
         * This function called returns the same conditions as this function*/
        return m_callback(m_fd, m_ms, m_state_data, &key);
    }
}

/**
 * Updates the kui context's map set with the new key.
 *
 * @param key
 * The new key to push into the kui context's map set.
 *
 * @param map_found
 * Return's as 1 if a map was found while pushing this key, otherwise 0.
 *
 * @return
 * 0 on success, -1 on error.
 */
int kuictx::update_map_set(int key, int& map_found)
{
    map_found = 0;

    /* Continue if at least 1 of the lists still says 
     * KUI_MAP_STILL_LOOKING. If none of the lists is at this state, then
     * there is no need to keep looking
     */

    if (!m_map_set->push_key(key, &map_found))
        return -1;

    if (map_found) {
        /* If a map was found, reset the extra char's read */
        m_volatile_buffer.clear();
    }

    return 0;
}

/**
 * Checks to see if the map set is matching a map.
 * If it is, then the kui context should keep trying to match.
 * Otherwise it should stop trying to match.
 *
 * @return
 * true if the kui context should keep looking, otherwise false.
 */
bool kuictx::should_continue_looking() const
{
    /* Continue if at least 1 of the lists still says
     * KUI_MAP_STILL_LOOKING. If none of the lists is at this state, then
     * there is no need to keep looking
     */
    return m_map_set->get_state() == kui_tree::kui_tree_state::MATCHING;
}

/**
 * Checks to see if a kui map has been found.
 *
 * @return
 * The map found, or nullptr if none was found.
 */
kui_map *kuictx::get_found_map() const
{
    /* If the kui context's map set has the value FOUND,
     * than a map was found, and it should be the value used.
     *
     * Otherwise, no map is found.
     */

    if (m_map_set->get_state() == kui_tree::kui_tree_state::FOUND) {
        return m_map_set->get_data();
    }

    return nullptr;
}

/**
 * Get's the next char.
 *
 * \param ws_map_found
 * returns as 0 if no map was found. In this case, the return value is valid.
 * returns as 1 if a mapping was found. In this case the return value is not 
 * valid.
 *
 * @return
 * -1 on error
 * The key on success ( valid if map_found == 0 )
 */
int kuictx::findkey(int& was_map_found)
{
    int key, retval;
    int map_found;

    /* Initialize variables on stack */
    key = -1;
    was_map_found = 0;

    if (!m_map_set) {
        retval = findchar(key);
        if (retval == -1) {
            return -1;
        } else {
            return key;
        }
    }

    m_volatile_buffer.clear();
    m_map_set->reset_state();

    /* Start the main loop */
    while (1) {
        retval = findchar(key);
        if (retval == -1)
            return -1;

        /* If there is no more data ready, stop. */
        if (retval == 0)
            break;

        m_volatile_buffer.push_front(key);

        /* Update each list, with the character read, and the position. */
        if (update_map_set(key, map_found) == -1)
            return -1;

        /* Check to see if at least a single map is being matched */
        if (!should_continue_looking())
            break;
    }

    /* This should no longer be used. Enforcing that. */
    key = 0;

    /* All done looking for chars, let lists that matched a mapping
     * be known. ex KUI_MAP_STILL_LOOKING => KUI_MAP_FOUND. This 
     * happens when 
     *    map abc   xyz
     *    map abcde xyz
     *
     * If the user types abcd, the list will still be looking,
     * even though it already found a mapping.
     */
    m_map_set->finalize_state();

    /* Update the buffer and get the final char. */
    kui_map *map = get_found_map();
    was_map_found = map != nullptr;
    update_buffer(map, &key);
    return key;
}

int kuictx::getkey()
{
    int map_found;
    int key;

    /* If a map was found, restart the algorithm. */
    do {
        key = findkey(map_found);

        if (key == -1)
            return -1;

    } while (map_found == 1);

    return key;
}
