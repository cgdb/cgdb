%option prefix="go_"
%option outfile="lex.yy.c"

D       [0-9]
H       [0-9a-fA-F_]
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
"bool"                  { return(TOKENIZER_TYPE); }
"uint"                  { return(TOKENIZER_TYPE); }
"int"                   { return(TOKENIZER_TYPE); }
"uintptr"               { return(TOKENIZER_TYPE); }
"uint16"                { return(TOKENIZER_TYPE); }
"uint32"                { return(TOKENIZER_TYPE); }
"uint64"                { return(TOKENIZER_TYPE); }
"int8"                  { return(TOKENIZER_TYPE); }
"int16"                 { return(TOKENIZER_TYPE); }
"int32"                 { return(TOKENIZER_TYPE); }
"int64"                 { return(TOKENIZER_TYPE); }
"float32"               { return(TOKENIZER_TYPE); }
"float64"               { return(TOKENIZER_TYPE); }
"complex64"             { return(TOKENIZER_TYPE); }
"complex128"            { return(TOKENIZER_TYPE); }
"byte"                  { return(TOKENIZER_TYPE); }
"rune"                  { return(TOKENIZER_TYPE); }
"string"                { return(TOKENIZER_TYPE); }
"struct"                { return(TOKENIZER_TYPE); }
"func"                  { return(TOKENIZER_TYPE); }
"interface"             { return(TOKENIZER_TYPE); }
"map"                   { return(TOKENIZER_TYPE); }
"chan"                  { return(TOKENIZER_TYPE); }

"break"                 { return(TOKENIZER_KEYWORD); }
"case"                  { return(TOKENIZER_KEYWORD); }
"const"                 { return(TOKENIZER_KEYWORD); }
"continue"              { return(TOKENIZER_KEYWORD); }
"default"               { return(TOKENIZER_KEYWORD); }
"defer"                 { return(TOKENIZER_KEYWORD); }
"else"                  { return(TOKENIZER_KEYWORD); }
"fallthrough"           { return(TOKENIZER_KEYWORD); }
"false"                 { return(TOKENIZER_KEYWORD); }
"for"                   { return(TOKENIZER_KEYWORD); }
"go"                    { return(TOKENIZER_KEYWORD); }
"goto"                  { return(TOKENIZER_KEYWORD); }
"if"                    { return(TOKENIZER_KEYWORD); }
"import"                { return(TOKENIZER_KEYWORD); }
"package"               { return(TOKENIZER_KEYWORD); }
"range"                 { return(TOKENIZER_KEYWORD); }
"return"                { return(TOKENIZER_KEYWORD); }
"select"                { return(TOKENIZER_KEYWORD); }
"switch"                { return(TOKENIZER_KEYWORD); }
"true"                  { return(TOKENIZER_KEYWORD); }
"type"                  { return(TOKENIZER_KEYWORD); }
"var"                   { return(TOKENIZER_KEYWORD); }

"/*"                    { BEGIN(comment); return(TOKENIZER_COMMENT); }
<comment>[^*\r\n]*      { return(TOKENIZER_COMMENT); }
<comment>"*"+[^*/\r\n]* { return(TOKENIZER_COMMENT); }
<comment>\n             { return(TOKENIZER_NEWLINE); }
<comment>\r             { return(TOKENIZER_NEWLINE); }
<comment>\r\n           { return(TOKENIZER_NEWLINE); }
<comment>"*"+"/"        { BEGIN(INITIAL); return(TOKENIZER_COMMENT); }

\/\/[^\r\n]*              { return(TOKENIZER_COMMENT); }

\"                                          { BEGIN(string_literal);    return(TOKENIZER_LITERAL); }
<string_literal>(\\[^\r\n]|[^\\"\r\n])*     { return(TOKENIZER_LITERAL); }
<string_literal>\n                          { return(TOKENIZER_NEWLINE); }
<string_literal>\r                          { return(TOKENIZER_NEWLINE); }
<string_literal>\r\n                        { return(TOKENIZER_NEWLINE); }
<string_literal>\"                          { BEGIN(INITIAL);           return(TOKENIZER_LITERAL); }


{D}+                    { return(TOKENIZER_LITERAL); }
{D}+[lL]                { return(TOKENIZER_LITERAL); }
{D}+[uU]                { return(TOKENIZER_LITERAL); }
{D}+[lL][lL]            { return(TOKENIZER_LITERAL); }
{D}+[uU][lL]            { return(TOKENIZER_LITERAL); }
{D}+[uU][lL][lL]        { return(TOKENIZER_LITERAL); }
{D}+[lL][uU]            { return(TOKENIZER_LITERAL); }
{D}+[lL][lL][uU]        { return(TOKENIZER_LITERAL); }
0x{H}+                  { return(TOKENIZER_LITERAL); }
0x{H}+[lL]              { return(TOKENIZER_LITERAL); }
0x{H}+[uU]              { return(TOKENIZER_LITERAL); }
0x{H}+[lL][lL]          { return(TOKENIZER_LITERAL); }
0x{H}+[uU][lL]          { return(TOKENIZER_LITERAL); }
0x{H}+[uU][lL][lL]      { return(TOKENIZER_LITERAL); }
0x{H}+[lL][uU]          { return(TOKENIZER_LITERAL); }
0x{H}+[lL][lL][uU]      { return(TOKENIZER_LITERAL); }

{D}*\.?{D}+([eE][-+]?{D}+)?    { return(TOKENIZER_LITERAL); }
{D}+\.?({D})?+[fF]      { return(TOKENIZER_LITERAL); }
\.{D}+[fF]              { return(TOKENIZER_LITERAL); }

'.'                     { return(TOKENIZER_LITERAL); }
'\\.'                   { return(TOKENIZER_LITERAL); }

\n                      { return(TOKENIZER_NEWLINE); }
\r\n                    { return(TOKENIZER_NEWLINE); }
\r                      { return(TOKENIZER_NEWLINE); }
[ \t\v\f]               { return(TOKENIZER_TEXT);    }
{IDENTIFIER}            { return(TOKENIZER_TEXT);    }
.                       { return(TOKENIZER_TEXT);    }

%%

int go_wrap ( void ) {
	{
		/* Silly impossible function call to stop warning of unused functions */
		if ( 0 ) {
			yyunput(0, "");
		}
	}
    return 1;
}
