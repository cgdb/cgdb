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
"{"                      { return OPEN_BRACE; }
"}"                      { return CLOSED_BRACE; }
"+"                      { return ADD_OP; }
"*"                      { return MULT_OP; }
"="                      { return EQUAL_SIGN; }
"~"                      { return TILDA; }
"@"                      { return AT_SYMBOL; }
"&"                      { return AMPERSAND; }
"["                      { return OPEN_BRACKET; }
"]"                      { return CLOSED_BRACKET; }

\n                      { return NEWLINE; }
\r\n                    { return NEWLINE; }
\r                      { return NEWLINE; }

stopped                 { return STOPPED; }
done                    { return DONE; }
running                 { return RUNNING; }
connected               { return CONNECTED; }
error                   { return ERROR; }
exit                    { return EXIT; }
\(gdb\)                 { return GDB; }

{DIGIT}+                { return INTEGER_LITERAL; }

[ \t\v\f]               {}
{IDENTIFIER}            { return STRING_LITERAL;    }
.                       { return STRING_LITERAL;    }

\"(\\.|[^\\"])*\"       { return CSTRING; }


%%

int gdbmi_wrap ( void ) {
	{
		/* Silly impossible function call to stop warning of unused functions */
		if ( 0 ) {
			yyunput(0, "");
		}
	}

    return 1;
}
