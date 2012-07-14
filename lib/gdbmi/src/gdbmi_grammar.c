
/* A Bison parser, made by GNU Bison 2.3a+.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.3a+"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 1

/* Pull parsers.  */
#define YYPULL 0

/* Using locations.  */
#define YYLSP_NEEDED 0

/* Substitute the variable and function names.  */
#define yypush_parse    gdbmi_push_parse
#define yypstate_new    gdbmi_pstate_new
#define yypstate_delete gdbmi_pstate_delete
#define yypstate        gdbmi_pstate
#define yylex           gdbmi_lex
#define yyerror         gdbmi_error
#define yylval          gdbmi_lval
#define yychar          gdbmi_char
#define yydebug         gdbmi_debug
#define yynerrs         gdbmi_nerrs

/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
#line 8 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "gdbmi_pt.h"

extern char *gdbmi_text;
extern int gdbmi_lex(void);
extern int gdbmi_lineno;

void gdbmi_error(gdbmi_pdata_ptr gdbmi_pdata, const char *s)
{
    fprintf(stderr, "%s:%d Error %s", __FILE__, __LINE__, s);
    if (strcmp(gdbmi_text, "\n") == 0)
        fprintf(stderr, "%s:%d at end of line %d\n", __FILE__, __LINE__,
                gdbmi_lineno);
    else {
        fprintf(stderr, "%s:%d at token(%s), line (%d)\n", __FILE__, __LINE__,
                gdbmi_text, gdbmi_lineno);
        gdbmi_lex();
        fprintf(stderr, "%s:%d before (%s)\n", __FILE__, __LINE__, gdbmi_text);
    }
}

/* Line 189 of yacc.c  */
#line 111 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.c"

/* Enabling traces.  */
#ifndef YYDEBUG
#define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
#undef YYERROR_VERBOSE
#define YYERROR_VERBOSE 1
#else
#define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
#define YYTOKEN_TABLE 0
#endif

/* "%code requires" blocks.  */

/* Line 209 of yacc.c  */
#line 5 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
struct gdbmi_pdata;

/* Line 209 of yacc.c  */
#line 139 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.c"

/* Tokens.  */
#ifndef YYTOKENTYPE
#define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
enum yytokentype {
    OPEN_BRACE = 258,
    CLOSED_BRACE = 259,
    OPEN_PAREN = 260,
    CLOSED_PAREN = 261,
    ADD_OP = 262,
    MULT_OP = 263,
    EQUAL_SIGN = 264,
    TILDA = 265,
    AT_SYMBOL = 266,
    AMPERSAND = 267,
    OPEN_BRACKET = 268,
    CLOSED_BRACKET = 269,
    NEWLINE = 270,
    INTEGER_LITERAL = 271,
    STRING_LITERAL = 272,
    CSTRING = 273,
    COMMA = 274,
    CARROT = 275
};
#endif
/* Tokens.  */
#define OPEN_BRACE 258
#define CLOSED_BRACE 259
#define OPEN_PAREN 260
#define CLOSED_PAREN 261
#define ADD_OP 262
#define MULT_OP 263
#define EQUAL_SIGN 264
#define TILDA 265
#define AT_SYMBOL 266
#define AMPERSAND 267
#define OPEN_BRACKET 268
#define CLOSED_BRACKET 269
#define NEWLINE 270
#define INTEGER_LITERAL 271
#define STRING_LITERAL 272
#define CSTRING 273
#define COMMA 274
#define CARROT 275

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE {

/* Line 214 of yacc.c  */
#line 53 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"

    struct gdbmi_output *u_output;
    struct gdbmi_oob_record *u_oob_record;
    struct gdbmi_result_record *u_result_record;
    int u_result_class;
    int u_async_record_choice;
    struct gdbmi_result *u_result;
    long u_token;
    struct gdbmi_async_record *u_async_record;
    struct gdbmi_stream_record *u_stream_record;
    int u_async_class;
    char *u_variable;
    struct gdbmi_value *u_value;
    struct gdbmi_tuple *u_tuple;
    struct gdbmi_list *u_list;
    int u_stream_record_choice;

/* Line 214 of yacc.c  */
#line 216 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.c"
} YYSTYPE;

#define YYSTYPE_IS_TRIVIAL 1
#define yystype YYSTYPE         /* obsolescent; will be withdrawn */
#define YYSTYPE_IS_DECLARED 1
#endif

#ifndef YYPUSH_DECLS
#define YYPUSH_DECLS
struct yypstate;
typedef struct yypstate yypstate;
enum { YYPUSH_MORE = 4 };

#if defined __STDC__ || defined __cplusplus
int yypush_parse(yypstate * yyps, int yypushed_char,
        YYSTYPE const *yypushed_val, struct gdbmi_pdata *gdbmi_pdata);
#else
int yypush_parse();
#endif

#if defined __STDC__ || defined __cplusplus
yypstate *yypstate_new(void);
#else
yypstate *yypstate_new();
#endif
#if defined __STDC__ || defined __cplusplus
void yypstate_delete(yypstate * yyps);
#else
void yypstate_delete();
#endif
#endif

/* Copy the second part of user declarations.  */

/* Line 264 of yacc.c  */
#line 252 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.c"

#ifdef short
#undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
#ifdef __SIZE_TYPE__
#define YYSIZE_T __SIZE_TYPE__
#elif defined size_t
#define YYSIZE_T size_t
#elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#include <stddef.h>             /* INFRINGES ON USER NAME SPACE */
#define YYSIZE_T size_t
#else
#define YYSIZE_T unsigned int
#endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
#if YYENABLE_NLS
#if ENABLE_NLS
#include <libintl.h>            /* INFRINGES ON USER NAME SPACE */
#define YY_(msgid) dgettext ("bison-runtime", msgid)
#endif
#endif
#ifndef YY_
#define YY_(msgid) msgid
#endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
#define YYUSE(e) ((void) (e))
#else
#define YYUSE(e)                /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
#define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int YYID(int yyi)
#else
static int YYID(yyi)
     int yyi;
