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
    TOKENIZER_ERROR,

    TOKENIZER_SEARCH,
    TOKENIZER_STATUS_BAR,

    TOKENIZER_EXECUTING_LINE_ARROW,
    TOKENIZER_SELECTED_LINE_ARROW,
    TOKENIZER_EXECUTING_LINE_HIGHLIGHT,
    TOKENIZER_SELECTED_LINE_HIGHLIGHT,
    TOKENIZER_EXECUTING_LINE_BLOCK,
    TOKENIZER_SELECTED_LINE_BLOCK,

    TOKENIZER_ENABLED_BREAKPOINT,
    TOKENIZER_DISABLED_BREAKPOINT,
    TOKENIZER_SELECTED_LINE_NUMBER,
    TOKENIZER_SCROLL_MODE_STATUS,
    TOKENIZER_LOGO,
    TOKENIZER_COLOR,
};

enum tokenizer_language_support {
    TOKENIZER_LANGUAGE_C = TOKENIZER_ENUM_START_POS,
    TOKENIZER_LANGUAGE_ASM,
    TOKENIZER_LANGUAGE_D,
    TOKENIZER_LANGUAGE_GO,
    TOKENIZER_LANGUAGE_RUST,
    TOKENIZER_LANGUAGE_ADA,
    TOKENIZER_LANGUAGE_CGDBHELP,
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
struct tokenizer *tokenizer_init(void);

/* tokenizer_destroy
 * -----------------
 *
 * This destroy's a tokenizer
 *
 *  t:      The tokenizer object to work on
 */
void tokenizer_destroy(struct tokenizer *t);

/**
 *  This functions will prepare the tokenizer to parse a particular buffer.
 *  
 *  t:      The tokenizer object to work on
 *
 *  Return: -1 on error. 0 on success
 */
int tokenizer_set_buffer(struct tokenizer *t, const char *buffer,
                         enum tokenizer_language_support l);

/* tokenizer_get_token
 * -------------------
 *
 *  This function will get the next token packet from the file.
 *
 *  t:      The tokenizer object to work on
 *
 *  Return: -1 on error, 0 on end of file, 1 on success
 */
struct token_data {
    enum tokenizer_type e;
    const char *data;
};
int tokenizer_get_token(struct tokenizer *t, struct token_data *token_data);

/* tokenizer_print_enum
 * --------------------
 *
 * Returns the string representation of the enum
 * 
 * e: The enum
 *
 * Returns: NULL on error, or the enum on success.
 */
const char *tokenizer_get_printable_enum(enum tokenizer_type e);

/* tokenizer_get_default_file_type
 *
 * This will return the type of file the tokenizer thinks the
 * extension FILE_EXTENSION belongs too.
 * 
 *  t:      The tokenizer object to work on
 *  e: 		The file extension the tokenizer will use to determine filetype.
 */
enum tokenizer_language_support tokenizer_get_default_file_type(const char
        *file_extension);

#endif /* __TOKENIZER_H__ */
