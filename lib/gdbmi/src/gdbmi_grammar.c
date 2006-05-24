/* A Bison parser, made by GNU Bison 2.0.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Written by Richard Stallman by simplifying the original so called
   ``semantic'' parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0

/* Substitute the variable and function names.  */
#define yyparse gdbmi_parse
#define yylex   gdbmi_lex
#define yyerror gdbmi_error
#define yylval  gdbmi_lval
#define yychar  gdbmi_char
#define yydebug gdbmi_debug
#define yynerrs gdbmi_nerrs


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
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




/* Copy the first part of user declarations.  */
#line 4 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "gdbmi_pt.h"

extern char *gdbmi_text;
extern int gdbmi_lex (void);
void gdbmi_error (const char *s);
extern int gdbmi_lineno;
gdbmi_output_ptr tree;

void gdbmi_error (const char *s)
{ 
  fprintf (stderr, "%s:%d Error %s", __FILE__, __LINE__, s);
  if (strcmp (gdbmi_text, "\n") == 0)
    fprintf (stderr, "%s:%d at end of line %d\n", __FILE__, __LINE__, 
	     gdbmi_lineno);
  else 
    {
      fprintf (stderr, "%s:%d at token(%s), line (%d)\n", __FILE__, __LINE__, 
	       gdbmi_text, gdbmi_lineno );
      gdbmi_lex();
      fprintf (stderr, "%s:%d before (%s)\n", __FILE__, __LINE__, gdbmi_text);
    }
}


/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 51 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
typedef union YYSTYPE {
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
} YYSTYPE;
/* Line 190 of yacc.c.  */
#line 170 "gdbmi_grammar.c"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 213 of yacc.c.  */
#line 182 "gdbmi_grammar.c"

#if ! defined (yyoverflow) || YYERROR_VERBOSE

