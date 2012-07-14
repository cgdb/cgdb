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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "std_hash.h"

#define HASH_TABLE_MIN_SIZE 11
#define HASH_TABLE_MAX_SIZE 13845163

struct ghashnode {
    void *key;
    void *value;
    struct ghashnode *next;
};

struct std_hashtable {
    int size;
    int nnodes;
    struct ghashnode **nodes;
    STDHashFunc hash_func;
    STDEqualFunc key_equal_func;
    STDDestroyNotify key_destroy_func;
    STDDestroyNotify value_destroy_func;
};

static const unsigned int std_primes[] = {
    11,
    19,
    37,
    73,
    109,
    163,
    251,
    367,
    557,
    823,
    1237,
    1861,
    2777,
    4177,
    6247,
    9371,
    14057,
    21089,
    31627,
    47431,
    71143,
    106721,
    160073,
    240101,
    360163,
    540217,
    810343,
    1215497,
    1823231,
    2734867,
    4102283,
    6153409,
    9230113,
    13845163,
};

static const unsigned int std_nprimes =
        sizeof (std_primes) / sizeof (std_primes[0]);

unsigned int std_spaced_primes_closest(unsigned int num)
{
    int i;

    for (i = 0; i < std_nprimes; i++)
        if (std_primes[i] > num)
            return std_primes[i];

    return std_primes[std_nprimes - 1];
}

#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

static void std_hash_table_resize(struct std_hashtable *hash_table);

void G_HASH_TABLE_RESIZE(struct std_hashtable *hash_table)
{
    if ((hash_table->size >= 3 * hash_table->nnodes &&
                    hash_table->size > HASH_TABLE_MIN_SIZE) ||
            (3 * hash_table->size <= hash_table->nnodes &&
                    hash_table->size < HASH_TABLE_MAX_SIZE))
        std_hash_table_resize(hash_table);
}

static struct ghashnode **std_hash_table_lookup_node(struct std_hashtable
        *hash_table, const void *key);
static struct ghashnode *std_hash_node_new(void *key, void *value);
static void std_hash_node_destroy(struct ghashnode *hash_node,
        STDDestroyNotify key_destroy_func, STDDestroyNotify value_destroy_func);
static int
std_hash_nodes_destroy(struct ghashnode *hash_node,
        STDFreeFunc key_destroy_func, STDFreeFunc value_destroy_func);
static unsigned int std_hash_table_foreach_remove_or_steal(struct std_hashtable
        *hash_table, STDHRFunc func, void *user_data, int notify);

/**
 * std_hash_table_new:
 * @hash_func: a function to create a hash value from a key.
 *   Hash values are used to determine where keys are stored within the
 *   #struct std_hashtable data structure. The std_direct_hash(), std_int_hash() and 
 *   std_str_hash() functions are provided for some common types of keys. 
 *   If hash_func is %NULL, std_direct_hash() is used.
 * @key_equal_func: a function to check two keys for equality.  This is
 *   used when looking up keys in the #struct std_hashtable.  The std_direct_equal(),
 *   std_int_equal() and std_str_equal() functions are provided for the most
 *   common types of keys. If @key_equal_func is %NULL, keys are compared
 *   directly in a similar fashion to std_direct_equal(), but without the
 *   overhead of a function call.
 *
 * Creates a new #struct std_hashtable.
 * 
 * Return value: a new #struct std_hashtable.
 **/
struct std_hashtable *std_hash_table_new(STDHashFunc hash_func,
        STDEqualFunc key_equal_func)
{
    return std_hash_table_new_full(hash_func, key_equal_func, NULL, NULL);
}

/**
 * std_hash_table_new_full:
 * @hash_func: a function to create a hash value from a key.
 * @key_equal_func: a function to check two keys for equality.
 * @key_destroy_func: a function to free the memory allocated for the key 
 *   used when removing the entry from the #struct std_hashtable or %NULL if you 
 *   don't want to supply such a function.
 * @value_destroy_func: a function to free the memory allocated for the 
 *   value used when removing the entry from the #struct std_hashtable or %NULL if 
 *   you don't want to supply such a function.
 * 
 * Creates a new #struct std_hashtable like std_hash_table_new() and allows to specify
 * functions to free the memory allocated for the key and value that get 
 * called when removing the entry from the #struct std_hashtable.
 * 
 * Return value: a new #struct std_hashtable.
 **/
