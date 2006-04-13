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
	/* the termcap capability name  */
    char *tname;
	/* the terminfo capability name  */
    char *tiname;
	/* the ascii representation of this key  */
    char *cgdb_key_code;
} seqlist[] = {
  { "@7", "kend",   "<End>" },
  { "kh", "khome",  "<Home>" },
  { "kH", "kll",    "<Home>" },
  { "dc", "dch1",   "<Del>" },
  { "kD", "kdch1",  "<Del>" },
  { "ic", "ich1",   "<Insert>" },
  { "kI", "kich1",  "<Insert>" },
  { "kN", "knp",    "<PageDown>" },
  { "kP", "kpp",    "<PageUp>" },

  /* For arrow keys */
  { "kd", "kcud1",  "<Down>" },
  { "kl", "kcub1",  "<Left>" },
  { "kr", "kcuf1",  "<Right>" },
  { "ku", "kcuu1",  "<Up>" },
  { "le", "cub1",   "<Left>" },
  { "nd", "cuf1",   "<Right>" },
  { "up", "cuu1",   "<Up>" },

  /* Function keys */
  { "k1", "kf1",    "<F1>" },
  { "k2", "kf2",    "<F2>" },
  { "k3", "kf3",    "<F3>" },
  { "k4", "kf4",    "<F4>" },
  { "k5", "kf5",    "<F5>" },
  { "k6", "kf6",    "<F6>" },
  { "k7", "kf7",    "<F7>" },
  { "k8", "kf8",    "<F8>" },
  { "k9", "kf9",    "<F9>" },
  { "k;", "kf10",   "<F10>" },
  { "F1", "kf11",   "<F11>" },
  { "F2", "kf12",   "<F12>" },
  { NULL, NULL,     NULL }
};

/** 
 * This adds key bindings that many terminals use. 
 *
 * @return
 * 0 on success, or -1 on error
 */
