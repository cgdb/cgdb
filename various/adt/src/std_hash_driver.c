/* GLIB - Library of useful routines for C programming
 * Copyright (C) 1995-1997  Peter Mattis, Spencer Kimball and Josh MacDonald
 * Copyright (C) 1999 The Free Software Foundation
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "std_hash.h"

int array[10000];

static int my_hash_callback_remove(void *key, void *value, void *user_data)
{
    int *d = value;

    if ((*d) % 2)
        return 1;

    return 0;
}

static void
my_hash_callback_remove_test(void *key, void *value, void *user_data)
{
    int *d = value;

    if ((*d) % 2)
        fprintf(stderr, "%s:%d should not be reached\n", __FILE__, __LINE__);
}

static void my_hash_callback(void *key, void *value, void *user_data)
{
    int *d = value;

    *d = 1;
}

static unsigned int my_hash(const void *key)
{
    return (unsigned int) *((const int *) key);
}

static int my_hash_equal(const void *a, const void *b)
{
    return *((const int *) a) == *((const int *) b);
}

/*
 * This is a simplified version of the pathalias hashing function.
 * Thanks to Steve Belovin and Peter Honeyman
 *
 * hash a string into a long int.  31 bit crc (from andrew appel).
 * the crc table is computed at run time by crcinit() -- we could
 * precompute, but it takes 1 clock tick on a 750.
 *
 * This fast table calculation works only if POLY is a prime polynomial
 * in the field of integers modulo 2.  Since the coefficients of a
 * 32-bit polynomial won't fit in a 32-bit word, the high-order bit is
 * implicit.  IT MUST ALSO BE THE CASE that the coefficients of orders
 * 31 down to 25 are zero.  Happily, we have candidates, from
 * E. J.  Watson, "Primitive Polynomials (Mod 2)", Math. Comp. 16 (1962):
 *	x^32 + x^7 + x^5 + x^3 + x^2 + x^1 + x^0
 *	x^31 + x^3 + x^0
 *
 * We reverse the bits to get:
 *	111101010000000000000000000000001 but drop the last 1
 *         f   5   0   0   0   0   0   0
 *	010010000000000000000000000000001 ditto, for 31-bit crc
 *	   4   8   0   0   0   0   0   0
 */

#define POLY 0x48000000L        /* 31-bit polynomial (avoids sign problems) */

static unsigned int CrcTable[128];

/*
 - crcinit - initialize tables for hash function
 */
static void crcinit(void)
{
    int i, j;
    unsigned int sum;

    for (i = 0; i < 128; ++i) {
        sum = 0L;
        for (j = 7 - 1; j >= 0; --j)
            if (i & (1 << j))
                sum ^= POLY >> j;
        CrcTable[i] = sum;
    }
}

/*
 - hash - Honeyman's nice hashing function
 */
static unsigned int honeyman_hash(const void *key)
{
    const char *name = (const char *) key;
    int size;
    unsigned int sum = 0;

    assert(name != NULL);
    assert(*name != 0);

    size = strlen(name);

    while (size--) {
        sum = (sum >> 7) ^ CrcTable[(sum ^ (*name++)) & 0x7f];
    }

    return (sum);
}

static int second_hash_cmp(const void *a, const void *b)
{
    return (strcmp(a, b) == 0);
}

static unsigned int one_hash(const void *key)
{
    return 1;
}

static void not_even_foreach(void *key, void *value, void *user_data)
{
    const char *_key = (const char *) key;
    const char *_value = (const char *) value;
    int i;
    char val[20];

    assert(_key != NULL);
    assert(*_key != 0);
    assert(_value != NULL);
    assert(*_value != 0);

    i = atoi(_key);

    sprintf(val, "%d value", i);
    assert(strcmp(_value, val) == 0);

    assert((i % 2) != 0);
    assert(i != 3);
}

static int remove_even_foreach(void *key, void *value, void *user_data)
{
    const char *_key = (const char *) key;
    const char *_value = (const char *) value;
    int i;
    char val[20];

    assert(_key != NULL);
    assert(*_key != 0);
    assert(_value != NULL);
    assert(*_value != 0);

    i = atoi(_key);

    sprintf(val, "%d value", i);
    assert(strcmp(_value, val) == 0);

    return ((i % 2) == 0) ? 1 : 0;
}

