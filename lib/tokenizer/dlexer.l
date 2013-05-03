%option prefix="d_"
%option outfile="lex.yy.c"

B       [01_]
D       [0-9_]
H       [0-9a-fA-F_]
L       [a-zA-Z_]
T       [0-9a-zA-Z_]
SP      [cwd]
IDENTIFIER {L}+{T}*

%{

#include <stdio.h>
#include "tokenizer.h"

int nesting_level;
%}

%x comment
%x nesting_comment
%x string_literal
%x wysiwyg_literal
%x alt_wysiwyg_literal

%%
"__gshared"             { return(TOKENIZER_TYPE); }
"__thread"              { return(TOKENIZER_TYPE); }
"__vector"              { return(TOKENIZER_TYPE); }
"abstract"              { return(TOKENIZER_TYPE); }
"auto"                  { return(TOKENIZER_TYPE); }
"int"                   { return(TOKENIZER_TYPE); }
"byte"                  { return(TOKENIZER_TYPE); }
"bit"                   { return(TOKENIZER_TYPE); }
"bool"                  { return(TOKENIZER_TYPE); }
"char"                  { return(TOKENIZER_TYPE); }
"cdouble"               { return(TOKENIZER_TYPE); }
"cent"                  { return(TOKENIZER_TYPE); }
"cfloat"                { return(TOKENIZER_TYPE); }
"class"                 { return(TOKENIZER_TYPE); }
"const"                 { return(TOKENIZER_TYPE); }
"creal"                 { return(TOKENIZER_TYPE); }
"deprecated"            { return(TOKENIZER_TYPE); }
"enum"                  { return(TOKENIZER_TYPE); }
"export"                { return(TOKENIZER_TYPE); }
"extern"                { return(TOKENIZER_TYPE); }
"dchar"                 { return(TOKENIZER_TYPE); }
"dstring"               { return(TOKENIZER_TYPE); }
"double"                { return(TOKENIZER_TYPE); }
"equals_t"              { return(TOKENIZER_TYPE); }
"hash_t"                { return(TOKENIZER_TYPE); }
"idouble"               { return(TOKENIZER_TYPE); }
"final"                 { return(TOKENIZER_TYPE); }
"float"                 { return(TOKENIZER_TYPE); }
"ifloat"                { return(TOKENIZER_TYPE); }
"immutable"             { return(TOKENIZER_TYPE); }
"inout"                 { return(TOKENIZER_TYPE); }
"interface"             { return(TOKENIZER_TYPE); }
"ireal"                 { return(TOKENIZER_TYPE); }
"lazy"                  { return(TOKENIZER_TYPE); }
"long"                  { return(TOKENIZER_TYPE); }
"nothrow"               { return(TOKENIZER_TYPE); }
"override"              { return(TOKENIZER_TYPE); }
"ptrdiff_t"             { return(TOKENIZER_TYPE); }
"pure"                  { return(TOKENIZER_TYPE); }
"real"                  { return(TOKENIZER_TYPE); }
"ref"                   { return(TOKENIZER_TYPE); }
"scope"                 { return(TOKENIZER_TYPE); }
"shared"                { return(TOKENIZER_TYPE); }
"short"                 { return(TOKENIZER_TYPE); }
"size_t"                { return(TOKENIZER_TYPE); }
"sizediff_t"            { return(TOKENIZER_TYPE); }
"static"                { return(TOKENIZER_TYPE); }
"string"                { return(TOKENIZER_TYPE); }
"struct"                { return(TOKENIZER_TYPE); }
"synchronized"          { return(TOKENIZER_TYPE); }
"ubyte"                 { return(TOKENIZER_TYPE); }
"ucent"                 { return(TOKENIZER_TYPE); }
"uint"                  { return(TOKENIZER_TYPE); }
"ulong"                 { return(TOKENIZER_TYPE); }
"union"                 { return(TOKENIZER_TYPE); }
"ushort"                { return(TOKENIZER_TYPE); }
"void"                  { return(TOKENIZER_TYPE); }
"volatile"              { return(TOKENIZER_TYPE); }
"wchar"                 { return(TOKENIZER_TYPE); }
"wchar_t"               { return(TOKENIZER_TYPE); }
"wstring"               { return(TOKENIZER_TYPE); }

