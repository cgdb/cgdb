#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <list>

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#if HAVE_STRINGS_H
#include <strings.h>
#endif /* HAVE_STRINGS_H */

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif /* HAVE_STDLIB_H */

#include "cgdbrc.h"
#include "command_lexer.h"
#include "tgdb.h"
#include "sys_util.h"
#include "cgdb.h"
#include "interface.h"
#include "tokenizer.h"
#include "highlight_groups.h"
#include "kui_term.h"

extern struct tgdb *tgdb;

/**
 * The general idea is that the configuration will read in the users ~/.cgdbrc
 * file, or ~/.cgdb/config or whatever, and execute each command.  This will
 * also be responsible for processing all : commands in the tui.
 *
 */

enum ConfigType {
    CONFIG_TYPE_BOOL,           /* set ic / set noic */
    CONFIG_TYPE_INT,            /* set tabstop=8 */
    CONFIG_TYPE_STRING,
    CONFIG_TYPE_FUNC_VOID,
    CONFIG_TYPE_FUNC_BOOL,
    CONFIG_TYPE_FUNC_INT,
    CONFIG_TYPE_FUNC_STRING
};

static int command_set_arrowstyle(const char *value);
static int command_set_cgdb_mode_key(const char *value);
static int command_set_executing_line_display(const char *value);
static int command_set_selected_line_display(const char *value);
static int command_set_timeout(int value);
static int command_set_timeoutlen(int value);
static int command_set_ttimeout(int value);
static int command_set_ttimeoutlen(int value);
static int command_set_winminheight(int value);
static int command_set_winminwidth(int value);
static int command_set_winsplit(const char *value);
static int command_set_winsplitorientation(const char *value);
static int command_set_syntax_type(const char *value);
static int command_set_sdc(int value);
static int cgdbrc_set_val(struct cgdbrc_config_option config_option);

struct cgdbrc_attach_item {
    enum cgdbrc_option_kind option;
    cgdbrc_notify notify_hook;
};

static std::list<struct cgdbrc_attach_item> cgdbrc_attach_list;

static int command_do_tgdbcommand(enum tgdb_command_type param);

static int command_focus_cgdb(int param);
static int command_focus_gdb(int param);

static int command_do_bang(int param);
static int command_do_focus(int param);
static int command_do_help(int param);
static int command_do_logo(int param);
static int command_do_quit(int param);
static int command_do_shell(int param);
static int command_source_reload(int param);

static int command_parse_syntax(int param);
static int command_parse_highlight(int param);
static int command_parse_map(int param);
static int command_parse_unmap(int param);

typedef int (*action_t) (int param);
typedef struct COMMANDS {
    const char *name;
    /* these functions will return 0 on success and 1 on error. */
    /* Should the configuration file processing continue after an error? */
    action_t action;
    int param;
} COMMANDS;

COMMANDS commands[] = {
    /* bang         */ {"bang", (action_t)command_do_bang, 0},
    /* edit         */ {"edit", (action_t)command_source_reload, 0},
    /* edit         */ {"e", (action_t)command_source_reload, 0},
    /* focus        */ {"focus", (action_t)command_do_focus, 0},
    /* help         */ {"help", (action_t)command_do_help, 0},
    /* logo         */ {"logo", (action_t)command_do_logo, 0},
    /* highlight    */ {"highlight", (action_t)command_parse_highlight, 0},
    /* highlight    */ {"hi", (action_t)command_parse_highlight, 0},
    /* imap         */ {"imap", (action_t)command_parse_map, 0},
    /* imap         */ {"im", (action_t)command_parse_map, 0},
    /* iunmap       */ {"iunmap", (action_t)command_parse_unmap, 0},
    /* iunmap       */ {"iu", (action_t)command_parse_unmap, 0},
    /* insert       */ {"insert", (action_t)command_focus_gdb, 0},
    /* map          */ {"map", (action_t)command_parse_map, 0},
    /* quit         */ {"quit", (action_t)command_do_quit, 0},
    /* quit         */ {"q", (action_t)command_do_quit, 0},
    /* shell        */ {"shell", (action_t)command_do_shell, 0},
    /* shell        */ {"sh", (action_t)command_do_shell, 0},
    /* syntax       */ {"syntax", (action_t)command_parse_syntax, 0},
    /* unmap        */ {"unmap", (action_t)command_parse_unmap, 0},
    /* unmap        */ {"unm", (action_t)command_parse_unmap, 0},
    /* continue     */ {"continue", (action_t)command_do_tgdbcommand, TGDB_CONTINUE},
    /* continue     */ {"c", (action_t)command_do_tgdbcommand, TGDB_CONTINUE},
    /* down         */ {"down", (action_t)command_do_tgdbcommand, TGDB_DOWN},
    /* finish       */ {"finish", (action_t)command_do_tgdbcommand, TGDB_FINISH},
    /* finish       */ {"f", (action_t)command_do_tgdbcommand, TGDB_FINISH},
    /* next         */ {"next", (action_t)command_do_tgdbcommand, TGDB_NEXT},
    /* next         */ {"n", (action_t)command_do_tgdbcommand, TGDB_NEXT},
    /* run          */ {"run", (action_t)command_do_tgdbcommand, TGDB_RUN},
    /* run          */ {"r", (action_t)command_do_tgdbcommand, TGDB_RUN},
    /* kill         */ {"kill", (action_t)command_do_tgdbcommand, TGDB_KILL},
    /* kill         */ {"k", (action_t)command_do_tgdbcommand, TGDB_KILL},
    /* step         */ {"step", (action_t)command_do_tgdbcommand, TGDB_STEP},
    /* step         */ {"s", (action_t)command_do_tgdbcommand, TGDB_STEP},
    /* start        */ {"start", (action_t)command_do_tgdbcommand, TGDB_START},
    /* until        */ {"until", (action_t)command_do_tgdbcommand, TGDB_UNTIL},
    /* until        */ {"u", (action_t)command_do_tgdbcommand, TGDB_UNTIL},
    /* up           */ {"up", (action_t)command_do_tgdbcommand, TGDB_UP}
};

