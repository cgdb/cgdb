#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#if HAVE_STRINGS_H
#include <strings.h>
#endif /* HAVE_STRINGS_H */

#if HAVE_STDLIB_H 
#include <stdlib.h>
#endif  /* HAVE_STDLIB_H */


#include "cgdbrc.h"
#include "command_lexer.h"
#include "tgdb.h"
#include "interface.h"
#include "tokenizer.h"
#include "highlight_groups.h"
#include "cgdb.h"
#include "sys_util.h"
#include "std_list.h"

extern struct tgdb *tgdb;

/**
 * The general idea is that the configuration will read in the users ~/.cgdbrc
 * file, or ~/.cgdb/config or whatever, and execute each command.  This will
 * also be responsible for processing all : commands in the tui.
 *
 */

enum ConfigType
{
    CONFIG_TYPE_BOOL, /* set ic / set noic */
    CONFIG_TYPE_INT,  /* set tabstop=8 */
    CONFIG_TYPE_STRING,
    CONFIG_TYPE_FUNC_VOID,
    CONFIG_TYPE_FUNC_BOOL,
    CONFIG_TYPE_FUNC_INT,
    CONFIG_TYPE_FUNC_STRING
};

static int command_set_arrowstyle( const char *value );
static int command_set_focus( const char *value );
static int command_set_winsplit( const char *value );
static int command_set_timeout( int value );
static int command_set_timeoutlen( int value );
static int command_set_ttimeout( int value );
static int command_set_ttimeoutlen( int value );
static int command_set_winminheight( int value );
static int command_set_syntax_type( const char *value );
static int command_set_stc ( int value );
static int cgdbrc_set_val (struct cgdbrc_config_option config_option);

/**
 * This data structure stores all the values of all the config options.
 * It is initialized with the default values.
 */
static struct cgdbrc_config_option cgdbrc_config_options[CGDBRC_WRAPSCAN+1] =
{
  {CGDBRC_ARROWSTYLE, {ARROWSTYLE_SHORT}},
  {CGDBRC_AUTOSOURCERELOAD, {0}},
  {CGDBRC_IGNORECASE, {0}},
  {CGDBRC_SHORTCUT, {0}},
  {CGDBRC_SHOWTGDBCOMMANDS, {0}},
  {CGDBRC_SYNTAX, {TOKENIZER_LANGUAGE_UNKNOWN}},
  {CGDBRC_TABSTOP, {8}},
  {CGDBRC_TIMEOUT, {1}},
  {CGDBRC_TIMEOUT_LEN, {1000}},
  {CGDBRC_TTIMEOUT, {1}},
  {CGDBRC_TTIMEOUT_LEN, {100}},
  {CGDBRC_WINMINHEIGHT, {0}},
  {CGDBRC_WINSPLIT, {WIN_SPLIT_EVEN}},
  {CGDBRC_WRAPSCAN, {1}}
};

static struct std_list *cgdbrc_attach_list;
static unsigned long cgdbrc_attach_handle = 1;
struct cgdbrc_attach_item {
  enum cgdbrc_option_kind option;
  int handle;
  cgdbrc_notify notify_hook;
};

