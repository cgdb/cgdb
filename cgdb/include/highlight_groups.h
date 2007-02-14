#ifndef __HIGHLIGHT_GROUPS_H__
#define __HIGHLIGHT_GROUPS_H__

/*!
 * \file
 * highlight_group.h
 *
 * \brief
 * This file is dedicated to abstracting away the attributes and colors that 
 * are used to draw particular characters on the terminal. Each group has its
 * own characteristics, and can be configured by the user from the cgdbrc file.
 *
 * Basically, manages the colors and attributes (bold, reverse) etc. of text on
 * the screen.
 */

/* hl_group_kind {{{*/

/**
 * This is all of the different syntax highlighting color possibilities.
 * Each enumeration value represents a highlighting group. The rest of CGDB
 * can use these group names to represent which attributes the user wants to
 * use when drawing this particular group. Each highlighting group has 
 * default values that CGDB assigns to it. However, each of them are also
 * configurable.
 *
 * If you modify this enumeration, update the manual!
 */
enum hl_group_kind
{
  HLG_KEYWORD = 1,
  HLG_TYPE,
  HLG_LITERAL,
  HLG_COMMENT,
  HLG_DIRECTIVE,
  HLG_TEXT,
  HLG_SEARCH,
  HLG_STATUS_BAR,
  HLG_ARROW,
  HLG_LINE_HIGHLIGHT,
  HLG_ENABLED_BREAKPOINT,
  HLG_DISABLED_BREAKPOINT,
  HLG_SELECTED_LINE_NUMBER,
  HLG_LOGO,

  HLG_LAST
};

/* }}}*/

/* Createing and Destroying a hl_groups context. {{{*/
/******************************************************************************/
/**
 * @name Createing and Destroying a hl_groups context.
 * These functions are for createing and destroying a hl_groups context.
 */
/******************************************************************************/

/*@{*/

/**
 *  This struct is a reference to a hl_groups instance.
 */
struct hl_groups;
typedef struct hl_groups *hl_groups_ptr;

/** 
 * Currently, there is only a single instance. This is used (init/destroyed)
 * externally to this file. The entire application can use this to represent 
 * the current highlighting groups. In the future, it's possible each window 
 * could have there own * instance.
 */
extern hl_groups_ptr hl_groups_instance;

/**
 * This initializes an hl_groups instance.
 *
 * The client must call this function before any other function in the 
 * hl_groups library.
 *
 * @return
 * NULL on error, a valid context on success.
 */
hl_groups_ptr hl_groups_initialize (void);

/**
 * This will terminate a hl_groups session. No functions should be called on
 * the hl_groups context passed into this function after this call.
 *
 * \param hl_groups
 * An instance of hl_groups to operate on.
 *
 * @return
 * 0 on success or -1 on error
 */
int hl_groups_shutdown (hl_groups_ptr hl_groups);

/*@}*/
/* }}}*/

/* Functional commands {{{*/
/******************************************************************************/
/**
 * @name Functional commands
 * These functinos are used to ask the hl_groups context to perform a task.
 */
/******************************************************************************/

/*@{*/

/**
 * Setup the highlighting group to have all of the default values.
 *
 * \param hl_groups
 * An instance of hl_groups to operate on.
 *
 * \return
 * 0 on success or -1 on error
 */
int hl_groups_setup (hl_groups_ptr hl_groups);

/**
 * Get the attributes that may be passed to wattron to tell the curses library
 * how to print this particular group.
 *
 * \param hl_groups
 * An instance of hl_groups to operate on.
 *
 * \param kind
 * The particular group to get the attributes for.
 *
 * \param attr
 * The attributes associated with this pair.
 *
 * \return
 * 0 on success or -1 on error
 */
int hl_groups_get_attr (hl_groups_ptr hl_groups, enum hl_group_kind kind, int *attr);

/**
 * Parse a particular command. This may move into the cgdbrc file later on.
 *
 * \param hl_groups
 * An instance of hl_groups to operate on.
 *
 * \return
 * 0 on success or -1 on error
 */
int hl_groups_parse_config (hl_groups_ptr hl_groups);

/*@}*/
/* }}}*/

#endif 	/* __HIGHLIGHT_GROUPS_H__ */