#endif
{
    return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

#ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#define YYSTACK_ALLOC_MAXIMUM 4032  /* reasonable circa 2006 */
#endif
#else
#define YYSTACK_ALLOC YYMALLOC
#define YYSTACK_FREE YYFREE
#ifndef YYSTACK_ALLOC_MAXIMUM
#define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#endif
#if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#include <stdlib.h>             /* INFRINGES ON USER NAME SPACE */
#ifndef _STDLIB_H
#define _STDLIB_H 1
#endif
#endif
#ifndef YYMALLOC
#define YYMALLOC malloc
#if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc(YYSIZE_T);         /* INFRINGES ON USER NAME SPACE */
#endif
#endif
#ifndef YYFREE
#define YYFREE free
#if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free(void *);              /* INFRINGES ON USER NAME SPACE */
#endif
#endif
#endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc {
    yytype_int16 yyss_alloc;
    YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
#define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
#define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
#ifndef YYCOPY
#if defined __GNUC__ && 1 < __GNUC__
#define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#else
#define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#endif
#endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
#define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  4
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   46

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  21
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  22
/* YYNRULES -- Number of rules.  */
#define YYNRULES  40
/* YYNRULES -- Number of states.  */
#define YYNSTATES  61

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   275

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] = {
    0, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 1, 2, 3, 4,
    5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint8 yyprhs[] = {
    0, 0, 3, 5, 8, 15, 16, 20, 21, 24,
    28, 34, 36, 38, 42, 48, 50, 52, 54, 56,
    58, 60, 64, 68, 70, 72, 76, 78, 80, 82,
    85, 89, 92, 96, 100, 103, 105, 107, 109, 110,
    112
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] = {
    22, 0, -1, 23, -1, 22, 23, -1, 24, 25,
    5, 34, 6, 15, -1, -1, 24, 27, 15, -1,
    -1, 26, 15, -1, 41, 20, 30, -1, 41, 20,
    30, 19, 32, -1, 28, -1, 39, -1, 41, 29,
    31, -1, 41, 29, 31, 19, 32, -1, 8, -1,
    7, -1, 9, -1, 17, -1, 17, -1, 33, -1,
    32, 19, 33, -1, 34, 9, 36, -1, 17, -1,
    36, -1, 35, 19, 36, -1, 18, -1, 37, -1,
    38, -1, 3, 4, -1, 3, 32, 4, -1, 13,
    14, -1, 13, 35, 14, -1, 13, 32, 14, -1,
    40, 18, -1, 10, -1, 11, -1, 12, -1, -1,
    42, -1, 16, -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] = {
    0, 96, 96, 100, 105, 116, 120, 124, 128, 132,
    139, 146, 152, 158, 165, 173, 177, 181, 185, 200,
    207, 211, 215, 221, 225, 229, 233, 239, 245, 251,
    255, 260, 264, 270, 276, 282, 286, 290, 294, 298,
    302
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] = {
    "$end", "error", "$undefined", "OPEN_BRACE", "CLOSED_BRACE",
    "OPEN_PAREN", "CLOSED_PAREN", "ADD_OP", "MULT_OP", "EQUAL_SIGN", "TILDA",
    "AT_SYMBOL", "AMPERSAND", "OPEN_BRACKET", "CLOSED_BRACKET", "NEWLINE",
    "INTEGER_LITERAL", "STRING_LITERAL", "CSTRING", "COMMA", "CARROT",
    "$accept", "output_list", "output", "opt_oob_record_list",
    "opt_result_record", "result_record", "oob_record", "async_record",
    "async_record_class", "result_class", "async_class", "result_list",
    "result", "variable", "value_list", "value", "tuple", "list",
    "stream_record", "stream_record_class", "opt_token", "token", 0
};
#endif

#ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] = {
    0, 256, 257, 258, 259, 260, 261, 262, 263, 264,
    265, 266, 267, 268, 269, 270, 271, 272, 273, 274,
    275
};
#endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] = {
    0, 21, 22, 22, 23, 24, 24, 25, 25, 26,
    26, 27, 27, 28, 28, 29, 29, 29, 30, 31,
    32, 32, 33, 34, 35, 35, 36, 36, 36, 37,
    37, 38, 38, 38, 39, 40, 40, 40, 41, 41,
    42
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] = {
    0, 2, 1, 2, 6, 0, 3, 0, 2, 3,
    5, 1, 1, 3, 5, 1, 1, 1, 1, 1,
    1, 3, 3, 1, 1, 3, 1, 1, 1, 2,
    3, 2, 3, 3, 2, 1, 1, 1, 0, 1,
    1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] = {
    5, 5, 2, 38, 1, 3, 35, 36, 37, 40,
    0, 0, 0, 11, 12, 0, 0, 39, 0, 8,
    6, 34, 16, 15, 17, 0, 0, 23, 0, 18,
    9, 19, 13, 0, 0, 0, 4, 10, 20, 0,
    14, 0, 0, 21, 0, 0, 26, 22, 27, 28,
    29, 0, 31, 0, 0, 24, 30, 33, 32, 0,
    25
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] = {
    -1, 1, 2, 3, 10, 11, 12, 13, 26, 30,
    32, 37, 38, 39, 54, 47, 48, 49, 14, 15,
    16, 17
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -40
static const yytype_int8 yypact[] = {
    -40, 7, -40, 19, -40, -40, -40, -40, -40, -40,
    -4, -6, -2, -40, -40, 10, 14, -40, 16, -40,
    -40, -40, -40, -40, -40, 20, 21, -40, 30, -40,
    22, -40, 23, 24, 16, 16, -40, 25, -40, 31,
    25, 16, -1, -40, 1, -3, -40, -40, -40, -40,
    -40, 0, -40, -11, 13, -40, -40, -40, -40, -1,
    -40
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] = {
    -40, -40, 42, -40, -40, -40, -40, -40, -40, -40,
    -40, -19, 4, 28, -40, -39, -40, -40, -40, -40,
    -40, -40
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -8
static const yytype_int8 yytable[] = {
    44, 18, 44, 57, 56, 50, 55, 4, 41, 19,
    45, 52, 45, 20, 27, 46, 40, 46, 27, 41,
    60, 22, 23, 24, -7, 51, 53, 58, 21, 6,
    7, 8, 59, 27, 25, 9, 33, 29, 31, 36,
    42, 34, 35, 5, 41, 43, 28
};

static const yytype_uint8 yycheck[] = {
    3, 5, 3, 14, 4, 4, 45, 0, 19, 15,
    13, 14, 13, 15, 17, 18, 35, 18, 17, 19,
    59, 7, 8, 9, 5, 44, 45, 14, 18, 10,
    11, 12, 19, 17, 20, 16, 6, 17, 17, 15,
    9, 19, 19, 1, 19, 41, 18
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] = {
    0, 22, 23, 24, 0, 23, 10, 11, 12, 16,
    25, 26, 27, 28, 39, 40, 41, 42, 5, 15,
    15, 18, 7, 8, 9, 20, 29, 17, 34, 17,
    30, 17, 31, 6, 19, 19, 15, 32, 33, 34,
    32, 19, 9, 33, 3, 13, 18, 36, 37, 38,
    4, 32, 14, 32, 35, 36, 4, 14, 14, 19,
    36
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab

/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (gdbmi_pdata, YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))

#define YYTERROR	1
#define YYERRCODE	256

/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
#define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif

/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
#if YYLTYPE_IS_TRIVIAL
#define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
#else
#define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif
#endif

/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
#define YYLEX yylex (&yylval, YYLEX_PARAM)
#else
#define YYLEX yylex (&yylval)
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

#ifndef YYFPRINTF
#include <stdio.h>              /* INFRINGES ON USER NAME SPACE */
#define YYFPRINTF fprintf
#endif

#define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

#define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value, gdbmi_pdata); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))

/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

 /*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print(FILE * yyoutput, int yytype,
        YYSTYPE const *const yyvaluep, struct gdbmi_pdata *gdbmi_pdata)
#else
static void yy_symbol_value_print(yyoutput, yytype, yyvaluep, gdbmi_pdata)
     FILE *yyoutput;
     int yytype;
     YYSTYPE const *const yyvaluep;
     struct gdbmi_pdata *gdbmi_pdata;
#endif
{
    if (!yyvaluep)
        return;
    YYUSE(gdbmi_pdata);
#ifdef YYPRINT
    if (yytype < YYNTOKENS)
        YYPRINT(yyoutput, yytoknum[yytype], *yyvaluep);
#else
    YYUSE(yyoutput);
#endif
    switch (yytype) {
        default:
            break;
    }
}

/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print(FILE * yyoutput, int yytype, YYSTYPE const *const yyvaluep,
        struct gdbmi_pdata *gdbmi_pdata)
#else
static void yy_symbol_print(yyoutput, yytype, yyvaluep, gdbmi_pdata)
     FILE *yyoutput;
     int yytype;
     YYSTYPE const *const yyvaluep;
     struct gdbmi_pdata *gdbmi_pdata;
#endif
{
    if (yytype < YYNTOKENS)
        YYFPRINTF(yyoutput, "token %s (", yytname[yytype]);
    else
        YYFPRINTF(yyoutput, "nterm %s (", yytname[yytype]);

    yy_symbol_value_print(yyoutput, yytype, yyvaluep, gdbmi_pdata);
    YYFPRINTF(yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void yy_stack_print(yytype_int16 * yybottom, yytype_int16 * yytop)
#else
static void yy_stack_print(yybottom, yytop)
     yytype_int16 *yybottom;
     yytype_int16 *yytop;
#endif
{
    YYFPRINTF(stderr, "Stack now");
    for (; yybottom <= yytop; yybottom++) {
        int yybot = *yybottom;

        YYFPRINTF(stderr, " %d", yybot);
    }
    YYFPRINTF(stderr, "\n");
}

#define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))

/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print(YYSTYPE * yyvsp, int yyrule, struct gdbmi_pdata *gdbmi_pdata)
#else
static void yy_reduce_print(yyvsp, yyrule, gdbmi_pdata)
     YYSTYPE *yyvsp;
     int yyrule;
     struct gdbmi_pdata *gdbmi_pdata;
#endif
{
    int yynrhs = yyr2[yyrule];
    int yyi;
    unsigned long int yylno = yyrline[yyrule];

    YYFPRINTF(stderr, "Reducing stack by rule %d (line %lu):\n",
            yyrule - 1, yylno);
    /* The symbols being reduced.  */
    for (yyi = 0; yyi < yynrhs; yyi++) {
        YYFPRINTF(stderr, "   $%d = ", yyi + 1);
        yy_symbol_print(stderr, yyrhs[yyprhs[yyrule] + yyi],
                &(yyvsp[(yyi + 1) - (yynrhs)])
                , gdbmi_pdata);
        YYFPRINTF(stderr, "\n");
    }
}

#define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule, gdbmi_pdata); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
#define YYDPRINTF(Args)
#define YY_SYMBOL_PRINT(Title, Type, Value, Location)
#define YY_STACK_PRINT(Bottom, Top)
#define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */

/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

#if YYERROR_VERBOSE

#ifndef yystrlen
#if defined __GLIBC__ && defined _STRING_H
#define yystrlen strlen
#else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T yystrlen(const char *yystr)
#else
static YYSIZE_T yystrlen(yystr)
     const char *yystr;
#endif
{
    YYSIZE_T yylen;

    for (yylen = 0; yystr[yylen]; yylen++)
        continue;
    return yylen;
}
#endif
#endif

#ifndef yystpcpy
#if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#define yystpcpy stpcpy
#else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *yystpcpy(char *yydest, const char *yysrc)
#else
static char *yystpcpy(yydest, yysrc)
     char *yydest;
     const char *yysrc;
#endif
{
    char *yyd = yydest;
    const char *yys = yysrc;

    while ((*yyd++ = *yys++) != '\0')
        continue;

    return yyd - 1;
}
#endif
#endif

#ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T yytnamerr(char *yyres, const char *yystr)
{
    if (*yystr == '"') {
        YYSIZE_T yyn = 0;
        char const *yyp = yystr;

        for (;;)
            switch (*++yyp) {
                case '\'':
                case ',':
                    goto do_not_strip_quotes;

                case '\\':
                    if (*++yyp != '\\')
                        goto do_not_strip_quotes;
                    /* Fall through.  */
                default:
                    if (yyres)
                        yyres[yyn] = *yyp;
                    yyn++;
                    break;

                case '"':
                    if (yyres)
                        yyres[yyn] = '\0';
                    return yyn;
            }
      do_not_strip_quotes:;
    }

    if (!yyres)
        return yystrlen(yystr);

    return yystpcpy(yyres, yystr) - yyres;
}
#endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T yysyntax_error(char *yyresult, int yystate, int yychar)
{
    int yyn = yypact[yystate];

    if (!(YYPACT_NINF < yyn && yyn <= YYLAST))
        return 0;
    else {
        int yytype = YYTRANSLATE(yychar);
        YYSIZE_T yysize0 = yytnamerr(0, yytname[yytype]);
        YYSIZE_T yysize = yysize0;
        YYSIZE_T yysize1;
        int yysize_overflow = 0;
        enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
        char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
        int yyx;

#if 0
        /* This is so xgettext sees the translatable formats that are
           constructed on the fly.  */
        YY_("syntax error, unexpected %s");
        YY_("syntax error, unexpected %s, expecting %s");
        YY_("syntax error, unexpected %s, expecting %s or %s");
        YY_("syntax error, unexpected %s, expecting %s or %s or %s");
        YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
#endif
        char *yyfmt;
        char const *yyf;
        static char const yyunexpected[] = "syntax error, unexpected %s";
        static char const yyexpecting[] = ", expecting %s";
        static char const yyor[] = " or %s";
        char yyformat[sizeof yyunexpected
                + sizeof yyexpecting - 1 + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
                        * (sizeof yyor - 1))];
        char const *yyprefix = yyexpecting;

        /* Start YYX at -YYN if negative to avoid negative indexes in
           YYCHECK.  */
        int yyxbegin = yyn < 0 ? -yyn : 0;

        /* Stay within bounds of both yycheck and yytname.  */
        int yychecklim = YYLAST - yyn + 1;
        int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
        int yycount = 1;

        yyarg[0] = yytname[yytype];
        yyfmt = yystpcpy(yyformat, yyunexpected);

        for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR) {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM) {
                    yycount = 1;
                    yysize = yysize0;
                    yyformat[sizeof yyunexpected - 1] = '\0';
                    break;
                }
                yyarg[yycount++] = yytname[yyx];
                yysize1 = yysize + yytnamerr(0, yytname[yyx]);
                yysize_overflow |= (yysize1 < yysize);
                yysize = yysize1;
                yyfmt = yystpcpy(yyfmt, yyprefix);
                yyprefix = yyor;
            }

        yyf = YY_(yyformat);
        yysize1 = yysize + yystrlen(yyf);
        yysize_overflow |= (yysize1 < yysize);
        yysize = yysize1;

        if (yysize_overflow)
            return YYSIZE_MAXIMUM;

        if (yyresult) {
            /* Avoid sprintf, as that infringes on the user's name space.
               Don't have undefined behavior even if the translation
               produced a string with the wrong number of "%s"s.  */
            char *yyp = yyresult;
            int yyi = 0;

            while ((*yyp = *yyf) != '\0') {
                if (*yyp == '%' && yyf[1] == 's' && yyi < yycount) {
                    yyp += yytnamerr(yyp, yyarg[yyi++]);
                    yyf += 2;
                } else {
                    yyp++;
                    yyf++;
                }
            }
        }
        return yysize;
    }
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

 /*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct(const char *yymsg, int yytype, YYSTYPE * yyvaluep,
        struct gdbmi_pdata *gdbmi_pdata)
#else
static void yydestruct(yymsg, yytype, yyvaluep, gdbmi_pdata)
     const char *yymsg;
     int yytype;
     YYSTYPE *yyvaluep;
     struct gdbmi_pdata *gdbmi_pdata;
#endif
{
    YYUSE(yyvaluep);
    YYUSE(gdbmi_pdata);

    if (!yymsg)
        yymsg = "Deleting";
    YY_SYMBOL_PRINT(yymsg, yytype, yyvaluep, yylocationp);

    switch (yytype) {

        default:
            break;
    }
}

struct yypstate {
    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

    /* Used to determine if this is the first time this instance has
       been used.  */
    int yynew;
};