static void second_hash_test(int simple_hash)
{
    int i;
    char key[20] = "", val[20] = "", *v, *orig_key, *orig_val;
    struct std_hashtable *h;
    int found;

    crcinit();

    h = std_hash_table_new(simple_hash ? one_hash : honeyman_hash,
            second_hash_cmp);
    assert(h != NULL);
    for (i = 0; i < 20; i++) {
        sprintf(key, "%d", i);
        assert(atoi(key) == i);

        sprintf(val, "%d value", i);
        assert(atoi(val) == i);

        std_hash_table_insert(h, strdup(key), strdup(val));
    }

    assert(std_hash_table_size(h) == 20);

    for (i = 0; i < 20; i++) {
        sprintf(key, "%d", i);
        assert(atoi(key) == i);

        v = (char *) std_hash_table_lookup(h, key);

        assert(v != NULL);
        assert(*v != 0);
        assert(atoi(v) == i);
    }

    sprintf(key, "%d", 3);
    std_hash_table_remove(h, key);
    std_hash_table_foreach_remove(h, remove_even_foreach, NULL);
    std_hash_table_foreach(h, not_even_foreach, NULL);

    for (i = 0; i < 20; i++) {
        if ((i % 2) == 0 || i == 3)
            continue;

        sprintf(key, "%d", i);
        assert(atoi(key) == i);

        sprintf(val, "%d value", i);
        assert(atoi(val) == i);

        orig_key = orig_val = NULL;
        found = std_hash_table_lookup_extended(h, key,
                (void *) &orig_key, (void *) &orig_val);
        assert(found);

        std_hash_table_remove(h, key);

        assert(orig_key != NULL);
        assert(strcmp(key, orig_key) == 0);
        free(orig_key);

        assert(orig_val != NULL);
        assert(strcmp(val, orig_val) == 0);
        free(orig_val);
    }

    std_hash_table_destroy(h);
}

static int find_first(void *key, void *value, void *user_data)
{
    int *v = value;
    int *test = user_data;

    return (*v == *test);
}

static void direct_hash_test(void)
{
    long i, rc;
    struct std_hashtable *h;

    h = std_hash_table_new(NULL, NULL);
    assert(h != NULL);
    for (i = 1; i <= 20; i++) {
        std_hash_table_insert(h, ((void *) (i)), (void *) (i + 42));
    }

    assert(std_hash_table_size(h) == 20);

    for (i = 1; i <= 20; i++) {
        rc = ((long) (std_hash_table_lookup(h, ((void *) (i)))));

        assert(rc != 0);
        assert((rc - 42) == i);
    }

    std_hash_table_destroy(h);
}

int main(int argc, char *argv[])
{
    struct std_hashtable *hash_table;
    int i;
    int value = 120;
    int *pvalue;

    hash_table = std_hash_table_new(my_hash, my_hash_equal);
    for (i = 0; i < 10000; i++) {
        array[i] = i;
        std_hash_table_insert(hash_table, &array[i], &array[i]);
    }
    pvalue = std_hash_table_find(hash_table, find_first, &value);
    if (!pvalue || *pvalue != value)
        fprintf(stderr, "%s:%d should not be reached\n", __FILE__, __LINE__);

    std_hash_table_foreach(hash_table, my_hash_callback, NULL);

    for (i = 0; i < 10000; i++)
        if (array[i] == 0)
            fprintf(stderr, "%s:%d should not be reached\n", __FILE__,
                    __LINE__);

    for (i = 0; i < 10000; i++)
        std_hash_table_remove(hash_table, &array[i]);

    for (i = 0; i < 10000; i++) {
        array[i] = i;
        std_hash_table_insert(hash_table, &array[i], &array[i]);
    }

    if (std_hash_table_foreach_remove(hash_table, my_hash_callback_remove,
                    NULL) != 5000 || std_hash_table_size(hash_table) != 5000)
        fprintf(stderr, "%s:%d should not be reached\n", __FILE__, __LINE__);

    std_hash_table_foreach(hash_table, my_hash_callback_remove_test, NULL);

    std_hash_table_destroy(hash_table);

    second_hash_test(1);
    second_hash_test(0);
    direct_hash_test();

    printf("PASSED\n");

    return 0;

}