static struct ConfigVariable
{
    const char *name, *s_name;
    enum ConfigType type;
    void *data;
} VARIABLES[] = {

    /* keep this stuff sorted! !sort*/
    /* arrowstyle */
    { "arrowstyle", "as", CONFIG_TYPE_FUNC_STRING, command_set_arrowstyle},

    /* autosourcereload */  
    { "autosourcereload", "asr", CONFIG_TYPE_BOOL, &cgdbrc_config_options[CGDBRC_AUTOSOURCERELOAD].variant.int_val },

    /* ignorecase */
    { "ignorecase", "ic", CONFIG_TYPE_BOOL, &cgdbrc_config_options[CGDBRC_IGNORECASE].variant.int_val },

    /* shortcut   */
    { "shortcut", "sc", CONFIG_TYPE_BOOL, &cgdbrc_config_options[CGDBRC_SHORTCUT].variant.int_val },

    /* showtgdbcommands */
    { "showtgdbcommands", "stc", CONFIG_TYPE_FUNC_BOOL, &command_set_stc },

    /* syntax */
    { "syntax", "syn", CONFIG_TYPE_FUNC_STRING, command_set_syntax_type }, 

    /* tabstop   */
    { "tabstop", "ts", CONFIG_TYPE_INT, &cgdbrc_config_options[CGDBRC_TABSTOP].variant.int_val },

    /* timeout   */
    { "timeout", "to", CONFIG_TYPE_FUNC_BOOL, &command_set_timeout },

    /* timeoutlen   */
    { "timeoutlen", "tm", CONFIG_TYPE_FUNC_INT, &command_set_timeoutlen },

    /* ttimeout   */
    { "ttimeout", "ttimeout", CONFIG_TYPE_FUNC_BOOL, &command_set_ttimeout },

    /* ttimeoutlen   */
    { "ttimeoutlen", "ttm", CONFIG_TYPE_FUNC_INT, &command_set_ttimeoutlen },

    /* winminheight */
    { "winminheight", "wmh", CONFIG_TYPE_FUNC_INT, &command_set_winminheight },

    /* winsplit */
    { "winsplit", "winsplit", CONFIG_TYPE_FUNC_STRING, command_set_winsplit }, 

    /* wrapscan */
    { "wrapscan", "ws", CONFIG_TYPE_BOOL, &cgdbrc_config_options[CGDBRC_WRAPSCAN].variant.int_val },
};

static int command_focus_cgdb( void );
static int command_focus_gdb( void );
static int command_focus_tty( void );

static int command_do_bang( void );
static int command_do_continue( void );
static int command_do_finish( void );
static int command_do_focus (void);
static int command_do_help( void );
static int command_do_next( void );
static int command_do_quit( void );
static int command_do_run( void );
static int command_do_shell( void );
static int command_do_step( void );
static int command_source_reload( void );

static int command_parse_syntax( void );
static int command_parse_highlight (void);
static int command_parse_map (void);
static int command_parse_unmap (void);

typedef int (*action_t)(void);
static struct commands
{
    const char *name, *s_name;
    /* these functions will return 0 on success and 1 on error.  */
    /* Should the configuration file processing continue after an error?*/
    action_t action;
} COMMANDS[] = {
    /* keep this stuff sorted, you can use !sort in vi*/
    /* bang        */ { "bang",        "bang",	    command_do_bang },
    /* continue    */ { "continue",    "continue",  command_do_continue },
    /* edit        */ { "edit",        "e",	    command_source_reload },
    /* finish      */ { "finish",      "finish",    command_do_finish },
    /* focus      */  { "focus",       "focus",	    command_do_focus },
    /* help        */ { "help",        "help",	    command_do_help },
    /* highlight   */ { "highlight",   "hi",	    command_parse_highlight },
    /* imap	   */ { "imap",	       "im",	    command_parse_map },
    /* iunmap	   */ { "iunmap",      "iu",	    command_parse_unmap },
    /* insert      */ { "insert",      "insert",    command_focus_gdb },
    /* map         */ { "map",         "map",	    command_parse_map },
    /* next        */ { "next",        "next",	    command_do_next },
    /* quit        */ { "quit",        "q",	    command_do_quit },
    /* run         */ { "run",         "run",	    command_do_run },
    /* shell       */ { "shell",       "sh",	    command_do_shell },
    /* step        */ { "step",        "step",	    command_do_step },
    /* syntax      */ { "syntax",      "syntax",    command_parse_syntax },
    /* unmap       */ { "unmap",       "unm",	    command_parse_unmap },
};

