/* GLIB - Library of useful routines for C programming
 * Copyright (C) 1995-1997  Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <stdlib.h>
#include <stdio.h>              /* For debug only */

#include "std_bbtree.h"

#define MAX(a, b)  (((a) > (b)) ? (a) : (b))

struct std_bbtreenode;

struct std_bbtree {
    struct std_bbtreenode *root;
    STDCompareDataFunc key_compare;
    STDDestroyNotify key_destroy_func;
    STDDestroyNotify value_destroy_func;
    void *key_compare_data;
};

struct std_bbtreenode {
    int balance;                /* height (left) - height (right) */
    struct std_bbtreenode *left;    /* left subtree */
    struct std_bbtreenode *right;   /* right subtree */
    void *key;                  /* key for this node */
    void *value;                /* value stored at this node */
};

static struct std_bbtreenode *std_bbtree_node_new(void *key, void *value);

static void std_bbtree_node_destroy(struct std_bbtreenode *node,
        STDDestroyNotify key_destroy_func, STDDestroyNotify value_destroy_func);

static struct std_bbtreenode *std_bbtree_node_insert(struct std_bbtree *tree,
        struct std_bbtreenode *node,
        void *key, void *value, int replace, int *inserted);

static struct std_bbtreenode *std_bbtree_node_remove(struct std_bbtree *tree,
        struct std_bbtreenode *node, const void *key, int notify);

static struct std_bbtreenode *std_bbtree_node_balance(struct std_bbtreenode
        *node);

static struct std_bbtreenode *std_bbtree_node_remove_leftmost(struct
        std_bbtreenode *node, struct std_bbtreenode **leftmost);

static struct std_bbtreenode *std_bbtree_node_restore_left_balance(struct
        std_bbtreenode *node, int old_balance);

static struct std_bbtreenode *std_bbtree_node_restore_right_balance(struct
        std_bbtreenode *node, int old_balance);

static struct std_bbtreenode *std_bbtree_node_lookup(struct std_bbtreenode
        *node, STDCompareDataFunc compare, void *comp_data, const void *key);

static int std_bbtree_node_count(struct std_bbtreenode *node);

static int std_bbtree_node_pre_order(struct std_bbtreenode *node,
        STDTraverseFunc traverse_func, void *data);

static int std_bbtree_node_in_order(struct std_bbtreenode *node,
        STDTraverseFunc traverse_func, void *data);

static int std_bbtree_node_post_order(struct std_bbtreenode *node,
        STDTraverseFunc traverse_func, void *data);

static void *std_bbtree_node_search(struct std_bbtreenode *node,
        STDCompareFunc search_func, const void *data);

static int std_bbtree_node_height(struct std_bbtreenode *node);

static struct std_bbtreenode *std_bbtree_node_rotate_left(struct std_bbtreenode
        *node);

static struct std_bbtreenode *std_bbtree_node_rotate_right(struct std_bbtreenode
        *node);

static void std_bbtree_node_check(struct std_bbtreenode *node);

static struct std_bbtreenode *std_bbtree_node_new(void *key, void *value)
{
    struct std_bbtreenode *node;

    node = malloc(sizeof (struct std_bbtreenode));

    node->balance = 0;
    node->left = NULL;
    node->right = NULL;
    node->key = key;
    node->value = value;

    return node;
}

static void
std_bbtree_node_destroy(struct std_bbtreenode *node,
        STDDestroyNotify key_destroy_func, STDDestroyNotify value_destroy_func)
{
    if (node) {
        std_bbtree_node_destroy(node->right,
                key_destroy_func, value_destroy_func);
        std_bbtree_node_destroy(node->left,
                key_destroy_func, value_destroy_func);

        if (key_destroy_func)
            key_destroy_func(node->key);
        if (value_destroy_func)
            value_destroy_func(node->value);

        node->left = NULL;
        node->key = NULL;
        node->value = NULL;

        node->right = NULL;
        free(node);
        node = NULL;
    }
}

struct std_bbtree *std_bbtree_new(STDCompareFunc key_compare_func)
{
    if (!key_compare_func)
        return NULL;

    return std_bbtree_new_full((STDCompareDataFunc) key_compare_func, NULL,
            NULL, NULL);
}

struct std_bbtree *std_bbtree_new_with_data(STDCompareDataFunc key_compare_func,
        void *key_compare_data)
{
    if (!key_compare_func)
        return NULL;

    return std_bbtree_new_full(key_compare_func, key_compare_data, NULL, NULL);
}