int command_sort_find(const void *_left_cmd, const void *_right_cmd)
{
    COMMANDS *right_cmd = (COMMANDS *) _right_cmd;
    COMMANDS *left_cmd = (COMMANDS *) _left_cmd;

    return strcmp(left_cmd->name, right_cmd->name);
}

#define COMMANDS_SIZE		(sizeof(COMMANDS))
#define COMMANDS_COUNT		(sizeof(commands) / sizeof(COMMANDS))

/**
 * This data structure stores all the active values of all the config options.
 */
static struct cgdbrc_config_option cgdbrc_config_options[CGDBRC_WRAPSCAN + 1];

/**
 * Initialize the config options with default values.
 */
void cgdbrc_init_config_options(void)
{
    int i = 0;
    struct cgdbrc_config_option option;

    option.option_kind = CGDBRC_ARROWSTYLE;
    option.variant.line_display_style = LINE_DISPLAY_SHORT_ARROW;
    cgdbrc_config_options[i++] = option;

    option.option_kind = CGDBRC_AUTOSOURCERELOAD;
    option.variant.int_val = 1;
    cgdbrc_config_options[i++] = option;

    option.option_kind = CGDBRC_CGDB_MODE_KEY;
    option.variant.int_val = CGDB_KEY_ESC;
    cgdbrc_config_options[i++] = option;

    option.option_kind = CGDBRC_COLOR;
    option.variant.int_val = 1;
    cgdbrc_config_options[i++] = option;

    option.option_kind = CGDBRC_DEBUGWINCOLOR;
    option.variant.int_val = 1;
    cgdbrc_config_options[i++] = option;

    option.option_kind = CGDBRC_DISASM;
    option.variant.int_val = 0;
    cgdbrc_config_options[i++] = option;

    option.option_kind = CGDBRC_EXECUTING_LINE_DISPLAY;
    option.variant.line_display_style = LINE_DISPLAY_LONG_ARROW;
    cgdbrc_config_options[i++] = option;

    option.option_kind = CGDBRC_HLSEARCH;
    option.variant.int_val = 0;
    cgdbrc_config_options[i++] = option;

    option.option_kind = CGDBRC_IGNORECASE;
    option.variant.int_val = 0;
    cgdbrc_config_options[i++] = option;

    option.option_kind = CGDBRC_SELECTED_LINE_DISPLAY;
    option.variant.line_display_style = LINE_DISPLAY_BLOCK;
    cgdbrc_config_options[i++] = option;

    option.option_kind = CGDBRC_SHOWDEBUGCOMMANDS;
    option.variant.int_val = 0;
    cgdbrc_config_options[i++] = option;

    option.option_kind = CGDBRC_SHOWMARKS;
    option.variant.int_val = 1;
    cgdbrc_config_options[i++] = option;

    option.option_kind = CGDBRC_SYNTAX;
    option.variant.language_support_val = TOKENIZER_LANGUAGE_UNKNOWN;
    cgdbrc_config_options[i++] = option;

    option.option_kind = CGDBRC_TABSTOP;
    option.variant.int_val = 8;
    cgdbrc_config_options[i++] = option;

    option.option_kind = CGDBRC_TIMEOUT;
    option.variant.int_val = 1;
    cgdbrc_config_options[i++] = option;

    option.option_kind = CGDBRC_TIMEOUT_LEN;
    option.variant.int_val = 1000;
    cgdbrc_config_options[i++] = option;

    option.option_kind = CGDBRC_TTIMEOUT;
    option.variant.int_val = 1;
    cgdbrc_config_options[i++] = option;

    option.option_kind = CGDBRC_TTIMEOUT_LEN;
    option.variant.int_val = 100;
    cgdbrc_config_options[i++] = option;

    option.option_kind = CGDBRC_WINMINHEIGHT;
    option.variant.int_val = 0;
    cgdbrc_config_options[i++] = option;

    option.option_kind = CGDBRC_WINMINWIDTH;
    option.variant.int_val = 0;
    cgdbrc_config_options[i++] = option;

    option.option_kind = CGDBRC_WINSPLIT;
    option.variant.win_split_val = WIN_SPLIT_EVEN;
    cgdbrc_config_options[i++] = option;

    option.option_kind = CGDBRC_WINSPLITORIENTATION;
    option.variant.win_split_orientation_val = WSO_HORIZONTAL;
    cgdbrc_config_options[i++] = option;

    option.option_kind = CGDBRC_WRAPSCAN;
    option.variant.int_val = 1;
    cgdbrc_config_options[i++] = option;
}

