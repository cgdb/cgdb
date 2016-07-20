/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

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

#ifndef YY_GDBMI_GDBMI_GRAMMAR_H_INCLUDED
# define YY_GDBMI_GDBMI_GRAMMAR_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int gdbmi_debug;
#endif
/* "%code requires" blocks.  */
#line 5 "gdbmi_grammar.y" /* yacc.c:1909  */
 struct gdbmi_pdata; 

#line 47 "gdbmi_grammar.h" /* yacc.c:1909  */

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
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

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 53 "gdbmi_grammar.y" /* yacc.c:1909  */

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

#line 117 "gdbmi_grammar.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



#ifndef YYPUSH_MORE_DEFINED
# define YYPUSH_MORE_DEFINED
enum { YYPUSH_MORE = 4 };
#endif

typedef struct gdbmi_pstate gdbmi_pstate;

int gdbmi_push_parse (gdbmi_pstate *ps, int pushed_char, YYSTYPE const *pushed_val, struct gdbmi_pdata *gdbmi_pdata);

gdbmi_pstate * gdbmi_pstate_new (void);
void gdbmi_pstate_delete (gdbmi_pstate *ps);

#endif /* !YY_GDBMI_GDBMI_GRAMMAR_H_INCLUDED  */
