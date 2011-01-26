#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include "highlight_groups.h"
#include "command_lexer.h"
#include "logger.h"

#if HAVE_CURSES_H
#include <curses.h>
#elif HAVE_NCURSES_CURSES_H
#include <ncurses/curses.h>
#endif /* HAVE_CURSES_H */

#include <strings.h>
#include <stdlib.h>

/* term.h prototypes */
extern int tgetnum();

/* internal {{{*/

#define UNSPECIFIED_COLOR (-2)

/** This represents all the data for a particular highlighting group. */
struct hl_group_info
{
  /** The kind of group */
  enum hl_group_kind kind;
  /** The attributes for a terminal, if it has no color support. */
  int mono_attrs;
  /** The attributes for a terminal, if it has color support. */
  int color_attrs;
  /** The id ncurses uses to represent a color-pair */
  int color_pair;
};

/** The main context used to represent all of the highlighting groups. */
struct hl_groups
{
  /** If 0 then the terminal doesn't support colors, otherwise it does. */
  int in_color;
  /** maximum numbers of colors on screen */
  int more_colors;
  /** This is the data for each highlighting group. */
  struct hl_group_info groups[HLG_LAST];
};

static struct hl_group_info *
lookup_group_info_by_key (struct hl_groups *groups, enum hl_group_kind kind)
{
  struct hl_group_info *lgroups;
  int i;

  for (i = 0; i < HLG_LAST; ++i)
    {
      lgroups = &groups->groups[i];
      if (kind == lgroups->kind)
	return lgroups;
    }

  return NULL;
}

/** The global instance, this is used externally */
hl_groups_ptr hl_groups_instance = NULL;

/** 
 * This describes all the attributes that a highlighting group can represent.
 * It is used to represent the default value for a group.
 */
struct default_hl_group_info
{
  /** The kind of group */
  enum hl_group_kind kind;
  /** Same as description above. */
  int mono_attrs;
  /** Same as description above. */
  int color_attrs;
  /** The foreground number, representing the color. */
  int fore_color;
  /** The background number, representing the color. */
  int back_color;
};

/** 
 * The default colors and attributes when use_default_colors is not available.
 * This means that -1 can not be used to represent the default color 
 * background. Currently, in this case, black is used as the color. This 
 * could eventually be configurable.
 */
static const struct default_hl_group_info default_groups_for_curses[] = {
  {HLG_KEYWORD, A_BOLD, A_BOLD, COLOR_BLUE, COLOR_BLACK},
  {HLG_TYPE, A_BOLD, A_BOLD, COLOR_GREEN, COLOR_BLACK},
  {HLG_LITERAL, A_BOLD, A_BOLD, COLOR_RED, COLOR_BLACK},
  {HLG_COMMENT, A_NORMAL, A_NORMAL, COLOR_YELLOW, COLOR_BLACK},
  {HLG_DIRECTIVE, A_BOLD, A_BOLD, COLOR_CYAN, COLOR_BLACK},
  {HLG_TEXT, A_NORMAL, A_NORMAL, COLOR_WHITE, COLOR_BLACK},
  {HLG_SEARCH, A_NORMAL, A_NORMAL, COLOR_BLACK, COLOR_WHITE},
  {HLG_STATUS_BAR, A_NORMAL, A_NORMAL, COLOR_BLACK, COLOR_WHITE},
  {HLG_ARROW, A_BOLD, A_BOLD, COLOR_GREEN, COLOR_BLACK},
  {HLG_LINE_HIGHLIGHT, A_BOLD, A_BOLD, COLOR_BLACK, COLOR_GREEN},
  {HLG_ENABLED_BREAKPOINT, A_BOLD, A_BOLD, COLOR_RED, COLOR_BLACK},
  {HLG_DISABLED_BREAKPOINT, A_BOLD, A_BOLD, COLOR_YELLOW, COLOR_BLACK},
  {HLG_SELECTED_LINE_NUMBER, A_BOLD, A_BOLD, COLOR_WHITE, COLOR_BLACK},
  {HLG_LOGO, A_BOLD, A_BOLD, COLOR_BLUE, COLOR_BLACK},
  {HLG_LAST, A_NORMAL, A_NORMAL, -1, -1}
};

/** 
 * The default colors and attributes for each type of token. 
 * This is used when ncurses is available.
 */