/**
 * A configuration variable.
 */
struct ConfigVariable {
    ConfigVariable(const char *name_p, const char *s_name_p,
            enum ConfigType type_p, void *data_p) :
            name(name_p), s_name(s_name_p), type(type_p), data(data_p) {}

    /**
     * The name and shortname of the configuration variable.
     */
    const char *name, *s_name;
    
    /** The type of configuration variable */
    enum ConfigType type;
    
    /** 
     * The data member or function to set this config variables value.
     */
    void *data;
};

typedef std::list<ConfigVariable> ConfigVariableList;

/** The list of configuration variables */
static ConfigVariableList cgdbrc_variables;

/**
 * Initialize the configuration variables.
 */
void cgdbrc_init_config_variables(void)
{
    /* keep this stuff sorted! !sort */

    /* arrowstyle */
    cgdbrc_variables.push_back(ConfigVariable(
        "arrowstyle", "as", CONFIG_TYPE_FUNC_STRING,
        (void *)command_set_arrowstyle));
    /* autosourcereload */
    cgdbrc_variables.push_back(ConfigVariable(
        "autosourcereload", "asr", CONFIG_TYPE_BOOL,
        (void*)&cgdbrc_config_options[
            CGDBRC_AUTOSOURCERELOAD].variant.int_val));
    /* cgdbmodekey */
    cgdbrc_variables.push_back(ConfigVariable(
        "cgdbmodekey", "cgdbmodekey", CONFIG_TYPE_FUNC_STRING,
        (void *)command_set_cgdb_mode_key));
    /* color */
    cgdbrc_variables.push_back(ConfigVariable(
        "color", "color", CONFIG_TYPE_BOOL,
        (void *)&cgdbrc_config_options[CGDBRC_COLOR].variant.int_val));
    /* debugwincolor */
    cgdbrc_variables.push_back(ConfigVariable(
        "debugwincolor", "dwc", CONFIG_TYPE_BOOL,
        (void *)&cgdbrc_config_options[CGDBRC_DEBUGWINCOLOR].variant.int_val));
    /* disassemble */
    cgdbrc_variables.push_back(ConfigVariable(
        "disasm", "dis", CONFIG_TYPE_BOOL,
        (void *)&cgdbrc_config_options[CGDBRC_DISASM].variant.int_val));
    /* executinglinedisplay */
    cgdbrc_variables.push_back(ConfigVariable(
        "executinglinedisplay", "eld", CONFIG_TYPE_FUNC_STRING,
        (void *)command_set_executing_line_display));
    /* hlsearch */
    cgdbrc_variables.push_back(ConfigVariable(
        "hlsearch", "hls", CONFIG_TYPE_BOOL,
        (void *)&cgdbrc_config_options[CGDBRC_HLSEARCH].variant.int_val));
    /* ignorecase */
    cgdbrc_variables.push_back(ConfigVariable(
        "ignorecase", "ic", CONFIG_TYPE_BOOL,
        (void *)&cgdbrc_config_options[CGDBRC_IGNORECASE].variant.int_val));
    /* selectedlinedisplay */
    cgdbrc_variables.push_back(ConfigVariable(
        "selectedlinedisplay", "sld", CONFIG_TYPE_FUNC_STRING,
        (void *)command_set_selected_line_display));
    /* showdebugcommands */
    cgdbrc_variables.push_back(ConfigVariable(
        "showdebugcommands", "sdc", CONFIG_TYPE_FUNC_BOOL,
        (void *)&command_set_sdc));
    /* showmarks */
    cgdbrc_variables.push_back(ConfigVariable(
        "showmarks", "showmarks", CONFIG_TYPE_BOOL,
        (void *)&cgdbrc_config_options[CGDBRC_SHOWMARKS].variant.int_val));
    /* syntax */
    cgdbrc_variables.push_back(ConfigVariable(
        "syntax", "syn", CONFIG_TYPE_FUNC_STRING,
        (void *)command_set_syntax_type));
    /* tabstop   */
    cgdbrc_variables.push_back(ConfigVariable(
        "tabstop", "ts", CONFIG_TYPE_INT,
        (void *)&cgdbrc_config_options[CGDBRC_TABSTOP].variant.int_val));
    /* timeout   */
    cgdbrc_variables.push_back(ConfigVariable(
        "timeout", "to", CONFIG_TYPE_FUNC_BOOL,
        (void *)&command_set_timeout));
    /* timeoutlen   */
    cgdbrc_variables.push_back(ConfigVariable(
        "timeoutlen", "tm", CONFIG_TYPE_FUNC_INT,
        (void *)&command_set_timeoutlen));
    /* ttimeout   */
    cgdbrc_variables.push_back(ConfigVariable(
        "ttimeout", "ttimeout", CONFIG_TYPE_FUNC_BOOL,
        (void *)&command_set_ttimeout));
    /* ttimeoutlen   */
    cgdbrc_variables.push_back(ConfigVariable(
        "ttimeoutlen", "ttm", CONFIG_TYPE_FUNC_INT,
        (void *)&command_set_ttimeoutlen));
    /* winminheight */
    cgdbrc_variables.push_back(ConfigVariable(
        "winminheight", "wmh", CONFIG_TYPE_FUNC_INT,
        (void *)&command_set_winminheight));
    /* winminwidth */
    cgdbrc_variables.push_back(ConfigVariable(
        "winminwidth", "wmw", CONFIG_TYPE_FUNC_INT,
        (void *)&command_set_winminwidth));
    /* winsplit */
    cgdbrc_variables.push_back(ConfigVariable(
        "winsplit", "winsplit", CONFIG_TYPE_FUNC_STRING,
        (void *)command_set_winsplit));
    /* splitorientation */
    cgdbrc_variables.push_back(ConfigVariable(
        "winsplitorientation", "wso", CONFIG_TYPE_FUNC_STRING,
        (void *)command_set_winsplitorientation));
    /* wrapscan */
    cgdbrc_variables.push_back(ConfigVariable(
        "wrapscan", "ws", CONFIG_TYPE_BOOL,
        (void *)&cgdbrc_config_options[CGDBRC_WRAPSCAN].variant.int_val));
}

