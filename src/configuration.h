
enum TOKENS {
    SET = 255,
    UNSET,
    BIND,
    MACRO,
    BOOLEAN,
    NUMBER,
    IDENTIFIER,
    COMMAND,
    KEY,
    STRING,
};

extern int yylex( void );
extern char *yytext;
