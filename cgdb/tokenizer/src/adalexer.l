%option prefix="ada_"
%option outfile="lex.yy.c"
%option case-insensitive

D                       [0-9]
L                       [a-zA-Z_]

%{

/* System Includes */
#include <stdio.h>
#include "tokenizer.h"

%}

%%
"array"                 { return(TOKENIZER_TYPE); }
"boolean"               { return(TOKENIZER_TYPE); }
"character"             { return(TOKENIZER_TYPE); }
"constant"              { return(TOKENIZER_TYPE); }
"fixed"                 { return(TOKENIZER_TYPE); }
"float"                 { return(TOKENIZER_TYPE); }
"integer"               { return(TOKENIZER_TYPE); }
"long_float"            { return(TOKENIZER_TYPE); }
"long_long_float"       { return(TOKENIZER_TYPE); }
"long_integer"          { return(TOKENIZER_TYPE); }
"long_long_integer"     { return(TOKENIZER_TYPE); }
"natural"               { return(TOKENIZER_TYPE); }
"positive"              { return(TOKENIZER_TYPE); }
"short_float"           { return(TOKENIZER_TYPE); }
"short_integer"         { return(TOKENIZER_TYPE); }
"short_short_integer"   { return(TOKENIZER_TYPE); }
"string"                { return(TOKENIZER_TYPE); }

"abort"                 { return(TOKENIZER_KEYWORD); }
"abs"                   { return(TOKENIZER_KEYWORD); }
"abstract"              { return(TOKENIZER_KEYWORD); }
"accept"                { return(TOKENIZER_KEYWORD); }
"access"                { return(TOKENIZER_KEYWORD); }
"aliased"               { return(TOKENIZER_KEYWORD); }
"all"                   { return(TOKENIZER_KEYWORD); }
"and"                   { return(TOKENIZER_KEYWORD); }
"at"                    { return(TOKENIZER_KEYWORD); }
"begin"                 { return(TOKENIZER_KEYWORD); }
"body"                  { return(TOKENIZER_KEYWORD); }
"case"                  { return(TOKENIZER_KEYWORD); }
"declare"               { return(TOKENIZER_KEYWORD); }
"delay"                 { return(TOKENIZER_KEYWORD); }
"delta"                 { return(TOKENIZER_KEYWORD); }
"digits"                { return(TOKENIZER_KEYWORD); }
"do"                    { return(TOKENIZER_KEYWORD); }
"else"                  { return(TOKENIZER_KEYWORD); }
"elsif"                 { return(TOKENIZER_KEYWORD); }
"end"                   { return(TOKENIZER_KEYWORD); }
"entry"                 { return(TOKENIZER_KEYWORD); }
"exception"             { return(TOKENIZER_KEYWORD); }
"exit"                  { return(TOKENIZER_KEYWORD); }
"for"                   { return(TOKENIZER_KEYWORD); }
"function"              { return(TOKENIZER_KEYWORD); }
"generic"               { return(TOKENIZER_KEYWORD); }
"goto"                  { return(TOKENIZER_KEYWORD); }
"if"                    { return(TOKENIZER_KEYWORD); }
"in"                    { return(TOKENIZER_KEYWORD); }
"is"                    { return(TOKENIZER_KEYWORD); }
"limited"               { return(TOKENIZER_KEYWORD); }
"loop"                  { return(TOKENIZER_KEYWORD); }
"mod"                   { return(TOKENIZER_KEYWORD); }
"new"                   { return(TOKENIZER_KEYWORD); }
"not"                   { return(TOKENIZER_KEYWORD); }
"null"                  { return(TOKENIZER_KEYWORD); }
"of"                    { return(TOKENIZER_KEYWORD); }
"or"                    { return(TOKENIZER_KEYWORD); }
"others"                { return(TOKENIZER_KEYWORD); }
"out"                   { return(TOKENIZER_KEYWORD); }
"package"               { return(TOKENIZER_KEYWORD); }
"pragma"                { return(TOKENIZER_KEYWORD); }
"private"               { return(TOKENIZER_KEYWORD); }
"procedure"             { return(TOKENIZER_KEYWORD); }
"protected"             { return(TOKENIZER_KEYWORD); }
"raise"                 { return(TOKENIZER_KEYWORD); }
"range"                 { return(TOKENIZER_KEYWORD); }
"record",               { return(TOKENIZER_KEYWORD); }
"rem"                   { return(TOKENIZER_KEYWORD); }
"renames"               { return(TOKENIZER_KEYWORD); }
"requeue"               { return(TOKENIZER_KEYWORD); }
"return"                { return(TOKENIZER_KEYWORD); }
"reverse"               { return(TOKENIZER_KEYWORD); }
"select"                { return(TOKENIZER_KEYWORD); }
"separate"              { return(TOKENIZER_KEYWORD); }
"subtype"               { return(TOKENIZER_KEYWORD); }
"tagged"                { return(TOKENIZER_KEYWORD); }
"task"                  { return(TOKENIZER_KEYWORD); }
"terminate"             { return(TOKENIZER_KEYWORD); }
"then"                  { return(TOKENIZER_KEYWORD); }
"type"                  { return(TOKENIZER_KEYWORD); }
"until"                 { return(TOKENIZER_KEYWORD); }
"use"                   { return(TOKENIZER_KEYWORD); }
"when"                  { return(TOKENIZER_KEYWORD); }
"while"                 { return(TOKENIZER_KEYWORD); }
"with"                  { return(TOKENIZER_KEYWORD); }
"xor"                   { return(TOKENIZER_KEYWORD); }
 
--([^\r\n\f\v])*        { return(TOKENIZER_COMMENT); }
\"(\".|.)*\"     { return(TOKENIZER_LITERAL); }
{D}+						   { return(TOKENIZER_LITERAL); }
'.'					      { return(TOKENIZER_LITERAL); }

\n						      { return(TOKENIZER_NEWLINE); }
\r\n 					      { return(TOKENIZER_NEWLINE); }
\r 						   { return(TOKENIZER_NEWLINE); }
[ \t\v\f]               { return(TOKENIZER_TEXT);    }
{L}+                    { return(TOKENIZER_TEXT); 	 }
.                       { return(TOKENIZER_TEXT);    }
%%

int ada_wrap ( void ) {
	{
		/* Silly impossible function call to stop warning of unused functions */
		if ( 0 ) {
			yyunput(0, "");
		}
	}

	return 1;
}