void cgdbrc_init(void)
{
    cgdbrc_init_config_options();

    cgdbrc_init_config_variables();

    qsort((void *) commands, COMMANDS_COUNT, COMMANDS_SIZE, command_sort_find);
}

COMMANDS *get_command(const char *cmd)
{
    COMMANDS command = { cmd, NULL, 0 };

    return (COMMANDS *)bsearch((void *) &command, (void *) commands,
            COMMANDS_COUNT, COMMANDS_SIZE, command_sort_find);
}

struct ConfigVariable *get_variable(const char *variable)
{
    /* FIXME: replace with binary search */
    ConfigVariableList::iterator iter = cgdbrc_variables.begin();
    for (; iter != cgdbrc_variables.end(); ++iter) {
        if (strcmp(variable, iter->name) == 0 ||
                strcmp(variable, iter->s_name) == 0) {
            return &*iter;
        }
    }

    return NULL;
}

int command_set_arrowstyle(const char *value)
{
    struct cgdbrc_config_option option;

    option.option_kind = CGDBRC_EXECUTING_LINE_DISPLAY;

    if (strcasecmp(value, "short") == 0)
        option.variant.line_display_style = LINE_DISPLAY_SHORT_ARROW;
    else if (strcasecmp(value, "long") == 0)
        option.variant.line_display_style = LINE_DISPLAY_LONG_ARROW;
    else if (strcasecmp(value, "highlight") == 0)
        option.variant.line_display_style = LINE_DISPLAY_HIGHLIGHT;
    else
        return 1;

    return cgdbrc_set_val(option);
}

int command_set_cgdb_mode_key(const char *value)
{
    struct cgdbrc_config_option option;

    option.option_kind = CGDBRC_CGDB_MODE_KEY;

    if (value) {
        /* If the user typed in a single key, use it. */
        if (strlen(value) == 1) {
            option.variant.int_val = value[0];
        } else {
            /* The user may have typed in a keycode. (e.g. <Esc>)
             * attempt to translate it. */
            int key = kui_term_get_cgdb_key_from_keycode(value);

            if (key == -1)
                return -1;
            option.variant.int_val = key;
        }
    } else {
        return -1;
    }

    return cgdbrc_set_val(option);
}

int command_set_executing_line_display(const char *value)
{
    struct cgdbrc_config_option option;

    option.option_kind = CGDBRC_EXECUTING_LINE_DISPLAY;

    if (strcasecmp(value, "shortarrow") == 0)
        option.variant.line_display_style = LINE_DISPLAY_SHORT_ARROW;
    else if (strcasecmp(value, "longarrow") == 0)
        option.variant.line_display_style = LINE_DISPLAY_LONG_ARROW;
    else if (strcasecmp(value, "highlight") == 0)
        option.variant.line_display_style = LINE_DISPLAY_HIGHLIGHT;
    else if (strcasecmp(value, "block") == 0)
        option.variant.line_display_style = LINE_DISPLAY_BLOCK;
    else
        return 1;

    return cgdbrc_set_val(option);
}

