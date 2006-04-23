#ifndef COMMAND_LEXER_H
#define COMMAND_LEXER_H

/* enum TOKENS: the set of tokens that can be recognized by yylex
 * -------------
 *  SET: "set"
 *  UNSET: for future expansion
 *  BIND: for future expansion
 *  MACRO: for future expansion
 *  BOOLEAN: yes or no
 *  NUMBER: a series of digits
 *  IDENTIFIER: a valid identifier (letter followed by letters or numbers)
 *  COMMAND: a recognized command (for future expansion)
 *  STRING: a quoted-string.
 *  EOL: end of line
 */
enum TOKENS {
    SET = 255,
    UNSET,
    BIND,
    MACRO,
    SHELL,
    BANG,
    BOOLEAN,
    NUMBER,
    IDENTIFIER,
    COMMAND,
    STRING,
	EOL
};

/* yylex: retreive the next token from the current scan buffer
 * --------------
 *  return: an integer value representing the token type (enum TOKEN).  0 when
 *          no more input.
 */
int yylex( void );

/* get_token: Get the scanned token.  This value will change the next time yylex is called 
 * --------------
 */
const char *get_token( void );

#endif
