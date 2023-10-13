#ifndef __RL_H__
#define __RL_H__

/*!
 * \file
 * rline.h
 *
 * \brief
 * This interface is an abstraction layer to the key bindings in the GNU readline library.
 */

#include <string>
#include <list>

/*@{*/

/**
 * This initializes an rline library instance.
 *
 * The client must call this function before any other function in the
 * rline library.
 *
 * @return
 * 0 on success or -1 on error
 */
int rline_initialize(void);

/*@}*/

/*@{*/

/**
 * This will get the key sequences that readline uses for a certain key.
 *
 * \param named_function
 * The readline function to get the key sequences for.
 * Examples are "beginning-of-line" and "end-of-line"
 *
 * \param keyseq
 * A list of key sequences that can be used to map to
 * the named_function the user requested.
 *
 * \return
 * 0 on success or -1 on error
 */
int rline_get_keyseq(const char *named_function, std::list<std::string> &keyseq);

/*@}*/

#endif /* __RL_H__ */