struct std_bbtree *std_bbtree_new_full(STDCompareDataFunc key_compare_func,
        void *key_compare_data,
        STDDestroyNotify key_destroy_func, STDDestroyNotify value_destroy_func)
{
    struct std_bbtree *tree;

    if (!key_compare_func)
        return NULL;

    tree = malloc(sizeof (struct std_bbtree));
    tree->root = NULL;
    tree->key_compare = key_compare_func;
    tree->key_destroy_func = key_destroy_func;
    tree->value_destroy_func = value_destroy_func;
    tree->key_compare_data = key_compare_data;

    return tree;
}

int std_bbtree_destroy(struct std_bbtree *tree)
{
    if (!tree)
        return -1;

    std_bbtree_node_destroy(tree->root,
            tree->key_destroy_func, tree->value_destroy_func);

    free(tree);

    return 0;
}

int std_bbtree_insert(struct std_bbtree *tree, void *key, void *value)
{
    int inserted;

    if (!tree)
        return -1;

    inserted = 0;
    tree->root = std_bbtree_node_insert(tree,
            tree->root, key, value, 0, &inserted);
    return 0;
}

int std_bbtree_replace(struct std_bbtree *tree, void *key, void *value)
{
    int inserted;

    if (!tree)
        return -1;

    inserted = 0;
    tree->root = std_bbtree_node_insert(tree,
            tree->root, key, value, 1, &inserted);
    return 0;
}

int std_bbtree_remove(struct std_bbtree *tree, const void *key)
{
    if (!tree)
        return -1;

    tree->root = std_bbtree_node_remove(tree, tree->root, key, 1);

    return 0;
}

int std_bbtree_steal(struct std_bbtree *tree, const void *key)
{
    if (!tree)
        return -1;

    tree->root = std_bbtree_node_remove(tree, tree->root, key, 0);

    return 0;
}

void *std_bbtree_lookup(struct std_bbtree *tree, const void *key)
{
    struct std_bbtreenode *node;

    if (!tree)
        return NULL;

    node = std_bbtree_node_lookup(tree->root,
            tree->key_compare, tree->key_compare_data, key);

    return node ? node->value : NULL;
}

int
std_bbtree_lookup_extended(struct std_bbtree *tree,
        const void *lookup_key, void * *orig_key, void * *value)
{
    struct std_bbtreenode *node;

    if (!tree)
        return 0;

    node = std_bbtree_node_lookup(tree->root,
            tree->key_compare, tree->key_compare_data, lookup_key);

    if (node) {
        if (orig_key)
            *orig_key = node->key;
        if (value)
            *value = node->value;
        return 1;
    } else
        return 0;
}

int
std_bbtree_foreach(struct std_bbtree *tree,
        STDTraverseFunc func, void *user_data)
{
    if (!tree)
        return -1;

    if (!tree->root)
        return -1;

    std_bbtree_node_in_order(tree->root, func, user_data);

    return 0;
}

void *std_bbtree_search(struct std_bbtree *tree,
        STDCompareFunc search_func, const void *user_data)
{
    if (!tree)
        return NULL;

    if (tree->root)
        return std_bbtree_node_search(tree->root, search_func, user_data);
    else
        return NULL;
}

int std_bbtree_height(struct std_bbtree *tree)
{
    if (!tree)
        return 0;

    if (tree->root)
        return std_bbtree_node_height(tree->root);
    else
        return 0;
}

int std_bbtree_nnodes(struct std_bbtree *tree)
{
    if (!tree)
        return 0;

    if (tree->root)
        return std_bbtree_node_count(tree->root);
    else
        return 0;
}

static struct std_bbtreenode *std_bbtree_node_insert(struct std_bbtree *tree,
        struct std_bbtreenode *node,
        void *key, void *value, int replace, int *inserted)
{
    int old_balance;
    int cmp;

    if (!node) {
        *inserted = 1;
        return std_bbtree_node_new(key, value);
    }

    cmp = tree->key_compare(key, node->key, tree->key_compare_data);
    if (cmp == 0) {
        *inserted = 0;

        if (tree->value_destroy_func)
            tree->value_destroy_func(node->value);

        node->value = value;

        if (replace) {
            if (tree->key_destroy_func)
                tree->key_destroy_func(node->key);

            node->key = key;
        } else {
            /* free the passed key */
            if (tree->key_destroy_func)
                tree->key_destroy_func(key);
        }

        return node;
    }

    if (cmp < 0) {
        if (node->left) {
            old_balance = node->left->balance;
            node->left = std_bbtree_node_insert(tree,
                    node->left, key, value, replace, inserted);

            if ((old_balance != node->left->balance) && node->left->balance)
                node->balance -= 1;
        } else {
            *inserted = 1;
            node->left = std_bbtree_node_new(key, value);
            node->balance -= 1;
        }
    } else if (cmp > 0) {
        if (node->right) {
            old_balance = node->right->balance;
            node->right = std_bbtree_node_insert(tree,
                    node->right, key, value, replace, inserted);

            if ((old_balance != node->right->balance) && node->right->balance)
                node->balance += 1;
        } else {
            *inserted = 1;
            node->right = std_bbtree_node_new(key, value);
            node->balance += 1;
        }
    }

    if (*inserted) {
        if ((node->balance < -1) || (node->balance > 1))
            node = std_bbtree_node_balance(node);
    }

    return node;
}