static const struct default_hl_group_info default_groups_for_background_dark[]
  = {
  {HLG_KEYWORD, A_BOLD, A_BOLD, COLOR_BLUE, -1},
  {HLG_TYPE, A_BOLD, A_BOLD, COLOR_GREEN, -1},
  {HLG_LITERAL, A_BOLD, A_BOLD, COLOR_RED, -1},
  {HLG_COMMENT, A_NORMAL, A_NORMAL, COLOR_YELLOW, -1},
  {HLG_DIRECTIVE, A_BOLD, A_BOLD, COLOR_CYAN, -1},
  {HLG_TEXT, A_NORMAL, A_NORMAL, -1, -1},
  {HLG_SEARCH, A_REVERSE, A_REVERSE, -1, -1},
  {HLG_STATUS_BAR, A_REVERSE, A_REVERSE, -1, -1},
  {HLG_ARROW, A_BOLD, A_BOLD, COLOR_GREEN, -1},
  {HLG_LINE_HIGHLIGHT, A_BOLD, A_BOLD, COLOR_BLACK, COLOR_GREEN},
  {HLG_ENABLED_BREAKPOINT, A_BOLD, A_BOLD, COLOR_RED, -1},
  {HLG_DISABLED_BREAKPOINT, A_BOLD, A_BOLD, COLOR_YELLOW, -1},
  {HLG_SELECTED_LINE_NUMBER, A_BOLD, A_BOLD, -1, -1},
  {HLG_LOGO, A_BOLD, A_BOLD, COLOR_BLUE, -1},
  {HLG_LAST, A_NORMAL, A_NORMAL, -1, -1}
};

struct hl_group_name
{
  /** The kind of group */
  enum hl_group_kind kind;
  /** The name of the group */
  const char *name;
};

static struct hl_group_name hl_group_names[] = {
  {HLG_KEYWORD, "Statement"},
  {HLG_TYPE, "Type"},
  {HLG_LITERAL, "Constant"},
  {HLG_COMMENT, "Comment"},
  {HLG_DIRECTIVE, "PreProc"},
  {HLG_TEXT, "Normal"},
  {HLG_SEARCH, "IncSearch"},
  {HLG_STATUS_BAR, "StatusLine"},
  {HLG_ARROW, "Arrow"},
  {HLG_LINE_HIGHLIGHT, "LineHighlight"},
  {HLG_ENABLED_BREAKPOINT, "Breakpoint"},
  {HLG_DISABLED_BREAKPOINT, "DisabledBreakpoint"},
  {HLG_SELECTED_LINE_NUMBER, "SelectedLineNr"},
  {HLG_LOGO, "Logo"},
  {HLG_LAST, NULL}
};

/**
 * Simply lookup the hl_group_kind from the name.
 *
 * \param name
 * The name to look up.
 *
 * \param kind
 * If the name is valid, kind will be returned as the group kind.
 *
 * \return 
 * 0 on success, -1 on error.
 */
static int
get_hl_group_kind_from_name (const char *name, enum hl_group_kind *kind)
{
  int i;

  if (!name || !kind)
    return -1;

  for (i = 0; hl_group_names[i].name != NULL; ++i)
    if (strcasecmp (name, hl_group_names[i].name) == 0)
      {
	*kind = hl_group_names[i].kind;
	return 0;
      }

  return -1;
}

/** This maps a particular attribute to a name. */
struct attr_pair
{
  /** The name of the attribute. */
  const char *name;
  /** The attribute value */
  int value;
};

/** The list of terminal attributes that CGDB supports */
static const struct attr_pair attr_names[] = {
  {"bold", A_BOLD},
  {"underline", A_UNDERLINE},
  {"reverse", A_REVERSE},
  {"inverse", A_REVERSE},
  {"standout", A_STANDOUT},
  {"NONE", A_NORMAL},
  {"normal", A_NORMAL},
  {"blink", A_BLINK},
  {"dim", A_DIM},
  {NULL, 0}
};

const struct attr_pair *
lookup_attr_pair_by_name (const char *name)
{
  int i;

  for (i = 0; attr_names[i].name != NULL; ++i)
    if (strcasecmp (name, attr_names[i].name) == 0)
      return &attr_names[i];

  return NULL;
}

/** A structure to represent a specific color. */
struct color_info
{
  /* The name of the color */
  const char *name;
  /* The number this color represents if the terminal supports 16 colors. */
  int nr16Color;
  /* The number this color represents if the terminal supports 8 colors. */
  int nr8Color;
  /* If the terminal has 8 colors, they could be bold. */
  int nr8ForegroundBold;
};