struct std_hashtable *std_hash_table_new_full(STDHashFunc hash_func,
        STDEqualFunc key_equal_func,
        STDDestroyNotify key_destroy_func, STDDestroyNotify value_destroy_func)
{
    struct std_hashtable *hash_table;
    unsigned int i;

    hash_table = malloc(sizeof (struct std_hashtable));
    hash_table->size = HASH_TABLE_MIN_SIZE;
    hash_table->nnodes = 0;
    hash_table->hash_func = hash_func ? hash_func : std_direct_hash;
    hash_table->key_equal_func = key_equal_func;
    hash_table->key_destroy_func = key_destroy_func;
    hash_table->value_destroy_func = value_destroy_func;
    hash_table->nodes = malloc(sizeof (struct ghashnode *) * hash_table->size);

    for (i = 0; i < hash_table->size; i++)
        hash_table->nodes[i] = NULL;

    return hash_table;
}

/**
 * std_hash_table_destroy:
 * @hash_table: a #struct std_hashtable.
 * 
 * Destroys the #struct std_hashtable. If keys and/or values are dynamically 
 * allocated, you should either free them first or create the #struct std_hashtable
 * using std_hash_table_new_full(). In the latter case the destroy functions 
 * you supplied will be called on all keys and values before destroying 
 * the #struct std_hashtable.
 **/
void std_hash_table_destroy(struct std_hashtable *hash_table)
{
    unsigned int i;

    if (!hash_table)
        return;

    for (i = 0; i < hash_table->size; i++)
        std_hash_nodes_destroy(hash_table->nodes[i],
                hash_table->key_destroy_func, hash_table->value_destroy_func);

    free(hash_table->nodes);
    free(hash_table);
}

static struct ghashnode **std_hash_table_lookup_node(struct std_hashtable
        *hash_table, const void *key)
{
    struct ghashnode **node;

    node = &hash_table->nodes
            [(*hash_table->hash_func) (key) % hash_table->size];

    /* Hash table lookup needs to be fast.
     *  We therefore remove the extra conditional of testing
     *  whether to call the key_equal_func or not from
     *  the inner loop.
     */
    if (hash_table->key_equal_func)
        while (*node && !(*hash_table->key_equal_func) ((*node)->key, key))
            node = &(*node)->next;
    else
        while (*node && (*node)->key != key)
            node = &(*node)->next;

    return node;
}

/**
 * std_hash_table_lookup:
 * @hash_table: a #struct std_hashtable.
 * @key: the key to look up.
 * 
 * Looks up a key in a #struct std_hashtable.
 * 
 * Return value: the associated value, or %NULL if the key is not found.
 **/
void *std_hash_table_lookup(struct std_hashtable *hash_table, const void *key)
{
    struct ghashnode *node;

    if (!hash_table)
        return NULL;

    node = *std_hash_table_lookup_node(hash_table, key);

    return node ? node->value : NULL;
}

/**
 * std_hash_table_lookup_extended:
 * @hash_table: a #struct std_hashtable.
 * @lookup_key: the key to look up.
 * @oristd_key: returns the original key.
 * @value: returns the value associated with the key.
 * 
 * Looks up a key in the #struct std_hashtable, returning the original key and the
 * associated value and a #int which is %TRUE if the key was found. This 
 * is useful if you need to free the memory allocated for the original key, 
 * for example before calling std_hash_table_remove().
 * 
 * Return value: %TRUE if the key was found in the #struct std_hashtable.
 **/
