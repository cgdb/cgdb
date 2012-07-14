/*
 * std_btree_driver: A test driver for the binary tree structure.
 *
 * Copyright (c) 2004, Mike Mueller & Bob Rossi
 * Subject to the terms of the GNU General Public Licence
 */

/* Standard Includes */
#include <stdio.h>

/* Local Includes */
#include "std_btree.h"

/* 
 * Macros
 */

#ifdef DEBUG
#define debug(args...) fprintf(stderr, args)
#else
#define debug(args...)
#endif

/*
 * Local function prototypes
 */

/* Tests */
static int test_add(std_btree tree);
static int test_remove(std_btree tree);
static int test_replace(std_btree tree);
static int test_isroot(std_btree tree);
static int test_isleaf(std_btree tree);

/* Destructor method used by tree */
static int destructor(char *string);

/* main:
 *
 * Description of test procedure here.
 */
int main(int argc, char *argv[])
{
    std_btree tree = NULL;
    int result = 0;

    /* Create a tree */
    debug("Creating tree... ");
    tree = std_btree_create((STDDestroyNotify) destructor);
    if (tree == NULL) {
        printf("FAILED\n");
        return 1;
    }
    debug("Succeeded.\n");

    /* Run tests */
    result |= test_add(tree);
    result |= test_remove(tree);
    result |= test_replace(tree);
    result |= test_isroot(tree);
    result |= test_isleaf(tree);

    debug("Destroying tree...\n");
    result |= std_btree_destroy(tree);

    if (result) {
        printf("FAILED\n");
        return 2;
    }

    printf("PASSED\n");
    return 0;
}

/*
 * Local function implementations
 */

static int test_add(std_btree tree)
{
    std_btree_iterator i = NULL;

    debug("Test starting: Add\n");

    if (std_btree_add(tree, NULL, STD_BTREE_LEFT, "First string")) {
        debug("Add \"First string\" as root node failed\n");
        return 1;
    }

    i = std_btree_root(tree);
    if (i == NULL) {
        debug("Added root node, but root came back as NULL\n");
        return 2;
    }

    if (std_btree_add(tree, i, STD_BTREE_RIGHT, "Right of root")) {
        debug("Add \"Right of root\" failed\n");
        return 3;
    }

    if (std_btree_add(tree, std_btree_child(i, STD_BTREE_RIGHT),
                    STD_BTREE_LEFT, "Left of 2nd node (right of root)")) {
        debug("Add \"Left of 2nd node\" failed\n");
        return 4;
    }

    /* Child already exists, expect to fail */
    if (std_btree_add(tree, i, STD_BTREE_RIGHT, "New right of root") == 0) {
        debug("Add \"New right of root\" didn't fail, but it should have\n");
        return 5;
    }

    debug("Test complete: Add\n");
    return 0;
}

static int test_remove(std_btree tree)
{
    std_btree_iterator i = std_btree_root(tree);

    debug("Test starting: Remove\n");

    std_btree_add(tree, i, STD_BTREE_LEFT, "Left of root");
    i = std_btree_child(i, STD_BTREE_LEFT);

    std_btree_add(tree, i, STD_BTREE_LEFT, "Left of left of root");
    std_btree_add(tree, i, STD_BTREE_RIGHT, "Right of left of root");

    if (std_btree_remove(std_btree_child(i, STD_BTREE_LEFT))) {
        debug("Remove \"Left of left of roof\" failed\n");
        return 1;
    }

    if (std_btree_remove(i)) {
        debug("Remove \"Left of root\" (and its right child) failed\n");
        return 2;
    }

    debug("Test complete: Remove\n");
    return 0;
}

static int test_replace(std_btree tree)
{
    std_btree_iterator i = std_btree_root(tree);

    debug("Test starting: Replace\n");

    if (std_btree_replace(i, "New root in town, baby!")) {
        debug("Replace root node with new data failed\n");
        return 1;
    }

    debug("Test complete: Replace\n");
    return 0;
}

static int test_isroot(std_btree tree)
{
    std_btree_iterator i = std_btree_root(tree);

    debug("Test starting: Isroot\n");

    if (!std_btree_isroot(i)) {
        debug("Isroot returned false when it's clearly true\n");
        return 1;
    }

    if (std_btree_isroot(std_btree_child(i, STD_BTREE_RIGHT))) {
        debug("Isroot returned true when it's clearly false\n");
        return 2;
    }

    debug("Test complete: Isroot\n");
    return 0;
}

static int test_isleaf(std_btree tree)
{
    std_btree_iterator i = std_btree_root(tree);

    debug("Test starting: Isleaf\n");

    if (std_btree_isleaf(i)) {
        debug("Isleaf returned true when it's clearly false\n");
        return 1;
    }

    /* Get to an actual leaf */
    i = std_btree_child(i, STD_BTREE_RIGHT);
    i = std_btree_child(i, STD_BTREE_LEFT);

    if (!std_btree_isleaf(i)) {
        debug("Isleaf returned false when it's clearly true\n");
        return 2;
    }

    debug("Test complete: Isleaf\n");
    return 0;
}

static int destructor(char *string)
{
    debug("Destructor called on: %s\n", string);
    return 0;
}
