#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_CURSES_H
#include <curses.h>
#endif /* HAVE_CURSES_H */

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#if HAVE_STRINGS_H
#include <strings.h>
#endif /* HAVE_STRINGS_H */

#if HAVE_STDLIB_H
#include <stdlib.h> /* for getenv */
#endif /* HAVE_STDLIB_H */

#if HAVE_STDIO_H
#include <stdio.h> /* for stderr */
#endif /* HAVE_STDIO_H */

/* term.h prototypes */
extern int tgetent();
extern int tgetflag();
extern int tgetnum();
extern char *tgetstr();
extern int tputs();
extern char *tgoto();

#include "kui_term.h"

#define MAXLINE 4096
#define MAX_SEQ_LIST_SIZE 8

/** 
 * This contains all of the ESC sequences this unit cares about. 
 * It contains the correct information to get esc sequences out of both
 * termcap and terminfo.
 */
struct tlist {
  /* The key */
  enum cgdb_key key;
  /* the termcap capability name  */
  char *tname;
  /* the termcap key sequence */
  char *tname_seq;
  /* the terminfo capability name  */
  char *tiname;
  /* the terminfo key sequence */
  char *tiname_seq;
} seqlist[] = {
  { CGDB_KEY_END,   "@7", NULL, "kend",   NULL },
  { CGDB_KEY_HOME,  "kh", NULL, "khome",  NULL },
  { CGDB_KEY_HOME,  "kH", NULL, "kll",    NULL },
  { CGDB_KEY_DC,    "dc", NULL, "dch1",   NULL },
  { CGDB_KEY_DC,    "kD", NULL, "kdch1",  NULL },
  { CGDB_KEY_IC,    "ic", NULL, "ich1",   NULL },
  { CGDB_KEY_IC,    "kI", NULL, "kich1",  NULL },
  { CGDB_KEY_NPAGE, "kN", NULL, "knp",    NULL },
  { CGDB_KEY_PPAGE, "kP", NULL, "kpp",    NULL },

  /* For arrow keys */
  { CGDB_KEY_DOWN,  "kd", NULL, "kcud1",  NULL },
  { CGDB_KEY_LEFT,  "kl", NULL, "kcub1",  NULL },
  { CGDB_KEY_RIGHT, "kr", NULL, "kcuf1",  NULL },
  { CGDB_KEY_UP,    "ku", NULL, "kcuu1",  NULL },
  { CGDB_KEY_LEFT,  "le", NULL, "cub1",   NULL },
  { CGDB_KEY_RIGHT, "nd", NULL, "cuf1",   NULL },
  { CGDB_KEY_UP,    "up", NULL, "cuu1",   NULL },

  /* Function keys */
  { CGDB_KEY_F1,  "k1", NULL, "kf1",    NULL },
  { CGDB_KEY_F2,  "k2", NULL, "kf2",    NULL },
  { CGDB_KEY_F3,  "k3", NULL, "kf3",    NULL },
  { CGDB_KEY_F4,  "k4", NULL, "kf4",    NULL },
  { CGDB_KEY_F5,  "k5", NULL, "kf5",    NULL },
  { CGDB_KEY_F6,  "k6", NULL, "kf6",    NULL },
  { CGDB_KEY_F7,  "k7", NULL, "kf7",    NULL },
  { CGDB_KEY_F8,  "k8", NULL, "kf8",    NULL },
  { CGDB_KEY_F9,  "k9", NULL, "kf9",    NULL },
  { CGDB_KEY_F10,  "k;", NULL, "kf10",   NULL },
  { CGDB_KEY_F11,  "F1", NULL, "kf11",   NULL },
  { CGDB_KEY_F12,  "F2", NULL, "kf12",   NULL },
  { CGDB_KEY_ERROR,NULL, NULL, NULL,     NULL }
};

