%option prefix="gdbmi_"
%option outfile="lex.yy.c"
%option yylineno

DIGIT       [0-9]
L       [a-zA-Z_]
T       [0-9a-zA-Z_-]
IDENTIFIER {L}+{T}*

%{

#include <stdio.h>
#include "gdbmi_grammar.h"
%}

%%

"^"                      { return CARROT; }
","                      { return COMMA; }
"+"                      { return ADD_OP; }
"*"                      { return MULT_OP; }
"="                      { return EQUAL_SIGN; }
"~"                      { return TILDA; }
"@"                      { return AT_SYMBOL; }
"&"                      { return AMPERSAND; }
"["                      { return OPEN_BRACKET; }
"]"                      { return CLOSED_BRACKET; }
"{"                      { return OPEN_BRACE; }
"}"                      { return CLOSED_BRACE; }
"("                      { return OPEN_PAREN; }
")"                      { return CLOSED_PAREN; }

\n                      { return NEWLINE; }
\r\n                    { return NEWLINE; }
\r                      { return NEWLINE; }

{DIGIT}+                { return INTEGER_LITERAL; }

[ \t\v\f]               {}
{IDENTIFIER}            { return STRING_LITERAL;    }
.                       { return STRING_LITERAL;    }

\"(\\.|[^\\"])*\"       { return CSTRING; }


%%

/* I don't know what this function does.  However, it avoids a link error.  */
int 
gdbmi_wrap (void)
{
  if (0)
    yyunput(0, "");

  return 1;
}
