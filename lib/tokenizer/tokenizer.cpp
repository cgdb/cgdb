#include <stdio.h>
#include <string.h>
#include "tokenizer.h"
#include "sys_util.h"

/* Some default file extensions */
const char *c_extensions[] = {
    ".c", ".cc", ".cpp", ".cxx", ".c++", ".h", ".hpp", ".hxx", ".hh", ".ipp", ".inl", ".moc", ".cu", ".cuh"
};
const char *asm_extensions[] = { ".s" };
const char *d_extensions[] = { ".d", ".di" };
const char *go_extensions[] = { ".go" };
const char *rust_extensions[] = { ".rs" };
const char *ada_extensions[] = { ".adb", ".ads", ".ada" };

typedef struct yy_buffer_state *YY_BUFFER_STATE;

#define DECLARE_LEX_FUNCTIONS(_LANG) \
    extern int _LANG ## _lex(void); \
    extern char *_LANG ## _text; \
    extern YY_BUFFER_STATE _LANG ## __scan_string(const char *base); \
    void _LANG ## __delete_buffer (YY_BUFFER_STATE b);

DECLARE_LEX_FUNCTIONS(c)
DECLARE_LEX_FUNCTIONS(asm)
DECLARE_LEX_FUNCTIONS(d)
DECLARE_LEX_FUNCTIONS(go)
DECLARE_LEX_FUNCTIONS(rust)
DECLARE_LEX_FUNCTIONS(ada)
DECLARE_LEX_FUNCTIONS(cgdbhelp)

#undef DECLARE_LEX_FUNCTIONS

struct tokenizer {
    enum tokenizer_language_support lang;

    char **yy_tokenizer_text;
    int (*yy_lex_func) (void);
    void (*yy_delete_buffer_func)(YY_BUFFER_STATE b);

    YY_BUFFER_STATE str_buffer;
};

struct tokenizer *tokenizer_init(void)
{
    struct tokenizer *t =
            (struct tokenizer *) cgdb_malloc(sizeof (struct tokenizer));

    t->lang = TOKENIZER_LANGUAGE_UNKNOWN;

    t->yy_lex_func = NULL;
    t->yy_delete_buffer_func = NULL;

    t->yy_tokenizer_text = NULL;
    t->str_buffer = NULL;
    return t;
}

void tokenizer_destroy(struct tokenizer *t)
{
    if (t) {
        (*t->yy_delete_buffer_func)(t->str_buffer);
        t->str_buffer = NULL;

        free(t);
    }
}

int tokenizer_set_buffer(struct tokenizer *t, const char *buffer, enum tokenizer_language_support l)
{
    if (t->str_buffer) {
        (*t->yy_delete_buffer_func)(t->str_buffer);
        t->str_buffer = NULL;
    }

    if (l < TOKENIZER_ENUM_START_POS || l >= TOKENIZER_LANGUAGE_UNKNOWN)
        return 0;

    t->lang = l;

#define INIT_LEX(_LANG) \
    t->yy_lex_func = _LANG ## _lex; \
    t->yy_delete_buffer_func = _LANG ## __delete_buffer; \
    t->yy_tokenizer_text = &(_LANG ## _text); \
    t->str_buffer = _LANG ## __scan_string(buffer);

    if (l == TOKENIZER_LANGUAGE_C) {
        INIT_LEX(c);
    } else if (l == TOKENIZER_LANGUAGE_ASM) {
        INIT_LEX(asm);
    } else if (l == TOKENIZER_LANGUAGE_D) {
        INIT_LEX(d);
    } else if (l == TOKENIZER_LANGUAGE_GO) {
        INIT_LEX(go);
    } else if (l == TOKENIZER_LANGUAGE_CGDBHELP) {
        INIT_LEX(cgdbhelp);
    } else if (l == TOKENIZER_LANGUAGE_RUST) {
        INIT_LEX(rust);
    } else {
        INIT_LEX(ada);
    }

#undef INIT_LEX

    return 0;
}

int tokenizer_get_token(struct tokenizer *t, struct token_data *token_data)
{
    if (!t || !t->yy_lex_func)
        return 0;

    enum tokenizer_type tpacket = (enum tokenizer_type)(t->yy_lex_func)();

    token_data->e = tpacket;
    token_data->data = *t->yy_tokenizer_text;
    return !!tpacket;
}

const char *tokenizer_get_printable_enum(enum tokenizer_type e)
{
    const char *enum_array[] = {
        "TOKENIZER_KEYWORD",
        "TOKENIZER_TYPE",
        "TOKENIZER_LITERAL",
        "TOKENIZER_NUMBER",
        "TOKENIZER_COMMENT",
        "TOKENIZER_DIRECTIVE",
        "TOKENIZER_TEXT",
        "TOKENIZER_NEWLINE"
    };

    if (e >= TOKENIZER_KEYWORD && e < TOKENIZER_ERROR)
        return enum_array[e - TOKENIZER_ENUM_START_POS];
    else if (e == TOKENIZER_ERROR)
        return "TOKEN ERROR!";

    return NULL;
}

enum tokenizer_language_support tokenizer_get_default_file_type(const char
        *file_extension)
{
    enum tokenizer_language_support l = TOKENIZER_LANGUAGE_UNKNOWN;
    int i;

    if (!file_extension)
        return TOKENIZER_LANGUAGE_UNKNOWN;

    for (i = 0; i < sizeof (c_extensions) / sizeof (char *); i++)
        if (strcasecmp(file_extension, c_extensions[i]) == 0)
            return TOKENIZER_LANGUAGE_C;

    for (i = 0; i < sizeof (asm_extensions) / sizeof (char *); i++)
        if (strcasecmp(file_extension, asm_extensions[i]) == 0)
            return TOKENIZER_LANGUAGE_ASM;

    for (i = 0; i < sizeof (d_extensions) / sizeof (char *); i++)
        if (strcasecmp(file_extension, d_extensions[i]) == 0)
            return TOKENIZER_LANGUAGE_D;

    for (i = 0; i < sizeof (go_extensions) / sizeof (char *); i++)
        if (strcasecmp(file_extension, go_extensions[i]) == 0)
            return TOKENIZER_LANGUAGE_GO;

    for (i = 0; i < sizeof (rust_extensions) / sizeof (char *); i++)
        if (strcasecmp(file_extension, rust_extensions[i]) == 0)
            l = TOKENIZER_LANGUAGE_RUST;

    for (i = 0; i < sizeof (ada_extensions) / sizeof (char *); i++)
        if (strcasecmp(file_extension, ada_extensions[i]) == 0)
            return TOKENIZER_LANGUAGE_ADA;

    return l;
}
