#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#if HAVE_STRINGS_H
#include <strings.h>
#endif /* HAVE_STRINGS_H */

#if HAVE_STDLIB_H
#include <stdlib.h>             /* for getenv */
#endif /* HAVE_STDLIB_H */

#if HAVE_STDIO_H
#include <stdio.h>              /* for stderr */
#endif /* HAVE_STDIO_H */

#include <array>
#include <cassert>

/* term.h prototypes */
#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C extern
#endif

EXTERN_C int tgetent(char *, const char *);
EXTERN_C char *tgetstr(const char *, char **);

#include "sys_util.h"
#include "sys_win.h"
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
    const char *tname;
    /* the termcap key sequence */
    char *tname_seq;
    /* the terminfo capability name  */
    const char *tiname;
    /* the terminfo key sequence */
    char *tiname_seq;
};

static std::array<tlist, 26> seqlist{{
    { CGDB_KEY_END  , "@7", nullptr, "kend" , nullptr },
    { CGDB_KEY_HOME , "kh", nullptr, "khome", nullptr },
    { CGDB_KEY_HOME , "kH", nullptr, "kll"  , nullptr },
    { CGDB_KEY_DC   , "kD", nullptr, "kdch1", nullptr },
    { CGDB_KEY_IC   , "kI", nullptr, "kich1", nullptr },
    { CGDB_KEY_NPAGE, "kN", nullptr, "knp"  , nullptr },
    { CGDB_KEY_PPAGE, "kP", nullptr, "kpp"  , nullptr },
    /* For arrow keys */
    { CGDB_KEY_DOWN , "kd", nullptr, "kcud1", nullptr },
    { CGDB_KEY_LEFT , "kl", nullptr, "kcub1", nullptr },
    { CGDB_KEY_RIGHT, "kr", nullptr, "kcuf1", nullptr },
    { CGDB_KEY_UP   , "ku", nullptr, "kcuu1", nullptr },
    { CGDB_KEY_LEFT , "le", nullptr, "cub1" , nullptr },
    { CGDB_KEY_RIGHT, "nd", nullptr, "cuf1" , nullptr },
    { CGDB_KEY_UP   , "up", nullptr, "cuu1" , nullptr },
    /* Function keys */
    { CGDB_KEY_F1   , "k1", nullptr, "kf1" , nullptr },
    { CGDB_KEY_F2   , "k2", nullptr, "kf2" , nullptr },
    { CGDB_KEY_F3   , "k3", nullptr, "kf3" , nullptr },
    { CGDB_KEY_F4   , "k4", nullptr, "kf4" , nullptr },
    { CGDB_KEY_F5   , "k5", nullptr, "kf5" , nullptr },
    { CGDB_KEY_F6   , "k6", nullptr, "kf6" , nullptr },
    { CGDB_KEY_F7   , "k7", nullptr, "kf7" , nullptr },
    { CGDB_KEY_F8   , "k8", nullptr, "kf8" , nullptr },
    { CGDB_KEY_F9   , "k9", nullptr, "kf9" , nullptr },
    { CGDB_KEY_F10  , "k;", nullptr, "kf10", nullptr },
    { CGDB_KEY_F11  , "F1", nullptr, "kf11", nullptr },
    { CGDB_KEY_F12  , "F2", nullptr, "kf12", nullptr }
}};

/* This represents all of the hard coded key data.  */
struct keydata {
    enum cgdb_key key;
    const char *key_seq;
};

