D           [0-9]
L           [a-zA-Z_]

%{
/* config_lexer.c
 * --------------
 * THIS FILE IS GENERATED, DO NOT EDIT 
 */

#include <string.h>
#include "command_lexer.h"

const char * get_token( void )
{
    return yytext;
}


%}

        int enter_map_id = 0;

/* An identifier used in a map like command */
%x MAP_ID

%%
        if (enter_map_id)
                BEGIN (MAP_ID);
        else
                BEGIN (INITIAL);

<INITIAL>
{
#[^\n]*                                                    { /* ignore comments */ }
"unset"                                                    { return UNSET; }
"set"                                                      { return SET; }
"bind"                                                     { return BIND; } 
"macro"                                                    { return MACRO; } 

(yes|YES|Yes|Y|y|no|NO|No|n|N)                             { return BOOLEAN; }
((<({L}|{D}|[^>])*>)|({L}({L}|{D})*))+\!?                   { return IDENTIFIER; }
[+-]?{D}+                                                  { return NUMBER; }
\"(\\.|[^\\"])*\"                                          { return STRING; }

"="                                                        { return '='; }
";"                                                        { return ';'; }
","                                                        { return ','; }


(\r\n|\n\r|\n|\r)                                          { return EOL; }
[ \t\v\f]                                                  { /* ignore white-space */ }
.                                                          { /* ignore bad-characters */ }
}

<MAP_ID>
{
" "     { /* ignore white space */ }
[^ ]+   { return IDENTIFIER; }
.       { /* ignore bad-characters */ }
}

%%

int yywrap( void )
{
	{
		/* Silly impossible function call to stop warning of unused functions */
		if ( 0 ) {
			yyunput(0, "");
		}
	}

    return 1;
}
