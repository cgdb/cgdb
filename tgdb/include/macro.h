#ifndef __MACRO_H__
#define __MACRO_H__

/* macro_start: Starts a new macro. This starts recording a new macro.
 *              To save a macro, macro_save must be called.
 *
 * Returns: -1 on error, 0 on success
 */
int macro_start(void);

/* macro_save: Saves a macro to filename
 * Returns: -1 on error, 0 on success.
 */
int macro_save(const char *filename);
int macro_load(const char *filename);

int macro_write_char(char c);
int macro_write_str(const char *str);
int macro_turn_macro_on(void);
int macro_turn_macro_off(void);

#endif /* __MACRO_H__ */