"__traits"              { return(TOKENIZER_KEYWORD); }
"alias"                 { return(TOKENIZER_KEYWORD); }
"align"                 { return(TOKENIZER_KEYWORD); }
"asm"                   { return(TOKENIZER_KEYWORD); }
"assert"                { return(TOKENIZER_KEYWORD); }
"body"                  { return(TOKENIZER_KEYWORD); }
"break"                 { return(TOKENIZER_KEYWORD); }
"case"                  { return(TOKENIZER_KEYWORD); }
"catch"                 { return(TOKENIZER_KEYWORD); }
"cast"                  { return(TOKENIZER_KEYWORD); }
"continue"              { return(TOKENIZER_KEYWORD); }
"default"               { return(TOKENIZER_KEYWORD); }
"debug"                 { return(TOKENIZER_KEYWORD); }
"delegate"              { return(TOKENIZER_KEYWORD); }
"delete"                { return(TOKENIZER_KEYWORD); }
"do"                    { return(TOKENIZER_KEYWORD); }
"else"                  { return(TOKENIZER_KEYWORD); }
"finally"               { return(TOKENIZER_KEYWORD); }
"for"                   { return(TOKENIZER_KEYWORD); }
"foreach"               { return(TOKENIZER_KEYWORD); }
"foreach_reverse"       { return(TOKENIZER_KEYWORD); }
"function"              { return(TOKENIZER_KEYWORD); }
"goto"                  { return(TOKENIZER_KEYWORD); }
"if"                    { return(TOKENIZER_KEYWORD); }
"in"                    { return(TOKENIZER_KEYWORD); }
"invariant"             { return(TOKENIZER_KEYWORD); }
"is"                    { return(TOKENIZER_KEYWORD); }
"mixin"                 { return(TOKENIZER_KEYWORD); }
"new"                   { return(TOKENIZER_KEYWORD); }
"out"                   { return(TOKENIZER_KEYWORD); }
"private"               { return(TOKENIZER_KEYWORD); }
"protected"             { return(TOKENIZER_KEYWORD); }
"public"                { return(TOKENIZER_KEYWORD); }
"return"                { return(TOKENIZER_KEYWORD); }
"sizeof"                { return(TOKENIZER_KEYWORD); }
"switch"                { return(TOKENIZER_KEYWORD); }
"template"              { return(TOKENIZER_KEYWORD); }
"this"                  { return(TOKENIZER_KEYWORD); }
"super"                 { return(TOKENIZER_KEYWORD); }
"throw"                 { return(TOKENIZER_KEYWORD); }
"try"                   { return(TOKENIZER_KEYWORD); }
"typedef"               { return(TOKENIZER_KEYWORD); }
"typeid"                { return(TOKENIZER_KEYWORD); }
"typeof"                { return(TOKENIZER_KEYWORD); }
"unittest"              { return(TOKENIZER_KEYWORD); }
"version"               { return(TOKENIZER_KEYWORD); }
"while"                 { return(TOKENIZER_KEYWORD); }
"with"                  { return(TOKENIZER_KEYWORD); }

"/*"                        { BEGIN(comment);     return(TOKENIZER_COMMENT); }
<comment>[^*\r\n]*          { return(TOKENIZER_COMMENT); }
<comment>"*"+[^*/\r\n]*     { return(TOKENIZER_COMMENT); }
<comment>\n                 { return(TOKENIZER_NEWLINE); }
<comment>\r                 { return(TOKENIZER_NEWLINE); }
<comment>\r\n               { return(TOKENIZER_NEWLINE); }
<comment>"*"+"/"            { BEGIN(INITIAL);     return(TOKENIZER_COMMENT); }