/* This represents all of the hard coded key data.  */
struct keydata
{
  enum cgdb_key key;
  char *key_seq;
} hard_coded_bindings [] = 
{
  { CGDB_KEY_ESC, "\033" },

  /* Arrow bindings */
  /* These should be first, because readline hard codes them and when I
   * walk this list to get the key bindings, I want these for the arrows */
  { CGDB_KEY_UP, "\033[A" },
  { CGDB_KEY_DOWN, "\033[B" },
  { CGDB_KEY_RIGHT, "\033[C" },
  { CGDB_KEY_LEFT, "\033[D" },
  { CGDB_KEY_HOME, "\033[H" },
  { CGDB_KEY_END, "\033[F" },

  /* Arrow bindings, MSDOS */
  { CGDB_KEY_UP, "\033[0A" },
  { CGDB_KEY_LEFT, "\033[0B" },
  { CGDB_KEY_RIGHT, "\033[0C" },
  { CGDB_KEY_DOWN, "\033[0D" },

  { CGDB_KEY_UP, "\033OA" },
  { CGDB_KEY_DOWN, "\033OB" },
  { CGDB_KEY_RIGHT, "\033OC" },
  { CGDB_KEY_LEFT, "\033OD" },
  { CGDB_KEY_HOME, "\033OH" },
  { CGDB_KEY_END, "\033OF" },

  /* Ctrl bindings */
  { CGDB_KEY_CTRL_A, "\001" },
  { CGDB_KEY_CTRL_B, "\002" },
  { CGDB_KEY_CTRL_C, "\003" },
  { CGDB_KEY_CTRL_D, "\004" },
  { CGDB_KEY_CTRL_E, "\005" },
  { CGDB_KEY_CTRL_F, "\006" },
  { CGDB_KEY_CTRL_G, "\007" },
  { CGDB_KEY_CTRL_H, "\010" },
  { CGDB_KEY_CTRL_I, "\011" },
  { CGDB_KEY_CTRL_J, "\012" },
  { CGDB_KEY_CTRL_K, "\013" },
  { CGDB_KEY_CTRL_L, "\014" },
  { CGDB_KEY_CTRL_M, "\015" },
  { CGDB_KEY_CTRL_N, "\016" },
  { CGDB_KEY_CTRL_O, "\017" },
  { CGDB_KEY_CTRL_P, "\020" },
  { CGDB_KEY_CTRL_Q, "\021" },
  { CGDB_KEY_CTRL_R, "\022" },
  { CGDB_KEY_CTRL_S, "\023" },
  { CGDB_KEY_CTRL_T, "\024" },
  { CGDB_KEY_CTRL_U, "\025" },
  { CGDB_KEY_CTRL_V, "\026" },
  { CGDB_KEY_CTRL_W, "\027" },
  { CGDB_KEY_CTRL_X, "\030" },
  { CGDB_KEY_CTRL_Y, "\031" },
  { CGDB_KEY_CTRL_Z, "\032" },

  /* Alt bindings */
  { CGDB_KEY_ALT_A, "\033a" },
  { CGDB_KEY_ALT_B, "\033b" },
  { CGDB_KEY_ALT_C, "\033c" },
  { CGDB_KEY_ALT_D, "\033d" },
  { CGDB_KEY_ALT_E, "\033e" },
  { CGDB_KEY_ALT_F, "\033f" },
  { CGDB_KEY_ALT_G, "\033g" },
  { CGDB_KEY_ALT_H, "\033h" },
  { CGDB_KEY_ALT_I, "\033i" },
  { CGDB_KEY_ALT_J, "\033j" },
  { CGDB_KEY_ALT_K, "\033k" },
  { CGDB_KEY_ALT_L, "\033l" },
  { CGDB_KEY_ALT_M, "\033m" },
  { CGDB_KEY_ALT_N, "\033n" },
  { CGDB_KEY_ALT_O, "\033o" },
  { CGDB_KEY_ALT_P, "\033p" },
  { CGDB_KEY_ALT_Q, "\033q" },
  { CGDB_KEY_ALT_R, "\033r" },
  { CGDB_KEY_ALT_S, "\033s" },
  { CGDB_KEY_ALT_T, "\033t" },
  { CGDB_KEY_ALT_U, "\033u" },
  { CGDB_KEY_ALT_V, "\033v" },
  { CGDB_KEY_ALT_W, "\033w" },
  { CGDB_KEY_ALT_X, "\033x" },
  { CGDB_KEY_ALT_Y, "\033y" },
  { CGDB_KEY_ALT_Z, "\033z" },

  /* Alt Shift bindings */
  { CGDB_KEY_ALT_SHIFT_A, "\033A" },
  { CGDB_KEY_ALT_SHIFT_B, "\033B" },
  { CGDB_KEY_ALT_SHIFT_C, "\033C" },
  { CGDB_KEY_ALT_SHIFT_D, "\033D" },
  { CGDB_KEY_ALT_SHIFT_E, "\033E" },
  { CGDB_KEY_ALT_SHIFT_F, "\033F" },
  { CGDB_KEY_ALT_SHIFT_G, "\033G" },
  { CGDB_KEY_ALT_SHIFT_H, "\033H" },
  { CGDB_KEY_ALT_SHIFT_I, "\033I" },
  { CGDB_KEY_ALT_SHIFT_J, "\033J" },
  { CGDB_KEY_ALT_SHIFT_K, "\033K" },
  { CGDB_KEY_ALT_SHIFT_L, "\033L" },
  { CGDB_KEY_ALT_SHIFT_M, "\033M" },
  { CGDB_KEY_ALT_SHIFT_N, "\033N" },
  { CGDB_KEY_ALT_SHIFT_O, "\033O" },
  { CGDB_KEY_ALT_SHIFT_P, "\033P" },
  { CGDB_KEY_ALT_SHIFT_Q, "\033Q" },
  { CGDB_KEY_ALT_SHIFT_R, "\033R" },
  { CGDB_KEY_ALT_SHIFT_S, "\033S" },
  { CGDB_KEY_ALT_SHIFT_T, "\033T" },
  { CGDB_KEY_ALT_SHIFT_U, "\033U" },
  { CGDB_KEY_ALT_SHIFT_V, "\033V" },
  { CGDB_KEY_ALT_SHIFT_W, "\033W" },
  { CGDB_KEY_ALT_SHIFT_X, "\033X" },
  { CGDB_KEY_ALT_SHIFT_Y, "\033Y" },
  { CGDB_KEY_ALT_SHIFT_Z, "\033Z" },

  /* Alt Numbers */
  { CGDB_KEY_ALT_1, "\0331" },
  { CGDB_KEY_ALT_2, "\0332" },
  { CGDB_KEY_ALT_3, "\0333" },
  { CGDB_KEY_ALT_4, "\0334" },
  { CGDB_KEY_ALT_5, "\0335" },
  { CGDB_KEY_ALT_6, "\0336" },
  { CGDB_KEY_ALT_7, "\0337" },
  { CGDB_KEY_ALT_8, "\0338" },
  { CGDB_KEY_ALT_9, "\0339" },
  { CGDB_KEY_ALT_0, "\0330" },

  /* Alt Shifted Numbers */
  { CGDB_KEY_ALT_SHIFT_1, "\033!" },
  { CGDB_KEY_ALT_SHIFT_2, "\033@" },
  { CGDB_KEY_ALT_SHIFT_3, "\033#" },
  { CGDB_KEY_ALT_SHIFT_4, "\033$" },
  { CGDB_KEY_ALT_SHIFT_5, "\033%" },
  { CGDB_KEY_ALT_SHIFT_6, "\033^" },
  { CGDB_KEY_ALT_SHIFT_7, "\033&" },
  { CGDB_KEY_ALT_SHIFT_8, "\033*" },
  { CGDB_KEY_ALT_SHIFT_9, "\033(" },
  { CGDB_KEY_ALT_SHIFT_0, "\033)" },

  /* Alt Special */
  { CGDB_KEY_ALT_MINUS, "\033-" },
  { CGDB_KEY_ALT_EQUAL, "\033=" },
  { CGDB_KEY_ALT_LEFT_BRACKET, "\033[" },
  { CGDB_KEY_ALT_RIGHT_BRACKET, "\033]" },
  { CGDB_KEY_ALT_BACKSLASH, "\033\\" },
  { CGDB_KEY_ALT_SEMICOLON, "\033;" },
  { CGDB_KEY_ALT_APOSTROPHE, "\033'" },
  { CGDB_KEY_ALT_COMMA, "\033," },
  { CGDB_KEY_ALT_PERIOD, "\033." },
  { CGDB_KEY_ALT_DIVIDE, "\033/" },
  { CGDB_KEY_ALT_ACCENT_MARK, "\033`" },

  /* Alt Shifte Special */
  { CGDB_KEY_ALT_SHIFT_UNDERSCORE, "\033_" },
  { CGDB_KEY_ALT_SHIFT_PLUS, "\033+" },
  { CGDB_KEY_ALT_SHIFT_LEFT_CURLY_BRACKET, "\033{" },
  { CGDB_KEY_ALT_SHIFT_RIGHT_CURLY_BRACKET, "\033}" },
  { CGDB_KEY_ALT_SHIFT_PIPE, "\033|" },
  { CGDB_KEY_ALT_SHIFT_COLON, "\033:" },
  { CGDB_KEY_ALT_SHIFT_QUOTE, "\033\"" },
  { CGDB_KEY_ALT_SHIFT_LESS_THAN, "\033<" },
  { CGDB_KEY_ALT_SHIFT_GREATER_THAN, "\033>" },
  { CGDB_KEY_ALT_SHIFT_QUESTION_MARK, "\033?" },
  { CGDB_KEY_ALT_SHIFT_TILDA, "\033~" },

  { CGDB_KEY_ERROR, NULL }
};
	
