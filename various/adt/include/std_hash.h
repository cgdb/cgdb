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

#ifndef __STD_HASH_H__
#define __STD_HASH_H__

#include <stdlib.h>
#include "std_types.h"

/**
 * Associations between keys and values so that given a key the value can be 
 * found quickly.
 */
struct std_hashtable;

typedef int (*STDHRFunc) (void *key, void *value, void *user_data);

/**
 * Creates a new hash table.
 *
 * \param hash_func
 * A function to create a hash value from a key. 
 * 
 * Hash values are used to determine where keys are stored within the hash 
 * table data structure. The std_direct_hash(), std_int_hash() and 
 * std_str_hash() functions are provided for some common types of keys. If 
 * hash_func is NULL, std_direct_hash() is used.
 *
 * \param key_equal_func
 * A function to check two keys for equality. 
 *
 * This is used when looking up keys in the hash table. 
 * The std_direct_equal(), std_int_equal() and std_str_equal() functions are 
 * provided for the most common types of keys. If key_equal_func is NULL, keys 
 * are compared directly in a similar fashion to std_direct_equal(), but 
 * without the overhead of a function call.
 *
 * @return
 * A new hash table
 */
struct std_hashtable *std_hash_table_new(STDHashFunc hash_func,
        STDEqualFunc key_equal_func);

/**
 * Creates a new hash table like std_hash_table_new() and allows to specify 
 * functions to free the memory allocated for the key and value that get 
 * called when removing the entry from the hash table.
 *
 * \param hash_func
 * A function to create a hash value from a key
 *
 * \param key_equal_func
 * A function to check two keys for equality.
 *
 * \param key_destroy_func
 * A function to free the memory allocated for the key used when removing 
 * the entry from the hash table or NULL if you don't want to supply such 
 * a function.
 *
 * \param value_destroy_func
 * A function to free the memory allocated for the value used when removing 
 * the entry from the hash table or NULL if you don't want to supply such a 
 * function.
 *
 * @return
 * A new hash table
 */
struct std_hashtable *std_hash_table_new_full(STDHashFunc hash_func,
        STDEqualFunc key_equal_func,
        STDDestroyNotify key_destroy_func, STDDestroyNotify value_destroy_func);

/**
 * Destroys the hash table. If keys and/or values are dynamically allocated, 
 * you should either free them first or create the hash table using 
 * std_hash_table_new_full(). In the latter case the destroy functions you 
 * supplied will be called on all keys and values before destroying the 
 * hash table.
 *
 * \param hash_table
 * The hash table to destroy
 */
void std_hash_table_destroy(struct std_hashtable *hash_table);

/**
 * Inserts a new key and value into a hash table.
 *
 * If the key already exists in the hash table its current value is replaced 
 * with the new value. If you supplied a value_destroy_func when creating the 
 * hash table, the old value is freed using that function. If you supplied a 
 * key_destroy_func when creating the hash table, the passed key is freed 
 * using that function.
 *
 * \param hash_table
 * The hash table to insert into
 *
 * \param key
 * A key to insert
 *
 * \param value
 * The value to associate with the key.
 */
void std_hash_table_insert(struct std_hashtable *hash_table,
        void *key, void *value);

/**
 * Inserts a new key and value into a hash table similar to 
 * std_hash_table_insert(). The difference is that if the key already exists 
 * in the hash table, it gets replaced by the new key. If you supplied a 
 * value_destroy_func when creating the hash table, the old value is freed 
 * using that function. If you supplied a key_destroy_func when creating the 
 * hash table, the old key is freed using that function.
 *
 * \param hash_table
 * The hash table to insert into
 *
 * \param key
 * A key to insert
 *
 * \param value
 * The value to associate with the key.
 */
void std_hash_table_replace(struct std_hashtable *hash_table,
        void *key, void *value);

/**
 * Removes a key and its associated value from a hash table.
 *
 * If the hash table was created using std_hash_table_new_full(), the key and 
 * value are freed using the supplied destroy functions, otherwise you have to 
 * make sure that any dynamically allocated values are freed yourself.
 *
 * \param hash_table
 * The hash table to remove from.
 *
 * \param key
 * The key to remove
 *
 * @return
 * 1 if the key was found and removed from the hash table. 
 */
int std_hash_table_remove(struct std_hashtable *hash_table, const void *key);

/**
 * Removes a key and its associated value from a hash table without calling 
 * the key and value destroy functions.
 *
 * \param hash_table
 * The hash to steal from
 *
 * \param key
 * The key to remove
 *
 * @return
 * 1 if the key was found and removed from the hash table. 
 */
int std_hash_table_steal(struct std_hashtable *hash_table, const void *key);

/**
 * Looks up a key in a hash table.
 *
 * \param hash_table
 * The hash table to look a key up in
 *
 * \param key
 * The key to lookup
 *
 * @return
 * The associated value, or NULL if the key is not found.
 */
