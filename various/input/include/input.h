#ifndef __INPUT_H__
#define __INPUT_H__

enum cgdb_input_macro {
    CGDB_KEY_ESC = 10000,
    CGDB_KEY_UP,
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

	/* Ctrl Keys */
	CGDB_KEY_CTRL_A,
	CGDB_KEY_CTRL_B,
	CGDB_KEY_CTRL_C,
	CGDB_KEY_CTRL_D,
	CGDB_KEY_CTRL_E,
	CGDB_KEY_CTRL_F,
	CGDB_KEY_CTRL_G,
	CGDB_KEY_CTRL_H,
	CGDB_KEY_CTRL_I,
	CGDB_KEY_CTRL_J,
	CGDB_KEY_CTRL_K,
	CGDB_KEY_CTRL_L,
	CGDB_KEY_CTRL_M,
	CGDB_KEY_CTRL_N,
	CGDB_KEY_CTRL_O,
	CGDB_KEY_CTRL_P,
	CGDB_KEY_CTRL_Q,
	CGDB_KEY_CTRL_R,
	CGDB_KEY_CTRL_S,
	CGDB_KEY_CTRL_T,
	CGDB_KEY_CTRL_U,
	CGDB_KEY_CTRL_V,
	CGDB_KEY_CTRL_W,
	CGDB_KEY_CTRL_X,
	CGDB_KEY_CTRL_Y,
	CGDB_KEY_CTRL_Z,

    /* Alt Letters */
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

    /* Alt Shifted Letters */
    CGDB_KEY_ALT_SHIFT_A,
    CGDB_KEY_ALT_SHIFT_B,
    CGDB_KEY_ALT_SHIFT_C,
    CGDB_KEY_ALT_SHIFT_D,
    CGDB_KEY_ALT_SHIFT_E,
    CGDB_KEY_ALT_SHIFT_F,
    CGDB_KEY_ALT_SHIFT_G,
    CGDB_KEY_ALT_SHIFT_H,
    CGDB_KEY_ALT_SHIFT_I,
    CGDB_KEY_ALT_SHIFT_J,
    CGDB_KEY_ALT_SHIFT_K,
    CGDB_KEY_ALT_SHIFT_L,
    CGDB_KEY_ALT_SHIFT_M,
    CGDB_KEY_ALT_SHIFT_N,
    CGDB_KEY_ALT_SHIFT_O,
    CGDB_KEY_ALT_SHIFT_P,
    CGDB_KEY_ALT_SHIFT_Q,
    CGDB_KEY_ALT_SHIFT_R,
    CGDB_KEY_ALT_SHIFT_S,
    CGDB_KEY_ALT_SHIFT_T,
    CGDB_KEY_ALT_SHIFT_U,
    CGDB_KEY_ALT_SHIFT_V,
    CGDB_KEY_ALT_SHIFT_W,
    CGDB_KEY_ALT_SHIFT_X,
    CGDB_KEY_ALT_SHIFT_Y,
    CGDB_KEY_ALT_SHIFT_Z,

    /* Alt Numbers */
    CGDB_KEY_ALT_1,
    CGDB_KEY_ALT_2,
    CGDB_KEY_ALT_3,
    CGDB_KEY_ALT_4,
    CGDB_KEY_ALT_5,
    CGDB_KEY_ALT_6,
    CGDB_KEY_ALT_7,
    CGDB_KEY_ALT_8,
    CGDB_KEY_ALT_9,
    CGDB_KEY_ALT_0,

    /* Alt Shifted Numbers */
    CGDB_KEY_ALT_SHIFT_1,
    CGDB_KEY_ALT_SHIFT_2,
    CGDB_KEY_ALT_SHIFT_3,
    CGDB_KEY_ALT_SHIFT_4,
    CGDB_KEY_ALT_SHIFT_5,
    CGDB_KEY_ALT_SHIFT_6,
    CGDB_KEY_ALT_SHIFT_7,
    CGDB_KEY_ALT_SHIFT_8,
    CGDB_KEY_ALT_SHIFT_9,
    CGDB_KEY_ALT_SHIFT_0,

    /* Alt Special */
    CGDB_KEY_ALT_MINUS,
    CGDB_KEY_ALT_EQUAL,
    CGDB_KEY_ALT_LEFT_BRACKET,
    CGDB_KEY_ALT_RIGHT_BRACKET,
    CGDB_KEY_ALT_BACKSLASH,
    CGDB_KEY_ALT_SEMICOLON,
    CGDB_KEY_ALT_APOSTROPHE,
    CGDB_KEY_ALT_COMMA,
    CGDB_KEY_ALT_PERIOD,
    CGDB_KEY_ALT_DIVIDE,
    CGDB_KEY_ALT_ACCENT_MARK,

    /* Alt Shift Special */
    CGDB_KEY_ALT_SHIFT_UNDERSCORE,
    CGDB_KEY_ALT_SHIFT_PLUS,
    CGDB_KEY_ALT_SHIFT_LEFT_CURLY_BRACKET,
    CGDB_KEY_ALT_SHIFT_RIGHT_CURLY_BRACKET,
    CGDB_KEY_ALT_SHIFT_PIPE,
    CGDB_KEY_ALT_SHIFT_COLON,
    CGDB_KEY_ALT_SHIFT_QUOTE,
    CGDB_KEY_ALT_SHIFT_LESS_THAN,
    CGDB_KEY_ALT_SHIFT_GREATER_THAN,
    CGDB_KEY_ALT_SHIFT_QUESTION_MARK,
    CGDB_KEY_ALT_SHIFT_TILDA,
     
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

/* input_set_escape_sequence_timeout_value
 * ---------------------------------------
 *
 *  i       The context to determine the last sequence read.
 *  msec 	Number of milliseconds to wait before ESC is returned.
 */
void input_set_escape_sequence_timeout_value ( 
			struct input * i, unsigned int msec );

#endif /* __INPUT_H__ */