/**
 * This is the main data structure in determining the string representation
 * of a terminal key sequence, or an unprintable char.
 *
 * With the proper functions built on top of this data structure, the user
 * should be able to get the string representation from the CGDB_KEY and
 * vice versa.
 */
struct cgdb_key_data {
	/* This is the "key" that can be converted to text.  */
	enum cgdb_key key;
	/* This is the text value associated with the key  */
	const char *keycode;
	/* This is here purely for debugging purposes, it is used to print out the key
	 * in human readable form.  */
	const char *key_as_string;
} cgdb_keys[CGDB_KEY_ERROR-CGDB_KEY_ESC+1] = {
	{
		CGDB_KEY_ESC,
		"<Esc>",
		"CGDB_KEY_ESC"
	},
	{
		CGDB_KEY_UP,
		"<Up>",
		"CGDB_KEY_UP"
	},
	{
		CGDB_KEY_DOWN,
		"<Down>",
		"CGDB_KEY_DOWN"
	},
	{
		CGDB_KEY_LEFT,
		"<Left>",
		"CGDB_KEY_LEFT"
	},
	{
		CGDB_KEY_RIGHT,
		"<Right>",
		"CGDB_KEY_RIGHT"
	},
	{
		CGDB_KEY_HOME,
		"<Home>",
		"CGDB_KEY_HOME"
	},
	{
		CGDB_KEY_END,
		"<End>",
		"CGDB_KEY_END"
	},
	{
		CGDB_KEY_PPAGE,
		"<PageUp>",
		"CGDB_KEY_PPAGE"
	},
	{
		CGDB_KEY_NPAGE,
		"<PageDown>",
		"CGDB_KEY_NPAGE"
	},
	{
		CGDB_KEY_DC,
		"<Del>",
		"CGDB_KEY_DC"
	},
	{
		CGDB_KEY_IC,
		"<Insert>",
		"CGDB_KEY_IC"
	},
	{
		CGDB_KEY_F1,
		"<F1>",
		"CGDB_KEY_F1"
	},
	{
		CGDB_KEY_F2,
		"<F2>",
		"CGDB_KEY_F2"
	},
	{
		CGDB_KEY_F3,
		"<F3>",
		"CGDB_KEY_F3"
	},
	{
		CGDB_KEY_F4,
		"<F4>",
		"CGDB_KEY_F4"
	},
	{
		CGDB_KEY_F5,
		"<F5>",
		"CGDB_KEY_F5"
	},
	{
		CGDB_KEY_F6,
		"<F6>",
		"CGDB_KEY_F6"
	},
	{
		CGDB_KEY_F7,
		"<F7>",
		"CGDB_KEY_F7"
	},
	{
		CGDB_KEY_F8,
		"<F8>",
		"CGDB_KEY_F8"
	},
	{
		CGDB_KEY_F9,
		"<F9>",
		"CGDB_KEY_F9"
	},
	{
		CGDB_KEY_F10,
		"<F10>",
		"CGDB_KEY_F10"
	},			
	{
		CGDB_KEY_F11,
		"<F11>",
		"CGDB_KEY_F11"
	},
	{
		CGDB_KEY_F12,
		"<F12>",
		"CGDB_KEY_F12"
	},
	{
		CGDB_KEY_CTRL_A,
		"<C-a>",
		"CGDB_KEY_CTRL_A"
	},
	{
		CGDB_KEY_CTRL_B,
		"<C-b>",
		"CGDB_KEY_CTRL_B"
	},
	{
		CGDB_KEY_CTRL_C,
		"<C-c>",
		"CGDB_KEY_CTRL_C"
	},
	{
		CGDB_KEY_CTRL_D,
		"<C-d>",
		"CGDB_KEY_CTRL_D"
	},
	{
		CGDB_KEY_CTRL_E,
		"<C-e>",
		"CGDB_KEY_CTRL_E"
	},
	{
		CGDB_KEY_CTRL_F,
		"<C-f>",
		"CGDB_KEY_CTRL_F"
	},
	{
		CGDB_KEY_CTRL_G,
		"<C-g>",
		"CGDB_KEY_CTRL_G"
	},
	{
		CGDB_KEY_CTRL_H,
		"<C-h>",
		"CGDB_KEY_CTRL_H"
	},
	{
		CGDB_KEY_CTRL_I,
		"<C-i>",
		"CGDB_KEY_CTRL_I"
	},
	{
		CGDB_KEY_CTRL_J,
		"<C-j>",
		"CGDB_KEY_CTRL_J"
	},
	{
		CGDB_KEY_CTRL_K,
		"<C-k>",
		"CGDB_KEY_CTRL_K"
	},
	{
		CGDB_KEY_CTRL_L,
		"<C-l>",
		"CGDB_KEY_CTRL_L"
	},
	{
		CGDB_KEY_CTRL_M,
		"<C-m>",
		"CGDB_KEY_CTRL_M"
	},
	{
		CGDB_KEY_CTRL_N,
		"<C-n>",
		"CGDB_KEY_CTRL_N"
	},
	{
		CGDB_KEY_CTRL_O,
		"<C-o>",
		"CGDB_KEY_CTRL_O"
	},
	{
		CGDB_KEY_CTRL_P,
		"<C-p>",
		"CGDB_KEY_CTRL_P"
	},
	{
		CGDB_KEY_CTRL_Q,
		"<C-q>",
		"CGDB_KEY_CTRL_Q"
	},
	{
		CGDB_KEY_CTRL_R,
		"<C-r>",
		"CGDB_KEY_CTRL_R"
	},
	{
		CGDB_KEY_CTRL_S,
		"<C-s>",
		"CGDB_KEY_CTRL_S"
	},
	{
		CGDB_KEY_CTRL_T,
		"<C-t>",
		"CGDB_KEY_CTRL_T"
	},
	{
		CGDB_KEY_CTRL_U,
		"<C-u>",
		"CGDB_KEY_CTRL_U"
	},
	{
		CGDB_KEY_CTRL_V,
		"<C-v>",
		"CGDB_KEY_CTRL_V"
	},
	{
		CGDB_KEY_CTRL_W,
		"<C-w>",
		"CGDB_KEY_CTRL_W"
	},
	{
		CGDB_KEY_CTRL_X,
		"<C-x>",
		"CGDB_KEY_CTRL_X"
	},
	{
		CGDB_KEY_CTRL_Y,
		"<C-y>",
		"CGDB_KEY_CTRL_Y"
	},
	{
		CGDB_KEY_CTRL_Z,
		"<C-z>",
		"CGDB_KEY_CTRL_Z"
	},
	{
		CGDB_KEY_ALT_A,
		"<A-a>",
		"CGDB_KEY_ALT_A"
	},
	{
		CGDB_KEY_ALT_B,
		"<A-b>",
		"CGDB_KEY_ALT_B"
	},
	{
		CGDB_KEY_ALT_C,
		"<A-c>",
		"CGDB_KEY_ALT_C"
	},
	{
		CGDB_KEY_ALT_D,
		"<A-d>",
		"CGDB_KEY_ALT_D"
	},
	{
		CGDB_KEY_ALT_E,
		"<A-e>",
		"CGDB_KEY_ALT_E"
	},
	{
		CGDB_KEY_ALT_F,
		"<A-f>",
		"CGDB_KEY_ALT_F"
	},
	{
		CGDB_KEY_ALT_G,
		"<A-g>",
		"CGDB_KEY_ALT_G"
	},
	{
		CGDB_KEY_ALT_H,
		"<A-h>",
		"CGDB_KEY_ALT_H"
	},
	{
		CGDB_KEY_ALT_I,
		"<A-i>",
		"CGDB_KEY_ALT_I"
	},
	{
		CGDB_KEY_ALT_J,
		"<A-j>",
		"CGDB_KEY_ALT_J"
	},
	{
		CGDB_KEY_ALT_K,
		"<A-k>",
		"CGDB_KEY_ALT_K"
	},
	{
		CGDB_KEY_ALT_L,
		"<A-l>",
		"CGDB_KEY_ALT_L"
	},
	{
		CGDB_KEY_ALT_M,
		"<A-m>",
		"CGDB_KEY_ALT_M"
	},
	{
		CGDB_KEY_ALT_N,
		"<A-n>",
		"CGDB_KEY_ALT_N"
	},
	{
		CGDB_KEY_ALT_O,
		"<A-o>",
		"CGDB_KEY_ALT_O"
	},
	{
		CGDB_KEY_ALT_P,
		"<A-p>",
		"CGDB_KEY_ALT_P"
	},
	{
		CGDB_KEY_ALT_Q,
		"<A-q>",
		"CGDB_KEY_ALT_Q"
	},
	{
		CGDB_KEY_ALT_R,
		"<A-r>",
		"CGDB_KEY_ALT_R"
	},
	{
		CGDB_KEY_ALT_S,
		"<A-s>",
		"CGDB_KEY_ALT_S"
	},
	{
		CGDB_KEY_ALT_T,
		"<A-t>",
		"CGDB_KEY_ALT_T"
	},
	{
		CGDB_KEY_ALT_U,
		"<A-u>",
		"CGDB_KEY_ALT_U"
	},
	{
		CGDB_KEY_ALT_V,
		"<A-v>",
		"CGDB_KEY_ALT_V"
	},
	{
		CGDB_KEY_ALT_W,
		"<A-w>",
		"CGDB_KEY_ALT_W"
	},
	{
		CGDB_KEY_ALT_X,
		"<A-x>",
		"CGDB_KEY_ALT_X"
	},
	{
		CGDB_KEY_ALT_Y,
		"<A-y>",
		"CGDB_KEY_ALT_Y"
	},
	{
		CGDB_KEY_ALT_Z,
		"<A-z>",
		"CGDB_KEY_ALT_Z"
	},
	{
		CGDB_KEY_ALT_SHIFT_A,
		"<A-A>",
		"CGDB_KEY_ALT_SHIFT_A"
	},
	{
		CGDB_KEY_ALT_SHIFT_B,
		"<A-B>",
		"CGDB_KEY_ALT_SHIFT_B"
	},
	{
		CGDB_KEY_ALT_SHIFT_C,
		"<A-C>",
		"CGDB_KEY_ALT_SHIFT_C"
	},
	{
		CGDB_KEY_ALT_SHIFT_D,
		"<A-D>",
		"CGDB_KEY_ALT_SHIFT_D"
	},
	{
		CGDB_KEY_ALT_SHIFT_E,
		"<A-E>",
		"CGDB_KEY_ALT_SHIFT_E"
	},
	{
		CGDB_KEY_ALT_SHIFT_F,
		"<A-F>",
		"CGDB_KEY_ALT_SHIFT_F"
	},
	{
		CGDB_KEY_ALT_SHIFT_G,
		"<A-G>",
		"CGDB_KEY_ALT_SHIFT_G"
	},
	{
		CGDB_KEY_ALT_SHIFT_H,
		"<A-H>",
		"CGDB_KEY_ALT_SHIFT_H"
	},
	{
		CGDB_KEY_ALT_SHIFT_I,
		"<A-I>",
		"CGDB_KEY_ALT_SHIFT_I"
	},
	{
		CGDB_KEY_ALT_SHIFT_J,
		"<A-J>",
		"CGDB_KEY_ALT_SHIFT_J"
	},
	{
		CGDB_KEY_ALT_SHIFT_K,
		"<A-K>",
		"CGDB_KEY_ALT_SHIFT_K"
	},
	{
		CGDB_KEY_ALT_SHIFT_L,
		"<A-L>",
		"CGDB_KEY_ALT_SHIFT_L"
	},
	{
		CGDB_KEY_ALT_SHIFT_M,
		"<A-M>",
		"CGDB_KEY_ALT_SHIFT_M"
	},
	{
		CGDB_KEY_ALT_SHIFT_N,
		"<A-N>",
		"CGDB_KEY_ALT_SHIFT_N"
	},
	{
		CGDB_KEY_ALT_SHIFT_O,
		"<A-O>",
		"CGDB_KEY_ALT_SHIFT_O"
	},
	{
		CGDB_KEY_ALT_SHIFT_P,
		"<A-P>",
		"CGDB_KEY_ALT_SHIFT_P"
	},
	{
		CGDB_KEY_ALT_SHIFT_Q,
		"<A-Q>",
		"CGDB_KEY_ALT_SHIFT_Q"
	},
	{
		CGDB_KEY_ALT_SHIFT_R,
		"<A-R>",
		"CGDB_KEY_ALT_SHIFT_R"
	},
	{
		CGDB_KEY_ALT_SHIFT_S,
		"<A-S>",
		"CGDB_KEY_ALT_SHIFT_S"
	},
	{
		CGDB_KEY_ALT_SHIFT_T,
		"<A-T>",
		"CGDB_KEY_ALT_SHIFT_T"
	},
	{
		CGDB_KEY_ALT_SHIFT_U,
		"<A-U>",
		"CGDB_KEY_ALT_SHIFT_U"
	},
	{
		CGDB_KEY_ALT_SHIFT_V,
		"<A-V>",
		"CGDB_KEY_ALT_SHIFT_V"
	},
	{
		CGDB_KEY_ALT_SHIFT_W,
		"<A-W>",
		"CGDB_KEY_ALT_SHIFT_W"
	},
	{
		CGDB_KEY_ALT_SHIFT_X,
		"<A-X>",
		"CGDB_KEY_ALT_SHIFT_X"
	},
	{
		CGDB_KEY_ALT_SHIFT_Y,
		"<A-Y>",
		"CGDB_KEY_ALT_SHIFT_Y"
	},
	{
		CGDB_KEY_ALT_SHIFT_Z,
		"<A-Z>",
		"CGDB_KEY_ALT_SHIFT_Z"
	},
	{
		CGDB_KEY_ALT_1,
		"<A-1>",
		"CGDB_KEY_ALT_1"
	},
	{
		CGDB_KEY_ALT_2,
		"<A-2>",
		"CGDB_KEY_ALT_2"
	},
	{
		CGDB_KEY_ALT_3,
		"<A-3>",
		"CGDB_KEY_ALT_3"
	},
	{
		CGDB_KEY_ALT_4,
		"<A-4>",
		"CGDB_KEY_ALT_4"
	},
	{
		CGDB_KEY_ALT_5,
		"<A-5>",
		"CGDB_KEY_ALT_5"
	},
	{
		CGDB_KEY_ALT_6,
		"<A-6>",
		"CGDB_KEY_ALT_6"
	},
	{
		CGDB_KEY_ALT_7,
		"<A-7>",
		"CGDB_KEY_ALT_7"
	},
	{
		CGDB_KEY_ALT_8,
		"<A-8>",
		"CGDB_KEY_ALT_8"
	},
	{
		CGDB_KEY_ALT_9,
		"<A-9>",
		"CGDB_KEY_ALT_9"
	},
	{
		CGDB_KEY_ALT_0,
		"<A-0>",
		"CGDB_KEY_ALT_0"
	},
	{
		CGDB_KEY_ALT_SHIFT_1,
		"<A-!>",
		"CGDB_KEY_ALT_SHIFT_1"
	},
	{
		CGDB_KEY_ALT_SHIFT_2,
		"<A-@>",
		"CGDB_KEY_ALT_SHIFT_2"
	},
	{
		CGDB_KEY_ALT_SHIFT_3,
		"<A-#>",
		"CGDB_KEY_ALT_SHIFT_3"
	},
	{
		CGDB_KEY_ALT_SHIFT_4,
		"<A-$>",
		"CGDB_KEY_ALT_SHIFT_4"
	},
	{
		CGDB_KEY_ALT_SHIFT_5,
		"<A-%>",
		"CGDB_KEY_ALT_SHIFT_5"
	},
	{
		CGDB_KEY_ALT_SHIFT_6,
		"<A-^>",
		"CGDB_KEY_ALT_SHIFT_6"
	},
	{
		CGDB_KEY_ALT_SHIFT_7,
		"<A-&>",
		"CGDB_KEY_ALT_SHIFT_7"
	},
	{
		CGDB_KEY_ALT_SHIFT_8,
		"<A-*>",
		"CGDB_KEY_ALT_SHIFT_8"
	},
	{
		CGDB_KEY_ALT_SHIFT_9,
		"<A-(>",
		"CGDB_KEY_ALT_SHIFT_9"
	},
	{
		CGDB_KEY_ALT_SHIFT_0,
		"<A-)>",
		"CGDB_KEY_ALT_SHIFT_0"
	},
	{
		CGDB_KEY_ALT_MINUS,
		"<A-->",
		"CGDB_KEY_ALT_MINUS"
	},
	{
		CGDB_KEY_ALT_EQUAL,
		"<A-=>",
		"CGDB_KEY_ALT_EQUAL"
	},
	{
		CGDB_KEY_ALT_LEFT_BRACKET,
		"<A-[>",
		"CGDB_KEY_ALT_LEFT_BRACKET"
	},
	{
		CGDB_KEY_ALT_RIGHT_BRACKET,
		"<A-]>",
		"CGDB_KEY_ALT_RIGHT_BRACKET"
	},
	{
		CGDB_KEY_ALT_BACKSLASH,
		"<A-\\>",
		"CGDB_KEY_ALT_BACKSLASH"
	},
	{
		CGDB_KEY_ALT_SEMICOLON,
		"<A-;>",
		"CGDB_KEY_ALT_SEMICOLON"
	},
	{
		CGDB_KEY_ALT_APOSTROPHE,
		"<A-'>",
		"CGDB_KEY_ALT_APOSTROPHE"
	},
	{
		CGDB_KEY_ALT_COMMA,
		"<A-,>",
		"CGDB_KEY_ALT_COMMA"
	},
	{
		CGDB_KEY_ALT_PERIOD,
		"<A-.>",
		"CGDB_KEY_ALT_PERIOD"
	},
	{
		CGDB_KEY_ALT_DIVIDE,
		"<A-/>",
		"CGDB_KEY_ALT_DIVIDE"
	},
	{
		CGDB_KEY_ALT_ACCENT_MARK,
		"<A-'>",
		"CGDB_KEY_ALT_ACCENT_MARK"
	},
	{
		CGDB_KEY_ALT_SHIFT_UNDERSCORE,
		"<A-_>",
		"CGDB_KEY_ALT_SHIFT_UNDERSCORE"
	},
	{
		CGDB_KEY_ALT_SHIFT_PLUS,
		"<A-+>",
		"CGDB_KEY_ALT_SHIFT_PLUS"
	},
	{
		CGDB_KEY_ALT_SHIFT_LEFT_CURLY_BRACKET,
		"<A-{>",
		"CGDB_KEY_ALT_SHIFT_LEFT_CURLY_BRACKET"
	},
	{
		CGDB_KEY_ALT_SHIFT_RIGHT_CURLY_BRACKET,
		"<A-}>",
		"CGDB_KEY_ALT_SHIFT_RIGHT_CURLY_BRACKET"
	},
	{
		CGDB_KEY_ALT_SHIFT_PIPE,
		"<A-|>",
		"CGDB_KEY_ALT_SHIFT_PIPE"
	},
	{
		CGDB_KEY_ALT_SHIFT_COLON,
		"<A-:>",
		"CGDB_KEY_ALT_SHIFT_COLON"
	},
	{
		CGDB_KEY_ALT_SHIFT_QUOTE,
		"<A-\">",
		"CGDB_KEY_ALT_SHIFT_QUOTE"
	},
	{
		CGDB_KEY_ALT_SHIFT_LESS_THAN,
		"<A-<>",
		"CGDB_KEY_ALT_SHIFT_LESS_THAN"
	},
	{
		CGDB_KEY_ALT_SHIFT_GREATER_THAN,
		"<A->>",
		"CGDB_KEY_ALT_SHIFT_GREATER_THAN"
	},
	{
		CGDB_KEY_ALT_SHIFT_QUESTION_MARK,
		"<A-?>",
		"CGDB_KEY_ALT_SHIFT_QUESTION_MARK"
	},
	{
		CGDB_KEY_ALT_SHIFT_TILDA,
		"<A-~>",
		"CGDB_KEY_ALT_SHIFT_TILDA"
	},
	{
		CGDB_KEY_ERROR,
		"CGDB_KEY_ERROR",
		"CGDB_KEY_ERROR"
	}
};


