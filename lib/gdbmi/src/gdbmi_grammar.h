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
/* Line 1318 of yacc.c.  */
#line 95 "gdbmi_grammar.h"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE gdbmi_lval;



