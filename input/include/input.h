#ifndef __INPUT_H__
#define __INPUT_H__

enum cgdb_input_macro {
    CGDB_KEY_UP = 10000,
    CGDB_KEY_DOWN,
    CGDB_KEY_LEFT,
    CGDB_KEY_RIGHT,
    CGDB_KEY_HOME,
    CGDB_KEY_END,
    CGDB_KEY_PPAGE,
    CGDB_KEY_NPAGE,
    CGDB_KEY_DC,
    CGDB_KEY_IC,
    CGDB_KEY_F1,
    CGDB_KEY_F2,
    CGDB_KEY_F3,
    CGDB_KEY_F4,
    CGDB_KEY_F5,
    CGDB_KEY_F6,
    CGDB_KEY_F7,
    CGDB_KEY_F8,
    CGDB_KEY_F9,
    CGDB_KEY_F10,
    CGDB_KEY_F11,
    CGDB_KEY_F12,
    CGDB_KEY_ERROR
};

struct input;

/* input_init: Initializes the input unit. 
 *
 * stdinfd - The descriptor to read from when looking for the next char
 *
 * Returns NULL on error, or the new instance on success.
 */
struct input *input_init(int stdinfd);

/* input_getkey: Reads the next character 
 *
 * i       - The context to read the next char from.
 *
 * Returns: The character read, or one of the macros below
 *      CGDB_KEY_UP
 *      CGDB_KEY_DOWN
 *      CGDB_KEY_LEFT
 *      CGDB_KEY_RIGHT
 *      CGDB_KEY_HOME
 *      CGDB_KEY_END
 *      CGDB_KEY_PPAGE
 *      CGDB_KEY_NPAGE
 *      CGDB_KEY_DC
 *      CGDB_KEY_F1
 *      CGDB_KEY_F2
 *      ...
 *      CGDB_KEY_F12
 */
int input_getkey(struct input *i);

/* input_get_last_seq: Gets the char sequence that made up the last macro.
 *
 * i       - The context to determine the last sequence read.
 *
 * Returns - NULL on error, the sequence on success.
 */
char *input_get_last_seq(struct input *i);

char *input_get_last_seq_name(struct input *i);

#endif /* __INPUT_H__ */