/* Initialize the parser data structure.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
yypstate *yypstate_new(void)
#else
yypstate *yypstate_new()
#endif
{
    yypstate *yyps;

    yyps = (yypstate *) malloc(sizeof *yyps);
    if (!yyps)
        return 0;
    yyps->yynew = 1;
    return yyps;
}

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void yypstate_delete(yypstate * yyps)
#else
void yypstate_delete(yyps)
     yypstate *yyps;
#endif
{
#ifndef yyoverflow
    /* If the stack was reallocated but the parse did not complete, then the
       stack still needs to be freed.  */
    if (!yyps->yynew && yyps->yyss != yyps->yyssa)
        YYSTACK_FREE(yyps->yyss);
#endif
    free(yyps);
}

#define gdbmi_nerrs yyps->gdbmi_nerrs
#define yystate yyps->yystate
#define yyerrstatus yyps->yyerrstatus
#define yyssa yyps->yyssa
#define yyss yyps->yyss
#define yyssp yyps->yyssp
#define yyvsa yyps->yyvsa
#define yyvs yyps->yyvs
#define yyvsp yyps->yyvsp
#define yystacksize yyps->yystacksize

/*-------------------------.
| yyparse or yypush_parse.  |
`-------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yypush_parse(yypstate * yyps, int yypushed_char, YYSTYPE const *yypushed_val,
        struct gdbmi_pdata *gdbmi_pdata)
#else
int yypush_parse(yyps, yypushed_char, yypushed_val, gdbmi_pdata)
     yypstate *yyps;
     int yypushed_char;
     YYSTYPE const *yypushed_val;
     struct gdbmi_pdata *gdbmi_pdata;
#endif
{
/* The lookahead symbol.  */
    int yychar;

