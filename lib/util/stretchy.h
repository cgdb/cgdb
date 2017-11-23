#ifndef __STRETCHY_H__
#define __STRETCHY_H__

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

// ---- stretchy buffers (From Sean's stb.h)
// https://github.com/nothings/stb/blob/master/stretchy_buffer.h

#define sbfree( a ) ( ( a ) ? free( stb__sbraw( a ) ), NULL : NULL )
#define sbpush( a, v ) ( stb__sbmaybegrow( a, 1 ), ( a )[ stb__sbn( a )++ ] = ( v ) )
#define sbpop( a ) ( ( a )[ --stb__sbn( a ) ] )
#define sbcount( a ) ( ( a ) ? stb__sbn( a ) : 0 )
#define sbadd( a, n ) ( stb__sbmaybegrow( a, n ), stb__sbn( a ) += ( n ), &( a )[ stb__sbn( a ) - ( n ) ] )
#define sblast( a ) ( ( a )[ stb__sbn( a ) - 1 ] )
#define sbforeach( v, arr ) for ( ( v ) = ( arr ); ( v ) < ( arr ) + sbcount( arr ); ++( v ) )
#define sbsetcount( a, n ) ( stb__sbmaybegrow( a, n ), stb__sbn( a ) = n )
#define sbpopfront(a) (sbpush(a,*(a)), stb__shl(a), (a)[--stb__sbn(a)])

#define stb__sbraw( a ) ( ( int * )( a )-2 )
#define stb__sbm( a ) stb__sbraw( a )[ 0 ]
#define stb__sbn( a ) stb__sbraw( a )[ 1 ]

int stb__sbgrowf( void **arr, int increment, int itemsize );
void stb__shlf(void **arr, int itemsize);
#define stb__sbneedgrow( a, n ) ( ( a ) == 0 || stb__sbn( a ) + n >= stb__sbm( a ) )
#define stb__sbmaybegrow( a, n ) ( stb__sbneedgrow( a, ( n ) ) ? stb__sbgrow( a, n ) : 0 )
#define stb__sbgrow( a, n ) stb__sbgrowf( ( void ** )&( a ), ( n ), sizeof( *( a ) ) )
#define stb__shl(a) stb__shlf((void **)&(a), sizeof(*(a)))

#endif
