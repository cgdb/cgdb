D           [0-9]
L           [a-zA-Z_]

%{
/* config_lexer.c
 * --------------
 * THIS FILE IS GENERATED, DO NOT EDIT 
 */

#include <string.h>
#include "configuration.h"

int yylinenumber = 1;

static void count( void );


%}

%%

#[^\n]*                                                    { count(); /* ignore comments */ }
"unset"                                                    { count(); return UNSET; }
"set"                                                      { count(); return SET; }
"bind"                                                     { count(); return BIND; }
"macro"                                                    { count(); return MACRO; }

(yes|YES|Yes|Y|y|no|NO|No|n|N)                             { count(); return BOOLEAN; }
((<({L}|{D}|[^>])*>)|({L}({L}|{D})*))\!?                   { count(); return IDENTIFIER; }
{D}+                                                       { count(); return NUMBER; }
(([cCmM]-)?{L})+                                           { count(); return KEY; }
\"(\\.|[^\\"])*\"                                          { count(); return STRING; }

"="                                                        { count(); return '='; }
";"                                                        { count(); return ';'; }


[ \t\v\r\n\f]                                              { count(); /* ignore white-space */ }
.                                                          { count(); /* ignore bad-characters */ }

%%

void count( void )
{
    if ( yytext[0] == '\n' || 
         yytext[0] == '\r' ) {
         ++yylinenumber;
    }
}

int yywrap( void )
{
    return 1;
}