static std::array<keydata, 47> hard_coded_bindings{{
    { CGDB_KEY_ESC, "\033" },
    /* Arrow bindings */

    /* A cgdb key can be represented by several different terminal
     * mappings. Different terminals produce different key sequences, for
     * say F11, and we hard code common cases for the popular terminals.
     *
     * cgdb returns the enumeration cgdb_key back up to cgdb
     * instead of the key sequence. The first key mapping is the one that
     * will be used when asking what character mapping makes up a cgdb key.
     *
     * For this reason, we put the ones that are hard coded in readline
     * on top. That way, we know when we pass readline the key mapping,
     * it will understand it.
     */
    { CGDB_KEY_UP     , "\033[A" },
    { CGDB_KEY_DOWN   , "\033[B" },
    { CGDB_KEY_RIGHT  , "\033[C" },
    { CGDB_KEY_LEFT   , "\033[D" },
    { CGDB_KEY_HOME   , "\033[H" },
    { CGDB_KEY_END    , "\033[F" },
    /* Arrow bindings , MSDOS */
    { CGDB_KEY_UP     , "\033[0A" },
    { CGDB_KEY_LEFT   , "\033[0B" },
    { CGDB_KEY_RIGHT  , "\033[0C" },
    { CGDB_KEY_DOWN   , "\033[0D" },
    { CGDB_KEY_UP     , "\033OA" },
    { CGDB_KEY_DOWN   , "\033OB" },
    { CGDB_KEY_RIGHT  , "\033OC" },
    { CGDB_KEY_LEFT   , "\033OD" },
    { CGDB_KEY_HOME   , "\033OH" },
    { CGDB_KEY_END    , "\033OF" },
    /* Passed through to readline */
    { CGDB_KEY_BACKWARD_WORD      , "\033b" },
    { CGDB_KEY_FORWARD_WORD       , "\033f" },
    { CGDB_KEY_BACKWARD_KILL_WORD , "\033\b" },
    { CGDB_KEY_FORWARD_KILL_WORD  , "\033d" },
    /* Ctrl bindings */
    { CGDB_KEY_CTRL_A , "\001" },
    { CGDB_KEY_CTRL_B , "\002" },
    { CGDB_KEY_CTRL_C , "\003" },
    { CGDB_KEY_CTRL_D , "\004" },
    { CGDB_KEY_CTRL_E , "\005" },
    { CGDB_KEY_CTRL_F , "\006" },
    { CGDB_KEY_CTRL_G , "\007" },
    { CGDB_KEY_CTRL_H , "\010" },
    { CGDB_KEY_CTRL_I , "\011" },
    { CGDB_KEY_CTRL_J , "\012" },
    { CGDB_KEY_CTRL_K , "\013" },
    { CGDB_KEY_CTRL_L , "\014" },
    { CGDB_KEY_CTRL_M , "\015" },
    { CGDB_KEY_CTRL_N , "\016" },
    { CGDB_KEY_CTRL_O , "\017" },
    { CGDB_KEY_CTRL_P , "\020" },
    { CGDB_KEY_CTRL_Q , "\021" },
    { CGDB_KEY_CTRL_R , "\022" },
    { CGDB_KEY_CTRL_S , "\023" },
    { CGDB_KEY_CTRL_T , "\024" },
    { CGDB_KEY_CTRL_U , "\025" },
    { CGDB_KEY_CTRL_V , "\026" },
    { CGDB_KEY_CTRL_W , "\027" },
    { CGDB_KEY_CTRL_X , "\030" },
    { CGDB_KEY_CTRL_Y , "\031" },
    { CGDB_KEY_CTRL_Z , "\032" }
}};

/**
 * This is the main data structure in determining the string representation
 * of a keycode that can be used in a map command. All of the hard coded 
 * keycodes that can be used in a map is stored here. Some keycodes can not
 * be hard coded and are read from the termcap/terminfo library.
 */
struct cgdb_keycode_data {
    /* This is the "key" that can be converted to text.  */
    int key;
    /* This is the text value associated with the key  */
    const char *keycode;
    /* This is here purely for debugging purposes, it is used to print out the key
     * in human readable form.  */
    const char *key_as_string;
};

