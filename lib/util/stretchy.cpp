#if HAVE_STDLIB_H
#include <stdlib.h>
#endif /* HAVE_STDLIB_H */

#include "sys_util.h"
#include "stretchy.h"

/* stb__sbgrowf: internal stretchy buffer grow function. */
int stb__sbgrowf(void **arr, int increment, int itemsize)
{
    int m = *arr ? 2 * stb__sbm( *arr ) + increment : increment + 1;
    void *p = cgdb_realloc( *arr ? stb__sbraw( *arr ) : 0,
                            itemsize * m + sizeof( int ) * 2 );

    if ( !*arr )
        ( ( int * )p )[ 1 ] = 0;
    *arr = ( void * )( ( int * )p + 2 );
    stb__sbm( *arr ) = m;

    return 0;
}

void stb__shlf(void **arr, int itemsize)
{
    int n = stb__sbn(*arr);
    char *arrptr = (char *)(*arr);

    memmove(arrptr, arrptr + itemsize, itemsize * (n - 1));

    stb__sbn(*arr) = n - 1;
}