action_t get_command( const char *cmd )
{
    /* FIXME: replace with binary search */
    int i;
    for ( i = 0; i < (sizeof( COMMANDS )/sizeof( COMMANDS[0] )); ++i ) {
        if ( strcmp( cmd, COMMANDS[i].name ) == 0 ||
             strcmp( cmd, COMMANDS[i].s_name ) == 0) {
            return COMMANDS[i].action;
        }
    }

    return NULL;
}

struct ConfigVariable* get_variable( const char *variable ) 
{
    /* FIXME: replace with binary search */
    int i;
    for ( i = 0; i < (sizeof( VARIABLES )/sizeof( VARIABLES[0])); ++i ) {
        if ( strcmp( variable, VARIABLES[i].name ) == 0 ||
             strcmp( variable, VARIABLES[i].s_name ) == 0 ) {
            return &VARIABLES[i];
        }
    }

    return NULL;
}

int 
command_set_arrowstyle (const char *value)
{
  struct cgdbrc_config_option option;
  option.option_kind = CGDBRC_ARROWSTYLE;

  if (strcasecmp(value, "short") == 0)
    option.variant.arrow_style = ARROWSTYLE_SHORT;
  else if (strcasecmp (value, "long") == 0)
    option.variant.arrow_style = ARROWSTYLE_LONG;
  else if (strcasecmp(value, "highlight") == 0)
    option.variant.arrow_style = ARROWSTYLE_HIGHLIGHT;
  else
    return 1;

  return cgdbrc_set_val (option);
}

static int 
command_set_stc (int value)
{
  if ((value == 0) || (value == 1)) 
  {
    struct cgdbrc_config_option option;
    option.option_kind = CGDBRC_SHOWTGDBCOMMANDS;
    option.variant.int_val = value;

    if (cgdbrc_set_val (option))
      return 1;

    /* TODO: Make this not a member function. */
    tgdb_set_verbose_gui_command_output (tgdb, value);
  }
  else
    return 1;

  return 0;
}

int
command_set_winsplit (const char *value)
{
  struct cgdbrc_config_option option;
  WIN_SPLIT_TYPE split_type = WIN_SPLIT_EVEN;

  option.option_kind = CGDBRC_WINSPLIT;

  if (strcasecmp (value, "top_big") == 0)
    split_type = WIN_SPLIT_TOP_BIG;
  else if (strcasecmp (value, "top_full") == 0)
    split_type = WIN_SPLIT_TOP_FULL;
  else if (strcasecmp (value, "bottom_big") == 0)
    split_type = WIN_SPLIT_BOTTOM_BIG;
  else if (strcasecmp (value, "bottom_full") == 0)
    split_type = WIN_SPLIT_BOTTOM_FULL;
  else
    split_type = WIN_SPLIT_EVEN;

  option.variant.win_split_val = split_type;
  if (cgdbrc_set_val (option))
    return 1;
  if_set_winsplit (split_type);

  return 0;
}

static int 
command_set_winminheight (int value)
{
  struct cgdbrc_config_option option;
  option.option_kind = CGDBRC_WINMINHEIGHT;

  if (if_change_winminheight (value) == -1)
    return 1;

  option.variant.int_val = value;
  return cgdbrc_set_val (option);
}

static int 
command_set_timeout (int value)
{
  struct cgdbrc_config_option option;
  option.option_kind = CGDBRC_TIMEOUT;
  option.variant.int_val = value;

  if (cgdbrc_set_val (option))
    return 1;
  
  return 0;
}

static int 
command_set_timeoutlen (int value)
{
  if (value >= 0 && value <= 10000)
  {
    struct cgdbrc_config_option option;
    option.option_kind = CGDBRC_TIMEOUT_LEN;
    option.variant.int_val = value;

    if (cgdbrc_set_val (option))
      return 1;
  }
  
  return 0;
}

static int 
command_set_ttimeout (int value)
{
  struct cgdbrc_config_option option;
  option.option_kind = CGDBRC_TTIMEOUT;
  option.variant.int_val = value;

  if (cgdbrc_set_val (option))
    return 1;
  
  return 0;
}