/* The semantic value of the lookahead symbol.  */
    YYSTYPE yylval;

    int yyn;
    int yyresult;

    /* Lookahead token as an internal (translated) token number.  */
    int yytoken;

    /* The variables used to return semantic value and location from the
       action routines.  */
    YYSTYPE yyval;

#if YYERROR_VERBOSE
    /* Buffer for error messages, and its allocated size.  */
    char yymsgbuf[128];
    char *yymsg = yymsgbuf;
    YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

    /* The number of symbols on the RHS of the reduced rule.
       Keep to zero when no symbol should be popped.  */
    int yylen = 0;

    if (!yyps->yynew) {
        yyn = yypact[yystate];
        goto yyread_pushed_token;
    }

    yytoken = 0;
    yyss = yyssa;
    yyvs = yyvsa;

    yystacksize = YYINITDEPTH;

    YYDPRINTF((stderr, "Starting parse\n"));

    yystate = 0;
    yyerrstatus = 0;
    yynerrs = 0;
    yychar = YYEMPTY;           /* Cause a token to be read.  */

    /* Initialize stack pointers.
       Waste one element of value and location stack
       so that they stay on the same level as the state stack.
       The wasted elements are never initialized.  */
    yyssp = yyss;
    yyvsp = yyvs;

    goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
  yynewstate:
    /* In all cases, when you get here, the value and location stacks
       have just been pushed.  So pushing a state here evens the stacks.  */
    yyssp++;

  yysetstate:
    *yyssp = yystate;

    if (yyss + yystacksize - 1 <= yyssp) {
        /* Get the current used size of the three stacks, in elements.  */
        YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
        {
            /* Give user a chance to reallocate the stack.  Use copies of
               these so that the &'s don't force the real ones into
               memory.  */
            YYSTYPE *yyvs1 = yyvs;
            yytype_int16 *yyss1 = yyss;

            /* Each stack pointer address is followed by the size of the
               data in use in that stack, in bytes.  This used to be a
               conditional around just the two extra args, but that might
               be undefined if yyoverflow is a macro.  */
            yyoverflow(YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp), &yystacksize);

            yyss = yyss1;
            yyvs = yyvs1;
        }