static std::array<cgdb_keycode_data, 92> cgdb_keycodes{{
    /* Shift keys */
    { 'A', "<S-a>", "<shift a>" },
    { 'B', "<S-b>", "<shift b>" },
    { 'C', "<S-c>", "<shift c>" },
    { 'D', "<S-d>", "<shift d>" },
    { 'E', "<S-e>", "<shift e>" },
    { 'F', "<S-f>", "<shift f>" },
    { 'G', "<S-g>", "<shift g>" },
    { 'H', "<S-h>", "<shift h>" },
    { 'I', "<S-i>", "<shift i>" },
    { 'J', "<S-j>", "<shift j>" },
    { 'K', "<S-k>", "<shift k>" },
    { 'L', "<S-l>", "<shift l>" },
    { 'M', "<S-m>", "<shift m>" },
    { 'N', "<S-n>", "<shift n>" },
    { 'O', "<S-o>", "<shift o>" },
    { 'P', "<S-p>", "<shift p>" },
    { 'Q', "<S-q>", "<shift q>" },
    { 'R', "<S-r>", "<shift r>" },
    { 'S', "<S-s>", "<shift s>" },
    { 'T', "<S-t>", "<shift t>" },
    { 'U', "<S-u>", "<shift u>" },
    { 'V', "<S-v>", "<shift v>" },
    { 'W', "<S-w>", "<shift w>" },
    { 'X', "<S-x>", "<shift x>" },
    { 'Y', "<S-y>", "<shift y>" },
    { 'Z', "<S-z>", "<shift z>" },
    /* The enum cgdb keys in order */
    { CGDB_KEY_ESC                , "<Esc>"          , "CGDB_KEY_ESC"   },
    { CGDB_KEY_UP                 , "<Up>"           , "CGDB_KEY_UP"    },
    { CGDB_KEY_DOWN               , "<Down>"         , "CGDB_KEY_DOWN"  },
    { CGDB_KEY_LEFT               , "<Left>"         , "CGDB_KEY_LEFT"  },
    { CGDB_KEY_RIGHT              , "<Right>"        , "CGDB_KEY_RIGHT" },
    { CGDB_KEY_HOME               , "<Home>"         , "CGDB_KEY_HOME"  },
    { CGDB_KEY_END                , "<End>"          , "CGDB_KEY_END"   },
    { CGDB_KEY_PPAGE              , "<PageUp>"       , "CGDB_KEY_PPAGE" },
    { CGDB_KEY_NPAGE              , "<PageDown>"     , "CGDB_KEY_NPAGE" },
    { CGDB_KEY_DC                 , "<Del>"          , "CGDB_KEY_DC"    },
    { CGDB_KEY_IC                 , "<Insert>"       , "CGDB_KEY_IC"    },
    { CGDB_KEY_F1                 , "<F1>"           , "CGDB_KEY_F1"    },
    { CGDB_KEY_F2                 , "<F2>"           , "CGDB_KEY_F2"    },
    { CGDB_KEY_F3                 , "<F3>"           , "CGDB_KEY_F3"    },
    { CGDB_KEY_F4                 , "<F4>"           , "CGDB_KEY_F4"    },
    { CGDB_KEY_F5                 , "<F5>"           , "CGDB_KEY_F5"    },
    { CGDB_KEY_F6                 , "<F6>"           , "CGDB_KEY_F6"    },
    { CGDB_KEY_F7                 , "<F7>"           , "CGDB_KEY_F7"    },
    { CGDB_KEY_F8                 , "<F8>"           , "CGDB_KEY_F8"    },
    { CGDB_KEY_F9                 , "<F9>"           , "CGDB_KEY_F9"    },
    { CGDB_KEY_F10                , "<F10>"          , "CGDB_KEY_F10"   },
    { CGDB_KEY_F11                , "<F11>"          , "CGDB_KEY_F11"   },
    { CGDB_KEY_F12                , "<F12>"          , "CGDB_KEY_F12"   },
    { CGDB_KEY_BACKWARD_WORD      , "<BACKWARD-WORD>",
                                    "CGDB_KEY_BACKWARD_WORD"               },
    { CGDB_KEY_FORWARD_WORD       , "<FORWARD-WORD>" ,
                                    "CGDB_KEY_FORWARD_WORD"                },
    { CGDB_KEY_BACKWARD_KILL_WORD , "<BACKWARD-KILL_WORD>",
                                    "CGDB_KEY_BACKWARD_KILL_WORD"          },
    { CGDB_KEY_FORWARD_KILL_WORD  , "<FORWARD-KILL_WORD>",
                                    "CGDB_KEY_FORWARD_KILL_WORD"           },
    { CGDB_KEY_CTRL_A             , "<C-a>"          , "CGDB_KEY_CTRL_A"   },
    { CGDB_KEY_CTRL_B             , "<C-b>"          , "CGDB_KEY_CTRL_B"   },
    { CGDB_KEY_CTRL_C             , "<C-c>"          , "CGDB_KEY_CTRL_C"   },
    { CGDB_KEY_CTRL_D             , "<C-d>"          , "CGDB_KEY_CTRL_D"   },
    { CGDB_KEY_CTRL_E             , "<C-e>"          , "CGDB_KEY_CTRL_E"   },
    { CGDB_KEY_CTRL_F             , "<C-f>"          , "CGDB_KEY_CTRL_F"   },
    { CGDB_KEY_CTRL_G             , "<C-g>"          , "CGDB_KEY_CTRL_G"   },
    { CGDB_KEY_CTRL_H             , "<C-h>"          , "CGDB_KEY_CTRL_H"   },
    { CGDB_KEY_CTRL_I             , "<C-i>"          , "CGDB_KEY_CTRL_I"   },
    { CGDB_KEY_CTRL_J             , "<C-j>"          , "CGDB_KEY_CTRL_J"   },
    { CGDB_KEY_CTRL_K             , "<C-k>"          , "CGDB_KEY_CTRL_K"   },
    { CGDB_KEY_CTRL_L             , "<C-l>"          , "CGDB_KEY_CTRL_L"   },
    { CGDB_KEY_CTRL_M             , "<C-m>"          , "CGDB_KEY_CTRL_M"   },
    { CGDB_KEY_CTRL_N             , "<C-n>"          , "CGDB_KEY_CTRL_N"   },
    { CGDB_KEY_CTRL_O             , "<C-o>"          , "CGDB_KEY_CTRL_O"   },
    { CGDB_KEY_CTRL_P             , "<C-p>"          , "CGDB_KEY_CTRL_P"   },
    { CGDB_KEY_CTRL_Q             , "<C-q>"          , "CGDB_KEY_CTRL_Q"   },
    { CGDB_KEY_CTRL_R             , "<C-r>"          , "CGDB_KEY_CTRL_R"   },
    { CGDB_KEY_CTRL_S             , "<C-s>"          , "CGDB_KEY_CTRL_S"   },
    { CGDB_KEY_CTRL_T             , "<C-t>"          , "CGDB_KEY_CTRL_T"   },
    { CGDB_KEY_CTRL_U             , "<C-u>"          , "CGDB_KEY_CTRL_U"   },
    { CGDB_KEY_CTRL_V             , "<C-v>"          , "CGDB_KEY_CTRL_V"   },
    { CGDB_KEY_CTRL_W             , "<C-w>"          , "CGDB_KEY_CTRL_W"   },
    { CGDB_KEY_CTRL_X             , "<C-x>"          , "CGDB_KEY_CTRL_X"   },
    { CGDB_KEY_CTRL_Y             , "<C-y>"          , "CGDB_KEY_CTRL_Y"   },
    { CGDB_KEY_CTRL_Z             , "<C-z>"          , "CGDB_KEY_CTRL_Z"   },
    { 0                           , "<Nul>"          , "<Zero>"            },
    { CGDB_KEY_CTRL_H             , "<BS>"           , "<Backspace>"       },
    { CGDB_KEY_CTRL_I             , "<Tab>"          , "<Tab>"             },
    { CGDB_KEY_CTRL_J             , "<NL>"           , "<linefeed>"        },
    { CGDB_KEY_CTRL_L             , "<FF>"           , "<formfeed>"        },
    { CGDB_KEY_CTRL_M             , "<CR>"           , "<carriage return>" },
    { CGDB_KEY_CTRL_M             , "<Return>"       , "<carriage return>" },
    { CGDB_KEY_CTRL_M             , "<Enter>"        , "<carriage return>" },
    { 32                          , "<Space>"        , "<space>"           },
    { 60                          , "<lt>"           , "<less-than>"       },
    { 92                          , "<Bslash>"       , "<backslash>"       },
    { 124                         , "<Bar>"          , "<vertical bar>"    },
    { 127                         , "<Del>"          , "<delete>"          },
}};

