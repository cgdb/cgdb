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
gdbmi_parser_ptr gdbmi_parser_create(void);

/**
 * Destroy a gdbmi_parser context.
 *
 * \param parser
 * The instance the parser to destroy
 *
 * \return
 * 0 on succes, or -1 on error.
 */
int gdbmi_parser_destroy(gdbmi_parser_ptr parser);

/*@}*/

/**
 * @name parsing functions
 *
 * These functions tell the context what to parse.
 */

/*@{*/

/**
 * Tell the MI parser to parse the data.
 *
 * The data must be either a full line or multiple full lines. (ending in \n)
 * The normal usage of this function is to call it over and over again with
 * more data and wait for it to return an mi output command.
 *
 * It would be more convienent to allow the parser to parse any random
 * data but that would require flex to be a push flexer. It may even have
 * this feature but since I already changed bison, I don't want to think
 * anymore right now about how to improve this interface. It's good enough
 * for a first shot.
 *
 * \param parser
 * The gdbmi_parser context to operate on.
 *
 * \param mi_data
 * The null terminated mi data. This consists of one or more lines.
 *
 * \param pt
 * If this function is successful (returns 0), then pt may be set.
 * If the mi_data parameter completes an mi output command, than pt will
 * be non-null and represent the command parsed. Otherwise, if it is waiting
 * for more intput, it will be returned as NULL.
 *
 * The user is responsible for freeing this data structure on there own.
 *
 * \param parse_failed
 * 1 if the parser failed to parse the command, otherwise 0
 * If there was an error, it was written to the global logger.
 * Also, pt is invalid if there is an error, it should not be displayed or freed
 *
 * \return
 * 0 on succes, or -1 on error.
 */
int gdbmi_parser_parse_string(gdbmi_parser_ptr parser,
        const char *mi_data, gdbmi_output_ptr * pt, int *parse_failed);

/**
 * Tell the MI parser to parse the mi_command from a file.
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
 * If there was an error, it was written to the global logger.
 * Also, pt is invalid if there is an error, it should not be displayed or freed
 *
 * \return
 * 0 on succes, or -1 on error.
 */
int gdbmi_parser_parse_file(gdbmi_parser_ptr parser_ptr,
        const char *mi_command_file, gdbmi_output_ptr * pt, int *parse_failed);

/*@}*/

/* }}} */

#endif