int
std_hash_table_lookup_extended(struct std_hashtable *hash_table,
        const void *lookup_key, void * *oristd_key, void * *value)
{
    struct ghashnode *node;

    if (!hash_table)
        return 0;

    node = *std_hash_table_lookup_node(hash_table, lookup_key);

    if (node) {
        if (oristd_key)
            *oristd_key = node->key;
        if (value)
            *value = node->value;
        return 1;
    } else
        return 0;
}

/**
 * std_hash_table_insert:
 * @hash_table: a #struct std_hashtable.
 * @key: a key to insert.
 * @value: the value to associate with the key.
 * 
 * Inserts a new key and value into a #struct std_hashtable.
 * 
 * If the key already exists in the #struct std_hashtable its current value is replaced
 * with the new value. If you supplied a @value_destroy_func when creating the 
 * #struct std_hashtable, the old value is freed using that function. If you supplied
 * a @key_destroy_func when creating the #struct std_hashtable, the passed key is freed 
 * using that function.
 **/
void
std_hash_table_insert(struct std_hashtable *hash_table, void *key, void *value)
{
    struct ghashnode **node;

    if (!hash_table)
        return;

    node = std_hash_table_lookup_node(hash_table, key);

    if (*node) {
        /* do not reset node->key in this place, keeping
         * the old key is the intended behaviour. 
         * std_hash_table_replace() can be used instead.
         */

        /* free the passed key */
        if (hash_table->key_destroy_func)
            hash_table->key_destroy_func(key);

        if (hash_table->value_destroy_func)
            hash_table->value_destroy_func((*node)->value);

        (*node)->value = value;
    } else {
        *node = std_hash_node_new(key, value);
        hash_table->nnodes++;
        G_HASH_TABLE_RESIZE(hash_table);
    }
}

/**
 * std_hash_table_replace:
 * @hash_table: a #struct std_hashtable.
 * @key: a key to insert.
 * @value: the value to associate with the key.
 * 
 * Inserts a new key and value into a #struct std_hashtable similar to 
 * std_hash_table_insert(). The difference is that if the key already exists 
 * in the #struct std_hashtable, it gets replaced by the new key. If you supplied a 
 * @value_destroy_func when creating the #struct std_hashtable, the old value is freed 
 * using that function. If you supplied a @key_destroy_func when creating the 
 * #struct std_hashtable, the old key is freed using that function. 
 **/
void
std_hash_table_replace(struct std_hashtable *hash_table, void *key, void *value)
{
    struct ghashnode **node;

    if (!hash_table)
        return;

    node = std_hash_table_lookup_node(hash_table, key);

    if (*node) {
        if (hash_table->key_destroy_func)
            hash_table->key_destroy_func((*node)->key);

        if (hash_table->value_destroy_func)
            hash_table->value_destroy_func((*node)->value);

        (*node)->key = key;
        (*node)->value = value;
    } else {
        *node = std_hash_node_new(key, value);
        hash_table->nnodes++;
        G_HASH_TABLE_RESIZE(hash_table);
    }
}

/**
 * std_hash_table_remove:
 * @hash_table: a #struct std_hashtable.
 * @key: the key to remove.
 * 
 * Removes a key and its associated value from a #struct std_hashtable.
 *
 * If the #struct std_hashtable was created using std_hash_table_new_full(), the
 * key and value are freed using the supplied destroy functions, otherwise
 * you have to make sure that any dynamically allocated values are freed 
 * yourself.
 * 
 * Return value: %TRUE if the key was found and removed from the #struct std_hashtable.
 **/
int std_hash_table_remove(struct std_hashtable *hash_table, const void *key)
{
    struct ghashnode **node, *dest;

    if (!hash_table)
        return 0;

    node = std_hash_table_lookup_node(hash_table, key);
    if (*node) {
        dest = *node;
        (*node) = dest->next;
        std_hash_node_destroy(dest,
                hash_table->key_destroy_func, hash_table->value_destroy_func);
        hash_table->nnodes--;

        G_HASH_TABLE_RESIZE(hash_table);

        return 1;
    }

    return 0;
}

