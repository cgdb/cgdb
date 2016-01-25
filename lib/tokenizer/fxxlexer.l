%option prefix="fxx_"
%option outfile="lex.yy.c"
%option case-insensitive

D       [0-9]
L       [a-z_]
T       [0-9a-z_]
IDENTIFIER {L}+{T}*

%{

#include <stdio.h>
#include "tokenizer.h"

%}

%x string_literal

%%
  /*
   * All Fortran types, keywords, and intrinsic functions are simply treated
   * as TOKENIZER_KEYWORD in this file, because it can be difficult to really
   * distinguish some words in different contexts, e.g.
   *
   * real(kind = 8) :: r ! Herein "real" is a Fortran type.
   * x = real(r)         ! Herein "real" is a Fortran intrinsic function.
   */

  /*
   * Fortran intrinsic types are defined according to
   * http://fortranwiki.org/fortran/show/Intrinsic+types
   * (accessed on 25-Jan-16)
   */
"integer"               { return(TOKENIZER_KEYWORD); }
"real"                  { return(TOKENIZER_KEYWORD); }
"complex"               { return(TOKENIZER_KEYWORD); }
"logical"               { return(TOKENIZER_KEYWORD); }
"character"             { return(TOKENIZER_KEYWORD); }

  /*
   * Fortran keywords are defined according to
   * http://fortranwiki.org/fortran/show/Keywords
   * (accessed on 25-Jan-16)
   */
  /* FORTRAN 77 */
"assign"                { return(TOKENIZER_KEYWORD); }
"backspace"             { return(TOKENIZER_KEYWORD); }
"block data"            { return(TOKENIZER_KEYWORD); }
"call"                  { return(TOKENIZER_KEYWORD); }
"close"                 { return(TOKENIZER_KEYWORD); }
"common"                { return(TOKENIZER_KEYWORD); }
"continue"              { return(TOKENIZER_KEYWORD); }
"data"                  { return(TOKENIZER_KEYWORD); }
"dimension"             { return(TOKENIZER_KEYWORD); }
"do"                    { return(TOKENIZER_KEYWORD); }
"else"                  { return(TOKENIZER_KEYWORD); }
"else if"               { return(TOKENIZER_KEYWORD); }
"end"                   { return(TOKENIZER_KEYWORD); }
"endfile"               { return(TOKENIZER_KEYWORD); }
"endif"                 { return(TOKENIZER_KEYWORD); }
"entry"                 { return(TOKENIZER_KEYWORD); }
"equivalence"           { return(TOKENIZER_KEYWORD); }
"external"              { return(TOKENIZER_KEYWORD); }
"format"                { return(TOKENIZER_KEYWORD); }
"function"              { return(TOKENIZER_KEYWORD); }
"goto"                  { return(TOKENIZER_KEYWORD); }
"if"                    { return(TOKENIZER_KEYWORD); }
"implicit"              { return(TOKENIZER_KEYWORD); }
"inquire"               { return(TOKENIZER_KEYWORD); }
"intrinsic"             { return(TOKENIZER_KEYWORD); }
"open"                  { return(TOKENIZER_KEYWORD); }
"parameter"             { return(TOKENIZER_KEYWORD); }
"pause"                 { return(TOKENIZER_KEYWORD); }
"print"                 { return(TOKENIZER_KEYWORD); }
"program"               { return(TOKENIZER_KEYWORD); }
"read"                  { return(TOKENIZER_KEYWORD); }
"return"                { return(TOKENIZER_KEYWORD); }
"rewind"                { return(TOKENIZER_KEYWORD); }
"rewrite"               { return(TOKENIZER_KEYWORD); }
"save"                  { return(TOKENIZER_KEYWORD); }
"stop"                  { return(TOKENIZER_KEYWORD); }
"subroutine"            { return(TOKENIZER_KEYWORD); }
"then"                  { return(TOKENIZER_KEYWORD); }
"write"                 { return(TOKENIZER_KEYWORD); }
  /* Fortran 90 */