/** 
 * This adds key bindings that many terminals use. 
 *
 * @return
 * true on success, or false on error
 */
static bool add_keybindings(kui_map_set& map)
{
    for (const auto& binding : hard_coded_bindings) {
        const char *keycode = kui_term_get_keycode_from_cgdb_key(binding.key);
        if (!map.register_map(binding.key_seq, keycode))
            return false;
    }

    return true;
}

struct keyseq_initializer
{
    keyseq_initializer()
    {
        char term_buffer[4080] = { 0 };
        char buffer[4080] = { 0 };
        char *pbuffer = &buffer[0];
        char *env;

        if (!(env = getenv("TERM")))
        {
            return;
        }

        const int ret = tgetent(term_buffer, env);
        if (ret == 0 || ret == -1)
        {
            return;
        }

        for (auto& list : seqlist)
        {
            char *tname = tgetstr(list.tname, &pbuffer);
            if (tname && tname != (char *) -1)
            {
                list.tname_seq = strdup(tname);
            }
            else
            {
                list.tname_seq = nullptr;
            }
            char *tiname = swin_tigetstr(list.tiname);
            if (tiname && tiname != (char *) -1)
            {
                list.tiname_seq = strdup(tiname);
            }
            else
            {
                list.tiname_seq = nullptr;
            }
        }
    }