/**
 * std_hash_table_steal:
 * @hash_table: a #struct std_hashtable.
 * @key: the key to remove.
 * 
 * Removes a key and its associated value from a #struct std_hashtable without
 * calling the key and value destroy functions.
 *
 * Return value: %TRUE if the key was found and removed from the #struct std_hashtable.
 **/
int std_hash_table_steal(struct std_hashtable *hash_table, const void *key)
{
    struct ghashnode **node, *dest;

    if (!hash_table)
        return 0;

    node = std_hash_table_lookup_node(hash_table, key);
    if (*node) {
        dest = *node;
        (*node) = dest->next;
        std_hash_node_destroy(dest, NULL, NULL);
        hash_table->nnodes--;

        G_HASH_TABLE_RESIZE(hash_table);

        return 1;
    }

    return 0;
}

/**
 * std_hash_table_foreach_remove:
 * @hash_table: a #struct std_hashtable.
 * @func: the function to call for each key/value pair.
 * @user_data: user data to pass to the function.
 * 
 * Calls the given function for each key/value pair in the #struct std_hashtable.
 * If the function returns %TRUE, then the key/value pair is removed from the
 * #struct std_hashtable. If you supplied key or value destroy functions when creating
 * the #struct std_hashtable, they are used to free the memory allocated for the removed
 * keys and values.
 * 
 * Return value: the number of key/value pairs removed.
 **/
unsigned int
std_hash_table_foreach_remove(struct std_hashtable *hash_table,
        STDHRFunc func, void *user_data)
{
    if (!hash_table)
        return 0;

    if (!func)
        return 0;

    return std_hash_table_foreach_remove_or_steal(hash_table, func, user_data,
            1);
}

/**
 * std_hash_table_foreach_steal:
 * @hash_table: a #struct std_hashtable.
 * @func: the function to call for each key/value pair.
 * @user_data: user data to pass to the function.
 * 
 * Calls the given function for each key/value pair in the #struct std_hashtable.
 * If the function returns %TRUE, then the key/value pair is removed from the
 * #struct std_hashtable, but no key or value destroy functions are called.
 * 
 * Return value: the number of key/value pairs removed.
 **/
unsigned int
std_hash_table_foreach_steal(struct std_hashtable *hash_table,
        STDHRFunc func, void *user_data)
{
    if (!hash_table)
        return 0;
    if (!func)
        return 0;

    return std_hash_table_foreach_remove_or_steal(hash_table, func, user_data,
            0);
}

static unsigned int
std_hash_table_foreach_remove_or_steal(struct std_hashtable *hash_table,
        STDHRFunc func, void *user_data, int notify)
{
    struct ghashnode *node, *prev;
    unsigned int i;
    unsigned int deleted = 0;

    for (i = 0; i < hash_table->size; i++) {
      restart:

        prev = NULL;

        for (node = hash_table->nodes[i]; node; prev = node, node = node->next) {
            if ((*func) (node->key, node->value, user_data)) {
                deleted += 1;

                hash_table->nnodes -= 1;

                if (prev) {
                    prev->next = node->next;
                    std_hash_node_destroy(node,
                            notify ? hash_table->key_destroy_func : NULL,
                            notify ? hash_table->value_destroy_func : NULL);
                    node = prev;
                } else {
                    hash_table->nodes[i] = node->next;
                    std_hash_node_destroy(node,
                            notify ? hash_table->key_destroy_func : NULL,
                            notify ? hash_table->value_destroy_func : NULL);
                    goto restart;
                }
            }
        }
    }

    G_HASH_TABLE_RESIZE(hash_table);

    return deleted;
}

/**
 * std_hash_table_foreach:
 * @hash_table: a #struct std_hashtable.
 * @func: the function to call for each key/value pair.
 * @user_data: user data to pass to the function.
 * 
 * Calls the given function for each of the key/value pairs in the
 * #struct std_hashtable.  The function is passed the key and value of each
 * pair, and the given @user_data parameter.  The hash table may not
 * be modified while iterating over it (you can't add/remove
 * items). To remove all items matching a predicate, use
 * std_hash_table_remove().
 **/
