#ifndef __RLCTX_MAIN_H__
#define __RLCTX_MAIN_H__
    
/* rlctx_main: The entry point to a new readline context.
 * -----------
 *
 *  Pre -   stdin is a tty. It is where this function will get input from.
 *          stdout and stderr is where this function will write too.
 *
 *  fd  -   This is the descriptor that this function will read from to get
 *          commands from rlctx.
 *
 *  Returns -1 on error, 0 on success.
 */
int rlctx_main(int fd);

#endif
