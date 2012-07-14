%option prefix="c_"
%option outfile="lex.yy.c"

D       [0-9]
L       [a-zA-Z_]
T       [0-9a-zA-Z_]
IDENTIFIER {L}+{T}*

%{

#include <stdio.h>
#include "tokenizer.h"
%}

%x comment
%x string_literal

%%
"auto"                  { return(TOKENIZER_TYPE); }
"bool"                  { return(TOKENIZER_TYPE); }
"char"                  { return(TOKENIZER_TYPE); }
"class"                 { return(TOKENIZER_TYPE); }
"const"                 { return(TOKENIZER_TYPE); }
"double"                { return(TOKENIZER_TYPE); }
"enum"                  { return(TOKENIZER_TYPE); }
"extern"                { return(TOKENIZER_TYPE); }
"explicit"              { return(TOKENIZER_TYPE); }
"float"                 { return(TOKENIZER_TYPE); }
"friend"                { return(TOKENIZER_TYPE); }
"inline"                { return(TOKENIZER_TYPE); }
"int"                   { return(TOKENIZER_TYPE); }
"long"                  { return(TOKENIZER_TYPE); }
"mutable"               { return(TOKENIZER_TYPE); }
"namespace"             { return(TOKENIZER_TYPE); }
"register"              { return(TOKENIZER_TYPE); }
"short"                 { return(TOKENIZER_TYPE); }
"signed"                { return(TOKENIZER_TYPE); }
"static"                { return(TOKENIZER_TYPE); }
"struct"                { return(TOKENIZER_TYPE); }
"union"                 { return(TOKENIZER_TYPE); }
"unsigned"              { return(TOKENIZER_TYPE); }
"virtual"               { return(TOKENIZER_TYPE); }
"void"                  { return(TOKENIZER_TYPE); }
"volatile"              { return(TOKENIZER_TYPE); }
"wchar_t"               { return(TOKENIZER_TYPE); }

"and"                   { return(TOKENIZER_KEYWORD); }
"and_eq"                { return(TOKENIZER_KEYWORD); }
"asm"                   { return(TOKENIZER_KEYWORD); }
"bitand"                { return(TOKENIZER_KEYWORD); }
"bitor"                 { return(TOKENIZER_KEYWORD); }
"break"                 { return(TOKENIZER_KEYWORD); }
"case"                  { return(TOKENIZER_KEYWORD); }
"catch"                 { return(TOKENIZER_KEYWORD); }
"compl"                 { return(TOKENIZER_KEYWORD); }
"const_cast"            { return(TOKENIZER_KEYWORD); }
"continue"              { return(TOKENIZER_KEYWORD); }
"default"               { return(TOKENIZER_KEYWORD); }
"delete"                { return(TOKENIZER_KEYWORD); }
"do"                    { return(TOKENIZER_KEYWORD); }
"dynamic_cast"          { return(TOKENIZER_KEYWORD); }
"else"                  { return(TOKENIZER_KEYWORD); }
"export"                { return(TOKENIZER_KEYWORD); }
"false"                 { return(TOKENIZER_KEYWORD); }
"for"                   { return(TOKENIZER_KEYWORD); }
"goto"                  { return(TOKENIZER_KEYWORD); }
"if"                    { return(TOKENIZER_KEYWORD); }
"new"                   { return(TOKENIZER_KEYWORD); }
"not"                   { return(TOKENIZER_KEYWORD); }
"not_eq"                { return(TOKENIZER_KEYWORD); }
"operator"              { return(TOKENIZER_KEYWORD); }
"or"                    { return(TOKENIZER_KEYWORD); }
"or_eq"                 { return(TOKENIZER_KEYWORD); }
"private"               { return(TOKENIZER_KEYWORD); }
"protected"             { return(TOKENIZER_KEYWORD); }
"public"                { return(TOKENIZER_KEYWORD); }
"reinterpret_cast"      { return(TOKENIZER_KEYWORD); }
"return"                { return(TOKENIZER_KEYWORD); }
"sizeof"                { return(TOKENIZER_KEYWORD); }
"static_cast"           { return(TOKENIZER_KEYWORD); }
"switch"                { return(TOKENIZER_KEYWORD); }
"template"              { return(TOKENIZER_KEYWORD); }
"this"                  { return(TOKENIZER_KEYWORD); }
"throw"                 { return(TOKENIZER_KEYWORD); }
"true"                  { return(TOKENIZER_KEYWORD); }
"try"                   { return(TOKENIZER_KEYWORD); }
"typedef"               { return(TOKENIZER_KEYWORD); }
"typeid"                { return(TOKENIZER_KEYWORD); }
"typename"              { return(TOKENIZER_KEYWORD); }
"using"                 { return(TOKENIZER_KEYWORD); }
"while"                 { return(TOKENIZER_KEYWORD); }
"xor"                   { return(TOKENIZER_KEYWORD); }
"xor_eq"                { return(TOKENIZER_KEYWORD); }

"/*"                    { BEGIN(comment); return(TOKENIZER_COMMENT); }
<comment>[^*\r\n]*      { return(TOKENIZER_COMMENT); }
<comment>"*"+[^*/\r\n]* { return(TOKENIZER_COMMENT); }
<comment>\n             { return(TOKENIZER_NEWLINE); }
<comment>\r             { return(TOKENIZER_NEWLINE); }
<comment>\r\n           { return(TOKENIZER_NEWLINE); }
<comment>"*"+"/"        { BEGIN(INITIAL); return(TOKENIZER_COMMENT); }

\/\/[^\r\n]*              { return(TOKENIZER_COMMENT); }

#{L}+                   { return(TOKENIZER_DIRECTIVE); }

\"                                          { BEGIN(string_literal);    return(TOKENIZER_LITERAL); }
<string_literal>(\\[^\r\n]|[^\\"\r\n])*     { return(TOKENIZER_LITERAL); }
<string_literal>\n                          { return(TOKENIZER_NEWLINE); }
<string_literal>\r                          { return(TOKENIZER_NEWLINE); }
<string_literal>\r\n                        { return(TOKENIZER_NEWLINE); }
<string_literal>\"                          { BEGIN(INITIAL);           return(TOKENIZER_LITERAL); }


{D}+                    { return(TOKENIZER_LITERAL); }
'.'                     { return(TOKENIZER_LITERAL); }
'\\.'                   { return(TOKENIZER_LITERAL); }

\n                      { return(TOKENIZER_NEWLINE); }
\r\n                    { return(TOKENIZER_NEWLINE); }
\r                      { return(TOKENIZER_NEWLINE); }
[ \t\v\f]               { return(TOKENIZER_TEXT);    }
{IDENTIFIER}            { return(TOKENIZER_TEXT);    }
.                       { return(TOKENIZER_TEXT);    }

%%

int c_wrap ( void ) {
	{
		/* Silly impossible function call to stop warning of unused functions */
		if ( 0 ) {
			yyunput(0, "");
		}
	}

    return 1;
}
