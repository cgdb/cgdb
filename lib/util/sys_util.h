#ifndef __SYS_UTIL_H__
#define __SYS_UTIL_H__

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif /* HAVE_STDLIB_H */

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#include "cgdb_clog.h"

/* These are wrappers for the memory management functions 
 * If a memory allocation fails cgdb will exit
 * They act identical to the POSIX calls
 */
void *cgdb_calloc(size_t nmemb, size_t size);
void *cgdb_malloc(size_t size);
void *cgdb_realloc(void *ptr, size_t size);
char *cgdb_strdup(const char *s);
int cgdb_close(int fd);

/**
 * Convert a string to an integer.
 *
 * @param str
 * The string to convert.
 *
 * @param num
 * The integer result on success.
 * On failure, the value passed in will remain unchanged.
 *
 * @return
 * 0 on success or -1 on error.
 */
int cgdb_string_to_int(char *str, int *num);

/**
 * Convert a string to an integer.
 *
 * @param str
 * The hex string to convert.
 *
 * @param num
 * The integer result on success.
 * On failure, the value passed in will remain unchanged.
 *
 * @return
 * 0 on success or -1 on error.
 */
int cgdb_hexstr_to_u64(const char *str, uint64_t *num);

/**
 * Check to see if cgdb supports debugger attachment detection.
 *
 * @return
 * 1 if it does support detecting debugger attachment, otherwise 0.
 */
int cgdb_supports_debugger_attach_detection();

/* Check if debugger is attached to cgdb.
 * Return 0 for no, 1 for yes, -1 for error.
 */
int cgdb_is_debugger_attached();

/**
 * Get file size from file pointer.
 *
 * \param file
 * file pointer
 *
 * \return
 * file size on success, or -1 on error.
 */
long get_file_size(FILE *file);

/**
 * Get file size from file name.
 *
 * \param file
 * The file name.
 *
 * \return
 * file size on success, or -1 on error.
 */
long get_file_size_by_name(const char *filename);

/* Unsigned integer version of log10
*/
int log10_uint(unsigned int val);

char *sys_aprintf(const char *fmt, ...) ATTRIBUTE_PRINTF(1, 2);

#define MAX(a, b)  (((a) > (b)) ? (a) : (b))
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
char *sys_quote_nonprintables(const char *str, int len);

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

void sbpushstr(char **arr, const char *str, int len);
void sbpushstrf(char **arr, const char *fmt, ...) ATTRIBUTE_PRINTF(2, 3);

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