static int command_set_sdc(int value)
{
    if ((value == 0) || (value == 1)) {
        struct cgdbrc_config_option option;

        option.option_kind = CGDBRC_SHOWDEBUGCOMMANDS;
        option.variant.int_val = value;

        if (cgdbrc_set_val(option))
            return 1;
    } else
        return 1;

    return 0;
}

int command_set_winsplit(const char *value)
{
    struct cgdbrc_config_option option;
    WIN_SPLIT_TYPE split_type = WIN_SPLIT_EVEN;

    option.option_kind = CGDBRC_WINSPLIT;

    /* deprecated: use src_big */
    if (strcasecmp(value, "top_big") == 0)
        split_type = WIN_SPLIT_SRC_BIG;
    /* deprecated: use src_full */
    else if (strcasecmp(value, "top_full") == 0)
        split_type = WIN_SPLIT_SRC_FULL;
    /* deprecated: use gdb_big */
    else if (strcasecmp(value, "bottom_big") == 0)
        split_type = WIN_SPLIT_GDB_BIG;
    /* deprecated: use gdb_full */
    else if (strcasecmp(value, "bottom_full") == 0)
        split_type = WIN_SPLIT_GDB_FULL;
    else if (strcasecmp(value, "src_big") == 0)
        split_type = WIN_SPLIT_SRC_BIG;
    else if (strcasecmp(value, "src_full") == 0)
        split_type = WIN_SPLIT_SRC_FULL;
    else if (strcasecmp(value, "gdb_big") == 0)
        split_type = WIN_SPLIT_GDB_BIG;
    else if (strcasecmp(value, "gdb_full") == 0)
        split_type = WIN_SPLIT_GDB_FULL;
    else
        split_type = WIN_SPLIT_EVEN;

    option.variant.win_split_val = split_type;
    if (cgdbrc_set_val(option))
        return 1;
    if_set_winsplit(split_type);

    return 0;
}

int command_set_winsplitorientation(const char *value)
{
    struct cgdbrc_config_option option;
    WIN_SPLIT_ORIENTATION_TYPE orientation = WSO_HORIZONTAL;

    option.option_kind = CGDBRC_WINSPLITORIENTATION;

    if (strcasecmp(value, "horizontal") == 0)
        orientation = WSO_HORIZONTAL;
    else if (strcasecmp(value, "vertical") == 0)
        orientation = WSO_VERTICAL;

    option.variant.win_split_orientation_val = orientation;
    if (cgdbrc_set_val(option))
        return 1;
    if_set_winsplitorientation(orientation);

    return 0;
}

static int command_set_winminheight(int value)
{
    struct cgdbrc_config_option option;

    option.option_kind = CGDBRC_WINMINHEIGHT;

    if (if_change_winminheight(value) == -1)
        return 1;

    option.variant.int_val = value;
    return cgdbrc_set_val(option);
}

static int command_set_winminwidth(int value)
{
    struct cgdbrc_config_option option;

    option.option_kind = CGDBRC_WINMINWIDTH;

    if (if_change_winminwidth(value) == -1)
        return 1;

    option.variant.int_val = value;
    return cgdbrc_set_val(option);
}

int command_set_selected_line_display(const char *value)
{
    struct cgdbrc_config_option option;

    option.option_kind = CGDBRC_SELECTED_LINE_DISPLAY;

    if (strcasecmp(value, "shortarrow") == 0)
        option.variant.line_display_style = LINE_DISPLAY_SHORT_ARROW;
    else if (strcasecmp(value, "longarrow") == 0)
        option.variant.line_display_style = LINE_DISPLAY_LONG_ARROW;
    else if (strcasecmp(value, "highlight") == 0)
        option.variant.line_display_style = LINE_DISPLAY_HIGHLIGHT;
    else if (strcasecmp(value, "block") == 0)
        option.variant.line_display_style = LINE_DISPLAY_BLOCK;
    else
        return 1;

    return cgdbrc_set_val(option);
}

static int command_set_timeout(int value)
{
    struct cgdbrc_config_option option;

    option.option_kind = CGDBRC_TIMEOUT;
    option.variant.int_val = value;

    if (cgdbrc_set_val(option))
        return 1;

    return 0;
}

static int command_set_timeoutlen(int value)
{
    if (value >= 0 && value <= 10000) {
        struct cgdbrc_config_option option;

        option.option_kind = CGDBRC_TIMEOUT_LEN;
        option.variant.int_val = value;

        if (cgdbrc_set_val(option))
            return 1;
    }

    return 0;
}

static int command_set_ttimeout(int value)
{
    struct cgdbrc_config_option option;

    option.option_kind = CGDBRC_TTIMEOUT;
    option.variant.int_val = value;

    if (cgdbrc_set_val(option))
        return 1;

    return 0;
}