static int 
command_set_ttimeoutlen (int value)
{
  if (value >= 0 && value <= 10000)
  {
    struct cgdbrc_config_option option;
    option.option_kind = CGDBRC_TTIMEOUT_LEN;
    option.variant.int_val = value;

    if (cgdbrc_set_val (option))
      return 1;
  }
  
  return 0;
}

int 
command_set_syntax_type (const char *value)
{
  enum tokenizer_language_support lang;
  struct cgdbrc_config_option option;
  option.option_kind = CGDBRC_SYNTAX;

  if (strcasecmp (value, "c") == 0)
    lang = TOKENIZER_LANGUAGE_C;
  else if( strcasecmp( value, "ada" ) == 0 )
    lang = TOKENIZER_LANGUAGE_ADA;
  else if( strcasecmp( value, "off" ) == 0 )
    lang = TOKENIZER_LANGUAGE_UNKNOWN;
    
  option.variant.language_support_val = lang;
  if (cgdbrc_set_val (option))
    return 1;
  if_highlight_sviewer (lang);

  return 0;
}

int command_focus_cgdb( void )
{
    if_set_focus( CGDB );
    return 0;
}

int command_focus_gdb( void )
{
    if_set_focus( GDB );
    return 0;
}

int command_focus_tty( void )
{
    if_set_focus( TTY );
    return 0;
}

int command_do_bang( void )
{
    return 0;
}

int command_do_continue( void )
{
    tgdb_request_ptr request_ptr;
    request_ptr = tgdb_request_run_debugger_command (tgdb, TGDB_CONTINUE);
    if (!request_ptr)
        return -1;
    handle_request (tgdb, request_ptr);

    return 0;
}

int command_do_finish( void )
{
    tgdb_request_ptr request_ptr;
    request_ptr = tgdb_request_run_debugger_command (tgdb, TGDB_FINISH);
    if (!request_ptr)
        return -1;
    handle_request (tgdb, request_ptr);

    return 0;
}

int 
command_do_focus (void)
{
  int token = yylex ();
  const char *value;

  if (token != IDENTIFIER)
    return 1;
  value = get_token ();

  if (strcasecmp (value, "cgdb") == 0)
    command_focus_cgdb();
  else if (strcasecmp (value, "gdb") == 0)
    command_focus_gdb();
  else if (strcasecmp (value, "tty") == 0)
    command_focus_tty();
  else
    return 1;

  return 0;
}

int command_do_help( void )
{
    if_display_help();
    return 0;
}

int command_do_next( void )
{
    tgdb_request_ptr request_ptr;
    request_ptr = tgdb_request_run_debugger_command  (tgdb, TGDB_NEXT);
    if (!request_ptr)
        return -1;
    handle_request (tgdb, request_ptr);

    return 0;
}

int command_do_quit( void )
{
    /* FIXME: Test to see if debugged program is still running */
    cleanup();
    exit(0);
    return 0;
}

int command_do_run( void )
{
    /* FIXME: see if there are any other arguments to pass to the run command */
    tgdb_request_ptr request_ptr;
    request_ptr = tgdb_request_run_debugger_command (tgdb, TGDB_RUN);
    if (!request_ptr)
        return -1;
    handle_request (tgdb, request_ptr);

    return 0;
}

int command_do_shell( void )
{
    return run_shell_command(NULL);
}

int command_do_step( void )
{
    tgdb_request_ptr request_ptr;
    request_ptr = tgdb_request_run_debugger_command (tgdb, TGDB_STEP);
    if (!request_ptr)
        return -1;
    handle_request (tgdb, request_ptr);

    return 0;
}

int command_source_reload( void )
{
    struct sviewer *sview = if_get_sview ();

    if (!sview)
      return -1;

    /* If there is no current source file, then there is nothing to reload. */
    if (!sview->cur)
      return 0;

    if ( source_reload ( sview, sview->cur->path, 1 ) == -1 )
        return -1;

    return 0;
}