void *std_hash_table_lookup(struct std_hashtable *hash_table, const void *key);

/**
 * Looks up a key in the hash table, returning the original key and the 
 * associated value and a int which is 1 if the key was found. This is 
 * useful if you need to free the memory allocated for the original key, 
 * for example before calling std_hash_table_remove().
 *
 * \param hash_table
 * The hash to lookup data in
 *
 * \param lookup_key
 * The key to look up.
 *
 * \param oristd_key
 * returns the original key. 
 *
 * \param value
 * returns the value associated with the key.
 *
 * @return
 * 1 if the key was found in the hash table.
 */
int std_hash_table_lookup_extended(struct std_hashtable *hash_table,
        const void *lookup_key, void **oristd_key, void **value);

/**
 * Calls the given function for each of the key/value pairs in the hash table. 
 * The function is passed the key and value of each pair, and the given 
 * user_data parameter. The hash table may not be modified while iterating 
 * over it (you can't add/remove items). 
 *
 * To remove all items matching a predicate, use std_hash_table_remove().
 *
 * \param hash_table
 * The hash to iterate over
 *
 * \param func
 * The function to call for each key/value pair.
 *
 * \param user_data
 * user data to pass to the function.
 */
void std_hash_table_foreach(struct std_hashtable *hash_table,
        STDHFunc func, void *user_data);

/**
 * Calls the given function for key/value pairs in the hash table until 
 * predicate returns 1. The function is passed the key and value of each pair, 
 * and the given user_data parameter. The hash table may not be modified 
 * while iterating over it (you can't add/remove items).
 *
 * \param hash_table
 * The hash table
 *
 * \param predicate
 * function to test the key/value pairs for a certain property.
 *
 * \param user_data
 * user data to pass to the function.
 *
 * @return
 * The value of the first key/value pair is returned, for which func 
 * evaluates to 1. If no pair with the requested property is found, 
 * NULL is returned.
 */
void *std_hash_table_find(struct std_hashtable *hash_table,
        STDHRFunc predicate, void *user_data);

/**
 * Calls the given function for each key/value pair in the hash table. If the 
 * function returns 1, then the key/value pair is removed from the hash table. 
 * If you supplied key or value destroy functions when creating the hash table, 
 * they are used to free the memory allocated for the removed keys and values.
 *
 * \param hash_table
 * The hash table
 *
 * \param func
 * The function to call for each key/value pair.
 *
 * \param user_data
 * user data to pass to the function.
 *
 * @return
 * The number of key/value pairs removed.
 */
unsigned int std_hash_table_foreach_remove(struct std_hashtable *hash_table,
        STDHRFunc func, void *user_data);

/**
 * Calls the given function for each key/value pair in the hash table. If the 
 * function returns 1, then the key/value pair is removed from the hash table, 
 * but no key or value destroy functions are called.
 *
 * \param hash_table
 * The hash table
 *
 * \param func
 * The function to call for each key/value pair.
 *
 * \param user_data
 * user data to pass to the function.
 *
 * @return
 * the number of key/value pairs removed.
 */
unsigned int std_hash_table_foreach_steal(struct std_hashtable *hash_table,
        STDHRFunc func, void *user_data);

/**
 * Returns the number of elements contained in the hash table.
 *
 * \param hash_table
 * The hash table
 *
 * @return
 * The number of key/value pairs in the hash table.
 */
unsigned int std_hash_table_size(struct std_hashtable *hash_table);

/* 
 * Some standard hash functions 
 * TODO: These are unimplemented. Please implement them when needed.
 */

int std_str_equal(const void *v, const void *v2);
unsigned int std_str_hash(const void *v);
int std_int_equal(const void *v, const void *v2);
unsigned int std_int_hash(const void *v);

/** 
 * This "hash" function will just return the key's address as an
 * unsigned integer. Useful for hashing on plain addresses or
 * simple integer values.
 *
 * Passing NULL into std_hash_table_new() as STDHashFunc has the
 * same effect as passing std_direct_hash().
 *
 * Converts a void* to a hash value. It can be passed to std_hash_table_new() 
 * as the hash_func parameter, when using pointers as keys in a hash table.
 *
 * \param v
 * a void* key
 *
 * @return
 * A hash value corresponding to the key.
 */
unsigned int std_direct_hash(const void *v);

/**
 * Compares two void* arguments and returns 1 if they are equal. It can be 
 * passed to std_hash_table_new() as the key_equal_func parameter, when 
 * using pointers as keys in a hash table.
 *
 * \param v
 * a key
 *
 * \param v2
 * a key to compare with
 *
 * @return
 * 1 if the two keys match
 */
int std_direct_equal(const void *v, const void *v2);

#endif /* __STD_HASH_H__ */
