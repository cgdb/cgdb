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
    CGDB_KEY_ALT_A,
    CGDB_KEY_ALT_B,
    CGDB_KEY_ALT_C,
    CGDB_KEY_ALT_D,
    CGDB_KEY_ALT_E,
    CGDB_KEY_ALT_F,
    CGDB_KEY_ALT_G,
    CGDB_KEY_ALT_H,
    CGDB_KEY_ALT_I,
    CGDB_KEY_ALT_J,
    CGDB_KEY_ALT_K,
    CGDB_KEY_ALT_L,
    CGDB_KEY_ALT_M,
    CGDB_KEY_ALT_N,
    CGDB_KEY_ALT_O,
    CGDB_KEY_ALT_P,
    CGDB_KEY_ALT_Q,
    CGDB_KEY_ALT_R,
    CGDB_KEY_ALT_S,
    CGDB_KEY_ALT_T,
    CGDB_KEY_ALT_U,
    CGDB_KEY_ALT_V,
    CGDB_KEY_ALT_W,
    CGDB_KEY_ALT_X,
    CGDB_KEY_ALT_Y,
    CGDB_KEY_ALT_Z,
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
 *      CGDB_KEY_ALT_D
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