/** A list of all the default colors and their values */
static const struct color_info hl_color_names[] = {
  {"Black", 0, 0, 0},
  {"DarkBlue", 1, 4, 0},
  {"DarkGreen", 2, 2, 0},
  {"DarkCyan", 3, 6, 0},
  {"DarkRed", 4, 1, 0},
  {"DarkMagenta", 5, 5, 0},
  {"Brown", 6, 3, 0},
  {"DarkYellow", 6, 3, 0},
  {"LightGray", 7, 7, 0},
  {"LightGrey", 7, 7, 0},
  {"Gray", 7, 7, 0},
  {"Grey", 7, 7, 0},
  {"DarkGray", 8, 0, 1},
  {"DarkGrey", 8, 0, 1},
  {"Blue", 9, 4, 1},
  {"LightBlue", 9, 4, 1},
  {"Green", 10, 2, 1},
  {"LightGreen", 10, 2, 1},
  {"Cyan", 11, 6, 1},
  {"LightCyan", 11, 6, 1},
  {"Red", 12, 1, 1},
  {"LightRed", 12, 1, 1},
  {"Magenta", 13, 5, 1},
  {"LightMagenta", 13, 5, 1},
  {"Yellow", 14, 3, 1},
  {"LightYellow", 14, 3, 1},
  {"White", 15, 7, 1},
  {NULL, 0, 0, 0}
};

static const struct color_info *
color_spec_for_name (const char *name)
{
  int i;
  for (i = 0; hl_color_names[i].name != NULL; ++i)
    {
      if (strcasecmp (name, hl_color_names[i].name) == 0)
	return &hl_color_names[i];
    }

  return NULL;
}

/**
 * Set up a highlighting group to be displayed as the user wishes.
 *
 * \param group
 * The highlighting group
 *
 * \param mono_attrs
 * Attributes avialable in mono mode
 *
 * \param color_attrs
 * Attributes available during color mode
 *
 * \param fore_color
 * The foreground color
 *
 * \param back_color
 * The backgroud color
 *
 * \return
 * 0 on success, -1 on error.
 */
static int
setup_group (hl_groups_ptr hl_groups, enum hl_group_kind group,
	     int mono_attrs, int color_attrs, int fore_color, int back_color)
{
  /** Starts creating new colors at 1, and then is incremented each time. */
  static int next_color_pair = 1;
  struct hl_group_info *info;

  if (!hl_groups)
    return -1;

  info = lookup_group_info_by_key (hl_groups, group);

  if (!info)
    return -1;

  if (mono_attrs != UNSPECIFIED_COLOR)
    info->mono_attrs = mono_attrs;
  if (color_attrs != UNSPECIFIED_COLOR)
    info->color_attrs = color_attrs;

  /* The rest of this function sets up the colors, so we can stop here
   * if color isn't used. */
  if (!hl_groups->in_color)
    return 0;

  /* If no colors are specified, we're done. */
  if (fore_color == UNSPECIFIED_COLOR && back_color == UNSPECIFIED_COLOR)
    return 0;

  /* Don't allow -1 to be used in curses mode */
#ifndef NCURSES_VERSION
  if (fore_color < 0 || back_color < 0)
    return 0;
#endif

  /* If either the foreground or background color is unspecified, we
   * need to read the other so we don't clobber it. */
  if (fore_color == UNSPECIFIED_COLOR)
    {
      short old_fore_color, old_back_color;
      pair_content (info->color_pair, &old_fore_color, &old_back_color);
      fore_color = old_fore_color;
    }
  else if (back_color == UNSPECIFIED_COLOR)
    {
      short old_fore_color, old_back_color;
      pair_content (info->color_pair, &old_fore_color, &old_back_color);
      back_color = old_back_color;
    }

  /* If the group is already using the default colors, and we're not changing
   * it, don't consume a color pair. */
  if (fore_color < 0 && back_color < 0 && info->color_pair == 0)
    return 0;

  /* Allocate a new color pair if the group doesn't have one yet. */
  if (info->color_pair == 0)
    {
      info->color_pair = next_color_pair;
      next_color_pair += 1;
    }

  /* Set up the color pair. */
  if (info->color_pair < COLOR_PAIRS)
    {
      if (init_pair (info->color_pair, fore_color, back_color) != OK)
	return -1;
    }
  else
    return -1;

  return 0;
}