static struct std_bbtreenode *std_bbtree_node_remove(struct std_bbtree *tree,
        struct std_bbtreenode *node, const void *key, int notify)
{
    struct std_bbtreenode *new_root;
    int old_balance;
    int cmp;

    if (!node)
        return NULL;

    cmp = tree->key_compare(key, node->key, tree->key_compare_data);
    if (cmp == 0) {
        struct std_bbtreenode *garbage;

        garbage = node;

        if (!node->right) {
            node = node->left;
        } else {
            old_balance = node->right->balance;
            node->right =
                    std_bbtree_node_remove_leftmost(node->right, &new_root);
            new_root->left = node->left;
            new_root->right = node->right;
            new_root->balance = node->balance;
            node = std_bbtree_node_restore_right_balance(new_root, old_balance);
        }

        if (notify) {
            if (tree->key_destroy_func)
                tree->key_destroy_func(garbage->key);
            if (tree->value_destroy_func)
                tree->value_destroy_func(garbage->value);
        }

        garbage->left = NULL;
        garbage->key = NULL;
        garbage->value = NULL;

        garbage->right = NULL;
        free(garbage);
        garbage = NULL;
    } else if (cmp < 0) {
        if (node->left) {
            old_balance = node->left->balance;
            node->left = std_bbtree_node_remove(tree, node->left, key, notify);
            node = std_bbtree_node_restore_left_balance(node, old_balance);
        }
    } else if (cmp > 0) {
        if (node->right) {
            old_balance = node->right->balance;
            node->right =
                    std_bbtree_node_remove(tree, node->right, key, notify);
            node = std_bbtree_node_restore_right_balance(node, old_balance);
        }
    }

    return node;
}

static struct std_bbtreenode *std_bbtree_node_balance(struct std_bbtreenode
        *node)
{
    if (node->balance < -1) {
        if (node->left->balance > 0)
            node->left = std_bbtree_node_rotate_left(node->left);
        node = std_bbtree_node_rotate_right(node);
    } else if (node->balance > 1) {
        if (node->right->balance < 0)
            node->right = std_bbtree_node_rotate_right(node->right);
        node = std_bbtree_node_rotate_left(node);
    }

    return node;
}

static struct std_bbtreenode *std_bbtree_node_remove_leftmost(struct
        std_bbtreenode *node, struct std_bbtreenode **leftmost)
{
    int old_balance;

    if (!node->left) {
        *leftmost = node;
        return node->right;
    }

    old_balance = node->left->balance;
    node->left = std_bbtree_node_remove_leftmost(node->left, leftmost);
    return std_bbtree_node_restore_left_balance(node, old_balance);
}

static struct std_bbtreenode *std_bbtree_node_restore_left_balance(struct
        std_bbtreenode *node, int old_balance)
{
    if (!node->left)
        node->balance += 1;
    else if ((node->left->balance != old_balance) && (node->left->balance == 0))
        node->balance += 1;

    if (node->balance > 1)
        return std_bbtree_node_balance(node);
    return node;
}

static struct std_bbtreenode *std_bbtree_node_restore_right_balance(struct
        std_bbtreenode *node, int old_balance)
{
    if (!node->right)
        node->balance -= 1;
    else if ((node->right->balance != old_balance) &&
            (node->right->balance == 0))
        node->balance -= 1;

    if (node->balance < -1)
        return std_bbtree_node_balance(node);
    return node;
}

static struct std_bbtreenode *std_bbtree_node_lookup(struct std_bbtreenode
        *node, STDCompareDataFunc compare, void *compare_data, const void *key)
{
    int cmp;

    if (!node)
        return NULL;

    cmp = (*compare) (key, node->key, compare_data);
    if (cmp == 0)
        return node;

    if (cmp < 0) {
        if (node->left)
            return std_bbtree_node_lookup(node->left, compare, compare_data,
                    key);
    } else if (cmp > 0) {
        if (node->right)
            return std_bbtree_node_lookup(node->right, compare, compare_data,
                    key);
    }

    return NULL;
}

