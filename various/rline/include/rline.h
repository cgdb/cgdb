#ifndef __RL_H__
#define __RL_H__

/*!
 * \file
 * rl.h
 *
 * \brief
 * This interface is the abstraction layer to the GNU readline library.
 * Currently readline only supports 1 instance of itself per program. Until it
 * supports multiple instance's, this library will also only support a single
 * instance.
 */

#include "tgdb_list.h"
#include "std_list.h"

/* Createing and Destroying a librline context. {{{ */
/******************************************************************************/
/**
 * @name Createing and Destroying a librline context.
 * These functions are for createing and destroying a rline context.
 */
/******************************************************************************/

/*@{*/

/**
 *  This struct is a reference to a librline instance.
 */
struct rline;

/** 
 * The signature of the callback function that rline calls when it detects 
 * that a command has been typed by the user. */
typedef void command_cb (char *);
/** 
 * The signature of the callback function that rline calls when it detects 
 * that the user has requested completion on the current line. */
typedef int completion_cb (int, int);

/**
 * This initializes an rline library instance.
 *
 * The client must call this function before any other function in the 
 * rline library.
 *
 * \param slavefd
 * This is the file descriptor representing the slave side of a terminal device.
 *
 * \param command
 * The command callback that should be used when readline completes a command.
 *
 * \param completion
 * The completion callback that should be used when readline requests completion.
 *
 * \param TERM
 * Pass in the TERM that readline should think it's output should be like.
 *
 * @return
 * NULL on error, a valid context on success.
 */
struct rline* rline_initialize (int slavefd, command_cb *command, completion_cb *completion, 
				char *TERM);

/**
 * This will terminate a librline session. No functions should be called on
 * the rline context passed into this function after this call.
 *
 * \param rline
 * An instance of the rline library to operate on.
 *
 * @return
 * 0 on success or -1 on error
 */
int rline_shutdown (struct rline *rline);

/*@}*/
/* }}}*/

/* Reading and Writing the librline context. {{{*/
/******************************************************************************/
/**
 * @name Reading and Writing the librline context.
 * These functions are for reading and writing an rline context.
 */
/******************************************************************************/

/*@{*/

/**
 * Read readline history into memory.
 *
 * \param rline
 * The readline context to operate on.
 *
 * \param file
 * The filename to be used that contains the readline history.
 *
 * \return
 * 0 on success or -1 on error
 */
int rline_read_history (struct rline *rline, const char *file);

/**
 * Write readline history to file.
 *
 * \param rline
 * The readline context to operate on.
 *
 * \param file
 * The filename to write the readline history to.
 *
 * \return
 * 0 on success or -1 on error
 */
int rline_write_history (struct rline *rline, const char *file);

/*@}*/
/* }}}*/

/* Functional commands {{{*/
/******************************************************************************/
/**
 * @name Functional commands
 * These functinos are used to ask the librline context to perform a task.
 */
/******************************************************************************/

/*@{*/

/**
 * Ask librline to change the current prompt that it use's.
 *
 * \param rline
 * The readline context to operate on.
 *
 * \param prompt
 * The prompt to give to readline.
 *
 * \return
 * 0 on success or -1 on error.
 */
int rline_set_prompt (struct rline *rline, const char *prompt);

/**
 * Get the current prompt that readline is using.
 *
 * \param rline
 * The readline context to operate on.
 *
 * \param prompt
 * The prompt that readline is currently using. This will point to the same
 * memory that readline is using. Do not attempt to free or modify this 
 * memory directly.
 *
 * \return
 * 0 on success or -1 on error.
 */
int rline_get_prompt (struct rline *rline, char **prompt);

/**
 * Get the current line that readline has.
 *
 * \param rline
 * The readline context to operate on.
 *
 * \param prompt
 * The current line the user has typed in.
 *
 * \return
 * 0 on success or -1 on error.
 */
int rline_get_current_line (struct rline *rline, char **current_line);

/**
 * Clear the data currently entered at the prompt. This function currently
 * is implemented in such a way that the data is cleared upon the return
 * of this function. This is so that signal handlers can easily clear the data
 * without worrying about race conditions.
 *
 * \param rline
 * The readline context to operate on.
 *
 * \return
 * 0 on success or -1 on error.
 */
int rline_clear (struct rline *rline);

/**
 * Add a history entry to the list of history items.
 *
 * This function will fail if LINE == NULL.
 *
 * \param rline
 * The readline context to operate on.
 *
 * \param line
 * The line to add to the history.
 *
 * \return
 * 0 on success or -1 on error.
 */
int rline_add_history (struct rline *rline, const char *line);

/**
 * Force the update of readline. This will write the contents of its
 * current buffer out.
 *
 * \param rline
 * The readline context to operate on.
 *
 * \return
 * 0 on success or -1 on error.
 */
int rline_rl_forced_update_display (struct rline *rline);

/**
 * This tells readline to read from it's input. The caller of this function
 * must have detected that input was ready on this descritpor. If input is not
 * ready, this function will hang.
 *
 * \param rline
 * The readline context to operate on.
 *
 * \return
 * 0 on success or -1 on error.
 */
int rline_rl_callback_read_char (struct rline *rline);

typedef void (*display_callback)(char **,int,int);

/**
 * This function will complete the current line that readline has. It will 
 * get the completions and return them to the display callback to be 
 * displayed in any way the client sees appropriate. The current readline 
 * line will be modified after this call if appropriate.
 *
 * \param rline
 * The readline context to operate on.
 *
 * \param list
 * The list of possible items to complete.
 *
 * \param display_cb
 * This function will be called with a list of possible completions, if there are any.
 *
 * \return
 * 0 on success or -1 on error.
 */
int rline_rl_complete (struct rline *rline, struct tgdb_list *list, display_callback display_cb);

/**
 * This will adjust the size of the PTY that readline is working on, then 
 * it will alert readline that it will need to be redisplay it's data.
 * 
 * \param rline
 * The readline context to operate on.
 *
 * \param rows
 * The new number of rows the readline pty should be.
 *
 * \param cols
 * The new number of cols the readline pty should be.
 *
 * \return
 * 0 on success or -1 on error.
 */
int rline_resize_terminal_and_redisplay (struct rline *rline, int rows, int cols);

/**
 * Get's the value of rl_completion_query_items.
 *
 * \param rline
 * The readline context to operate on.
 * 
 * \param query_items
 *
 * \return
 * The value of rl_completion_query_items, if rline is NULL then -1.
 */
int rline_get_rl_completion_query_items (struct rline *rline);

/**
 * This will get the key sequences that readline uses for a certain key.
 *
 * \param rline
 * The readline context to operate on.
 *
 * \param named_function
 * The readline function to get the key sequences for.
 * Examples are "beginning-of-line" and "end-of-line"
 *
 * \param keyseq_list
 * A list of key sequences (char*) that can be used to map to
 * the named_function the user requested. This variable should
 * be allocated before calling this function and must be destroyed
 * by the caller later. Each item put into this list must also 
 * be destroyed. You can use xfree to do this.
 *
 * NOTE: If rline_get_keyseq fails with a -1, it's important for
 * this function to clean up the memory already put into the keyseq_list
 *
 * \return
 * 0 on success or -1 on error
 */
int rline_get_keyseq (struct rline *rline, const char *named_function,
      std_list keyseq_list);

/*@}*/
/* }}}*/

#endif /* __RL_H__ */