#else /* no yyoverflow */
#ifndef YYSTACK_RELOCATE
        goto yyexhaustedlab;
#else
        /* Extend the stack our own way.  */
        if (YYMAXDEPTH <= yystacksize)
            goto yyexhaustedlab;
        yystacksize *= 2;
        if (YYMAXDEPTH < yystacksize)
            yystacksize = YYMAXDEPTH;

        {
            yytype_int16 *yyss1 = yyss;
            union yyalloc *yyptr =
                    (union yyalloc *) YYSTACK_ALLOC(YYSTACK_BYTES(yystacksize));
            if (!yyptr)
                goto yyexhaustedlab;
            YYSTACK_RELOCATE(yyss_alloc, yyss);
            YYSTACK_RELOCATE(yyvs_alloc, yyvs);
#undef YYSTACK_RELOCATE
            if (yyss1 != yyssa)
                YYSTACK_FREE(yyss1);
        }
#endif
#endif /* no yyoverflow */

        yyssp = yyss + yysize - 1;
        yyvsp = yyvs + yysize - 1;

        YYDPRINTF((stderr, "Stack size increased to %lu\n",
                        (unsigned long int) yystacksize));

        if (yyss + yystacksize - 1 <= yyssp)
            YYABORT;
    }

    YYDPRINTF((stderr, "Entering state %d\n", yystate));

    if (yystate == YYFINAL)
        YYACCEPT;

    goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
  yybackup:

    /* Do appropriate processing given the current state.  Read a
       lookahead token if we need one and don't already have one.  */

    /* First try to decide what to do without reference to lookahead token.  */
    yyn = yypact[yystate];
    if (yyn == YYPACT_NINF)
        goto yydefault;

    /* Not known => get a lookahead token if don't already have one.  */

    /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
    if (yychar == YYEMPTY) {
        if (!yyps->yynew) {
            YYDPRINTF((stderr, "Return for a new token:\n"));
            yyresult = YYPUSH_MORE;
            goto yypushreturn;
        }
        yyps->yynew = 0;

      yyread_pushed_token:
        YYDPRINTF((stderr, "Reading a token: "));
        yychar = yypushed_char;
        if (yypushed_val)
            yylval = *yypushed_val;
    }

    if (yychar <= YYEOF) {
        yychar = yytoken = YYEOF;
        YYDPRINTF((stderr, "Now at end of input.\n"));
    } else {
        yytoken = YYTRANSLATE(yychar);
        YY_SYMBOL_PRINT("Next token is", yytoken, &yylval, &yylloc);
    }

    /* If the proper action on seeing token YYTOKEN is to reduce or to
       detect an error, take that action.  */
    yyn += yytoken;
    if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
        goto yydefault;
    yyn = yytable[yyn];
    if (yyn <= 0) {
        if (yyn == 0 || yyn == YYTABLE_NINF)
            goto yyerrlab;
        yyn = -yyn;
        goto yyreduce;
    }

    /* Count tokens shifted since error; after three, turn off error
       status.  */
    if (yyerrstatus)
        yyerrstatus--;

    /* Shift the lookahead token.  */
    YY_SYMBOL_PRINT("Shifting", yytoken, &yylval, &yylloc);

    /* Discard the shifted token.  */
    yychar = YYEMPTY;

    yystate = yyn;
    *++yyvsp = yylval;

    goto yynewstate;