static int add_keybindings( struct kui_map_set *map ) {
	/* Testing */
/*	if ( kui_ms_register_map ( map, "ab", "<ESC>xyz" ) == -1 ) return -1; */
/*	if ( kui_ms_register_map ( map, "abcdf", "never_reached" ) == -1 ) return -1; */
	
	if ( kui_ms_register_map ( map, "\033", "<Esc>" ) == -1 ) return -1;

	/* Arrow bindings */
	if ( kui_ms_register_map ( map, "\033[0A", "<Up>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033[0A", "<Up>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033[0B", "<Left>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033[0C", "<Right>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033[0D", "<Down>" ) == -1 ) return -1;

	if ( kui_ms_register_map ( map, "\033[A", "<Up>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033[B", "<Down>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033[C", "<Right>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033[D", "<Left>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033[H", "<Home>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033[F", "<End>" ) == -1 ) return -1;

	if ( kui_ms_register_map ( map, "\033OA", "<Up>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033OB", "<Down>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033OC", "<Right>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033OD", "<Left>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033OH", "<Home>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033OF", "<End>" ) == -1 ) return -1;

	/* Ctrl bindings */
	if ( kui_ms_register_map ( map, "\001", "<C-a>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\002", "<C-b>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\003", "<C-c>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\004", "<C-d>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\005", "<C-e>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\006", "<C-f>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\007", "<C-g>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\010", "<C-h>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\011", "<C-i>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\012", "<C-j>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\013", "<C-k>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\014", "<C-l>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\015", "<C-m>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\016", "<C-n>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\017", "<C-o>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\020", "<C-p>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\021", "<C-q>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\022", "<C-r>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\023", "<C-s>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\024", "<C-t>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\025", "<C-u>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\026", "<C-v>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\027", "<C-w>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\030", "<C-x>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\031", "<C-y>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\032", "<C-z>" ) == -1 ) return -1;

    /* Alt bindings */
	if ( kui_ms_register_map ( map, "\033a", "<A-a>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033b", "<A-b>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033c", "<A-c>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033d", "<A-d>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033e", "<A-e>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033f", "<A-f>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033g", "<A-g>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033h", "<A-h>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033i", "<A-i>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033j", "<A-j>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033k", "<A-k>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033l", "<A-l>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033m", "<A-m>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033n", "<A-n>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033o", "<A-o>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033p", "<A-p>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033q", "<A-q>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033r", "<A-r>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033s", "<A-s>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033t", "<A-t>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033u", "<A-u>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033v", "<A-v>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033w", "<A-w>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033x", "<A-x>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033y", "<A-y>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033z", "<A-z>" ) == -1 ) return -1;

    /* Alt Shift bindings */
	if ( kui_ms_register_map ( map, "\033A", "<A-A>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033B", "<A-B>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033C", "<A-C>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033D", "<A-D>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033E", "<A-E>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033F", "<A-F>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033G", "<A-G>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033H", "<A-H>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033I", "<A-I>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033J", "<A-J>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033K", "<A-K>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033L", "<A-L>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033M", "<A-M>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033N", "<A-N>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033O", "<A-O>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033P", "<A-P>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033Q", "<A-Q>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033R", "<A-R>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033S", "<A-S>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033T", "<A-T>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033U", "<A-U>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033V", "<A-V>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033W", "<A-W>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033X", "<A-X>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033Y", "<A-Y>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033Z", "<A-Z>" ) == -1 ) return -1;

    /* Alt Numbers */
	if ( kui_ms_register_map ( map, "\0331", "<A-1>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\0332", "<A-2>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\0333", "<A-3>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\0334", "<A-4>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\0335", "<A-5>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\0336", "<A-6>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\0337", "<A-7>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\0338", "<A-8>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\0339", "<A-9>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\0330", "<A-0>" ) == -1 ) return -1;

    /* Alt Shifted Numbers */
	if ( kui_ms_register_map ( map, "\033!", "<A-!>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033@", "<A-@>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033#", "<A-#>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033$", "<A-$>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033%", "<A-%>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033^", "<A-^>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033&", "<A-&>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033*", "<A-*>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033(", "<A-(>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033)", "<A-)>" ) == -1 ) return -1;

    /* Alt Special */
	if ( kui_ms_register_map ( map, "\033-", "<A-->" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033=", "<A-=>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033[", "<A-[>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033]", "<A-]>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033\\", "<A-\\>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033;", "<A-;>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033'", "<A-'>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033,", "<A-,>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033.", "<A-.>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033/", "<A-/>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033`", "<A-`>" ) == -1 ) return -1;

    /* Alt Shifte Special */
	if ( kui_ms_register_map ( map, "\033_", "<A-_>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033+", "<A-+>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033{", "<A-{>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033}", "<A-}>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033|", "<A-|>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033:", "<A-:>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033\"", "<A-\">" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033<", "<A-<>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033>", "<A->>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033?", "<A-?>" ) == -1 ) return -1;
	if ( kui_ms_register_map ( map, "\033~", "<A-~>" ) == -1 ) return -1;
	
	return 0;
}

/* Gets a single key sequence */
static int import_keyseq(struct tlist *i, struct kui_map_set *map) {
    char *terminfo, *termcap;
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

    if ( !env ) {
        fprintf ( stderr, "%s:%d TERM not set error", __FILE__, __LINE__);
        return -1;
    }
    
    if ( ( ret = tgetent(term_buffer, env)) == 0 ) {
        fprintf ( stderr, "%s:%d tgetent 'No such entry' error", __FILE__, __LINE__);
        return -1;
    } else if ( ret == -1 ) {
        fprintf ( stderr, "%s:%d tgetent 'terminfo database could not be found' error", __FILE__, __LINE__);
        return -1;
    }
    
    /* Set up the termcap seq */ 
    if ( (termcap = tgetstr(i->tname, &buffer)) == 0 ) {
        /*fprintf ( stderr, "CAPNAME (%s) is not present in this TERM's termcap description\n", i->tname);*/
	} else if (termcap == (char*)-1 ) {
        /* fprintf ( stderr, "CAPNAME (%s) is not a termcap string capability\n", i->tname); */
    } else {
		if ( kui_ms_register_map ( map, termcap, i->cgdb_key_code ) == -1 ) 
			return -1;
	}

    /* Set up the terminfo seq */ 
    if ( (terminfo = tigetstr(i->tiname)) == 0 ) {
        /* fprintf ( stderr, "CAPNAME (%s) is not present in this TERM's terminfo description\n", i->tiname);*/
	} else if (terminfo == (char*)-1 ) {
       /* fprintf ( stderr, "CAPNAME (%s) is not a terminfo string capability\n", i->tiname); */
    } else {
		if ( kui_ms_register_map ( map, terminfo, i->cgdb_key_code ) == -1 ) 
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
