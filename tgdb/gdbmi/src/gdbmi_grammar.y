%name-prefix="gdbmi_"
%defines

%{
#include <stdio.h>
extern int gdbmi_lex ( void );
void gdbmi_error (const char *s);
%}

%token OPEN_BRACE 		/* { */
%token CLOSED_BRACE 	/* } */
%token ADD_OP 			/* + */
%token MULT_OP 			/* * */
%token EQUAL_SIGN 		/* = */
%token TILDA 			/* ~ */
%token AT_SYMBOL 		/* @ */
%token AMPERSAND 		/* & */
%token OPEN_BRACKET 	/* [ */
%token CLOSED_BRACKET 	/* ] */
%token NEWLINE 			/* \n \r\n \r */
%token INTEGER_LITERAL 	/* A number 1234 */
%token STRING_LITERAL 	/* A string literal */
%token CSTRING 			/* "a string like \" this " */
%token COMMA 			/* , */
%token CARROT 			/* ^ */
%token STOPPED 			/* stopped */
%token DONE 			/* done */
%token RUNNING 			/* running */
%token CONNECTED 		/* connected */
%token ERROR 			/* error */
%token EXIT 			/* exit */
%token GDB 				/* (gdb) */

%start output

%%

output: out_of_band_record_list opt_result_record GDB NEWLINE { 
	 printf ("VALID\n" ); 
	 } ;

out_of_band_record_list: out_of_band_record_list out_of_band_record
out_of_band_record_list:;

opt_result_record: result_record
opt_result_record:;

result_record: opt_token CARROT result_class result_list_prime NEWLINE;

out_of_band_record: async_record NEWLINE;
out_of_band_record: stream_record NEWLINE;

async_record: exec_async_output;
async_record: status_async_output;
async_record: notify_async_output;

exec_async_output: opt_token MULT_OP async_output;

status_async_output: opt_token ADD_OP async_output;

notify_async_output: opt_token EQUAL_SIGN async_output;

async_output: async_class result_list_prime NEWLINE;

result_class: DONE;
result_class: RUNNING;
result_class: CONNECTED;
result_class: ERROR;
result_class: EXIT;

async_class: STOPPED;

result_list_prime: result_list;
result_list_prime:;

result_list: result_list COMMA result;
result_list: COMMA result;

result: variable EQUAL_SIGN value;

variable: STRING_LITERAL;

value_list_prime: value_list;
value_list_prime:

value_list: value_list COMMA value; 
value_list: COMMA value;

value: CSTRING; 
value: tuple;
value: list;

tuple: OPEN_BRACE CLOSED_BRACE;
tuple: OPEN_BRACE result result_list_prime CLOSED_BRACE;

list: OPEN_BRACKET CLOSED_BRACKET;
list: OPEN_BRACKET value value_list_prime CLOSED_BRACKET;
list: OPEN_BRACKET result result_list_prime CLOSED_BRACKET;

stream_record: console_stream_output;
stream_record: target_stream_output;
stream_record: log_stream_output;

console_stream_output: TILDA CSTRING;

target_stream_output: AT_SYMBOL CSTRING;

log_stream_output: AMPERSAND CSTRING;

opt_token: token | ;

token: INTEGER_LITERAL;