/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
  yydefault:
    yyn = yydefact[yystate];
    if (yyn == 0)
        goto yyerrlab;
    goto yyreduce;

/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
  yyreduce:
    /* yyn is the number of a rule to reduce with.  */
    yylen = yyr2[yyn];

    /* If YYLEN is nonzero, implement the default value of the action:
       `$$ = $1'.

       Otherwise, the following line sets YYVAL to garbage.
       This behavior is undocumented and Bison
       users should not rely upon it.  Assigning to YYVAL
       unconditionally makes the parser a bit smaller, and it avoids a
       GCC warning that YYVAL may be used uninitialized.  */
    yyval = yyvsp[1 - yylen];

    YY_REDUCE_PRINT(yyn);
    switch (yyn) {
        case 2:

/* Line 1464 of yacc.c  */
#line 96 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
        {
            gdbmi_pdata->tree = (yyvsp[(1) - (1)].u_output);
        }
            break;

        case 3:

/* Line 1464 of yacc.c  */
#line 100 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
        {
            gdbmi_pdata->tree =
                    append_gdbmi_output(gdbmi_pdata->tree,
                    (yyvsp[(2) - (2)].u_output));
            gdbmi_pdata->parsed_one = 1;
        }
            break;

        case 4:

/* Line 1464 of yacc.c  */
#line 105 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
        {
            (yyval.u_output) = create_gdbmi_output();
            (yyval.u_output)->oob_record = (yyvsp[(1) - (6)].u_oob_record);
            (yyval.u_output)->result_record =
                    (yyvsp[(2) - (6)].u_result_record);

            if (strcmp("gdb", (yyvsp[(4) - (6)].u_variable)) != 0)
                gdbmi_error(gdbmi_pdata, "Syntax error, expected 'gdb'");

            free((yyvsp[(4) - (6)].u_variable));
        }
            break;

        case 5:

/* Line 1464 of yacc.c  */
#line 116 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
        {
            (yyval.u_oob_record) = NULL;
        }
            break;

        case 6:

/* Line 1464 of yacc.c  */
#line 120 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
        {
            (yyval.u_oob_record) =
                    append_gdbmi_oob_record((yyvsp[(1) - (3)].u_oob_record),
                    (yyvsp[(2) - (3)].u_oob_record));
        }
            break;

        case 7:

/* Line 1464 of yacc.c  */
#line 124 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
        {
            (yyval.u_result_record) = NULL;
        }
            break;

        case 8:

/* Line 1464 of yacc.c  */
#line 128 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
        {
            (yyval.u_result_record) = (yyvsp[(1) - (2)].u_result_record);
        }
            break;

        case 9:

/* Line 1464 of yacc.c  */
#line 132 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
        {
            (yyval.u_result_record) = create_gdbmi_result_record();
            (yyval.u_result_record)->token = (yyvsp[(1) - (3)].u_token);
            (yyval.u_result_record)->result_class =
                    (yyvsp[(3) - (3)].u_result_class);
            (yyval.u_result_record)->result = NULL;
        }
            break;

        case 10:

/* Line 1464 of yacc.c  */
#line 139 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
        {
            (yyval.u_result_record) = create_gdbmi_result_record();
            (yyval.u_result_record)->token = (yyvsp[(1) - (5)].u_token);
            (yyval.u_result_record)->result_class =
                    (yyvsp[(3) - (5)].u_result_class);
            (yyval.u_result_record)->result = (yyvsp[(5) - (5)].u_result);
        }
            break;

        case 11:

/* Line 1464 of yacc.c  */
#line 146 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
        {
            (yyval.u_oob_record) = create_gdbmi_oob_record();
            (yyval.u_oob_record)->record = GDBMI_ASYNC;
            (yyval.u_oob_record)->option.async_record =
                    (yyvsp[(1) - (1)].u_async_record);
        }
            break;

        case 12:

/* Line 1464 of yacc.c  */
#line 152 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
        {
            (yyval.u_oob_record) = create_gdbmi_oob_record();
            (yyval.u_oob_record)->record = GDBMI_STREAM;
            (yyval.u_oob_record)->option.stream_record =
                    (yyvsp[(1) - (1)].u_stream_record);
        }
            break;

        case 13:

/* Line 1464 of yacc.c  */
#line 158 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
        {
            (yyval.u_async_record) = create_gdbmi_async_record();
            (yyval.u_async_record)->token = (yyvsp[(1) - (3)].u_token);
            (yyval.u_async_record)->async_record =
                    (yyvsp[(2) - (3)].u_async_record_choice);
            (yyval.u_async_record)->async_class =
                    (yyvsp[(3) - (3)].u_async_class);
        }
            break;

        case 14:

/* Line 1464 of yacc.c  */
#line 165 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
        {
            (yyval.u_async_record) = create_gdbmi_async_record();
            (yyval.u_async_record)->token = (yyvsp[(1) - (5)].u_token);
            (yyval.u_async_record)->async_record =
                    (yyvsp[(2) - (5)].u_async_record_choice);
            (yyval.u_async_record)->async_class =
                    (yyvsp[(3) - (5)].u_async_class);
            (yyval.u_async_record)->result = (yyvsp[(5) - (5)].u_result);
        }
            break;

        case 15:

/* Line 1464 of yacc.c  */
#line 173 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
        {
            (yyval.u_async_record_choice) = GDBMI_EXEC;
        }
            break;

        case 16:

/* Line 1464 of yacc.c  */
#line 177 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
        {
            (yyval.u_async_record_choice) = GDBMI_STATUS;
        }
            break;

        case 17:

/* Line 1464 of yacc.c  */
#line 181 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
        {
            (yyval.u_async_record_choice) = GDBMI_NOTIFY;
        }
            break;

        case 18:

/* Line 1464 of yacc.c  */
#line 185 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
        {
            if (strcmp("done", gdbmi_text) == 0)
                (yyval.u_result_class) = GDBMI_DONE;
            else if (strcmp("running", gdbmi_text) == 0)
                (yyval.u_result_class) = GDBMI_RUNNING;
            else if (strcmp("connected", gdbmi_text) == 0)
                (yyval.u_result_class) = GDBMI_CONNECTED;
            else if (strcmp("error", gdbmi_text) == 0)
                (yyval.u_result_class) = GDBMI_ERROR;
            else if (strcmp("exit", gdbmi_text) == 0)
                (yyval.u_result_class) = GDBMI_EXIT;
            else
                gdbmi_error(gdbmi_pdata,
                        "Syntax error, expected 'done|running|connected|error|exit");
        }
            break;

        case 19:

/* Line 1464 of yacc.c  */
#line 200 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
        {
            if (strcmp("stopped", gdbmi_text) != 0)
                gdbmi_error(gdbmi_pdata, "Syntax error, expected 'stopped'");

            (yyval.u_async_class) = GDBMI_STOPPED;
        }
            break;

        case 20:

/* Line 1464 of yacc.c  */
#line 207 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
        {
            (yyval.u_result) =
                    append_gdbmi_result(NULL, (yyvsp[(1) - (1)].u_result));
        }
            break;

        case 21:

/* Line 1464 of yacc.c  */
#line 211 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
        {
            (yyval.u_result) =
                    append_gdbmi_result((yyvsp[(1) - (3)].u_result),
                    (yyvsp[(3) - (3)].u_result));
        }
            break;

        case 22:

/* Line 1464 of yacc.c  */
#line 215 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
        {
            (yyval.u_result) = create_gdbmi_result();
            (yyval.u_result)->variable = (yyvsp[(1) - (3)].u_variable);
            (yyval.u_result)->value = (yyvsp[(3) - (3)].u_value);
        }
            break;

        case 23:

/* Line 1464 of yacc.c  */
#line 221 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
        {
            (yyval.u_variable) = strdup(gdbmi_text);
        }
            break;

        case 24:

/* Line 1464 of yacc.c  */
#line 225 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
        {
            (yyval.u_value) =
                    append_gdbmi_value(NULL, (yyvsp[(1) - (1)].u_value));
        }
            break;

        case 25:

/* Line 1464 of yacc.c  */
#line 229 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
        {
            (yyval.u_value) =
                    append_gdbmi_value((yyvsp[(1) - (3)].u_value),
                    (yyvsp[(3) - (3)].u_value));
        }
            break;

        case 26:

/* Line 1464 of yacc.c  */
#line 233 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
        {
            (yyval.u_value) = create_gdbmi_value();
            (yyval.u_value)->value_choice = GDBMI_CSTRING;
            (yyval.u_value)->option.cstring = strdup(gdbmi_text);
        }
            break;

        case 27:

/* Line 1464 of yacc.c  */
#line 239 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
        {
            (yyval.u_value) = create_gdbmi_value();
            (yyval.u_value)->value_choice = GDBMI_TUPLE;
            (yyval.u_value)->option.tuple = (yyvsp[(1) - (1)].u_tuple);
        }
            break;

        case 28:

/* Line 1464 of yacc.c  */
#line 245 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
        {
            (yyval.u_value) = create_gdbmi_value();
            (yyval.u_value)->value_choice = GDBMI_LIST;
            (yyval.u_value)->option.list = (yyvsp[(1) - (1)].u_list);
        }
            break;

        case 29:

/* Line 1464 of yacc.c  */
#line 251 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
        {
            (yyval.u_tuple) = NULL;
        }
            break;

        case 30:

/* Line 1464 of yacc.c  */
#line 255 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
        {
            (yyval.u_tuple) = create_gdbmi_tuple();
            (yyval.u_tuple)->result = (yyvsp[(2) - (3)].u_result);
        }
            break;

        case 31:

/* Line 1464 of yacc.c  */
#line 260 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
        {
            (yyval.u_list) = NULL;
        }
            break;

        case 32:

/* Line 1464 of yacc.c  */
#line 264 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
        {
            (yyval.u_list) = create_gdbmi_list();
            (yyval.u_list)->list_choice = GDBMI_VALUE;
            (yyval.u_list)->option.value = (yyvsp[(2) - (3)].u_value);
        }
            break;

        case 33:

/* Line 1464 of yacc.c  */
#line 270 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
        {
            (yyval.u_list) = create_gdbmi_list();
            (yyval.u_list)->list_choice = GDBMI_RESULT;
            (yyval.u_list)->option.result = (yyvsp[(2) - (3)].u_result);
        }
            break;

        case 34:

/* Line 1464 of yacc.c  */
#line 276 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
        {
            (yyval.u_stream_record) = create_gdbmi_stream_record();
            (yyval.u_stream_record)->stream_record =
                    (yyvsp[(1) - (2)].u_stream_record_choice);
            (yyval.u_stream_record)->cstring = strdup(gdbmi_text);
        }
            break;

        case 35:

/* Line 1464 of yacc.c  */
#line 282 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
        {
            (yyval.u_stream_record_choice) = GDBMI_CONSOLE;
        }
            break;

        case 36:

/* Line 1464 of yacc.c  */
#line 286 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
        {
            (yyval.u_stream_record_choice) = GDBMI_TARGET;
        }
            break;

        case 37:

/* Line 1464 of yacc.c  */
#line 290 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
        {
            (yyval.u_stream_record_choice) = GDBMI_LOG;
        }
            break;

        case 38:

/* Line 1464 of yacc.c  */
#line 294 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
        {
            (yyval.u_token) = -1;
        }
            break;

        case 39:

/* Line 1464 of yacc.c  */
#line 298 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
        {
            (yyval.u_token) = (yyvsp[(1) - (1)].u_token);
        }
            break;

        case 40:

/* Line 1464 of yacc.c  */
#line 302 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
        {
            (yyval.u_token) = atol(gdbmi_text);
        }
            break;

/* Line 1464 of yacc.c  */
#line 1934 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.c"
        default:
            break;
    }
    YY_SYMBOL_PRINT("-> $$ =", yyr1[yyn], &yyval, &yyloc);

    YYPOPSTACK(yylen);
    yylen = 0;
    YY_STACK_PRINT(yyss, yyssp);

    *++yyvsp = yyval;

    /* Now `shift' the result of the reduction.  Determine what state
       that goes to, based on the state we popped back to and the rule
       number reduced by.  */

    yyn = yyr1[yyn];

    yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
    if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
        yystate = yytable[yystate];
    else
        yystate = yydefgoto[yyn - YYNTOKENS];

    goto yynewstate;