int command_parse_syntax( void )
{
  /* This is something like: 
     :syntax
     :syntax on
     :syntax off
  */

  int rv = 1;
  switch( (rv = yylex()) ) {
  case EOL: {
    /* TODO: Print out syntax info (like vim?) */
  } break;
  case BOOLEAN: 
  case IDENTIFIER: {
    extern int sources_syntax_on;
    const char *value = get_token();
    if( strcasecmp( value, "on" ) == 0 || strcasecmp( value, "yes") == 0 )
        sources_syntax_on = 1;
    else if ( strcasecmp( value, "no" ) == 0 || strcasecmp( value, "off") == 0 )
        sources_syntax_on = 0;

    if_draw(); 
  } break;
  default:
    break;
  }

  return 0;
}

static int 
command_parse_highlight (void)
{
  return hl_groups_parse_config (hl_groups_instance);
}

extern struct kui_map_set *kui_map, *kui_imap;
static int
command_parse_map (void)
{
  struct kui_map_set *kui_map_choice;
  int key, value, val;
  char *key_token;

  if (strcmp (get_token (), "map") == 0)
    kui_map_choice = kui_map;
  else
    kui_map_choice = kui_imap;

  key = yylex ();
  if (key != IDENTIFIER)
    return -1;
  key_token = cgdb_strdup (get_token ());

  value = yylex ();
  if (value != IDENTIFIER)
  {
    xfree (key_token);
    return -1;
  }

  val = kui_ms_register_map (kui_map_choice, key_token, get_token ());
  if (val == -1)
  {
    free (key_token);
    return -1;
  }

  return 0;
}

static int
command_parse_unmap (void)
{
  struct kui_map_set *kui_map_choice;
  int key, val;
  char *key_token;

  if ((strcmp (get_token (), "unmap") == 0) || 
      (strcmp (get_token (), "unm") == 0))
    kui_map_choice = kui_map;
  else
    kui_map_choice = kui_imap;

  key = yylex ();
  if (key != IDENTIFIER)
    return -1;
  key_token = cgdb_strdup (get_token ());

  val = kui_ms_deregister_map (kui_map_choice, key_token);
  if (val == -1)
  {
    free (key_token);
    return -1;
  }

  return 0;
}