/** 
 * This adds key bindings that many terminals use. 
 *
 * @return
 * 0 on success, or -1 on error
 */
static int 
add_keybindings (struct kui_map_set *map)
{
  int i, val;
  const char *keycode;
  for (i = 0; hard_coded_bindings[i].key != CGDB_KEY_ERROR; ++i)
  {
    keycode = kui_term_get_keycode_from_cgdb_key (hard_coded_bindings[i].key);
    val = kui_ms_register_map (map, hard_coded_bindings[i].key_seq, keycode);
    if (val == -1)
      return -1;
  }

  return 0;
}

/* Gets a single key sequence */
static int 
import_keyseq (struct tlist *list, struct kui_map_set *map)
{
  int ret;
  static char *term_buffer = (char *)NULL;
  static char *buffer = (char *)NULL;
  char *env;

  if (term_buffer == 0)
  {
    term_buffer = (char *)malloc(4080);
    buffer = (char *)malloc(4080);
  }

  env = getenv("TERM");

  if (!env)
    return -1;
    
  ret = tgetent (term_buffer, env);
  if (ret == 0)
    return -1;
  else if (ret == -1)
    return -1;
    
  /* Set up the termcap seq */ 
  list->tname_seq = tgetstr (list->tname, &buffer);
  if (list->tname_seq == 0) {
    /*fprintf ( stderr, "CAPNAME (%s) is not present in this TERM's termcap description\n", i->tname);*/
  } else if (list->tname_seq == (char*)-1 ) {
    /* fprintf ( stderr, "CAPNAME (%s) is not a termcap string capability\n", i->tname); */
  } else {
    const char *keycode;
    keycode = kui_term_get_keycode_from_cgdb_key (list->key);
    ret = kui_ms_register_map (map, list->tname_seq, keycode);
    if (ret == -1)
      return -1;
  }

  /* Set up the terminfo seq */ 
  list->tiname_seq = tigetstr(list->tiname);
  if (list->tiname_seq == 0) {
    /* fprintf ( stderr, "CAPNAME (%s) is not present in this TERM's terminfo description\n", i->tiname);*/
  } else if (list->tiname_seq == (char*)-1){
    /* fprintf ( stderr, "CAPNAME (%s) is not a terminfo string capability\n", i->tiname); */
  } else {
    const char *keycode;
    keycode = kui_term_get_keycode_from_cgdb_key (list->key);
    ret = kui_ms_register_map (map, list->tiname_seq, keycode);
    if (ret == -1 ) 
      return -1;
  }

  return 0;
}

