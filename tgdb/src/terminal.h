#ifndef __TERMINAL_H__
#define __TERMINAL_H__

#ifdef __cplusplus
extern "C" {
#endif 

/* tty_cbreak: Sets terminal to cbreak mode. Also known as noncanonical mode.
 *    1. Signal handling is still turned on, so the user can still type those.
 *    2. echo is off
 *    3. Read in one char at a time.
 *
 * fd    - The file descriptor of the terminal
 * 
 * Returns: 0 on sucess, -1 on error
 */
int tty_cbreak(int fd);

/* tty_output_nl: set the pseudo - terminal to not map NL to CR NL.
 *      fd - The descriptor to the pseudo-terminal.
 *  RETURNS: -1 on error, 0 on success.
 */
int tty_output_nl(int fd);

/* tty_reset: Sets the terminal attributes back to their previous state.
 * PRE: tty_cbreak must have already been called.
 * 
 * fd    - The file descrioptor of the terminal to reset.
 * 
 * Returns: 0 on success, -1 on error
 */
int tty_reset(int fd);

#ifdef __cplusplus
}
#endif

#endif