"allocatable"           { return(TOKENIZER_KEYWORD); }
"allocate"              { return(TOKENIZER_KEYWORD); }
"case"                  { return(TOKENIZER_KEYWORD); }
"contains"              { return(TOKENIZER_KEYWORD); }
"cycle"                 { return(TOKENIZER_KEYWORD); }
"deallocate"            { return(TOKENIZER_KEYWORD); }
"elsewhere"             { return(TOKENIZER_KEYWORD); }
"exit"                  { return(TOKENIZER_KEYWORD); }
"include"               { return(TOKENIZER_KEYWORD); }
"interface"             { return(TOKENIZER_KEYWORD); }
"intent"                { return(TOKENIZER_KEYWORD); }
"module"                { return(TOKENIZER_KEYWORD); }
"namelist"              { return(TOKENIZER_KEYWORD); }
"nullify"               { return(TOKENIZER_KEYWORD); }
"only"                  { return(TOKENIZER_KEYWORD); }
"operator"              { return(TOKENIZER_KEYWORD); }
"optional"              { return(TOKENIZER_KEYWORD); }
"pointer"               { return(TOKENIZER_KEYWORD); }
"private"               { return(TOKENIZER_KEYWORD); }
"procedure"             { return(TOKENIZER_KEYWORD); }
"public"                { return(TOKENIZER_KEYWORD); }
"recursive"             { return(TOKENIZER_KEYWORD); }
"result"                { return(TOKENIZER_KEYWORD); }
"select"                { return(TOKENIZER_KEYWORD); }
"sequence"              { return(TOKENIZER_KEYWORD); }
"target"                { return(TOKENIZER_KEYWORD); }
"use"                   { return(TOKENIZER_KEYWORD); }
"while"                 { return(TOKENIZER_KEYWORD); }
"where"                 { return(TOKENIZER_KEYWORD); }
  /* Fortran 95 */
"elemental"             { return(TOKENIZER_KEYWORD); }
"forall"                { return(TOKENIZER_KEYWORD); }
"pure"                  { return(TOKENIZER_KEYWORD); }
  /* Fortran 03 */
"abstract"              { return(TOKENIZER_KEYWORD); }
"associate"             { return(TOKENIZER_KEYWORD); }
"asynchronous"          { return(TOKENIZER_KEYWORD); }
"bind"                  { return(TOKENIZER_KEYWORD); }
"class"                 { return(TOKENIZER_KEYWORD); }
"deferred"              { return(TOKENIZER_KEYWORD); }
"enum"                  { return(TOKENIZER_KEYWORD); }
"enumerator"            { return(TOKENIZER_KEYWORD); }
"extends"               { return(TOKENIZER_KEYWORD); }
"final"                 { return(TOKENIZER_KEYWORD); }
"flush"                 { return(TOKENIZER_KEYWORD); }
"generic"               { return(TOKENIZER_KEYWORD); }
"import"                { return(TOKENIZER_KEYWORD); }
"non_overridable"       { return(TOKENIZER_KEYWORD); }
"nopass"                { return(TOKENIZER_KEYWORD); }
"pass"                  { return(TOKENIZER_KEYWORD); }
"protected"             { return(TOKENIZER_KEYWORD); }
"value"                 { return(TOKENIZER_KEYWORD); }
"volatile"              { return(TOKENIZER_KEYWORD); }
"wait"                  { return(TOKENIZER_KEYWORD); }
  /* Fortran 08 */
"block"                 { return(TOKENIZER_KEYWORD); }
"codimension"           { return(TOKENIZER_KEYWORD); }
"do concurrent"         { return(TOKENIZER_KEYWORD); }
"contiguous"            { return(TOKENIZER_KEYWORD); }
"critical"              { return(TOKENIZER_KEYWORD); }
"error stop"            { return(TOKENIZER_KEYWORD); }
"submodule"             { return(TOKENIZER_KEYWORD); }
"sync all"              { return(TOKENIZER_KEYWORD); }
"sync images"           { return(TOKENIZER_KEYWORD); }
"sync memory"           { return(TOKENIZER_KEYWORD); }
"lock"                  { return(TOKENIZER_KEYWORD); }
"unlock"                { return(TOKENIZER_KEYWORD); }

  /*
   * Fortran intrinsic functions are defined according to
   * http://fortranwiki.org/fortran/show/Intrinsic+procedures
   * (accessed on 25-Jan-16)
   *
   * A Fortran intrinsic function is matched as:
   * function name + none or more blank characters + (
   * But the function name is then terminated by a null character.
   */