# ifndef YYFREE
#  define YYFREE free
# endif
# ifndef YYMALLOC
#  define YYMALLOC malloc
# endif

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   else
#    define YYSTACK_ALLOC alloca
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (defined (YYSTYPE_IS_TRIVIAL) && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short int yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short int) + sizeof (YYSTYPE))			\
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined (__GNUC__) && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short int yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  5
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   50

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  21
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  23
/* YYNRULES -- Number of rules. */
#define YYNRULES  42
/* YYNRULES -- Number of states. */
#define YYNSTATES  62

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   275

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned char yyprhs[] =
{
       0,     0,     3,     4,     6,     8,    11,    18,    19,    23,
      24,    27,    31,    37,    39,    41,    45,    51,    53,    55,
      57,    59,    61,    63,    67,    71,    73,    75,    79,    81,
      83,    85,    88,    92,    95,    99,   103,   106,   108,   110,
     112,   113,   115
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      22,     0,    -1,    -1,    23,    -1,    24,    -1,    23,    24,
      -1,    25,    26,     5,    35,     6,    15,    -1,    -1,    25,
      28,    15,    -1,    -1,    27,    15,    -1,    42,    20,    31,
      -1,    42,    20,    31,    19,    33,    -1,    29,    -1,    40,
      -1,    42,    30,    32,    -1,    42,    30,    32,    19,    33,
      -1,     8,    -1,     7,    -1,     9,    -1,    17,    -1,    17,
      -1,    34,    -1,    33,    19,    34,    -1,    35,     9,    37,
      -1,    17,    -1,    37,    -1,    36,    19,    37,    -1,    18,
      -1,    38,    -1,    39,    -1,     3,     4,    -1,     3,    33,
       4,    -1,    13,    14,    -1,    13,    36,    14,    -1,    13,
      33,    14,    -1,    41,    18,    -1,    10,    -1,    11,    -1,
      12,    -1,    -1,    43,    -1,    16,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short int yyrline[] =
{
       0,    95,    95,    99,   103,   107,   111,   122,   126,   130,
     134,   138,   145,   152,   158,   164,   171,   179,   183,   187,
     191,   206,   213,   217,   221,   227,   231,   235,   239,   245,
     251,   257,   261,   266,   270,   276,   282,   288,   292,   296,
     300,   304,   308
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "OPEN_BRACE", "CLOSED_BRACE",
  "OPEN_PAREN", "CLOSED_PAREN", "ADD_OP", "MULT_OP", "EQUAL_SIGN", "TILDA",
  "AT_SYMBOL", "AMPERSAND", "OPEN_BRACKET", "CLOSED_BRACKET", "NEWLINE",
  "INTEGER_LITERAL", "STRING_LITERAL", "CSTRING", "COMMA", "CARROT",
  "$accept", "opt_output_list", "output_list", "output",
  "opt_oob_record_list", "opt_result_record", "result_record",
  "oob_record", "async_record", "async_record_class", "result_class",
  "async_class", "result_list", "result", "variable", "value_list",
  "value", "tuple", "list", "stream_record", "stream_record_class",
  "opt_token", "token", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short int yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    21,    22,    22,    23,    23,    24,    25,    25,    26,
      26,    27,    27,    28,    28,    29,    29,    30,    30,    30,
      31,    32,    33,    33,    34,    35,    36,    36,    37,    37,
      37,    38,    38,    39,    39,    39,    40,    41,    41,    41,
      42,    42,    43
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     0,     1,     1,     2,     6,     0,     3,     0,
       2,     3,     5,     1,     1,     3,     5,     1,     1,     1,
       1,     1,     1,     3,     3,     1,     1,     3,     1,     1,
       1,     2,     3,     2,     3,     3,     2,     1,     1,     1,
       0,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       7,     0,     7,     4,    40,     1,     5,    37,    38,    39,
      42,     0,     0,     0,    13,    14,     0,     0,    41,     0,
      10,     8,    36,    18,    17,    19,     0,     0,    25,     0,
      20,    11,    21,    15,     0,     0,     0,     6,    12,    22,
       0,    16,     0,     0,    23,     0,     0,    28,    24,    29,
      30,    31,     0,    33,     0,     0,    26,    32,    35,    34,
       0,    27
};

/* YYDEFGOTO[NTERM-NUM]. */
static const yysigned_char yydefgoto[] =
{
      -1,     1,     2,     3,     4,    11,    12,    13,    14,    27,
      31,    33,    38,    39,    40,    55,    48,    49,    50,    15,
      16,    17,    18
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -41
static const yysigned_char yypact[] =
{
       7,     9,    28,   -41,    19,   -41,   -41,   -41,   -41,   -41,
     -41,    -4,    -2,    18,   -41,   -41,    20,    14,   -41,    22,
     -41,   -41,   -41,   -41,   -41,   -41,    23,    24,   -41,    30,
     -41,    25,   -41,    26,    27,    22,    22,   -41,    29,   -41,
      34,    29,    22,    -1,   -41,     1,    -3,   -41,   -41,   -41,
     -41,   -41,     0,   -41,   -11,    13,   -41,   -41,   -41,   -41,
      -1,   -41
};

/* YYPGOTO[NTERM-NUM].  */
static const yysigned_char yypgoto[] =
{
     -41,   -41,   -41,    35,   -41,   -41,   -41,   -41,   -41,   -41,
     -41,   -41,   -20,     4,    31,   -41,   -40,   -41,   -41,   -41,
     -41,   -41,   -41
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -10
static const yysigned_char yytable[] =
{
      45,    19,    45,    58,    57,    51,    56,    -2,    42,     5,
      46,    53,    46,    20,    28,    47,    41,    47,    28,    42,
      61,    23,    24,    25,    -9,    52,    54,    59,    -3,     7,
       8,     9,    60,    21,    26,    10,    34,     6,    22,    28,
      30,    32,    37,    43,    35,    36,    44,     0,    42,     0,
      29
};

static const yysigned_char yycheck[] =
{
       3,     5,     3,    14,     4,     4,    46,     0,    19,     0,
      13,    14,    13,    15,    17,    18,    36,    18,    17,    19,
      60,     7,     8,     9,     5,    45,    46,    14,     0,    10,
      11,    12,    19,    15,    20,    16,     6,     2,    18,    17,
      17,    17,    15,     9,    19,    19,    42,    -1,    19,    -1,
      19
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    22,    23,    24,    25,     0,    24,    10,    11,    12,
      16,    26,    27,    28,    29,    40,    41,    42,    43,     5,
      15,    15,    18,     7,     8,     9,    20,    30,    17,    35,
      17,    31,    17,    32,     6,    19,    19,    15,    33,    34,
      35,    33,    19,     9,    34,     3,    13,    18,    37,    38,
      39,     4,    33,    14,    33,    36,    37,     4,    14,    14,
      19,    37
};

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

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
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");\
      YYERROR;							\
    }								\
while (0)


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (N)								\
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
    while (0)
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
              (Loc).first_line, (Loc).first_column,	\
              (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (0)

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)		\
do {								\
  if (yydebug)							\
    {								\
      YYFPRINTF (stderr, "%s ", Title);				\
      yysymprint (stderr, 					\
                  Type, Value);	\
      YYFPRINTF (stderr, "\n");					\
    }								\
} while (0)

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short int *bottom, short int *top)
#else
static void
yy_stack_print (bottom, top)
    short int *bottom;
    short int *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (/* Nothing. */; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_reduce_print (int yyrule)
#else
static void
yy_reduce_print (yyrule)
    int yyrule;
#endif
{
  int yyi;
  unsigned int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %u), ",
             yyrule - 1, yylno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname [yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname [yyr1[yyrule]]);
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (Rule);		\
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

#endif /* !YYERROR_VERBOSE */



#if YYDEBUG
/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yysymprint (FILE *yyoutput, int yytype, YYSTYPE *yyvaluep)
#else
static void
yysymprint (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);


# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyoutput, ")");
}

#endif /* ! YYDEBUG */
/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
        break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM);
# else
int yyparse ();
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM)
# else
int yyparse (YYPARSE_PARAM)
  void *YYPARSE_PARAM;
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short int yyssa[YYINITDEPTH];
  short int *yyss = yyssa;
  register short int *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;