"/+"                            { BEGIN(nesting_comment); nesting_level = (0); return(TOKENIZER_COMMENT); }
<nesting_comment>[^+/\r\n]*     { return(TOKENIZER_COMMENT); }
<nesting_comment>"+"+[^+/\r\n]* { return(TOKENIZER_COMMENT); }
<nesting_comment>"/"+[^+/\r\n]* { return(TOKENIZER_COMMENT); }
<nesting_comment>"/+"           { nesting_level++; return(TOKENIZER_COMMENT); }
<nesting_comment>\n             { return(TOKENIZER_NEWLINE); }
<nesting_comment>\r             { return(TOKENIZER_NEWLINE); }
<nesting_comment>\r\n           { return(TOKENIZER_NEWLINE); }
<nesting_comment>"+"+"/"        { if (nesting_level-- == 0) BEGIN(INITIAL);  return(TOKENIZER_COMMENT); }

\/\/[^\r\n]*            { return(TOKENIZER_COMMENT); }

#{L}+                   { return(TOKENIZER_DIRECTIVE); }
"import"                { return(TOKENIZER_DIRECTIVE); }
"module"                { return(TOKENIZER_DIRECTIVE); }
"package"               { return(TOKENIZER_DIRECTIVE); }
"pragma"                { return(TOKENIZER_DIRECTIVE); }
"_arguments"            { return(TOKENIZER_DIRECTIVE); }
"_argptr"               { return(TOKENIZER_DIRECTIVE); }
"__vptr"                { return(TOKENIZER_DIRECTIVE); }
"__monitor"             { return(TOKENIZER_DIRECTIVE); }
"__ctfe"                { return(TOKENIZER_DIRECTIVE); }
"_ctor"                 { return(TOKENIZER_DIRECTIVE); }
"_dtor"                 { return(TOKENIZER_DIRECTIVE); }
"@safe"                 { return(TOKENIZER_DIRECTIVE); }
"@trusted"              { return(TOKENIZER_DIRECTIVE); }
"@system"               { return(TOKENIZER_DIRECTIVE); }
"@property"             { return(TOKENIZER_DIRECTIVE); }
"@disable"              { return(TOKENIZER_DIRECTIVE); }


