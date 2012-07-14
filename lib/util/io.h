#ifndef __IO_H__
#define __IO_H__

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */

#if HAVE_STDIO_H
#include <stdio.h>
#endif /* HAVE_STDIO_H */

#define MAX_LINE 4096

/* The Next three function handle reading and writing of bytes */

/* io_read_byte: reads the byte c from source.
 *  Return: -1 on error, 0 on success
 */
int io_read_byte(char *c, int source);

/* io_write_byte: Writes the byte 'c' to dest.
 *  Return: -1 on error, 0 on success
 */
int io_write_byte(int dest, char c);

/* io_rw_byte: Reads a byte from source and writes it to dest.
 *  Return: -1 on error, 0 on success
 */
int io_rw_byte(int source, int dest);

/* io_read: Attempts to read count bytes from fd and stores the 
 *          data into buf.
 *          Returns: The amount read on success.
 *                   0 on EOF and
 *                   -1 on error 
 */
ssize_t io_read(int fd, void *buf, size_t count);

/* io_writen: This will write n bytes of vptr to fd. 
 *
 *     It recieves:
 *         fd     - The file descriptor to write to.
 *         n      - The number of bytes to write.
 *         vptr   - A pointer to the data to be written.
 *          
 * RETURN:  The number of bytes written (should always be n bytes)
 *          -1 on error. 
 */
ssize_t io_writen(int fd, const void *vptr, size_t n);

/* io_debug_init: Puts tgdb in a mode where it writes a debug log of everything
 *    that is read from gdb. That is basically the entire session. This info
 *    is usefull in determining what is going on under tgdb since the gui 
 *    is good at hiding that info from the user.
 *    
 *    filename is the file that the debug info will go to. If it is null, the 
 *    debug data will be writting to $HOME/.tgdb/tgdb_debug.txt
 *
 *    Returns: 0 on success, or -1 if can not open file.
 */
int io_debug_init(const char *filename);

/* io_debug_write: Writes null terminated data cstring to debug file.  */
void io_debug_write(const char *write);
void io_debug_write_fmt(const char *fmt, ...);

/* io_display_char: Displays the char c in fd. 
 *    This is usefull when c is a '\r' or '\n' because it will be displayed
 *    that way in the file.
 */
void io_display_char(FILE * fd, char c);

/* io_data_ready:
 * --------------
 *
 * fd 	the descriptor to check for.
 * ms 	the amount of time in milliseconds to wait.
 * 		pass 0, if you do not want to wait.
 * 		pass -1, if you want the read to block.
 *
 * This function checks to see if data is ready on FD.
 * If this function returns 1 then at least one byte can be read.
 *
 * Return: 1 if data is ready, 0 if it's not, or -1 on error
 */
int io_data_ready(int fd, int ms);

/**
 * Read in 1 charachter.
 *
 * If no I/O is ready, then the function will wait ms milliseconds waiting 
 * for input. After that amount of time, this function will return, with
 * or without input.
 *
 * This function is non blocking if ms is 0.
 *
 * \param fd
 * The descriptor to read in from.
 *
 * \param ms
 * The The amount of time in milliseconds to wait for input.
 * Pass 0, if you do not want to wait.
 * Pass -1, if you want to block indefinately.
 *
 * \param key
 * The character read if the return value is successful
 *
 * @return
 * -1 on error, 0 if no data is ready, 1 on success
 */
int io_getchar(int fd, unsigned int ms, int *key);

#endif /* __IO_H__ */
