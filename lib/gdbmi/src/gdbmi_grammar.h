
/* A Bison parser, made by GNU Bison 2.3a+.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
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

/* "%code requires" blocks.  */

/* Line 1685 of yacc.c  */
#line 5 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.y"
struct gdbmi_pdata;

/* Line 1685 of yacc.c  */
#line 44 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.h"

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

/* Line 1685 of yacc.c  */
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

/* Line 1685 of yacc.c  */
#line 121 "../../../../cgdb/lib/gdbmi/src/gdbmi_grammar.h"
} YYSTYPE;

#define YYSTYPE_IS_TRIVIAL 1
#define yystype YYSTYPE         /* obsolescent; will be withdrawn */
#define YYSTYPE_IS_DECLARED 1
#endif

#ifndef YYPUSH_DECLS
#define YYPUSH_DECLS
struct gdbmi_pstate;
typedef struct gdbmi_pstate gdbmi_pstate;
enum { YYPUSH_MORE = 4 };

#if defined __STDC__ || defined __cplusplus
int gdbmi_push_parse(gdbmi_pstate * yyps, int yypushed_char,
        YYSTYPE const *yypushed_val, struct gdbmi_pdata *gdbmi_pdata);
#else
int gdbmi_push_parse();
#endif

#if defined __STDC__ || defined __cplusplus
gdbmi_pstate *gdbmi_pstate_new(void);
#else
gdbmi_pstate *gdbmi_pstate_new();
#endif
#if defined __STDC__ || defined __cplusplus
void gdbmi_pstate_delete(gdbmi_pstate * yyps);
#else
void gdbmi_pstate_delete();
#endif
#endif