static int std_bbtree_node_count(struct std_bbtreenode *node)
{
    int count;

    count = 1;
    if (node->left)
        count += std_bbtree_node_count(node->left);
    if (node->right)
        count += std_bbtree_node_count(node->right);

    return count;
}

static int
std_bbtree_node_pre_order(struct std_bbtreenode *node,
        STDTraverseFunc traverse_func, void *data)
{
    if ((*traverse_func) (node->key, node->value, data))
        return 1;
    if (node->left) {
        if (std_bbtree_node_pre_order(node->left, traverse_func, data))
            return 1;
    }
    if (node->right) {
        if (std_bbtree_node_pre_order(node->right, traverse_func, data))
            return 1;
    }

    return 0;
}

static int
std_bbtree_node_in_order(struct std_bbtreenode *node,
        STDTraverseFunc traverse_func, void *data)
{
    if (node->left) {
        if (std_bbtree_node_in_order(node->left, traverse_func, data))
            return 1;
    }
    if ((*traverse_func) (node->key, node->value, data))
        return 1;
    if (node->right) {
        if (std_bbtree_node_in_order(node->right, traverse_func, data))
            return 1;
    }

    return 0;
}

static int
std_bbtree_node_post_order(struct std_bbtreenode *node,
        STDTraverseFunc traverse_func, void *data)
{
    if (node->left) {
        if (std_bbtree_node_post_order(node->left, traverse_func, data))
            return 1;
    }
    if (node->right) {
        if (std_bbtree_node_post_order(node->right, traverse_func, data))
            return 1;
    }
    if ((*traverse_func) (node->key, node->value, data))
        return 1;

    return 0;
}

static void *std_bbtree_node_search(struct std_bbtreenode *node,
        STDCompareFunc search_func, const void *data)
{
    int dir;

    if (!node)
        return NULL;

    do {
        dir = (*search_func) (node->key, data);
        if (dir == 0)
            return node->value;

        if (dir < 0)
            node = node->left;
        else if (dir > 0)
            node = node->right;
    } while (node);

    return NULL;
}

static int std_bbtree_node_height(struct std_bbtreenode *node)
{
    int left_height;
    int right_height;

    if (node) {
        left_height = 0;
        right_height = 0;

        if (node->left)
            left_height = std_bbtree_node_height(node->left);

        if (node->right)
            right_height = std_bbtree_node_height(node->right);

        return MAX(left_height, right_height) + 1;
    }

    return 0;
}

static struct std_bbtreenode *std_bbtree_node_rotate_left(struct std_bbtreenode
        *node)
{
    struct std_bbtreenode *right;
    int a_bal;
    int b_bal;

    right = node->right;

    node->right = right->left;
    right->left = node;

    a_bal = node->balance;
    b_bal = right->balance;

    if (b_bal <= 0) {
        if (a_bal >= 1)
            right->balance = b_bal - 1;
        else
            right->balance = a_bal + b_bal - 2;
        node->balance = a_bal - 1;
    } else {
        if (a_bal <= b_bal)
            right->balance = a_bal - 2;
        else
            right->balance = b_bal - 1;
        node->balance = a_bal - b_bal - 1;
    }

    return right;
}

static struct std_bbtreenode *std_bbtree_node_rotate_right(struct std_bbtreenode
        *node)
{
    struct std_bbtreenode *left;
    int a_bal;
    int b_bal;

    left = node->left;

    node->left = left->right;
    left->right = node;

    a_bal = node->balance;
    b_bal = left->balance;

    if (b_bal <= 0) {
        if (b_bal > a_bal)
            left->balance = b_bal + 1;
        else
            left->balance = a_bal + 2;
        node->balance = a_bal - b_bal + 1;
    } else {
        if (a_bal <= -1)
            left->balance = b_bal + 1;
        else
            left->balance = a_bal + b_bal + 2;
        node->balance = a_bal + 1;
    }

    return left;
}

static void std_bbtree_node_check(struct std_bbtreenode *node)
{
    int left_height;
    int right_height;
    int balance;

    if (node) {
        left_height = 0;
        right_height = 0;

        if (node->left)
            left_height = std_bbtree_node_height(node->left);
        if (node->right)
            right_height = std_bbtree_node_height(node->right);

        balance = right_height - left_height;
        if (balance != node->balance)
            printf("std_bbtree_node_check: failed: %d ( %d )\n",
                    balance, node->balance);

        if (node->left)
            std_bbtree_node_check(node->left);
        if (node->right)
            std_bbtree_node_check(node->right);
    }
}