/**
 * Read's in all of the termcap and terminfo key sequences.
 *
 * @return
 * 0 on success, or -1 on error
 */
static int import_keyseqs(struct kui_map_set *map) {
    int i;

    for( i = 0; seqlist[i].tname != NULL; i++)
        import_keyseq(&seqlist[i], map);

	return 0;
}

struct kui_map_set *kui_term_get_terminal_mappings ( void ) {
	struct kui_map_set *map;

	map = kui_ms_create ();

	if ( import_keyseqs ( map ) == -1 )
		return  NULL;

	if ( !map )
		return NULL;

	/* Add all the extra's */
	if ( add_keybindings ( map ) == -1 ) {
		/* TODO: Free map and return */
		return  NULL;
	}

	return map;
}

int kui_term_get_cgdb_key_from_keycode ( const char *keycode ) {
	int i;	

	for ( i = 0; cgdb_keys[i].key != CGDB_KEY_ERROR; ++i ) {
		struct cgdb_key_data *ckey = &cgdb_keys[i];

		if ( strcasecmp ( keycode, ckey->keycode ) == 0 )
			return ckey->key;
	}

	return CGDB_KEY_ERROR;
}

const char *kui_term_get_string_from_cgdb_key ( int key ) {
	int i;	

	for ( i = 0; cgdb_keys[i].key != CGDB_KEY_ERROR; ++i ) {
		struct cgdb_key_data *ckey = &cgdb_keys[i];
		int tkey = ckey->key;

		if ( key == tkey )
			return ckey->key_as_string;
	}

	return NULL;
}

