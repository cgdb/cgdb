#include <stdio.h>
#include "std_list.h"

int printType(void *data, void *user_data)
{
    int i = *(int *) data;

    printf("FUNCTION:%d\n", i);
    return 1;
}

int destroy(void *data)
{
    int *a = (int *) data;

    free(a);
    a = NULL;
    return 0;
}

/* Used for the sorted list test */
int sorted_test();
int compare_ints(const void *a, const void *b);

int main(int argc, char **argv)
{

    std_list l;
    std_list_iterator iter;
    int i;

    l = std_list_create(destroy);

    if (!l) {
        printf("creation failed\n");
        return -1;
    }

    /* Add 1000 items */
    for (i = 0; i < 1000; ++i) {
        int *data = malloc(sizeof (int));

        *data = i;
        if (std_list_append(l, data) == -1) {
            printf("append failed\n");
            return -1;
        }
    }

    /* traverse them */
    for (iter = std_list_begin(l);
            iter != std_list_end(l); iter = std_list_next(iter)) {

        int *data;

        if (std_list_get_data(iter, (void *) &data) == -1) {
            printf("get data failed\n");
            return -1;
        }

        printf("ITEM(%d)\n", *data);
    }

    if (std_list_foreach(l, printType, NULL) == -1) {
        printf("foreach failed\n");
        return -1;
    }

    /* destroy list */
    if (std_list_destroy(l) == -1) {
        printf("deletion failed\n");
        return -1;
    }

    return sorted_test();
}

int sorted_test()
{
    int data_array[10] = { 5, 3, 2, 5, 1, -50, 0, 100, -1, 99 };
    std_list l = std_list_create(NULL);
    std_list_iterator iter;
    int *current = 0, *previous = 0;
    int sorted = 1;
    int i;

    /* Insert the data from the array into the 'sorted' list */
    printf("-- Sorted list test starting...\n");
    printf("    Inserting: ");
    for (i = 0; i < 10; i++) {
        printf("%d ", data_array[i]);
        std_list_insert_sorted(l, &data_array[i], compare_ints);
    }
    printf("\n");

    /* Read it back and verify it's sorted */
    printf("    Reading:   ");
    for (iter = std_list_begin(l);
            iter != std_list_end(l); iter = std_list_next(iter)) {
        if (std_list_get_data(iter, &current)) {
            printf("\nFailed to read data");
            return -1;
        }
        printf("%d ", *current);

        /* Compare to previous item if this is not the first */
        if (iter != std_list_begin(l) && *current < *previous) {
            sorted = 0;
        }

        previous = current;
    }
    printf("\n");

    /* Destroy list */
    std_list_destroy(l);

    /* Display result */
    if (sorted) {
        printf("    Status: List sorted successfully!\n");
    } else {
        printf("    Status: List is not sorted.\n");
    }
    printf("-- Sorted list test end\n");

    return !sorted;
}

int compare_ints(const void *a, const void *b)
{
    int int_a = *(int *) a;
    int int_b = *(int *) b;

    if (int_a < int_b) {
        return -1;
    } else if (int_a == int_b) {
        return 0;
    }

    return 1;
}