static int command_set_ttimeoutlen(int value)
{
    if (value >= 0 && value <= 10000) {
        struct cgdbrc_config_option option;

        option.option_kind = CGDBRC_TTIMEOUT_LEN;
        option.variant.int_val = value;

        if (cgdbrc_set_val(option))
            return 1;
    }

    return 0;
}

int command_set_syntax_type(const char *value)
{
    /* In sources.c */
    extern int sources_syntax_on;

    struct cgdbrc_config_option option;
    enum tokenizer_language_support lang = TOKENIZER_LANGUAGE_UNKNOWN;

    option.option_kind = CGDBRC_SYNTAX;

    if (strcasecmp(value, "c") == 0)
        lang = TOKENIZER_LANGUAGE_C;
    else if (strcasecmp(value, "asm") == 0)
        lang = TOKENIZER_LANGUAGE_ASM;
    else if (strcasecmp(value, "d") == 0)
        lang = TOKENIZER_LANGUAGE_D;
    else if (strcasecmp(value, "go") == 0)
        lang = TOKENIZER_LANGUAGE_GO;
    else if (strcasecmp(value, "ada") == 0)
        lang = TOKENIZER_LANGUAGE_ADA;
    else if (strcasecmp(value, "rust") == 0)
        lang = TOKENIZER_LANGUAGE_RUST;

    /* If caller specified a language or 'on', enable highlighting */
    if (lang != TOKENIZER_LANGUAGE_UNKNOWN || strcasecmp(value, "on") == 0 || strcasecmp(value, "yes") == 0)
        sources_syntax_on = 1;
    else if (strcasecmp(value, "no") == 0 || strcasecmp(value, "off") == 0)
        sources_syntax_on = 0;

    option.variant.language_support_val = lang;
    if (cgdbrc_set_val(option))
        return 1;

    if_highlight_sviewer(lang);
    return 0;
}

int command_focus_cgdb(int param)
{
    if_set_focus(CGDB);
    return 0;
}

int command_focus_gdb(int param)
{
    if_set_focus(GDB);
    return 0;
}

int command_do_bang(int param)
{
    return 0;
}

int command_do_tgdbcommand(enum tgdb_command_type param)
{
    tgdb_request_run_debugger_command(tgdb, param);
    return 0;
}

int command_do_focus(int param)
{
    int token = yylex();
    const char *value;

    if (token != IDENTIFIER)
        return 1;
    value = get_token();

    if (strcasecmp(value, "cgdb") == 0)
        command_focus_cgdb(0);
    else if (strcasecmp(value, "gdb") == 0)
        command_focus_gdb(0);
    else
        return 1;

    return 0;
}

int command_do_help(int param)
{
    if_display_help();
    return 0;
}

int command_do_logo(int param)
{
    if_display_logo(1);
    return 0;
}

int command_do_quit(int param)
{
    /* FIXME: Test to see if debugged program is still running */
    cgdb_cleanup_and_exit(0);
    return 0;
}

int command_do_shell(int param)
{
    return run_shell_command(NULL);
}

int command_source_reload(int param)
{
    struct sviewer *sview = if_get_sview();

    if (!sview)
        return -1;

    /* If there is no current source file, then there is nothing to reload. */
    if (!sview->cur)
        return 0;

    if (source_reload(sview, sview->cur->path, 1) == -1)
        return -1;

    return 0;
}

int command_parse_syntax(int param)
{
    /* This is something like: 
       :syntax
       :syntax on
       :syntax off
     */

    int rv = 1;

    switch ((rv = yylex())) {
        case EOL:{
            /* TODO: Print out syntax info (like vim?) */
        }
            break;
        case IDENTIFIER:{
            const char *value = get_token();

            command_set_syntax_type(value);

            if_draw();
        }
            break;
        default:
            break;
    }

    return 0;
}

static int command_parse_highlight(int param)
{
    return hl_groups_parse_config(hl_groups_instance);
}

extern struct kui_map_set *kui_map, *kui_imap;

static int command_parse_map(int param)
{
    struct kui_map_set *kui_map_choice;
    int key, value, val;
    char *key_token;
    extern int enter_map_id;

    enter_map_id = 1;

    if (strcmp(get_token(), "map") == 0)
        kui_map_choice = kui_map;
    else
        kui_map_choice = kui_imap;

    key = yylex();
    if (key != IDENTIFIER) {
        enter_map_id = 0;
        return -1;
    }
    key_token = cgdb_strdup(get_token());

    value = yylex();
    if (value != IDENTIFIER) {
        free(key_token);
        enter_map_id = 0;
        return -1;
    }

    val = kui_ms_register_map(kui_map_choice, key_token, get_token());
    if (val == -1) {
        free(key_token);
        enter_map_id = 0;
        return -1;
    }

    enter_map_id = 0;

    return 0;
}