const char *kui_term_get_keycode_from_cgdb_key ( int key ) {
	int i;	

	for ( i = 0; cgdb_keys[i].key != CGDB_KEY_ERROR; ++i ) {
		struct cgdb_key_data *ckey = &cgdb_keys[i];
		int tkey = ckey->key;

		if ( key == tkey )
			return ckey->keycode;
	}

	return NULL;
}

int kui_term_is_cgdb_key ( int key ) {
	if ( key >= CGDB_KEY_ESC && key <= CGDB_KEY_ERROR )
		return 1;

	return 0;
}

char *
kui_term_get_ascii_char_sequence_from_key (int key)
{
  int i, val;
  const char *keycode;

  if (key <= CGDB_KEY_ESC || key >= CGDB_KEY_ERROR)
    return NULL;

  for (i = 0; hard_coded_bindings[i].key != CGDB_KEY_ERROR; ++i)
  {
    if (key == hard_coded_bindings[i].key)
    {
      return hard_coded_bindings[i].key_seq;
    }
  }

  /* It wasn't one of the hardcoded values. The only thing left is the 
   * termcap or terminfo entries. Try the termcap first, since that's 
   * what readline uses. */
  for (i = 0; seqlist[i].key != CGDB_KEY_ERROR; ++i)
  {
    if (key == seqlist[i].key)
    {
      if (seqlist[i].tname_seq)
	return seqlist[i].tname_seq;
      else if (seqlist[i].tiname_seq)
	return seqlist[i].tiname_seq;
    }
  }

  return NULL;
}