int command_parse_set( void )
{
    /* commands could look like the following:
     * set ignorecase
     * set noignorecase
     * set focus=gdb
     * set tabstop=8
     */

    int rv = 1;
    int boolean = 1;
    const char * value = NULL;

    switch ( (rv = yylex()) ) {
    case EOL: {
        /* TODO: Print out all the variables that have been set. */
    } break;
    case IDENTIFIER: {
        const char *token = get_token();
        int length = strlen( token );
        struct ConfigVariable *variable = NULL;

        if ( length > 2 && token[0] == 'n' && token[1] == 'o' ) {
            value = token + 2;
            boolean = 0;
        } else {
            value = token;
        }

        if ( (variable = get_variable( value )) != NULL ) {
            rv = 0;
            if( boolean == 0 &&
                variable->type != CONFIG_TYPE_BOOL ) {
                /* this is an error, you cant' do:
                 * set notabstop 
                 */
                rv = 1;
            }

            switch( variable->type ) {
            case CONFIG_TYPE_BOOL:
                *(int*)(variable->data) = boolean;
                break;
            case CONFIG_TYPE_INT: {
                if( yylex() == '=' &&
                    yylex() == NUMBER ) {
                    int data = strtol( get_token(), NULL, 10 );
                    *(int*)(variable->data) = data;
                } else {
                    rv = 1;
                }
            } break;
            case CONFIG_TYPE_STRING: {
                if( yylex() == '=' &&
                   (rv = yylex(), rv == STRING || rv == IDENTIFIER) ) {
                    /* BAM! comma operator */
                    char * data = (char*)get_token();
                    if( rv == STRING ) {
                        /* get rid of quotes */
                        data = data + 1;
                        data[ strlen( data ) - 1 ] = '\0';
                    } 
                    if( variable->data ) 
                    { free( variable->data ); }
                    variable->data = strdup( data );
                } else {
                    rv = 1;
                }
            } break;
            case CONFIG_TYPE_FUNC_VOID: {
                int(*functor)( void ) = (int(*)(void))variable->data;
                if( functor )
                { rv = functor(); }
                else 
                { rv = 1; }
            } break;
            case CONFIG_TYPE_FUNC_BOOL: {
                int (*functor)( int ) = (int(*)(int))variable->data;
                    if( functor )
                    { rv = functor( boolean ); }
                    else
                    { rv = 1; }
            } break;
            case CONFIG_TYPE_FUNC_INT: {
                int (*functor)( int ) = (int(*)(int))variable->data;
                if( yylex() == '=' &&
                    yylex() == NUMBER ) {
                    int data = strtol( get_token(), NULL, 10 );
                    if( functor )
                    { rv = functor( data ); }
                    else
                    { rv = 1; }
                } else {
                    rv = 1;
                }
            } break;
            case CONFIG_TYPE_FUNC_STRING: {
                int (*functor)( const char* ) = (int(*)(const char*))variable->data;
                if( yylex() == '=' &&
                   (rv = yylex(), rv == STRING || rv == IDENTIFIER) ) {
                    /* BAM! comma operator */
                    char * data = (char*)get_token();
                    if( rv == STRING ) {
                        /* get rid of quotes */
                        data = data + 1;
                        data[ strlen( data ) - 1 ] = '\0';
                    } 
                    if( functor )
                    { rv = functor(data); }
                    else
                    { rv = 1; }
                } else 
                { rv = 1; }
            } break;
            default: 
                rv = 1;
                break;
            }
        }
    } break;
    default:
        break;
    }

    return rv;
}

int command_parse_string( const char *buffer )
{
    typedef struct yy_buffer_state* YY_BUFFER_STATE;
    extern YY_BUFFER_STATE yy_scan_string( const char *yy_str );
    extern void yy_delete_buffer( YY_BUFFER_STATE state );
    int rv = 1;
    YY_BUFFER_STATE state = yy_scan_string( (char*)buffer );

    switch ( yylex() ) {
    case SET:
        /* get the next token */
        rv = command_parse_set();
        break;

    case UNSET:
    case BIND:
    case MACRO:
        /* ignore this stuff for now. */
        rv = 1;
        break;

    case NUMBER: {
        const char *number = get_token();
        if ( number[0] == '+' ) {
            source_vscroll( if_get_sview(), atoi( number+1 ));
            rv = 0;
        } else if ( number[0] == '-' ) {
            source_vscroll( if_get_sview(), -atoi( number+1 ));
            rv = 0;
        } else {
            source_set_sel_line(if_get_sview(), atoi( number ));
            rv = 0;
        }
        if_draw();
    } break;

    case IDENTIFIER: {
        action_t action = get_command( get_token() );
        if ( action ) {
            action();
            rv = 0;
        } else {
            rv = 1;
        }
    } break;

    case EOL: 
        /* basically just an empty line, don't do nothin. */
        rv = 0;
        break;

    default:
        rv = 1;
        break;
    }

    yy_delete_buffer( state );
    return rv;
}

int command_parse_file( FILE *fp )
{
    char buffer[4096];
    char *p = buffer;
    int linenumber = 0;

    while( linenumber++, fgets( p, sizeof(buffer) - ( p - buffer ), fp ) )
    {
        int bufferlen = strlen( buffer );
        if ((bufferlen-2>=0) && buffer[bufferlen - 2] == '\\' ) 
        {
            /* line continuation character, read another line into the buffer */
            linenumber--;
            p = buffer + bufferlen - 2;
            continue;
        }

        if( command_parse_string( buffer ) )
        {
            /* buffer already has an \n */
            if_print_message ("Error parsing line %d: %s", linenumber, buffer );
            /* return -1; don't return, lets keep parsing the file. */
        }

        p = buffer;
    }

    return 0;
}