#define YYPOPSTACK   (yyvsp--, yyssp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;


  yyvsp[0] = yylval;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short int *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	short int *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a look-ahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to look-ahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;


  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
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
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 95 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
    {
  tree = NULL;
}
    break;

  case 3:
#line 99 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
    {
  tree = (yyvsp[0].u_output);	
}
    break;

  case 4:
#line 103 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
    {
  (yyval.u_output) = (yyvsp[0].u_output);
}
    break;

  case 5:
#line 107 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
    {
  (yyval.u_output) = append_gdbmi_output ((yyvsp[-1].u_output), (yyvsp[0].u_output));
}
    break;

  case 6:
#line 111 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
    { 
  (yyval.u_output) = create_gdbmi_output ();
  (yyval.u_output)->oob_record = (yyvsp[-5].u_oob_record);
  (yyval.u_output)->result_record = (yyvsp[-4].u_result_record);

  if (strcmp ("gdb", (yyvsp[-2].u_variable)) != 0)
    gdbmi_error ("Syntax error, expected 'gdb'");

  free ((yyvsp[-2].u_variable));
}
    break;

  case 7:
#line 122 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
    {
  (yyval.u_oob_record) = NULL;
}
    break;

  case 8:
#line 126 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
    {
  (yyval.u_oob_record) = append_gdbmi_oob_record ((yyvsp[-2].u_oob_record), (yyvsp[-1].u_oob_record));
}
    break;

  case 9:
#line 130 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
    {
  (yyval.u_result_record) = NULL;
}
    break;

  case 10:
#line 134 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
    {
  (yyval.u_result_record) = (yyvsp[-1].u_result_record);
}
    break;

  case 11:
#line 138 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
    {
  (yyval.u_result_record) = create_gdbmi_result_record ();
  (yyval.u_result_record)->token = (yyvsp[-2].u_token);
  (yyval.u_result_record)->result_class = (yyvsp[0].u_result_class);
  (yyval.u_result_record)->result = NULL;
}
    break;

  case 12:
#line 145 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
    {
  (yyval.u_result_record) = create_gdbmi_result_record ();
  (yyval.u_result_record)->token = (yyvsp[-4].u_token);
  (yyval.u_result_record)->result_class = (yyvsp[-2].u_result_class);
  (yyval.u_result_record)->result = (yyvsp[0].u_result);
}
    break;

  case 13:
#line 152 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
    {
  (yyval.u_oob_record) = create_gdbmi_oob_record();
  (yyval.u_oob_record)->record = GDBMI_ASYNC;
  (yyval.u_oob_record)->option.async_record = (yyvsp[0].u_async_record);
}
    break;

  case 14:
#line 158 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
    {
  (yyval.u_oob_record) = create_gdbmi_oob_record();
  (yyval.u_oob_record)->record = GDBMI_STREAM;
  (yyval.u_oob_record)->option.stream_record = (yyvsp[0].u_stream_record);
}
    break;

  case 15:
#line 164 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
    {
  (yyval.u_async_record) = create_gdbmi_async_record ();
  (yyval.u_async_record)->token = (yyvsp[-2].u_token);
  (yyval.u_async_record)->async_record = (yyvsp[-1].u_async_record_choice);
  (yyval.u_async_record)->async_class = (yyvsp[0].u_async_class);
}
    break;

  case 16:
#line 171 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
    {
  (yyval.u_async_record) = create_gdbmi_async_record ();
  (yyval.u_async_record)->token = (yyvsp[-4].u_token);
  (yyval.u_async_record)->async_record = (yyvsp[-3].u_async_record_choice);
  (yyval.u_async_record)->async_class = (yyvsp[-2].u_async_class);
  (yyval.u_async_record)->result = (yyvsp[0].u_result);
}
    break;

  case 17:
#line 179 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
    {
  (yyval.u_async_record_choice) = GDBMI_EXEC;
}
    break;

  case 18:
#line 183 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
    {
  (yyval.u_async_record_choice) = GDBMI_STATUS;
}
    break;

  case 19:
#line 187 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
    {
  (yyval.u_async_record_choice) = GDBMI_NOTIFY;	
}
    break;

  case 20:
#line 191 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
    {
  if (strcmp ("done", gdbmi_text) == 0)
    (yyval.u_result_class) = GDBMI_DONE;
  else if (strcmp ("running", gdbmi_text) == 0)
    (yyval.u_result_class) = GDBMI_RUNNING;
  else if (strcmp ("connected", gdbmi_text) == 0)
    (yyval.u_result_class) = GDBMI_CONNECTED;
  else if (strcmp ("error", gdbmi_text) == 0)
    (yyval.u_result_class) = GDBMI_ERROR;
  else if (strcmp ("exit", gdbmi_text) == 0)
    (yyval.u_result_class) = GDBMI_EXIT;
  else
    gdbmi_error ("Syntax error, expected 'done|running|connected|error|exit");
}
    break;

  case 21:
#line 206 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
    {
  if (strcmp ("stopped", gdbmi_text) != 0)
    gdbmi_error ( "Syntax error, expected 'stopped'" );

  (yyval.u_async_class) = GDBMI_STOPPED;
}
    break;

  case 22:
#line 213 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
    {
  (yyval.u_result) = append_gdbmi_result (NULL, (yyvsp[0].u_result));	
}
    break;

  case 23:
#line 217 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
    {
  (yyval.u_result) = append_gdbmi_result ((yyvsp[-2].u_result), (yyvsp[0].u_result));
}
    break;

  case 24:
#line 221 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
    {
  (yyval.u_result) = create_gdbmi_result ();
  (yyval.u_result)->variable = (yyvsp[-2].u_variable);
  (yyval.u_result)->value = (yyvsp[0].u_value);
}
    break;

  case 25:
#line 227 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
    {
  (yyval.u_variable) = strdup (gdbmi_text);
}
    break;

  case 26:
#line 231 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
    {
  (yyval.u_value) = append_gdbmi_value (NULL, (yyvsp[0].u_value));	
}
    break;

  case 27:
#line 235 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
    {
  (yyval.u_value) = append_gdbmi_value ((yyvsp[-2].u_value), (yyvsp[0].u_value)); 
}
    break;

  case 28:
#line 239 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
    {
  (yyval.u_value) = create_gdbmi_value ();
  (yyval.u_value)->value_choice = GDBMI_CSTRING;
  (yyval.u_value)->option.cstring = strdup (gdbmi_text); 
}
    break;

  case 29:
#line 245 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
    {
  (yyval.u_value) = create_gdbmi_value ();
  (yyval.u_value)->value_choice = GDBMI_TUPLE;
  (yyval.u_value)->option.tuple = (yyvsp[0].u_tuple);
}
    break;

  case 30:
#line 251 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
    {
  (yyval.u_value) = create_gdbmi_value ();
  (yyval.u_value)->value_choice = GDBMI_LIST;
  (yyval.u_value)->option.list = (yyvsp[0].u_list);
}
    break;

  case 31:
#line 257 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
    {
  (yyval.u_tuple) = NULL;
}
    break;

  case 32:
#line 261 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
    {
  (yyval.u_tuple) = create_gdbmi_tuple ();
  (yyval.u_tuple)->result = (yyvsp[-1].u_result);
}
    break;

  case 33:
#line 266 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
    {
  (yyval.u_list) = NULL;
}
    break;

  case 34:
#line 270 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
    {
  (yyval.u_list) = create_gdbmi_list ();
  (yyval.u_list)->list_choice = GDBMI_VALUE;
  (yyval.u_list)->option.value = (yyvsp[-1].u_value);
}
    break;

  case 35:
#line 276 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
    {
  (yyval.u_list) = create_gdbmi_list ();
  (yyval.u_list)->list_choice = GDBMI_RESULT;
  (yyval.u_list)->option.result = (yyvsp[-1].u_result);
}
    break;

  case 36:
#line 282 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
    {
  (yyval.u_stream_record) = create_gdbmi_stream_record ();
  (yyval.u_stream_record)->stream_record = (yyvsp[-1].u_stream_record_choice);
  (yyval.u_stream_record)->cstring = strdup ( gdbmi_text );
}
    break;

  case 37:
#line 288 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
    {
  (yyval.u_stream_record_choice) = GDBMI_CONSOLE;
}
    break;

  case 38:
#line 292 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
    {
  (yyval.u_stream_record_choice) = GDBMI_TARGET;
}
    break;

  case 39:
#line 296 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
    {
  (yyval.u_stream_record_choice) = GDBMI_LOG;
}
    break;

  case 40:
#line 300 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
    {
  (yyval.u_token) = -1;	
}
    break;

  case 41:
#line 304 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
    {
  (yyval.u_token) = (yyvsp[0].u_token);
}
    break;

  case 42:
#line 308 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
    {
  (yyval.u_token) = atol (gdbmi_text);
}
    break;


    }

/* Line 1037 of yacc.c.  */
#line 1487 "gdbmi_grammar.c"

  yyvsp -= yylen;
  yyssp -= yylen;


  YY_STACK_PRINT (yyss, yyssp);

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
  if (!yyerrstatus)
    {
      ++yynerrs;
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  int yytype = YYTRANSLATE (yychar);
	  const char* yyprefix;
	  char *yymsg;
	  int yyx;

	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  int yyxbegin = yyn < 0 ? -yyn : 0;

	  /* Stay within bounds of both yycheck and yytname.  */
	  int yychecklim = YYLAST - yyn;
	  int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
	  int yycount = 0;

	  yyprefix = ", expecting ";
	  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      {
		yysize += yystrlen (yyprefix) + yystrlen (yytname [yyx]);
		yycount += 1;
		if (yycount == 5)
		  {
		    yysize = 0;
		    break;
		  }
	      }
	  yysize += (sizeof ("syntax error, unexpected ")
		     + yystrlen (yytname[yytype]));
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "syntax error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[yytype]);

	      if (yycount < 5)
		{
		  yyprefix = ", expecting ";
		  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
		    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
		      {
			yyp = yystpcpy (yyp, yyprefix);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yyprefix = " or ";
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("syntax error; also virtual memory exhausted");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror ("syntax error");
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* If at end of input, pop the error token,
	     then the rest of the stack, then return failure.  */
	  if (yychar == YYEOF)
	     for (;;)
	       {

		 YYPOPSTACK;
		 if (yyssp == yyss)
		   YYABORT;
		 yydestruct ("Error: popping",
                             yystos[*yyssp], yyvsp);
	       }
        }
      else
	{
	  yydestruct ("Error: discarding", yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

#ifdef __GNUC__
  /* Pacify GCC when the user code never invokes YYERROR and the label
     yyerrorlab therefore never appears in user code.  */
  if (0)
     goto yyerrorlab;
#endif

yyvsp -= yylen;
  yyssp -= yylen;
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping", yystos[yystate], yyvsp);
      YYPOPSTACK;
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


  /* Shift the error token. */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

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
  yydestruct ("Error: discarding lookahead",
              yytoken, &yylval);
  yychar = YYEMPTY;
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*----------------------------------------------.
| yyoverflowlab -- parser overflow comes here.  |
`----------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}



