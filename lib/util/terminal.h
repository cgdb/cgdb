#ifndef __TERMINAL_H__
#define __TERMINAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#if HAVE_CONFIG_H
#include "config.h"
#endif                          /* HAVE_CONFIG_H */

#if HAVE_TERMIOS_H
#include <termios.h>
#endif                          /* HAVE_TERMIOS_H */

#if HAVE_UNISTD_H
#include <unistd.h>
#endif                          /* HAVE_UNISTD_H */

/** 
 * Sets terminal to cbreak mode. Also known as noncanonical mode.
 *    1. Signal handling is still turned on, so the user can still type those.
 *    2. echo is off
 *    3. Read in one char at a time.
 *
 * \param fd
 * The file descriptor of the terminal
 *
 * \param orig
 * Output parameter, pass in the address of a termios structure and it will
 * be populated with the current (aka original) state of the terminal.  Use
 * this to reset the state with tty_set_attributes later.
 * 
 * \return
 * 0 on success, -1 on error
 */
    int tty_cbreak(int fd, struct termios *orig);

/** 
 * set the pseudo - terminal to not map NL to CR NL.
 *
 * \param fd 
 * The descriptor to the pseudo-terminal.
 *
 * \return
 * 0 on success, -1 on error
 */
    int tty_output_nl(int fd);

/**
 * Turn off control flow on the terminal descriptor.
 *
 * \param fd
 * The terminal descriptor to turn the control flow off on.
 *
 * \return
 * 0 on success, -1 on error
 */
    int tty_off_xon_xoff(int fd);

/**
 * Either turn echo on or off for the PTY.
 *
 * \param fd
 * The terminal descriptor to turn the echo on or off.
 *
 * \param echo_on
 * If 1, the echo will be turned on, if 0 echo will be turned off.
 *
 * \return
 * 0 on success, -1 on error
 */
    int tty_set_echo(int fd, int echo_on);

/**
 * This will get the terminal attributes and store them into the buffer buf.
 * 
 * \param fd
 * The terminal to query
 *
 * \param buf
 * Make sure this data item is allocated for on the way in.
 * This will return with the terminal attributes of the terminal.
 *
 * \return
 * 0 on success, -1 on error
 */
    int tty_get_attributes(int fd, struct termios *buf);

/** 
 * Sets the terminal attributes back to their previous state.
 *
 * \param fd
 * The file descrioptor of the terminal to reset.
 *
 * \param buf
 * The attributes to set the terminal to.
 * 
 * \return
 * 0 on success, -1 on error
 */
    int tty_set_attributes(int fd, struct termios *buf);

#ifdef __cplusplus
}
#endif
#endif
