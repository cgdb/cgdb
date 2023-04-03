#ifndef __KUI_MAP_SET_H__
#define __KUI_MAP_SET_H__

#include "kui_tree.h"
#include "kui_map.h"

#include <map>
#include <string>

/* class kui_map_set {{{ */

/* Kui map set */

/** 
 * This maintains a list of maps. It also is capable of tracking if a user is 
 * matching any map in this set with the current key strokes being typed.
 */
class kui_map_set {

public:
    /**
     * Add a map to the map set.
     *
     * If a map already exists with this key in the map set,
     * it will be deleted and replaced by the new requested mapping.
     *
     * \param key
     * A key. Should be null terminated.
     *
     * \param value
     * A value. Should be null terminated.
     *
     * @return
     * true  on success, or false on error
     */
    bool register_map(const char *key_data, const char *value_data);

    /**
     * Remove a map from the map set.
     *
     * \param key
     * A key. Should be null terminated.
     *
     * @return
     * true on success, or false if map did not exist.
     */
    bool deregister_map(const char *key);

    /**
     * {{
     * wrappers for ktree
     */
    bool push_key(int key, int *map_found)
    {
        return ktree.push_key(key, map_found);
    }

    kui_tree::kui_tree_state get_state() const
    {
        return ktree.get_state();
    }

    void reset_state()
    {
        ktree.reset_state();
    }

    void finalize_state()
    {
        ktree.finalize_state();
    }

    kui_map *get_data() const
    {
        return ktree.get_data();
    }
    /**
     * }}
     */

private:
    /* The ktree used in determining if a map has been reached or is being
     * reached. Of course, nothing could match also. This structure is
     * efficient in doing the work, it looks only at the current key read.  */
    kui_tree ktree;

    /**
     * All of the maps in this kui map set
     *
     * The key is what you type to trigger the mapping. The value
     * is the kui_map representing the mapping.
     */
    std::map<std::string, std::shared_ptr<kui_map>> maps;
};

#endif
