#ifndef __RLCTX_H__
#define __RLCTX_H__

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */

struct rlctx;   /* readline context */

typedef int(*rlctx_recv_line)(void*, const char *);

/* rlctx_init: Initializes a new readline context. 
 *
 *      home_dir - This is the path to the home directory.
 *                 If not null this is where readline will store its history.
 *                 When not null, should contain trailing slash for directory 
 *                 path.
 *                 If null, readline will not store history.
 *      
 *      unique_id -This allows readline to store multiple history's. Use
 *                 the same unique id to get history between sessions.
 *                 If null, readline will not store history.
 *
 *      p        - The callback functions pass this as the first parameter
 *
 *      Returns: The new readline context.
 */
struct rlctx *rlctx_init(const char *home_dir, const char *unique_id, void* p);

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

/* rlctx_regester_command_callback: func will be called with every line.
 *      
 *      rl      - The readline context intended.
 *      func    - The func to call every time a line is recieved.
 *      
 *      Returns: 0 on success, -1 on error.
 */
int rlctx_register_command_callback(struct rlctx *rl, rlctx_recv_line func );

/* rlctx_regester_completion_callback: func will be called with every line.
 *      
 *      rl      - The readline context intended.
 *      func    - The func to call every time a completion command is recieved
 *      
 *      Returns: 0 on success, -1 on error.
 */
int rlctx_register_completion_callback(struct rlctx *rl, rlctx_recv_line func );

/* rlctx_change_prompt: This will tell readline the new prompt.
 *
 *      rl      - The readline context intended.
 *      prompt  - The new prompt readline will use.
 *
 *      Returns: 0 on success, -1 on error.
 */
int rlctx_change_prompt(struct rlctx *rl, const char *prompt);

/* rlctx_redisply: Forces readline to display what it has recieved so far.
 *
 *      rl      - The readline context intended.
 *
 *      Returns: 0 on success, -1 on error.
 */
int rlctx_redisplay(struct rlctx *rl);

/* rlctx_read_history: Tells this context to read the history in.
 *
 *      rl      - The readline context intended.
 *
 *      Returns: 0 on success, -1 on error.
 */
int rlctx_read_history(struct rlctx *rl);

/* rlctx_write_history: Tells this context to write the history file.
 *
 *      rl      - The readline context intended.
 *
 *      Returns: 0 on success, -1 on error.
 */
int rlctx_write_history(struct rlctx *rl);

#endif /* __RLCTX_H__ */
