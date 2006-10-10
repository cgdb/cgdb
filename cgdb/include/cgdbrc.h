#ifndef __CGDBRC_H__
#define __CGDBRC_H__

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_STDIO_H
#include <stdio.h>
#endif /* HAVE_STDIO_H */

/* TODO: Remove this include. It is simply used to get 
 * enum tokenizer_language_support.
 */
#include "tokenizer.h"

/**
 * \file
 * cgdbrc.h
 *
 * \brief
 * An interface for dealing with config parameters. A client can determine
 * the value of a configuration option by calling cgdbrc_get.
 */

/** 
 * Parse a string of command data and execute the commands that it represents.
 *
 * \param buffer
 * A line of command data to parse (representing a single command).
 *
 * \return
 * 0 on success, otherwise error.
 */
int command_parse_string (const char *buffer);

/**
 * Parse a configuration file, and execute teh commands that it contains.
 * The commands will be executed in order.
 *
 * \param fp
 * The file to parse.
 *
 * \return
 * Currently only returns 0.
 */
int command_parse_file (FILE * fp);

/** 
 * The different ways to highlight the current line the debugger is at.
 * This enum name is incorrect, it should be renamed to something like,
 * 'enum HighlightStyle'.
 */
enum ArrowStyle
{
  ARROWSTYLE_SHORT,
  ARROWSTYLE_LONG,
  ARROWSTYLE_HIGHLIGHT
};

/** window split type enumeration*/
typedef enum
{ WIN_SPLIT_FREE = -3,		/* split point not on quarter mark */

  WIN_SPLIT_BOTTOM_FULL = -2,	/* src window is 0%   of screen */
  WIN_SPLIT_BOTTOM_BIG = -1,	/* src window is 25%  of screen */
  WIN_SPLIT_EVEN = 0,		/* src window is 50%  of screen */
  WIN_SPLIT_TOP_BIG = 1,	/* src window is 75%  of screen */
  WIN_SPLIT_TOP_FULL = 2	/* src window is 100% of screen */
} WIN_SPLIT_TYPE;

/** All of the different configuration options */
enum cgdbrc_option_kind
{
  CGDBRC_ARROWSTYLE,
  CGDBRC_AUTOSOURCERELOAD,
  CGDBRC_ESCDELAY,
  CGDBRC_IGNORECASE,
  CGDBRC_SHORTCUT,
  CGDBRC_SHOWTGDBCOMMANDS,
  CGDBRC_SYNTAX,
  CGDBRC_TABSTOP,
  CGDBRC_WINMINHEIGHT,
  CGDBRC_WINSPLIT,
  CGDBRC_WRAPSCAN
};

/** This represents a single configuration option. */
struct cgdbrc_config_option
{
  enum cgdbrc_option_kind option_kind;
  union
  {
    /* option_kind == CGDBRC_ARROWSTYLE */
    enum ArrowStyle arrow_style;
    /* option_kind == CGDBRC_AUTOSOURCERELOAD */
    /* option_kind == CGDBRC_ESCDELAY */
    /* option_kind == CGDBRC_IGNORECASE */
    /* option_kind == CGDBRC_SHORTCUT */
    /* option_kind == CGDBRC_SHOWTGDBCOMMANDS */
    /* option_kind == CGDBRC_TABSTOP */
    /* option_kind == CGDBRC_TIMEOUT */
    /* option_kind == CGDBRC_TIMEOUTLEN */
    /* option_kind == CGDBRC_TTIMEOUT */
    /* option_kind == CGDBRC_TTIMEOUTLEN */
    /* option_kind == CGDBRC_WINMINHEIGHT */
    /* option_kind == CGDBRC_WRAPSCAN */
    int int_val;
    /* option_kind == CGDBRC_SYNTAX */
    enum tokenizer_language_support language_support_val;
    /* option_kind == CGDBRC_WINSPLIT */
    WIN_SPLIT_TYPE win_split_val;
  } variant;
};

typedef struct cgdbrc_config_option *cgdbrc_config_option_ptr;

#if 0
/**
 * If the notify function fails, this tells the configuration package
 * that the option was not acceptable. The option will not be kept.
 * This allows a mechanism for the configuration package to allow other
 * packages to validate if a particular config option value is OK. So,
 * a client might use this even if they are not interested in getting the
 * new values, but want to validate the values, in order to not tightly
 * couple the config reader with the other parts of CGDB.
 */
typedef int (*cgdbrc_notify) (struct cgdbrc_config_option * option);

/**
 * This will attach a new callback function for a particular option.
 * The client will be notified when the value is changed, and also
 * on program startup.
 *
 * \param option
 * The new option to attach a callback to.
 *
 * \param notify
 * The callback function to call when the state of the data changes.
 *
 * \return
 * 0 on success, or -1 on error.
 */
int cgdbrc_attach (enum cgdbrc_option_kind option, cgdbrc_notify notify);

/* NOTE: There is currently no need to detach, when this becomes necessary,
 * you'll have to add the functionality. */
#endif

/**
 * Get a configuration option.
 *
 * \param option
 * The option to get
 *
 * \return
 * This will never return NULL. It is returned as a pointer simply to not
 * pass the entire structure back. The configuration option corresponding
 * to the option asked for is returned.
 */
cgdbrc_config_option_ptr cgdbrc_get (enum cgdbrc_option_kind option);

#endif /* __CGDBRC_H__ */