    ~keyseq_initializer()
    {
        for (auto& list : seqlist)
        {
            free(list.tname_seq);
            free(list.tiname_seq);
        }
    }
};


bool kui_term_get_terminal_mappings(kui_map_set& map)
{
    static keyseq_initializer keyseq_init;

    /** Read in all of the termcap and terminfo key sequences.  */
    for (auto& list : seqlist)
    {
        const char *keycode = kui_term_get_keycode_from_cgdb_key(list.key);
        if (list.tname_seq)
        {
            map.register_map(list.tname_seq, keycode);
        }
        if (list.tiname_seq)
        {
            map.register_map(list.tiname_seq, keycode);
        }
    }

    /* Add all the extra's */
    return add_keybindings(map);
}

int kui_term_get_cgdb_key_from_keycode(const char *keycode)
{
    for (const auto& ckey : cgdb_keycodes) {
        if (strcasecmp(keycode, ckey.keycode) == 0)
            return ckey.key;
    }

    return -1;
}

const char *kui_term_get_string_from_key(int key)
{
    for (const auto& ckey : cgdb_keycodes) {
        if (key == ckey.key)
            return ckey.key_as_string;
    }

    return NULL;
}

const char *kui_term_get_keycode_from_cgdb_key(int key)
{
    for (const auto& ckey : cgdb_keycodes) {
        if (key == ckey.key)
            return ckey.keycode;
    }

    return NULL;
}

bool kui_term_is_cgdb_key(int key)
{
    return key >= CGDB_KEY_ESC && key <= CGDB_KEY_ERROR;
}

const char *kui_term_get_ascii_char_sequence_from_key(int key)
{
    if (!kui_term_is_cgdb_key(key))
        return NULL;

    for (const auto& binding : hard_coded_bindings) {
        if (key == binding.key) {
            return binding.key_seq;
        }
    }

    /* It wasn't one of the hardcoded values. The only thing left is the 
     * termcap or terminfo entries. Try the termcap first, since that's 
     * what readline uses. */
    for (const auto& list : seqlist) {
        if (key == list.key) {
            if (list.tname_seq)
                return list.tname_seq;
            else if (list.tiname_seq)
                return list.tiname_seq;
        }
    }

    return NULL;
}

