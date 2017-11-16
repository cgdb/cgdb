#include <stdlib.h>
#include <string.h>

#include <memory>
#include <map>
#include "kui_tree.h"

/* struct kui_tree_node {{{ */

struct kui_tree_node;
typedef std::shared_ptr<kui_tree_node> KuiTreeNodeSPtr;
typedef std::map<int, KuiTreeNodeSPtr> KuiTreeNodeMap;

/**
 * A node in a kui tree.
 */
struct kui_tree_node {

    kui_tree_node() : key(0), macro_value(NULL) {}

    /* The keyboard key this node represents. */
    int key;

    /* If non-null, this node represents the macro value reached.
     * Otherwise, this node is not the end of the macro.
     */
    void *macro_value;

    /**
     * The children macros from this nodes perspective.
     */
    KuiTreeNodeMap children;

    private:
        kui_tree_node(const kui_tree_node &other);
        kui_tree_node &operator=(const kui_tree_node &other);
};

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
 *
 * @return
 * 0 on success or -1 on error
 */
static int kui_tree_node_insert(KuiTreeNodeSPtr node, int *klist, void *data)
{
    if (!node) {
        return -1;
    }

    // If at the end of the list, done
    if (klist[0] == 0) {
        node->macro_value = data;
        return 0;
    } else {
        KuiTreeNodeSPtr new_node;
        KuiTreeNodeMap::const_iterator iter = node->children.find(klist[0]);

        if (iter == node->children.end()) {
            new_node = KuiTreeNodeSPtr(new kui_tree_node());
            new_node->key = klist[0];

            node->children.insert(std::make_pair(klist[0], new_node));
        } else {
            new_node = iter->second;
        }

        return kui_tree_node_insert(new_node, &klist[1], data);
    }
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
static int kui_tree_node_delete(KuiTreeNodeSPtr node, int *klist)
{
    auto iter = node->children.find(klist[0]);
    if (iter == node->children.end()) {
        return -1;
    } else {
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
            kui_tree_node_delete(iter->second, &klist[1]);
            if (iter->second->children.size() == 0 &&
                iter->second->macro_value == NULL) {
                node->children.erase(iter);
            }
        }
    }

    return 0;
}

/* }}} */

/* struct kui_tree {{{ */

/**
 * This data structure is capable of storing a set of maps internally in such a
 * way that it is easy to see what maps are active if a char at a time is fed
 * to this structure.
 *
 * Also, it can determine what mapping was reached if one was found.
 */
struct kui_tree {

    /* The root of the tree */
    KuiTreeNodeSPtr root;

    /* The current position pointing into the tree (while looking for a map) */
    KuiTreeNodeSPtr cur;

    /* The last node found while looking for a map. */
    /* This happens because maps can be subsets of other maps. */
    KuiTreeNodeSPtr found_node;

    /* The internal state of the tree ( still looking, map found, not found ) */
    enum kui_tree_state state;
    /* If a map was found at all, this is set to 1 while looking, otherwise 0. */
    int found;
};

int kui_tree_destroy(struct kui_tree *ktree)
{
    delete ktree;
    return 0;
}

struct kui_tree *kui_tree_create(void)
{
    struct kui_tree *ktree = new kui_tree();;
    ktree->root = KuiTreeNodeSPtr(new kui_tree_node());

    return ktree;
}

int kui_tree_insert(struct kui_tree *ktree, int *klist, void *data)
{
    return kui_tree_node_insert(ktree->root, klist, data);
}

int kui_tree_delete(struct kui_tree *ktree, int *klist)
{
    kui_tree_node_delete(ktree->root, klist);
    return 0;
}

int kui_tree_reset_state(struct kui_tree *ktree)
{
    if (!ktree)
        return -1;

    ktree->cur = ktree->root;
    ktree->state = KUI_TREE_MATCHING;
    ktree->found = 0;
    ktree->found_node = NULL;

    return 0;
}

int kui_tree_finalize_state(struct kui_tree *ktree)
{
    if (!ktree)
        return -1;

    if (ktree->found)
        ktree->state = KUI_TREE_FOUND;

    return 0;
}

int kui_tree_get_state(struct kui_tree *ktree, enum kui_tree_state *state)
{
    if (!ktree)
        return -1;

    *state = ktree->state;

    return 0;
}

int kui_tree_get_data(struct kui_tree *ktree, void *data)
{
    if (!ktree)
        return -1;

    if (!ktree->found)
        return -1;

    memcpy(data, &ktree->found_node->macro_value, sizeof (void *));

    return 0;
}

int kui_tree_push_key(struct kui_tree *ktree, int key, int *map_found)
{
    *map_found = 0;

    if (ktree->state != KUI_TREE_MATCHING)
        return -1;

    /* Check to see if this key matches */
    auto iter = ktree->cur->children.find(key);

    /* Not found */
    if (iter == ktree->cur->children.end()) {
        ktree->state = KUI_TREE_NOT_FOUND;
        ktree->cur = NULL;
    } else {
        ktree->cur = iter->second;

        if (iter->second->children.size() == 0) {
            ktree->state = KUI_TREE_FOUND;
        }

        if (iter->second->macro_value) {
            ktree->found = 1;
            ktree->found_node = iter->second;
            *map_found = 1;
        }
    }

    return 0;
}

/* }}} */