/* }}}*/

/* Creating and Destroying a hl_groups context. {{{*/
/*@{*/

hl_groups_ptr
hl_groups_initialize (void)
{
  int i;

  hl_groups_ptr hl_groups =
    (hl_groups_ptr) malloc (sizeof (struct hl_groups));
  if (!hl_groups)
    return NULL;

  hl_groups->in_color = 0;
  hl_groups->more_colors = 0;

  for (i = 0; i < HLG_LAST; ++i)
    {
      struct hl_group_info *info;
      info = &hl_groups->groups[i];
      info->kind = i + 1;
      info->mono_attrs = 0;
      info->mono_attrs = 0;
      info->color_pair = 0;
    }


  return hl_groups;
}

int
hl_groups_shutdown (hl_groups_ptr hl_groups)
{
  if (hl_groups)
    {
      free (hl_groups);
      hl_groups = NULL;
    }

  return 0;
}

/*@}*/
/* }}}*/

/* Functional commands {{{*/

/*@{*/

int
hl_groups_setup (hl_groups_ptr hl_groups)
{
  int i;
  const struct default_hl_group_info *ginfo;

  if (!hl_groups)
    return -1;

#ifdef NCURSES_VERSION
  ginfo = default_groups_for_background_dark;
#else
  ginfo = default_groups_for_curses;
#endif

  hl_groups->in_color = has_colors ();
  if (hl_groups->in_color)
    hl_groups->more_colors = (tgetnum ("Co") >= 16);

  /* Set up the default groups. */
  for (i = 0; ginfo[i].kind != HLG_LAST; ++i)
    {
      int val;
      const struct default_hl_group_info *spec = &ginfo[i];
      val = setup_group (hl_groups, spec->kind, spec->mono_attrs,
			 spec->color_attrs, spec->fore_color, spec->back_color);
      if (val == -1)
	{
	  logger_write_pos (logger, __FILE__, __LINE__, "setup group.");
	  return -1;
	}
    }

  return 0;
}

int
hl_groups_get_attr (hl_groups_ptr hl_groups, enum hl_group_kind kind, 
		    int *attr)
{
  struct hl_group_info *info = lookup_group_info_by_key (hl_groups, kind);

  if (!hl_groups || !info || !attr)
    return -1;

  if (!hl_groups->in_color)
    *attr = info->mono_attrs;
  else
    {
      *attr = info->color_attrs;
      if (info->color_pair)
	*attr |= COLOR_PAIR (info->color_pair);
    }

  return 0;
}