static int command_parse_unmap(int param)
{
    struct kui_map_set *kui_map_choice;
    int key, val;
    char *key_token;
    extern int enter_map_id;

    enter_map_id = 1;

    if ((strcmp(get_token(), "unmap") == 0) ||
            (strcmp(get_token(), "unm") == 0))
        kui_map_choice = kui_map;
    else
        kui_map_choice = kui_imap;

    key = yylex();
    if (key != IDENTIFIER) {
        enter_map_id = 0;
        return -1;
    }
    key_token = cgdb_strdup(get_token());

    val = kui_ms_deregister_map(kui_map_choice, key_token);
    if (val == -1) {
        free(key_token);
        enter_map_id = 0;
        return -1;
    }

    enter_map_id = 0;

    return 0;
}

static void variable_changed_cb(struct ConfigVariable *variable)
{
    /* User switched source/disasm mode. Request a frame update
       so the appropriate data is displayed in the source window. */
    if (!strcmp(variable->name, "disasm"))
        tgdb_request_current_location(tgdb);
}

int command_do_disassemble(int param)
{
    int ret;
    struct sviewer *sview = if_get_sview();

    ret = source_set_exec_addr(sview, 0);

    if (!ret) {
        if_draw();
    } else if (sview->addr_frame) {
        /* No disasm found - request it */
        tgdb_request_disassemble_func(tgdb, DISASSEMBLE_FUNC_SOURCE_LINES);
    }

    return 0;
}

int command_parse_set(void)
{
    /* commands could look like the following:
     * set ignorecase
     * set noignorecase
     * set focus=gdb
     * set tabstop=8
     */

    int rv = 1;

    switch ((rv = yylex())) {
        case EOL:{
            /* TODO: Print out all the variables that have been set. */
        }
            break;
        case IDENTIFIER:{
            int boolean = 1;
            int is_inverse = 0;
            const char *value = NULL;
            const char *token = get_token();
            int length = strlen(token);
            struct ConfigVariable *variable = NULL;

            if (length > 2 && token[0] == 'n' && token[1] == 'o') {
                value = token + 2;
                boolean = 0;
            } else if (length > 3
                    && token[0] == 'i' && token[1] == 'n' && token[2] == 'v') {
                value = token + 3;
                is_inverse = 1;
            } else {
                value = token;
            }

            if ((variable = get_variable(value)) != NULL) {
                rv = 0;
                if ((is_inverse || boolean == 0) && variable->type != CONFIG_TYPE_BOOL) {
                    /* this is an error, you cant' do:
                     * set notabstop 
                     */
                    rv = 1;
                }

                switch (variable->type) {
                    case CONFIG_TYPE_BOOL:
                        if (is_inverse) {
                            boolean = !(*(int *) (variable->data));
                        }

                        *(int *) (variable->data) = boolean;
                        break;
                    case CONFIG_TYPE_INT:{
                        if (yylex() == '=' && yylex() == NUMBER) {
                            int data = strtol(get_token(), NULL, 10);

                            *(int *) (variable->data) = data;
                        } else {
                            rv = 1;
                        }
                    }
                        break;
                    case CONFIG_TYPE_STRING:{
                        if (yylex() == '=' &&
                                (rv = yylex(), rv == STRING
                                        || rv == IDENTIFIER)) {
                            /* BAM! comma operator */
                            char *data = (char *) get_token();

                            if (rv == STRING) {
                                /* get rid of quotes */
                                data = data + 1;
                                data[strlen(data) - 1] = '\0';
                            }
                            if (variable->data) {
                                free(variable->data);
                            }
                            variable->data = strdup(data);
                        } else {
                            rv = 1;
                        }
                    }
                        break;
                    case CONFIG_TYPE_FUNC_VOID:{
                        int (*functor) (void) = (int (*)(void)) variable->data;

                        if (functor) {
                            rv = functor();
                        } else {
                            rv = 1;
                        }
                    }
                        break;
                    case CONFIG_TYPE_FUNC_BOOL:{
                        int (*functor) (int) = (int (*)(int)) variable->data;

                        if (functor) {
                            rv = functor(boolean);
                        } else {
                            rv = 1;
                        }
                    }
                        break;
                    case CONFIG_TYPE_FUNC_INT:{
                        int (*functor) (int) = (int (*)(int)) variable->data;

                        if (yylex() == '=' && yylex() == NUMBER) {
                            int data = strtol(get_token(), NULL, 10);

                            if (functor) {
                                rv = functor(data);
                            } else {
                                rv = 1;
                            }
                        } else {
                            rv = 1;
                        }
                    }
                        break;
                    case CONFIG_TYPE_FUNC_STRING:{
                        int (*functor) (const char *) =
                                (int (*)(const char *)) variable->data;
                        if (yylex() == '=' && (rv = yylex(), rv == STRING
                                        || rv == IDENTIFIER)) {
                            /* BAM! comma operator */
                            char *data = (char *) get_token();

                            if (rv == STRING) {
                                /* get rid of quotes */
                                data = data + 1;
                                data[strlen(data) - 1] = '\0';
                            }
                            if (functor) {
                                rv = functor(data);
                            } else {
                                rv = 1;
                            }
                        } else {
                            rv = 1;
                        }
                    }
                        break;
                    default:
                        rv = 1;
                        break;
                }

                /* Tell callback function this variable has changed. */
                variable_changed_cb(variable);
            }
        }
            break;
        default:
            break;
    }

    return rv;
}