\"                                          { BEGIN(string_literal);     return(TOKENIZER_LITERAL); }
x\"                                         { BEGIN(string_literal);     return(TOKENIZER_LITERAL); }
<string_literal>(\\[^\r\n]|[^\\"\r\n])*     { return(TOKENIZER_LITERAL); }
<string_literal>\n                          { return(TOKENIZER_NEWLINE); }
<string_literal>\r                          { return(TOKENIZER_NEWLINE); }
<string_literal>\r\n                        { return(TOKENIZER_NEWLINE); }
<string_literal>\"                          { BEGIN(INITIAL);            return(TOKENIZER_LITERAL); }
<string_literal>\"{SP}                      { BEGIN(INITIAL);            return(TOKENIZER_LITERAL); }

r\"                                         { BEGIN(wysiwyg_literal);    return(TOKENIZER_LITERAL); }
<wysiwyg_literal>(\\[^\r\n]|[^\\"\r\n])*    { return(TOKENIZER_LITERAL); }
<wysiwyg_literal>\n                         { return(TOKENIZER_NEWLINE); }
<wysiwyg_literal>\r                         { return(TOKENIZER_NEWLINE); }
<wysiwyg_literal>\r\n                       { return(TOKENIZER_NEWLINE); }
<wysiwyg_literal>\"                         { BEGIN(INITIAL);            return(TOKENIZER_LITERAL); }
<wysiwyg_literal>\"{SP}                     { BEGIN(INITIAL);            return(TOKENIZER_LITERAL); }

"`"                                         { BEGIN(alt_wysiwyg_literal);    return(TOKENIZER_LITERAL); }
<alt_wysiwyg_literal>([^`\r\n])*            { return(TOKENIZER_LITERAL); }
<alt_wysiwyg_literal>\n                     { return(TOKENIZER_NEWLINE); }
<alt_wysiwyg_literal>\r                     { return(TOKENIZER_NEWLINE); }
<alt_wysiwyg_literal>\r\n                   { return(TOKENIZER_NEWLINE); }
<alt_wysiwyg_literal>"`"                    { BEGIN(INITIAL);            return(TOKENIZER_LITERAL); }
<alt_wysiwyg_literal>"`"{SP}                { BEGIN(INITIAL);            return(TOKENIZER_LITERAL); }


{D}+                          { return(TOKENIZER_LITERAL); }
{D}+[fF]                      { return(TOKENIZER_LITERAL); }
{D}+i                         { return(TOKENIZER_LITERAL); }
{D}+L                         { return(TOKENIZER_LITERAL); }
{D}+L[uU]                     { return(TOKENIZER_LITERAL); }
{D}+[uU]                      { return(TOKENIZER_LITERAL); }
{D}+[uU]L                     { return(TOKENIZER_LITERAL); }
{D}+[eE]{D}+                  { return(TOKENIZER_LITERAL); }
{D}+[eE]{D}+[fF]              { return(TOKENIZER_LITERAL); }
{D}+[eE][-+]{D}+              { return(TOKENIZER_LITERAL); }
{D}+[eE][-+]{D}+[fF]          { return(TOKENIZER_LITERAL); }

0b{B}+                        { return(TOKENIZER_LITERAL); }
0b{B}+L                       { return(TOKENIZER_LITERAL); }
0b{B}+L[uU]                   { return(TOKENIZER_LITERAL); }
0b{B}+[uU]                    { return(TOKENIZER_LITERAL); }
0b{B}+[uU]L                   { return(TOKENIZER_LITERAL); }

0x{H}+                        { return(TOKENIZER_LITERAL); }
0x{H}+L                       { return(TOKENIZER_LITERAL); }
0x{H}+L[uU]                   { return(TOKENIZER_LITERAL); }
0x{H}+[uU]                    { return(TOKENIZER_LITERAL); }
0x{H}+[uU]L                   { return(TOKENIZER_LITERAL); }
0x{H}+[pP][-+]{D}+            { return(TOKENIZER_LITERAL); }
0x{H}+[pP][-+]{D}+i           { return(TOKENIZER_LITERAL); }
0x{H}+[pP][-+]{D}+L           { return(TOKENIZER_LITERAL); }
0x{H}+[pP][-+]{D}+L[uU]       { return(TOKENIZER_LITERAL); }
0x{H}+[pP][-+]{D}+[uU]        { return(TOKENIZER_LITERAL); }
0x{H}+[pP][-+]{D}+[uU]L       { return(TOKENIZER_LITERAL); }
0x{H}+.{H}+[pP][-+]{D}+       { return(TOKENIZER_LITERAL); }
0x{H}+.{H}+[pP][-+]{D}+i      { return(TOKENIZER_LITERAL); }
0x{H}+.{H}+[pP][-+]{D}+L      { return(TOKENIZER_LITERAL); }
0x{H}+.{H}+[pP][-+]{D}+L[uU]  { return(TOKENIZER_LITERAL); }
0x{H}+.{H}+[pP][-+]{D}+[uU]   { return(TOKENIZER_LITERAL); }
0x{H}+.{H}+[pP][-+]{D}+[uU]L  { return(TOKENIZER_LITERAL); }

'.'                           { return(TOKENIZER_LITERAL); }
'\\.'                         { return(TOKENIZER_LITERAL); }
"false"                       { return(TOKENIZER_LITERAL); }
"null"                        { return(TOKENIZER_LITERAL); }
"true"                        { return(TOKENIZER_LITERAL); }

\n                            { return(TOKENIZER_NEWLINE); }
\r\n                          { return(TOKENIZER_NEWLINE); }
\r                            { return(TOKENIZER_NEWLINE); }
[ \t\v\f]                     { return(TOKENIZER_TEXT);    }
{IDENTIFIER}                  { return(TOKENIZER_TEXT);    }
.                             { return(TOKENIZER_TEXT);    }

%%

int d_wrap ( void ) {
        {
                /* Silly impossible function call to stop warning of unused functions */
                if ( 0 ) {
                        yyunput(0, "");
                }
        }

    return 1;
}