int
hl_groups_parse_config (hl_groups_ptr hl_groups)
{
  int token, val;
  const char *name;
  int mono_attrs = UNSPECIFIED_COLOR, color_attrs = UNSPECIFIED_COLOR;
  int fg_color = UNSPECIFIED_COLOR, bg_color = UNSPECIFIED_COLOR;
  int key, attrs, color;
  const struct color_info *color_spec;
  enum hl_group_kind group_kind;
  const struct attr_pair *pair;
  enum
  {
    TERM,
    CTERM,
    FG,
    BG,
    IGNORE
  };

  if (!hl_groups)
    return 1;

  /* First, get the "group", that is, the group. */
  token = yylex ();
  if (token != IDENTIFIER)
    {
#ifdef LOG_HIGHLIGHT_COMMAND_ERRORS
      logger_write_pos (logger, __FILE__, __LINE__, "Missing group name.");
#endif
      return 1;
    }

  name = get_token ();

  val = get_hl_group_kind_from_name (name, &group_kind);
  if (val == -1)
    {
#ifdef LOG_HIGHLIGHT_COMMAND_ERRORS
      logger_write_pos (logger, __FILE__, __LINE__,
			"Bad parameter name (\"%s\").", name);
#endif
      return 1;
    }

  if (group_kind < 0)
    {
      /* Just ignore groups we don't know about (or "link", which is not
       * a group, but which could appear here too). */
      return 0;
    }

  /* Now parse the settings for the group. */
  /* At the end of each key/value(s) pair, "token" will be set to the *next*
   * token.  That's because we have to deal with comma-separated lists. */
  token = yylex ();
  while (1)
    {
      /* Get the next key. */
      if (token == 0 || token == EOL)
	break;
      if (token != IDENTIFIER)
	{
#ifdef LOG_HIGHLIGHT_COMMAND_ERRORS
	  logger_write_pos (logger, __FILE__, __LINE__,
			    "Bad parameter name (\"%s\").", get_token ());
#endif
	  return 1;
	}
      name = get_token ();

      /* What type of key is it? */
      if (strcasecmp (name, "term") == 0)
	key = TERM;
      else if (strcasecmp (name, "cterm") == 0)
	key = CTERM;
      else if (strcasecmp (name, "ctermfg") == 0)
	key = FG;
      else if (strcasecmp (name, "ctermbg") == 0)
	key = BG;
      else
	key = IGNORE;

      /* A '=' must come next. */
      if (yylex () != '=')
	{
#ifdef LOG_HIGHLIGHT_COMMAND_ERRORS
	  logger_write_pos (logger, __FILE__, __LINE__,
			    "Missing '=' in \"highlight\" command.");
#endif
	  return 1;
	}

      /* Process the settings. */
      token = yylex ();
      switch (key)
	{
	case TERM:
	case CTERM:
	  attrs = 0;
	  while (1)
	    {
	      /* Add in the attribute. */
	      if (token != IDENTIFIER)
		{
#ifdef LOG_HIGHLIGHT_COMMAND_ERRORS
		  logger_write_pos (logger, __FILE__, __LINE__,
				    "Bad attribute name: \"%s\".",
				    get_token ());
#endif
		  return 1;
		}
	      name = get_token ();

	      pair = lookup_attr_pair_by_name (name);
	      if (!pair)
		{
#ifdef LOG_HIGHLIGHT_COMMAND_ERRORS
		  logger_write_pos (logger, __FILE__, __LINE__,
				    "Unknown attribute name");
#endif
		  return 1;
		}
	      if (pair->value == -1)
		{
#ifdef LOG_HIGHLIGHT_COMMAND_ERRORS
		  logger_write_pos (logger, __FILE__, __LINE__,
				    "Unknown attribute name: \"%s\".", name);
#endif
		  return 1;
		}
	      attrs |= pair->value;

	      /* Are there more attributes? */
	      token = yylex ();
	      if (token != ',')
		break;
	      token = yylex ();
	    }
	  if (key == TERM)
	    {
	      if (mono_attrs == UNSPECIFIED_COLOR)
		mono_attrs = attrs;
	      else
		mono_attrs |= attrs;
	    }
	  else
	    {
	      if (color_attrs == UNSPECIFIED_COLOR)
		color_attrs = attrs;
	      else
		color_attrs |= attrs;
	    }
	  break;

	case FG:
	case BG:
	  attrs = 0;
	  switch (token)
	    {
	    case NUMBER:
	      color = atoi (get_token ());
	      break;

	    case IDENTIFIER:
	      name = get_token ();
	      color_spec = color_spec_for_name (name);
	      if (color_spec == NULL)
		{
#ifdef LOG_HIGHLIGHT_COMMAND_ERRORS
		  logger_write_pos (logger, __FILE__, __LINE__,
				    "Unknown color: \"%s\".", name);
#endif
		  return 1;
		}
	      if (hl_groups->more_colors)
		color = color_spec->nr16Color;
	      else
		{
		  color = color_spec->nr8Color;
		  if (color_spec->nr8ForegroundBold)
		    {
		      if (color_attrs == UNSPECIFIED_COLOR)
			color_attrs = A_BOLD;
		      else
			color_attrs |= A_BOLD;
		    }
		}
	      break;

	    default:
#ifdef LOG_HIGHLIGHT_COMMAND_ERRORS
	      logger_write_pos (logger, __FILE__, __LINE__,
				"Bad token for color (\"%s\").",
				get_token ());
#endif
	      return 1;
	    }
	  if (key == FG)
	    fg_color = color;
	  else
	    bg_color = color;
	  token = yylex ();
	  break;

	case IGNORE:
	default:
	  /* Ignore the value(s) (potentially a comma-separated list). */
	  while (1)
	    {
	      if (token != IDENTIFIER && token != NUMBER)
		return 1;
	      token = yylex ();
	      if (token != ',')
		break;
	    }
	  break;
	}
    }

  val = setup_group (hl_groups, group_kind, mono_attrs, color_attrs, fg_color,
		     bg_color);
  if (val == -1)
    {
      return 1;
    }

  return 0;
}

/*@}*/
/* }}}*/
