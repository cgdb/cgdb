#ifndef __RLCTX_H__
#define __RLCTX_H__

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */

struct rlctx;   /* readline context */

typedef int(*rlctx_recv_line)(const char *);

/* rlctx_init: Initializes a new readline context. 
 *      
 *      Returns: The new readline context.
 */
struct rlctx *rlctx_init(void);

/* rlctx_close: close a readline context. 
 *
 *      rl  - The readline context intended.
 *      
 *      Returns: 0 on success, -1 on error.
 * */
int rlctx_close(struct rlctx *rl);

/* rlctx_get_fd: Returns the file descriptor that will be selectable when 
 *               readline has output available.
 *      
 *      rl  - The readline context intended.
 *      
 *      Returns: fd to select on, -1 on error.
 */
int rlctx_get_fd(struct rlctx *rl);

/* rlctx_send_char: Sends a char to for readline to interpret.
 *      
 *      rl  - The readline context intended.
 *      c   - The char to send
 *      
 *      Returns: 0 on success, -1 on error.
 */
int rlctx_send_char(struct rlctx *rl, char c);

/* rlctx_recv: Gets data back from readline.
 *      
 *      rl      - The readline context intended.
 *      c       - The buf to recieve, It will be null-terminated on way out
 *      size    - The amount of data to attempt to read
 *      
 *      Returns: 0 on sucess, -1, on error, -2 on EOF.
 */
int rlctx_recv(struct rlctx *rl, char *c, ssize_t size);

/* rlctx_regester_callback: func will be called with every line.
 *      
 *      rl      - The readline context intended.
 *      func    - The func to call every time a line is recieved.
 *      
 *      Returns: 0 on success, -1 on error.
 */
int rlctx_register_callback(struct rlctx *rl, rlctx_recv_line func );

#endif /* __RLCTX_H__ */