int kui_term_string_to_cgdb_key_array ( 
		const char *string,
		int **cgdb_key_array ) {

	int i;
    enum state_type { KUI_MAP_STATE_NORMAL, KUI_MAP_STATE_MACRO } state;
	char cur_char;
	int length;

	/* A buffer to store macros */
	char *macro;
	int macro_pos;

	/* A buffer to store the outbound parameter */
	int *local_cgdb_key_array;
	int cgdb_key_array_pos;

	/* Verify parameters */
	if ( !string )
		return -1;

	if ( !cgdb_key_array )
		return -1;

	/* Initial stack parameters */
	state = KUI_MAP_STATE_NORMAL;
	length = strlen ( string );

	/*
	 * Assertion 
	 *
	 * The macro and output buffer will always be smaller than the input 
	 * buffer. So, to make life easy, they are always malloced to be the
	 * size of the input buffer. This may be a little wasteful, but it should 
	 * not be to bad. Later, if a better ADT is available, use it here.
	 */

	/* Initialize the macro buffer */
	macro = (char*)malloc ( sizeof ( char ) * (length+1) );
	macro_pos = 0;

	/* Initialize the output buffer  */
	local_cgdb_key_array = (int*)malloc ( sizeof ( int ) * (length+1) );
	cgdb_key_array_pos = 0;
       
	for (i = 0; i < length; i++){
		cur_char = string[i];

		switch(state){
			case KUI_MAP_STATE_NORMAL:
				/* Check for keycode start */
				if ( cur_char == '<' )  {
					state = KUI_MAP_STATE_MACRO;

					/* Capture macro */
					macro[macro_pos++] = cur_char;
				} else {
					/* Place into output buffer */
					local_cgdb_key_array[cgdb_key_array_pos++] = (int)cur_char;
				}

				break;
			case KUI_MAP_STATE_MACRO:

				/* Getting a macro start symbol within a macro means that from the 
				 * first '<' until here was not a macro. Dump it into the 
				 * output buffer and start fresh, assumming this is the start
				 * of a new macro.
				 */
				if ( cur_char == '<' ) {
					int j;
					for ( j = 0; j < macro_pos; ++j )
						local_cgdb_key_array[cgdb_key_array_pos++] = macro[j];
					macro_pos = 0;
				}

				/* Capture macro */
				macro[macro_pos++] = cur_char;

				if ( cur_char == '>' ) {
					int cgdb_key;

					state = KUI_MAP_STATE_NORMAL;

					/* Null terminate macro captured */
					macro[macro_pos] = '\0';

					/* Place captured macro into output buffer */
					cgdb_key = kui_term_get_cgdb_key_from_keycode ( macro );

					if ( cgdb_key == -1 )
						return -1;

					/* The key doesn't exist, write the data into the 
					 * buffer. */
					if ( cgdb_key == CGDB_KEY_ERROR ) {
						int j;
						for ( j = 0; j < macro_pos; ++j )
							local_cgdb_key_array[cgdb_key_array_pos++] = macro[j];
					} else
						local_cgdb_key_array[cgdb_key_array_pos++] = cgdb_key;
				
					macro_pos = 0;
				} 


				break;
		}
	}

	/* This means that there was a '<' symbol not eventually followed by a '>'
	 * symbol. Therefore, everything from the '<' on is not part of a macro. It
	 * should be copied into the output buffer exactly the way it is.
	 */
	if ( state == KUI_MAP_STATE_MACRO ) {
		int j;
		for ( j = 0; j < macro_pos; ++j )
			local_cgdb_key_array[cgdb_key_array_pos++] = macro[j];
	}

	local_cgdb_key_array[cgdb_key_array_pos++] = 0;
	
	*cgdb_key_array = local_cgdb_key_array;

	free ( macro );
	macro = NULL;

	return 0;
}

int kui_term_print_cgdb_key_array ( int *cgdb_key_array ) {
	int i;

	if ( !cgdb_key_array )
		return -1;

	/* Display output buffer */
	fprintf ( stderr, "CGDB_KEY_ARRAY(");

	for ( i = 0; cgdb_key_array[i] != 0; i++ ) {
		int is_ckey;

		is_ckey = kui_term_is_cgdb_key ( cgdb_key_array[i] );

		if ( is_ckey ) {
			fprintf ( stderr, "%s", kui_term_get_string_from_cgdb_key ( cgdb_key_array[i] ) );	
		} else {
			fprintf ( stderr, "%c", cgdb_key_array[i] );
		}
	}

	fprintf ( stderr, ")\r\n" );

	return 0;
}