int kui_term_string_to_key_array(const char *string, int **cgdb_key_array)
{

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
    if (!string)
        return -1;

    if (!cgdb_key_array)
        return -1;

    /* Initial stack parameters */
    state = KUI_MAP_STATE_NORMAL;
    length = strlen(string);

    /*
     * Assertion 
     *
     * The macro and output buffer will always be smaller than the input 
     * buffer. So, to make life easy, they are always malloced to be the
     * size of the input buffer. This may be a little wasteful, but it should 
     * not be to bad. Later, if a better ADT is available, use it here.
     */

    /* Initialize the macro buffer */
    macro = (char *) malloc(sizeof (char) * (length + 1));
    macro_pos = 0;

    /* Initialize the output buffer  */
    local_cgdb_key_array = (int *) malloc(sizeof (int) * (length + 1));
    *cgdb_key_array = local_cgdb_key_array;

    cgdb_key_array_pos = 0;

    for (i = 0; i < length; i++) {
        cur_char = string[i];

        switch (state) {
            case KUI_MAP_STATE_NORMAL:
                /* Check for keycode start */
                if (cur_char == '<') {
                    state = KUI_MAP_STATE_MACRO;

                    /* Capture macro */
                    macro[macro_pos++] = cur_char;
                } else {
                    /* Place into output buffer */
                    local_cgdb_key_array[cgdb_key_array_pos++] = (int) cur_char;
                }

                break;
            case KUI_MAP_STATE_MACRO:

                /* Getting a macro start symbol within a macro means that from the 
                 * first '<' until here was not a macro. Dump it into the 
                 * output buffer and start fresh, assumming this is the start
                 * of a new macro.
                 */
                if (cur_char == '<') {
                    int j;

                    for (j = 0; j < macro_pos; ++j)
                        local_cgdb_key_array[cgdb_key_array_pos++] = macro[j];
                    macro_pos = 0;
                }

                /* Capture macro */
                macro[macro_pos++] = cur_char;

                if (cur_char == '>') {
                    int cgdb_key;

                    state = KUI_MAP_STATE_NORMAL;

                    /* Null terminate macro captured */
                    macro[macro_pos] = '\0';

                    /* Place captured macro into output buffer */
                    cgdb_key = kui_term_get_cgdb_key_from_keycode(macro);

                    if (cgdb_key == -1)
                        return -1;

                    /* The key doesn't exist, write the data into the 
                     * buffer. */
                    if (cgdb_key == CGDB_KEY_ERROR) {
                        int j;

                        for (j = 0; j < macro_pos; ++j)
                            local_cgdb_key_array[cgdb_key_array_pos++] =
                                    macro[j];
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
    if (state == KUI_MAP_STATE_MACRO) {
        int j;

        for (j = 0; j < macro_pos; ++j)
            local_cgdb_key_array[cgdb_key_array_pos++] = macro[j];
    }

    local_cgdb_key_array[cgdb_key_array_pos++] = 0;

    free(macro);
    macro = NULL;

    return 0;
}

int kui_term_print_key_array(const int *cgdb_key_array)
{
    int i;

    if (!cgdb_key_array)
        return -1;

    /* Display output buffer */
    fprintf(stderr, "CGDB_KEY_ARRAY(");

    for (i = 0; cgdb_key_array[i] != 0; i++) {
        int is_ckey;

        is_ckey = kui_term_is_cgdb_key(cgdb_key_array[i]);

        if (is_ckey) {
            fprintf(stderr, "%s",
                    kui_term_get_string_from_key(cgdb_key_array[i]));
        } else {
            fprintf(stderr, "%c", cgdb_key_array[i]);
        }
    }

    fprintf(stderr, ")\r\n");

    return 0;
}
