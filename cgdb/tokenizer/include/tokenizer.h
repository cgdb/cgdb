#ifndef __TOKENIZER_H__
#define __TOKENIZER_H__

#include "ibuf.h"

struct tokenizer;

#define TOKENIZER_ENUM_START_POS 255

enum tokenizer_type {
    TOKENIZER_KEYWORD = TOKENIZER_ENUM_START_POS,
	TOKENIZER_TYPE,
	TOKENIZER_LITERAL,
	TOKENIZER_NUMBER,
	TOKENIZER_COMMENT,
	TOKENIZER_DIRECTIVE,
	TOKENIZER_TEXT,
	TOKENIZER_NEWLINE,
	TOKENIZER_ERROR
};

enum tokenizer_language_support {
	TOKENIZER_LANGUAGE_C = TOKENIZER_ENUM_START_POS,
	TOKENIZER_LANGUAGE_ADA,
	TOKENIZER_LANGUAGE_UNKNOWN
};

/* tokenizer_init
 * --------------
 *
 *  This initializers a new tokenizer.
 *
 *  t:      The tokenizer object to work on
 *
 *  Return: It will never fail.
 */
struct tokenizer *tokenizer_init ( void );

/* tokenizer_destroy
 * -----------------
 *
 * This destroy's a tokenizer
 *
 *  t:      The tokenizer object to work on
 */
void tokenizer_destroy ( struct tokenizer *t );

/* tokenizer_set_file
 * ------------------
 *
 *  This functions will prepare the tokenizer to parse a particular file.
 *  
 *  t:      The tokenizer object to work on
 *  file:   The absolute path to the file to tokenize.
 *
 *  Return: -1 on error. 0 on success
 */
int tokenizer_set_file ( struct tokenizer *t, const char *file, enum tokenizer_language_support l );

/* tokenizer_get_token
 * -------------------
 *
 *  This function will get the next token packet from the file.
 *
 *  t:      The tokenizer object to work on
 *
 *  Return: -1 on error, 0 on end of file, 1 on success
 */
int tokenizer_get_token ( struct tokenizer *t );

/* tokenizer_type
 * --------------
 *
 *  This will return the type of token a tokenizer is.
 *  
 *  t:      The tokenizer object to work on
 *
 *  Returns: the token type
 */
enum tokenizer_type tokenizer_get_packet_type ( struct tokenizer *t );

/* tokenizer_print_enum
 * --------------------
 *
 * Returns the string representation of the enum
 * 
 * e: The enum
 *
 * Returns: NULL on error, or the enum on success.
 */
const char *tokenizer_get_printable_enum ( enum tokenizer_type e );

/* tokenizer_get_data
 * ------------------
 *
 *  This gets the actual token.
 *
 *  t:      The tokenizer object to work on
 *
 *  Returns: The token data. This is from the heap. The user must free it.
 */
char *tokenizer_get_data ( struct tokenizer *t );

/* tokenizer_get_default_file_type
 *
 * This will return the type of file the tokenizer thinks the
 * extension FILE_EXTENSION belongs too.
 * 
 *  t:      The tokenizer object to work on
 *  e: 		The file extension the tokenizer will use to determine filetype.
 */
enum tokenizer_language_support tokenizer_get_default_file_type ( 
			const char *file_extension );

#endif /* __TOKENIZER_H__ */
