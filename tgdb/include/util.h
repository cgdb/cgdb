#ifndef __UTIL_H__
#define __UTIL_H__

#include <stdlib.h>

/* These are wrappers for the memory management functions 
 * If a memory allocation fails cgdb will exit
 * They act identical to the POSIX calls
 */
void *xcalloc(size_t nmemb, size_t size);
void *xmalloc(size_t size);
void *xrealloc(void *ptr, size_t size);
char *xstrdup(const char *s);
int xclose(int fd);


/* A simple interface to an infinate string */
struct string;

/* string_init: Returns a new infinate string.  */
struct string *string_init(void);

/* string_free: free a string */
void string_free(struct string *s);

/* clears the string s */
void string_clear(struct string *s);

/* string_addchar: Adds a char to the infinate buffer
 *  s - the infinate string to modify
 *  c - the char to add
 */
void string_addchar(struct string *s, char c);

/* string_delchar: Delete the last char put in */
void string_delchar(struct string *s);

/* string_get: returns the string 
 *  return  - the string returned is valid until string_free is called 
 *  NOTE: This function adds a '\0' to the end of the string if it is not 
 *        the last char
 */ 
char *string_get(struct string *s);

/* string_length: Returns the length of string s */
unsigned long string_length(struct string *s);

#endif
