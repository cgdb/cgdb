#include "kui_map_set.h"
#include "kui_term.h"

int kui_map_print_cgdb_key_array(kui_map *map)
{
    if (!map)
        return -1;

    if (kui_term_print_key_array(map->get_literal_value()) == -1)
        return -1;

    return 0;

}

bool kui_map_set::register_map(const char *key_data, const char *value_data)
{
    auto kmap = kui_map::create(key_data, value_data);
    if (!kmap)
        return false;

    auto iter = maps.find(key_data);
    if (iter != maps.end()) {
        maps.erase(iter);
    }

    maps[key_data] = kmap;

    ktree.insert(kmap->get_literal_key(), kmap.get());

    return true;
}

bool kui_map_set::deregister_map(const char *key)
{
    auto iter = maps.find(key);
    if (iter == maps.end()) {
        return false;
    }

    /* Delete from the tree */
    ktree.erase(iter->second->get_literal_key());
    maps.erase(iter);

    return true;

}