/**
 * Will set a configuration option. If any of the notify hook's reject the
 * update of the value, the value will not be set, and this function will fail.
 * Otherwise, the option will be set.
 *
 * \param config_option
 * The new configuration option and value to set.
 *
 * \return
 * 1 if the option is not set, otherwise 0.
 */
static int 
cgdbrc_set_val (struct cgdbrc_config_option config_option)
{
  std_list_iterator iter;
  void *data;
  struct cgdbrc_attach_item *item;

  cgdbrc_config_options[config_option.option_kind] = config_option;

  /* Alert anyone that wants to be notified that an option has changed. */
  for (iter =  std_list_begin (cgdbrc_attach_list);
       iter != std_list_end (cgdbrc_attach_list);
       iter = std_list_next (iter))
  {
    if (std_list_get_data (iter, &data) == -1)
      return 1;

    item = (struct cgdbrc_attach_item*)data;
    if (item->option == config_option.option_kind)
    {
      if (item->notify_hook (&config_option))
	return 1;
    }
  }

  return 0;
}

// Attach/Detach options {{{

static int
destroy_notify (void *data)
{
  free (data);
  return 0;
}

int 
cgdbrc_attach (enum cgdbrc_option_kind option, cgdbrc_notify notify, int *handle)
{
  struct cgdbrc_attach_item *item = (struct cgdbrc_attach_item*)
    cgdb_malloc (sizeof (struct cgdbrc_attach_item));

  item->option = option;
  item->handle = cgdbrc_attach_handle++;
  item->notify_hook = notify;

  if (!cgdbrc_attach_list)
    cgdbrc_attach_list = std_list_create (destroy_notify);

  std_list_append (cgdbrc_attach_list, item);

  if (handle)
    *handle = item->handle;

  return 0;
}

int
cgdbrc_detach (int handle)
{
  std_list_iterator iter;
  void *data;
  struct cgdbrc_attach_item *item;

  for (iter =  std_list_begin (cgdbrc_attach_list);
       iter != std_list_end (cgdbrc_attach_list);
       iter = std_list_next (iter))
  {
    if (std_list_get_data (iter, &data) == -1)
      return -1;

    item = (struct cgdbrc_attach_item*)data;
    if (item->handle == handle)
    {
      std_list_remove (cgdbrc_attach_list, iter);
      break;
    }
  }

  return 0;
}

// }}}

// Get options {{{

cgdbrc_config_option_ptr 
cgdbrc_get (enum cgdbrc_option_kind option)
{
  return &cgdbrc_config_options[option];
}

int 
cgdbrc_get_key_code_timeoutlen (void)
{
  cgdbrc_config_option_ptr timeout_option_ptr = cgdbrc_get (CGDBRC_TIMEOUT);
  cgdbrc_config_option_ptr ttimeout_option_ptr = cgdbrc_get (CGDBRC_TTIMEOUT);

  /* Do not time out. */
  if (timeout_option_ptr->variant.int_val == 0 &&
      ttimeout_option_ptr->variant.int_val == 0)
    return 0;

  if (cgdbrc_get(CGDBRC_TTIMEOUT_LEN)->variant.int_val < 0)
    return cgdbrc_get(CGDBRC_TIMEOUT_LEN)->variant.int_val;
  else
    return cgdbrc_get (CGDBRC_TTIMEOUT_LEN)->variant.int_val;
}

int 
cgdbrc_get_mapped_key_timeoutlen (void)
{
  cgdbrc_config_option_ptr timeout_option_ptr = cgdbrc_get (CGDBRC_TIMEOUT);

  /* Do not time out. */
  if (timeout_option_ptr->variant.int_val == 0)
    return 0;

  return cgdbrc_get (CGDBRC_TIMEOUT_LEN)->variant.int_val;
}

// }}}
