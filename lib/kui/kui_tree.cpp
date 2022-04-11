#include <string>
#include <map>

#include "kui_tree.h"

class kui_tree_node
{
public:
    static node_ptr_type create()
    {
        struct expose_ctor : public kui_tree_node {};
        return std::make_shared<expose_ctor>();
    }

    /* The keyboard key this node represents. */
    int key;

    /* If non-null, this node represents the macro value reached.
     * Otherwise, this node is not the end of the macro.
     */
    kui_map *macro_value;

    /**
     * The children macros from this nodes perspective.
     */
    std::map<int, node_ptr_type> children;

protected:
    kui_tree_node() : key(0), macro_value(NULL) {}

private:
    kui_tree_node(const kui_tree_node &) = delete;
    kui_tree_node(kui_tree_node &&) = delete;
    kui_tree_node &operator=(const kui_tree_node &) = delete;
};

kui_tree::kui_tree()
    : root(kui_tree_node::create())
{}

void kui_tree::insert(int *klist, kui_map *data)
{
    insert(root, klist, data);
}

/**
 * Inserts a map into the tree.
 *
 * @param node
 * The node to insert the map into
 *
 * @param klist
 * The key sequence that will trigger the key mapping substitution
 * The last item in the list will have a 0 value
 *
 * @param data
 * The substitution that should take place when the key mapping is reached.
 */
void kui_tree::insert(node_ptr_type node, int *klist, kui_map *data)
{
    // If at the end of the list, done
    if (klist[0] == 0) {
        node->macro_value = data;
    } else {
        node_ptr_type new_node;
        auto iter = node->children.find(klist[0]);

        if (iter == node->children.end()) {
            new_node = kui_tree_node::create();
            new_node->key = klist[0];

            node->children.emplace(klist[0], new_node);
        } else {
            new_node = iter->second;
        }

        insert(new_node, &klist[1], data);
    }
}

void kui_tree::erase(int *klist)
{
    erase(root, klist);
}

/**
 * Delete a map from the tree.
 *
 * @param node
 * The node to remove the map from
 *
 * @param klist
 * The key sequence to delete.
 * The last item in the list will have a 0 value
 */
void kui_tree::erase(node_ptr_type node, int *klist)
{
    auto iter = node->children.find(klist[0]);
    if (iter != node->children.end()) {
        // Delete the requested mapping
        if (klist[1] == 0) {
            iter->second->macro_value = NULL;

            // The found node (iter->second) is the leaf node
            // for this mapping. However, other mappings may be require
            // this node to continue existing. For example,
            //   imap u1 a
            //   imap u12 b
            //   iunmap u1
            // When unmapping u1, you'll want to keep the node for 1
            // because the u12 mapping still requires it.
            if (iter->second->children.size() == 0) {
                node->children.erase(iter);
            }
        } else {
            erase(iter->second, &klist[1]);
            if (iter->second->children.size() == 0 &&
                iter->second->macro_value == NULL) {
                node->children.erase(iter);
            }
        }
    }
}

kui_tree::kui_tree_state kui_tree::get_state() const
{
    return state;
}

void kui_tree::reset_state()
{
    cur = root;
    state = kui_tree_state::MATCHING;
    found = 0;
    found_node = NULL;
}

void kui_tree::finalize_state()
{
    if (found)
        state = kui_tree_state::FOUND;
}

kui_map *kui_tree::get_data() const
{
    return found ? found_node->macro_value : nullptr;
}

bool kui_tree::push_key(int key, int *map_found)
{
    *map_found = 0;

    if (state != kui_tree_state::MATCHING)
        return false;

    /* Check to see if this key matches */
    auto iter = cur->children.find(key);

    /* Not found */
    if (iter == cur->children.end()) {
        state = kui_tree_state::NOT_FOUND;
        cur = NULL;
    } else {
        cur = iter->second;

        if (iter->second->children.size() == 0) {
            state = kui_tree_state::FOUND;
        }

        if (iter->second->macro_value) {
            found = 1;
            found_node = iter->second;
            *map_found = 1;
        }
    }

    return true;
}

/* }}} */