"abs"                            { return(TOKENIZER_KEYWORD); }
"achar"                          { return(TOKENIZER_KEYWORD); }
"acos"                           { return(TOKENIZER_KEYWORD); }
"acosh"                          { return(TOKENIZER_KEYWORD); }
"adjustl"                        { return(TOKENIZER_KEYWORD); }
"adjustr"                        { return(TOKENIZER_KEYWORD); }
"aimag"                          { return(TOKENIZER_KEYWORD); }
"aint"                           { return(TOKENIZER_KEYWORD); }
"all"                            { return(TOKENIZER_KEYWORD); }
"allocated"                      { return(TOKENIZER_KEYWORD); }
"anint"                          { return(TOKENIZER_KEYWORD); }
"any"                            { return(TOKENIZER_KEYWORD); }
"asin"                           { return(TOKENIZER_KEYWORD); }
"asinh"                          { return(TOKENIZER_KEYWORD); }
"associated"                     { return(TOKENIZER_KEYWORD); }
"atan"                           { return(TOKENIZER_KEYWORD); }
"atan2"                          { return(TOKENIZER_KEYWORD); }
"atanh"                          { return(TOKENIZER_KEYWORD); }
"bessel_j0"                      { return(TOKENIZER_KEYWORD); }
"bessel_j1"                      { return(TOKENIZER_KEYWORD); }
"bessel_jn"                      { return(TOKENIZER_KEYWORD); }
"bessel_y0"                      { return(TOKENIZER_KEYWORD); }
"bessel_y1"                      { return(TOKENIZER_KEYWORD); }
"bessel_yn"                      { return(TOKENIZER_KEYWORD); }
"bge"                            { return(TOKENIZER_KEYWORD); }
"bgt"                            { return(TOKENIZER_KEYWORD); }
"bit_size"                       { return(TOKENIZER_KEYWORD); }
"ble"                            { return(TOKENIZER_KEYWORD); }
"blt"                            { return(TOKENIZER_KEYWORD); }
"btest"                          { return(TOKENIZER_KEYWORD); }
"c_associated"                   { return(TOKENIZER_KEYWORD); }
"ceiling"                        { return(TOKENIZER_KEYWORD); }
"c_f_pointer"                    { return(TOKENIZER_KEYWORD); }
"c_f_procpointer"                { return(TOKENIZER_KEYWORD); }
"c_funloc"                       { return(TOKENIZER_KEYWORD); }
"char"                           { return(TOKENIZER_KEYWORD); }
"c_loc"                          { return(TOKENIZER_KEYWORD); }
"cmplx"                          { return(TOKENIZER_KEYWORD); }
"command_argument_count"         { return(TOKENIZER_KEYWORD); }
"conjg"                          { return(TOKENIZER_KEYWORD); }
"cos"                            { return(TOKENIZER_KEYWORD); }
"cosh"                           { return(TOKENIZER_KEYWORD); }
"count"                          { return(TOKENIZER_KEYWORD); }
"cpu_time"                       { return(TOKENIZER_KEYWORD); }
"cshift"                         { return(TOKENIZER_KEYWORD); }
"c_sizeof"                       { return(TOKENIZER_KEYWORD); }
"date_and_time"                  { return(TOKENIZER_KEYWORD); }
"dble"                           { return(TOKENIZER_KEYWORD); }
"digits"                         { return(TOKENIZER_KEYWORD); }
"dim"                            { return(TOKENIZER_KEYWORD); }
"dot_product"                    { return(TOKENIZER_KEYWORD); }
"dprod"                          { return(TOKENIZER_KEYWORD); }
"dshiftl"                        { return(TOKENIZER_KEYWORD); }
"dshiftr"                        { return(TOKENIZER_KEYWORD); }
"eoshift"                        { return(TOKENIZER_KEYWORD); }
"epsilon"                        { return(TOKENIZER_KEYWORD); }
"erf"                            { return(TOKENIZER_KEYWORD); }
"erfc"                           { return(TOKENIZER_KEYWORD); }
"erfc_scaled"                    { return(TOKENIZER_KEYWORD); }
"execute_command_line"           { return(TOKENIZER_KEYWORD); }
"exp"                            { return(TOKENIZER_KEYWORD); }
"exponent"                       { return(TOKENIZER_KEYWORD); }
"extends_type_of"                { return(TOKENIZER_KEYWORD); }
"float"                          { return(TOKENIZER_KEYWORD); }
"floor"                          { return(TOKENIZER_KEYWORD); }
"fraction"                       { return(TOKENIZER_KEYWORD); }
"gamma"                          { return(TOKENIZER_KEYWORD); }
"get_command"                    { return(TOKENIZER_KEYWORD); }
"get_command_argument"           { return(TOKENIZER_KEYWORD); }
"get_environment_variable"       { return(TOKENIZER_KEYWORD); }
"huge"                           { return(TOKENIZER_KEYWORD); }
"hypot"                          { return(TOKENIZER_KEYWORD); }
"iachar"                         { return(TOKENIZER_KEYWORD); }
"iand"                           { return(TOKENIZER_KEYWORD); }
"ibclr"                          { return(TOKENIZER_KEYWORD); }
"ibits"                          { return(TOKENIZER_KEYWORD); }
"ibset"                          { return(TOKENIZER_KEYWORD); }
"ichar"                          { return(TOKENIZER_KEYWORD); }
"ieor"                           { return(TOKENIZER_KEYWORD); }
"index"                          { return(TOKENIZER_KEYWORD); }
"int"                            { return(TOKENIZER_KEYWORD); }
"ior"                            { return(TOKENIZER_KEYWORD); }
"ishft"                          { return(TOKENIZER_KEYWORD); }
"ishftc"                         { return(TOKENIZER_KEYWORD); }
"is_iostat_end"                  { return(TOKENIZER_KEYWORD); }
"is_iostat_eor"                  { return(TOKENIZER_KEYWORD); }
"kind"                           { return(TOKENIZER_KEYWORD); }
"lbound"                         { return(TOKENIZER_KEYWORD); }
"leadz"                          { return(TOKENIZER_KEYWORD); }
"len"                            { return(TOKENIZER_KEYWORD); }
"len_trim"                       { return(TOKENIZER_KEYWORD); }
"lge"                            { return(TOKENIZER_KEYWORD); }
"lgt"                            { return(TOKENIZER_KEYWORD); }
"lle"                            { return(TOKENIZER_KEYWORD); }
"llt"                            { return(TOKENIZER_KEYWORD); }
"log"                            { return(TOKENIZER_KEYWORD); }
"log10"                          { return(TOKENIZER_KEYWORD); }
"log_gamma"                      { return(TOKENIZER_KEYWORD); }
"logical"                        { return(TOKENIZER_KEYWORD); }
"maskl"                          { return(TOKENIZER_KEYWORD); }
"maskr"                          { return(TOKENIZER_KEYWORD); }
"matmul"                         { return(TOKENIZER_KEYWORD); }
"max"                            { return(TOKENIZER_KEYWORD); }
"maxexponent"                    { return(TOKENIZER_KEYWORD); }
"maxloc"                         { return(TOKENIZER_KEYWORD); }
"maxval"                         { return(TOKENIZER_KEYWORD); }
"merge"                          { return(TOKENIZER_KEYWORD); }
"merge_bits"                     { return(TOKENIZER_KEYWORD); }
"min"                            { return(TOKENIZER_KEYWORD); }
"minexponent"                    { return(TOKENIZER_KEYWORD); }
"minloc"                         { return(TOKENIZER_KEYWORD); }
"minval"                         { return(TOKENIZER_KEYWORD); }
"mod"                            { return(TOKENIZER_KEYWORD); }
"modulo"                         { return(TOKENIZER_KEYWORD); }
"move_alloc"                     { return(TOKENIZER_KEYWORD); }
"mvbits"                         { return(TOKENIZER_KEYWORD); }
"nearest"                        { return(TOKENIZER_KEYWORD); }
"new_line"                       { return(TOKENIZER_KEYWORD); }
"nint"                           { return(TOKENIZER_KEYWORD); }
"norm2"                          { return(TOKENIZER_KEYWORD); }
"not"                            { return(TOKENIZER_KEYWORD); }
"null"                           { return(TOKENIZER_KEYWORD); }
"pack"                           { return(TOKENIZER_KEYWORD); }
"parity"                         { return(TOKENIZER_KEYWORD); }
"popcnt"                         { return(TOKENIZER_KEYWORD); }
"poppar"                         { return(TOKENIZER_KEYWORD); }
"precision"                      { return(TOKENIZER_KEYWORD); }
"present"                        { return(TOKENIZER_KEYWORD); }
"product"                        { return(TOKENIZER_KEYWORD); }
"radix"                          { return(TOKENIZER_KEYWORD); }
"random_number"                  { return(TOKENIZER_KEYWORD); }
"random_seed"                    { return(TOKENIZER_KEYWORD); }
"range"                          { return(TOKENIZER_KEYWORD); }
"real"                           { return(TOKENIZER_KEYWORD); }
"repeat"                         { return(TOKENIZER_KEYWORD); }
"reshape"                        { return(TOKENIZER_KEYWORD); }
"rrspacing"                      { return(TOKENIZER_KEYWORD); }
"same_type_as"                   { return(TOKENIZER_KEYWORD); }
"scale"                          { return(TOKENIZER_KEYWORD); }
"scan"                           { return(TOKENIZER_KEYWORD); }
"selected_char_kind"             { return(TOKENIZER_KEYWORD); }
"selected_int_kind"              { return(TOKENIZER_KEYWORD); }
"selected_real_kind"             { return(TOKENIZER_KEYWORD); }
"set_exponent"                   { return(TOKENIZER_KEYWORD); }
"shape"                          { return(TOKENIZER_KEYWORD); }
"shifta"                         { return(TOKENIZER_KEYWORD); }
"shiftl"                         { return(TOKENIZER_KEYWORD); }
"shiftr"                         { return(TOKENIZER_KEYWORD); }
"sign"                           { return(TOKENIZER_KEYWORD); }
"sin"                            { return(TOKENIZER_KEYWORD); }
"sinh"                           { return(TOKENIZER_KEYWORD); }
"size"                           { return(TOKENIZER_KEYWORD); }
"sngl"                           { return(TOKENIZER_KEYWORD); }
"spacing"                        { return(TOKENIZER_KEYWORD); }
"spread"                         { return(TOKENIZER_KEYWORD); }
"sqrt"                           { return(TOKENIZER_KEYWORD); }
"sum"                            { return(TOKENIZER_KEYWORD); }
"system_clock"                   { return(TOKENIZER_KEYWORD); }
"tan"                            { return(TOKENIZER_KEYWORD); }
"tanh"                           { return(TOKENIZER_KEYWORD); }
"tiny"                           { return(TOKENIZER_KEYWORD); }
"trailz"                         { return(TOKENIZER_KEYWORD); }
"transfer"                       { return(TOKENIZER_KEYWORD); }
"transpose"                      { return(TOKENIZER_KEYWORD); }
"trim"                           { return(TOKENIZER_KEYWORD); }
"ubound"                         { return(TOKENIZER_KEYWORD); }
"unpack"                         { return(TOKENIZER_KEYWORD); }
"verify"                         { return(TOKENIZER_KEYWORD); }

  /* FORTRAN 77 comment is not supported at present. */