void
std_hash_table_foreach(struct std_hashtable *hash_table,
        STDHFunc func, void *user_data)
{
    struct ghashnode *node;
    int i;

    if (!hash_table)
        return;

    if (!func)
        return;

    for (i = 0; i < hash_table->size; i++)
        for (node = hash_table->nodes[i]; node; node = node->next)
            (*func) (node->key, node->value, user_data);
}

/**
 * std_hash_table_find:
 * @hash_table: a #struct std_hashtable.
 * @predicate:  function to test the key/value pairs for a certain property.
 * @user_data:  user data to pass to the function.
 * 
 * Calls the given function for key/value pairs in the #struct std_hashtable until 
 * @predicate returns %TRUE.  The function is passed the key and value of 
 * each pair, and the given @user_data parameter. The hash table may not
 * be modified while iterating over it (you can't add/remove items). 
 *
 * Return value: The value of the first key/value pair is returned, for which 
 * func evaluates to %TRUE. If no pair with the requested property is found, 
 * %NULL is returned.
 *
 * Since: 2.4
 **/
void *std_hash_table_find(struct std_hashtable *hash_table,
        STDHRFunc predicate, void *user_data)
{
    struct ghashnode *node;
    int i;

    if (!hash_table)
        return NULL;

    if (!predicate)
        return NULL;

    for (i = 0; i < hash_table->size; i++)
        for (node = hash_table->nodes[i]; node; node = node->next)
            if (predicate(node->key, node->value, user_data))
                return node->value;
    return NULL;
}

/**
 * std_hash_table_size:
 * @hash_table: a #struct std_hashtable.
 * 
 * Returns the number of elements contained in the #struct std_hashtable.
 * 
 * Return value: the number of key/value pairs in the #struct std_hashtable.
 **/
unsigned int std_hash_table_size(struct std_hashtable *hash_table)
{
    if (!hash_table)
        return 0;

    return hash_table->nnodes;
}

static void std_hash_table_resize(struct std_hashtable *hash_table)
{
    struct ghashnode **new_nodes;
    struct ghashnode *node;
    struct ghashnode *next;
    unsigned int hash_val;
    int new_size;
    int i;

    new_size = std_spaced_primes_closest(hash_table->nnodes);
    new_size = CLAMP(new_size, HASH_TABLE_MIN_SIZE, HASH_TABLE_MAX_SIZE);

    new_nodes = malloc(sizeof (struct ghashnode *) * new_size);

    for (i = 0; i < hash_table->size; i++)
        for (node = hash_table->nodes[i]; node; node = next) {
            next = node->next;

            hash_val = (*hash_table->hash_func) (node->key) % new_size;

            node->next = new_nodes[hash_val];
            new_nodes[hash_val] = node;
        }

    free(hash_table->nodes);
    hash_table->nodes = new_nodes;
    hash_table->size = new_size;
}

static struct ghashnode *std_hash_node_new(void *key, void *value)
{
    struct ghashnode *hash_node;

    hash_node = malloc(sizeof (struct ghashnode));

    hash_node->key = key;
    hash_node->value = value;
    hash_node->next = NULL;

    return hash_node;
}

static void
std_hash_node_destroy(struct ghashnode *hash_node,
        STDDestroyNotify key_destroy_func, STDDestroyNotify value_destroy_func)
{
    if (key_destroy_func)
        key_destroy_func(hash_node->key);
    if (value_destroy_func)
        value_destroy_func(hash_node->value);

    hash_node->key = NULL;
    hash_node->value = NULL;

    free(hash_node);
}

static int
std_hash_nodes_destroy(struct ghashnode *hash_node,
        STDFreeFunc key_destroy_func, STDFreeFunc value_destroy_func)
{
    while (hash_node) {
        struct ghashnode *next = hash_node->next;

        if (key_destroy_func)
            key_destroy_func(hash_node->key);
        if (value_destroy_func)
            value_destroy_func(hash_node->value);

        free(hash_node);
        hash_node = next;
    }

    return 0;
}

unsigned int std_direct_hash(const void *v)
{
    return (size_t) v;
}
