#ifndef __ERROR_H__
#define __ERROR_H__

#ifdef __cplusplus
extern "C" {
#endif

extern void cleanup(void);

/* Returns all of the error messages logged since last call as a 
 * pointer to a static buffer.
 * Returns NULL if nothing was logged.
 */
char *err_get(void);

/* Nonfatal error related to a system call.
 * Print a message and return. */
void err_ret(const char *fmt, ...);

/* Fatal error related to a system call.
 * Print a message and terminate. */
void err_sys(const char *fmt, ...);

/* Fatal error related to a system call.
 * Print a message, dump core, and terminate. */
void err_dump(const char *fmt, ...);

/* Nonfatal error unrelated to a system call.
 * Print a message and return. */
void err_msg(const char *fmt, ...);

/* Fatal error unrelated to a system call.
 * Print a message and terminate. */
void err_quit(const char *fmt, ...);

/* if value is 1, print to stderr, if 0, don't print */
void err_verbose ( int value );

#ifdef __cplusplus
}
#endif

#endif /* __ERROR_H__ */