typedef struct yy_buffer_state *YY_BUFFER_STATE;
int command_parse_string(const char *buffer)
{
    extern YY_BUFFER_STATE yy_scan_string(const char *yy_str);
    extern void yy_delete_buffer(YY_BUFFER_STATE state);
    int rv = 1;
    YY_BUFFER_STATE state = yy_scan_string((char *) buffer);

    switch (yylex()) {
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

        case NUMBER:{
            const char *number = get_token();

            if (number[0] == '+') {
                source_vscroll(if_get_sview(), atoi(number + 1));
                rv = 0;
            } else if (number[0] == '-') {
                source_vscroll(if_get_sview(), -atoi(number + 1));
                rv = 0;
            } else {
                source_set_sel_line(if_get_sview(), atoi(number));
                rv = 0;
            }
            if_draw();
        }
            break;

        case IDENTIFIER:{
            COMMANDS *command = get_command(get_token());

            if (command) {
                command->action(command->param);
                rv = 0;
            } else {
                rv = 1;
            }
        }
            break;

        case EOL:
            /* basically just an empty line, don't do nothin. */
            rv = 0;
            break;

        default:
            rv = 1;
            break;
    }

    yy_delete_buffer(state);
    return rv;
}

int command_parse_file(const char *config_file)
{
    FILE *fp;

    fp = fopen(config_file, "r");
    if (fp) {
        char buffer[4096];
        char *p = buffer;
        int linenumber = 0;

        while (linenumber++, fgets(p, sizeof (buffer) - (p - buffer), fp)) {
            int bufferlen = strlen(buffer);

            if ((bufferlen - 2 >= 0) && buffer[bufferlen - 2] == '\\') {
                /* line continuation character, read another line into the buffer */
                linenumber--;
                p = buffer + bufferlen - 2;
                continue;
            }

            if (command_parse_string(buffer)) {
                /* buffer already has an \n */
                if_print_message("Error parsing line %d: %s", linenumber, buffer);
                /* return -1; don't return, lets keep parsing the file. */
            }

            p = buffer;
        }

        fclose(fp);
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
static int cgdbrc_set_val(struct cgdbrc_config_option config_option)
{
    std::list<struct cgdbrc_attach_item>::iterator iter;

    cgdbrc_config_options[config_option.option_kind] = config_option;

    /* Alert anyone that wants to be notified that an option has changed. */
    iter = cgdbrc_attach_list.begin();
    for (; iter != cgdbrc_attach_list.end(); ++iter) {
        if (iter->option == config_option.option_kind) {
            if (iter->notify_hook(&config_option))
                return 1;
        }
    }

    return 0;
}

/* Attach/Detach options {{{ */

int
cgdbrc_attach(enum cgdbrc_option_kind option, cgdbrc_notify notify)
{
    struct cgdbrc_attach_item item;
    item.option = option;
    item.notify_hook = notify;

    cgdbrc_attach_list.push_back(item);

    return 0;
}

/* }}} */

/* Get options {{{ */

cgdbrc_config_option_ptr cgdbrc_get(enum cgdbrc_option_kind option)
{
    return &cgdbrc_config_options[option];
}

int cgdbrc_get_int(enum cgdbrc_option_kind option)
{
    return cgdbrc_get(option)->variant.int_val;
}

enum LineDisplayStyle cgdbrc_get_displaystyle(enum cgdbrc_option_kind option)
{
    return cgdbrc_get(option)->variant.line_display_style;
}

int cgdbrc_get_key_code_timeoutlen(void)
{
    int timeout_val = cgdbrc_get_int(CGDBRC_TIMEOUT);
    int ttimeout_val = cgdbrc_get_int(CGDBRC_TTIMEOUT);

    /* Do not time out. */
    if (timeout_val == 0 && ttimeout_val == 0)
        return 0;

    if (cgdbrc_get_int(CGDBRC_TTIMEOUT_LEN) < 0)
        return cgdbrc_get_int(CGDBRC_TIMEOUT_LEN);
    else
        return cgdbrc_get_int(CGDBRC_TTIMEOUT_LEN);
}

int cgdbrc_get_mapped_key_timeoutlen(void)
{
    /* Do not time out. */
    if (cgdbrc_get_int(CGDBRC_TIMEOUT) == 0)
        return 0;

    return cgdbrc_get_int(CGDBRC_TIMEOUT_LEN);
}

/* }}} */
