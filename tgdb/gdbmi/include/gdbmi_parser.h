#ifndef __GDBMI_PARSER_H__
#define __GDBMI_PARSER_H__

#include "gdbmi_pt.h"

/* Doxygen headers {{{ */
/*!
 * \file
 * gdbmi_parser.h
 *
 * \brief
 * This interface is intended to be the abstraction layer that is capable of
 * parsing MI commands that are sent by GDB and determining if they are valid
 * or not. The input to this interface is a single MI command. This consists 
 * of one or more lines. That output is an MI parse tree, which represents 
 * the MI command in memory. The application should only use this parse 
 * tree to understand the command.
 */
/* }}} */

/* struct gdbmi_parser {{{ */

/**
 * @name Creating and destroying a gdbmi_parser context
 *
 * The GDBMI parser context can be created and destroyed here.
 */

/*@{*/

struct gdbmi_parser;
typedef struct gdbmi_parser *gdbmi_parser_ptr;

/**
 * Create a gdbmi_parser context.
 *
 * \return
 * The new instance of a gdbmi_parser, or NULL on error
 */
gdbmi_parser_ptr gdbmi_parser_create ( void );

/**
 * Destroy a gdbmi_parser context.
 *
 * \param parser
 * The instance the parser to destroy
 *
 * \return
 * 0 on succes, or -1 on error.
 */
int gdbmi_parser_destroy ( gdbmi_parser_ptr parser );

/*@}*/

/**
 * @name parsing functions
 *
 * These functions tell the context what to parse.
 */

/*@{*/

/**
 * tell the MI parser to parse the mi_command.
 *
 * \param parser
 * The gdbmi_parser context to operate on.
 *
 * \param mi_command
 * The null terminated mi command. This consists of one line,
 * or multiple lines, in either case, it should be a complete command.
 *
 * \param pt
 * If this function is successful (returns 0), then pt will be
 * the parse tree representing the mi_command.
 *
 * The user is responsible for freeing this data structure on there own.
 *
 * \return
 * 0 on succes, or -1 on error.
 */
int gdbmi_parser_parse_string ( 
	gdbmi_parser_ptr parser, 
	const char *mi_command,
    gdbmi_output_ptr *pt );

/**
 * tell the MI parser to parse the mi_command from a file.
 *
 * \param parser
 * The gdbmi_parser context to operate on.
 *
 * \param mi_command_file
 * The command file containing the mi command to parse.
 *
 * \param pt
 * If this function is successful (returns 0), then pt will be
 * the parse tree representing the mi_command.
 *
 * The user is responsible for freeing this data structure on there own.
 *
 * \param parse_failed
 * 1 if the parser failed to parse the command, otherwise 0
 *
 * \return
 * 0 on succes, or -1 on error.
 */
int gdbmi_parser_parse_file ( 
	gdbmi_parser_ptr parser_ptr, 
	const char *mi_command_file,
    gdbmi_output_ptr *pt,
    int *parse_failed );

/**
 * Get the last error in text format.
 *
 * \param parser
 * The gdbmi_parser context to operate on.
 *
 * \param parser_error
 * The last error that the parser discovered.
 * NULL if there was no error.
 *
 * \return
 * 0 on succes, or -1 on error.
 */
int gdbmi_parser_get_error ( gdbmi_parser_ptr parser, char **parser_error );

/*@}*/

/* }}} */

#endif