/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
  yyerrlab:
    /* If not already recovering from an error, report this error.  */
    if (!yyerrstatus) {
        ++yynerrs;
#if ! YYERROR_VERBOSE
        yyerror(gdbmi_pdata, YY_("syntax error"));
#else
        {
            YYSIZE_T yysize = yysyntax_error(0, yystate, yychar);

            if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM) {
                YYSIZE_T yyalloc = 2 * yysize;

                if (!(yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
                    yyalloc = YYSTACK_ALLOC_MAXIMUM;
                if (yymsg != yymsgbuf)
                    YYSTACK_FREE(yymsg);
                yymsg = (char *) YYSTACK_ALLOC(yyalloc);
                if (yymsg)
                    yymsg_alloc = yyalloc;
                else {
                    yymsg = yymsgbuf;
                    yymsg_alloc = sizeof yymsgbuf;
                }
            }

            if (0 < yysize && yysize <= yymsg_alloc) {
                (void) yysyntax_error(yymsg, yystate, yychar);
                yyerror(gdbmi_pdata, yymsg);
            } else {
                yyerror(gdbmi_pdata, YY_("syntax error"));
                if (yysize != 0)
                    goto yyexhaustedlab;
            }
        }
#endif
    }

    if (yyerrstatus == 3) {
        /* If just tried and failed to reuse lookahead token after an
           error, discard it.  */

        if (yychar <= YYEOF) {
            /* Return failure if at end of input.  */
            if (yychar == YYEOF)
                YYABORT;
        } else {
            yydestruct("Error: discarding", yytoken, &yylval, gdbmi_pdata);
            yychar = YYEMPTY;
        }
    }

    /* Else will try to reuse lookahead token after shifting the error
       token.  */
    goto yyerrlab1;

/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
  yyerrorlab:

    /* Pacify compilers like GCC when the user code never invokes
       YYERROR and the label yyerrorlab therefore never appears in user
       code.  */
    if ( /*CONSTCOND*/ 0)
        goto yyerrorlab;

    /* Do not reclaim the symbols of the rule which action triggered
       this YYERROR.  */
    YYPOPSTACK(yylen);
    yylen = 0;
    YY_STACK_PRINT(yyss, yyssp);
    yystate = *yyssp;
    goto yyerrlab1;

/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
  yyerrlab1:
    yyerrstatus = 3;            /* Each real token shifted decrements this.  */

    for (;;) {
        yyn = yypact[yystate];
        if (yyn != YYPACT_NINF) {
            yyn += YYTERROR;
            if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR) {
                yyn = yytable[yyn];
                if (0 < yyn)
                    break;
            }
        }

        /* Pop the current state because it cannot handle the error token.  */
        if (yyssp == yyss)
            YYABORT;

        yydestruct("Error: popping", yystos[yystate], yyvsp, gdbmi_pdata);
        YYPOPSTACK(1);
        yystate = *yyssp;
        YY_STACK_PRINT(yyss, yyssp);
    }

    *++yyvsp = yylval;

    /* Shift the error token.  */
    YY_SYMBOL_PRINT("Shifting", yystos[yyn], yyvsp, yylsp);

    yystate = yyn;
    goto yynewstate;

/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
  yyacceptlab:
    yyresult = 0;
    goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
  yyabortlab:
    yyresult = 1;
    goto yyreturn;

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
  yyexhaustedlab:
    yyerror(gdbmi_pdata, YY_("memory exhausted"));
    yyresult = 2;
    /* Fall through.  */
#endif

  yyreturn:
    if (yychar != YYEMPTY)
        yydestruct("Cleanup: discarding lookahead",
                yytoken, &yylval, gdbmi_pdata);
    /* Do not reclaim the symbols of the rule which action triggered
       this YYABORT or YYACCEPT.  */
    YYPOPSTACK(yylen);
    YY_STACK_PRINT(yyss, yyssp);
    while (yyssp != yyss) {
        yydestruct("Cleanup: popping", yystos[*yyssp], yyvsp, gdbmi_pdata);
        YYPOPSTACK(1);
    }
#ifndef yyoverflow
    if (yyss != yyssa)
        YYSTACK_FREE(yyss);
#endif
    yyps->yynew = 1;

  yypushreturn:
#if YYERROR_VERBOSE
    if (yymsg != yymsgbuf)
        YYSTACK_FREE(yymsg);
#endif
    /* Make sure YYID is used.  */
    return YYID(yyresult);
}