![^\r\n]*                                    { return(TOKENIZER_COMMENT); }

\"                                          { BEGIN(string_literal);    return(TOKENIZER_LITERAL); }
<string_literal>(\\[^\r\n]|[^\\"\r\n])*     { return(TOKENIZER_LITERAL); }
<string_literal>\n                          { return(TOKENIZER_NEWLINE); }
<string_literal>\r                          { return(TOKENIZER_NEWLINE); }
<string_literal>\r\n                        { return(TOKENIZER_NEWLINE); }
<string_literal>\"                          { BEGIN(INITIAL);           return(TOKENIZER_LITERAL); }

{D}+                           { return(TOKENIZER_LITERAL); }
{D}*\.?{D}+([eE][-+]?{D}+)?    { return(TOKENIZER_LITERAL); }
{D}*\.?{D}+([dD][-+]?{D}+)?    { return(TOKENIZER_LITERAL); }

'.'                            { return(TOKENIZER_LITERAL); }
'\\.'                          { return(TOKENIZER_LITERAL); }

\n                      { return(TOKENIZER_NEWLINE); }
\r\n                    { return(TOKENIZER_NEWLINE); }
\r                      { return(TOKENIZER_NEWLINE); }
[ \t\v\f]               { return(TOKENIZER_TEXT);    }
{IDENTIFIER}            { return(TOKENIZER_TEXT);    }
.                       { return(TOKENIZER_TEXT);    }

%%

int fxx_wrap ( void ) {
	{
		/* Silly impossible function call to stop warning of unused functions */
		if ( 0 ) {
			yyunput(0, "");
		}
	}

    return 1;
}
