#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <term.h>
#include <string.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdarg.h>
#include <ctype.h>
#include "util.h"
#include "input.h"
#include "error.h"

#define MAXLINE 4096
#define MAX_SEQ_LIST_SIZE 8

/* This contains all of the ESC sequences this unit cares about. 
 * It contains the correct information to get esc sequences out of both
 * termcap and terminfo.
 *
 * tname        - The termcap capability name
 * tiname       - The terminfo capability name
 * description  - Human readable description about the capability name
 * tcodes       - The termcap esc sequence associated with tname
 * ticodes      - The terminfo esc sequence associated with tiname
 */
struct tlist {
    char *tname;
    char *tiname;
    char *description;
    char *tcodes;
    char *ticodes;
    enum cgdb_input_macro macro;
} seqlist[] = {
  { "@7", "kend",   "End Key",                  NULL, NULL, CGDB_KEY_END },
  { "kh", "khome",  "Home key",                 NULL, NULL, CGDB_KEY_HOME },
  { "kH", "kll",    "Home down",                NULL, NULL, CGDB_KEY_HOME },
  { "dc", "dch1",   "Delete",                   NULL, NULL, CGDB_KEY_DC },
  { "kD", "kdch1",  "Delete",                   NULL, NULL, CGDB_KEY_DC },
  { "ic", "ich1",   "Insert",                   NULL, NULL, CGDB_KEY_IC },
  { "kI", "kich1",  "Insert",                   NULL, NULL, CGDB_KEY_IC },
  { "kN", "knp",    "next page",                NULL, NULL, CGDB_KEY_NPAGE },
  { "kP", "kpp",    "previous page",            NULL, NULL, CGDB_KEY_PPAGE },

  /* For arrow keys */
  { "kd", "kcud1",  "Down arrow key",           NULL, NULL, CGDB_KEY_DOWN },
  { "kl", "kcub1",  "Left arrow key",           NULL, NULL, CGDB_KEY_LEFT },
  { "kr", "kcuf1",  "Right arrow key",          NULL, NULL, CGDB_KEY_RIGHT },
  { "ku", "kcuu1",  "Up arrow key",             NULL, NULL, CGDB_KEY_UP },
  { "le", "cub1",   "Move left one space",      NULL, NULL, CGDB_KEY_LEFT },
  { "nd", "cuf1",   "Move right one space",     NULL, NULL, CGDB_KEY_RIGHT },
  { "up", "cuu1",   "Up one line",              NULL, NULL, CGDB_KEY_UP },

  /* Function keys */
  { "k1", "kf1",    "F1 function key",          NULL, NULL, CGDB_KEY_F1 },
  { "k2", "kf2",    "F2 function key",          NULL, NULL, CGDB_KEY_F2 },
  { "k3", "kf3",    "F3 function key",          NULL, NULL, CGDB_KEY_F3 },
  { "k4", "kf4",    "F4 function key",          NULL, NULL, CGDB_KEY_F4 },
  { "k5", "kf5",    "F5 function key",          NULL, NULL, CGDB_KEY_F5 },
  { "k6", "kf6",    "F6 function key",          NULL, NULL, CGDB_KEY_F6 },
  { "k7", "kf7",    "F7 function key",          NULL, NULL, CGDB_KEY_F7 },
  { "k8", "kf8",    "F8 function key",          NULL, NULL, CGDB_KEY_F8 },
  { "k9", "kf9",    "F9 function key",          NULL, NULL, CGDB_KEY_F9 },
  { "k;", "kf10",   "F10 function key",         NULL, NULL, CGDB_KEY_F10 },
  { "F1", "kf11",   "F11 function key",         NULL, NULL, CGDB_KEY_F11 },
  { "F2", "kf12",   "F12 function key",         NULL, NULL, CGDB_KEY_F12 },
  { NULL, NULL,     NULL,                       NULL, NULL, CGDB_KEY_ERROR }
};

struct term_entry {
    char *name;
    char *description;
    char *sequence; 
    enum cgdb_input_macro macro;
} *list = NULL;
static int list_size = 0;

struct input {
    int stdinfd;
    struct term_entry *last_entry;
    char bad_esc_seq[MAX_SEQ_LIST_SIZE];
    int bad_esc_seq_counter;
    int bad_esc_seq_size;
};

/* insertIntoList: This should input in sorted order based on codes 
 *                 This function should be replaced with something smarter.
 */
static void insertIntoList(const char *tname, const char *tdesc, char *codes, enum cgdb_input_macro macro) {
    int length = strlen(codes), i;
    int entry = 0;

    while (entry < list_size ) {
        int seq_len = strlen(list[entry].sequence);
        int max = (length > seq_len)?seq_len:length;
        for ( i = 0; i < max; i++ ) {
            if ( codes[i] < list[entry].sequence[i] )
                goto finished;
            else if ( codes[i] == list[entry].sequence[i]) { 
                if ((i == max - 1) && (length < seq_len))
                    goto finished;
                else 
                    continue;
            } else 
                break;
        }
        entry++;
    }

finished:
    /* Must move everything down */
    if ( entry < list_size) {
        int t = list_size;
        for ( i = 0; i < list_size - entry; i++ ) {
            list[t].name = list[t - 1].name;
            list[t].description = list[t - 1].description;
            list[t].sequence = list[t - 1].sequence;
            list[t].macro = list[t - 1].macro;
            t--;
        }
    }

    list[entry].name = (char*)malloc(sizeof(char)*((length = strlen(tname) + 1)));
    strncpy(list[entry].name, tname, length);

    list[entry].description = (char*)malloc(sizeof(char)*((length = strlen(tdesc) + 1)));
    strncpy(list[entry].description, tdesc, length);

    list[entry].sequence = (char*)malloc(sizeof(char)*((length = strlen(codes) + 1)));
    strncpy(list[entry].sequence, codes, length);

    list[entry].macro = macro;
    
    list_size++;
}

/* add_keybindings: This adds special key bindings that many terminals use. */
static void add_keybindings(void) {
    insertIntoList ("Up arrow",     "Up arrow",     "\033[0A", CGDB_KEY_UP);
    insertIntoList ("Left arrow",   "Left arrow",   "\033[0B", CGDB_KEY_LEFT);
    insertIntoList ("Right arrow",  "Right arrow",  "\033[0C", CGDB_KEY_RIGHT);
    insertIntoList ("Down arrow",   "Down arrow",   "\033[0D", CGDB_KEY_DOWN);

    insertIntoList ("Up arrow",     "Up arrow",     "\033[A", CGDB_KEY_UP);
    insertIntoList ("Down arrow",   "Down arrow",   "\033[B", CGDB_KEY_DOWN);
    insertIntoList ("Right arrow",  "Right arrow",  "\033[C", CGDB_KEY_RIGHT);
    insertIntoList ("Left arrow",   "Left arrow",   "\033[D", CGDB_KEY_LEFT);
    insertIntoList ("Home",         "Home",         "\033[H", CGDB_KEY_HOME);
    insertIntoList ("End",          "End",          "\033[F", CGDB_KEY_END);

    insertIntoList ("Up arrow",     "Up arrow",     "\033OA", CGDB_KEY_UP);
    insertIntoList ("Down arrow",   "Down arrow",   "\033OB", CGDB_KEY_DOWN);
    insertIntoList ("Right arrow",  "Right arrow",  "\033OC", CGDB_KEY_RIGHT);
    insertIntoList ("Left arrow",   "Left arrow",   "\033OD", CGDB_KEY_LEFT);
    insertIntoList ("Home",         "Home",         "\033OH", CGDB_KEY_HOME);
    insertIntoList ("End",          "End",          "\033OF", CGDB_KEY_END);

    /* Alt bindings */
    insertIntoList ("Alt-A",        "Alt-A",        "\033a", CGDB_KEY_ALT_A);
    insertIntoList ("Alt-B",        "Alt-B",        "\033b", CGDB_KEY_ALT_B);
    insertIntoList ("Alt-C",        "Alt-C",        "\033c", CGDB_KEY_ALT_C);
    insertIntoList ("Alt-D",        "Alt-D",        "\033d", CGDB_KEY_ALT_D);
    insertIntoList ("Alt-E",        "Alt-E",        "\033e", CGDB_KEY_ALT_E);
    insertIntoList ("Alt-F",        "Alt-F",        "\033f", CGDB_KEY_ALT_F);
    insertIntoList ("Alt-G",        "Alt-G",        "\033g", CGDB_KEY_ALT_G);
    insertIntoList ("Alt-H",        "Alt-H",        "\033h", CGDB_KEY_ALT_H);
    insertIntoList ("Alt-I",        "Alt-I",        "\033i", CGDB_KEY_ALT_I);
    insertIntoList ("Alt-J",        "Alt-J",        "\033j", CGDB_KEY_ALT_J);
    insertIntoList ("Alt-K",        "Alt-K",        "\033k", CGDB_KEY_ALT_K);
    insertIntoList ("Alt-L",        "Alt-L",        "\033l", CGDB_KEY_ALT_L);
    insertIntoList ("Alt-M",        "Alt-M",        "\033m", CGDB_KEY_ALT_M);
    insertIntoList ("Alt-N",        "Alt-N",        "\033n", CGDB_KEY_ALT_N);
    insertIntoList ("Alt-O",        "Alt-O",        "\033o", CGDB_KEY_ALT_O);
    insertIntoList ("Alt-P",        "Alt-P",        "\033p", CGDB_KEY_ALT_P);
    insertIntoList ("Alt-Q",        "Alt-Q",        "\033q", CGDB_KEY_ALT_Q);
    insertIntoList ("Alt-R",        "Alt-R",        "\033r", CGDB_KEY_ALT_R);
    insertIntoList ("Alt-S",        "Alt-S",        "\033s", CGDB_KEY_ALT_S);
    insertIntoList ("Alt-T",        "Alt-T",        "\033t", CGDB_KEY_ALT_T);
    insertIntoList ("Alt-U",        "Alt-U",        "\033u", CGDB_KEY_ALT_U);
    insertIntoList ("Alt-V",        "Alt-V",        "\033v", CGDB_KEY_ALT_V);
    insertIntoList ("Alt-W",        "Alt-W",        "\033w", CGDB_KEY_ALT_W);
    insertIntoList ("Alt-X",        "Alt-X",        "\033x", CGDB_KEY_ALT_X);
    insertIntoList ("Alt-Y",        "Alt-Y",        "\033y", CGDB_KEY_ALT_Y);
    insertIntoList ("Alt-Z",        "Alt-Z",        "\033z", CGDB_KEY_ALT_Z);
}

/* Puts list into a searchable database */
static void store_list(void) {
    int size = 100, i = 0;

    /* Allocate for all the entries, plus 1 for the null termination */
    list = (struct term_entry *)malloc(sizeof(struct term_entry)*size + 1);

    /* Twice to save the data */
    for( i = 0; seqlist[i].tname != NULL; i++) {
        if ( seqlist[i].tcodes != NULL )
            insertIntoList(seqlist[i].tname, seqlist[i].description, seqlist[i].tcodes, seqlist[i].macro);

        if ( seqlist[i].ticodes != NULL )
            insertIntoList(seqlist[i].tiname, seqlist[i].description, seqlist[i].ticodes, seqlist[i].macro);
    }
}

/* Gets a single key sequence */
static int import_keyseq(struct tlist *i) {
    char *terminfo, *termcap;
    int ret;

    char *env = getenv("TERM");

    if ( !env ) {
        fprintf(stderr, "%s:%d TERM not set error", __FILE__, __LINE__);
        return -1;
    }
    
    if ( ( ret = tgetent(NULL, env)) == 0 ) {
        fprintf(stderr, "%s:%d tgetent 'No such entry' error", __FILE__, __LINE__);
        return -1;
    } else if ( ret == -1 ) {
        fprintf(stderr, "%s:%d tgetent 'terminfo database could not be found' error", __FILE__, __LINE__);
        return -1;
    }
    
    /* Set up the termcap seq */ 
    if ( (termcap = tgetstr(i->tname, NULL)) == 0 )
        err_msg("CAPNAME (%s) is not present in this TERM's termcap description\n", i->tname);
    else if (termcap == (char*)-1 )
        err_msg("CAPNAME (%s) is not a termcap string capability\n", i->tname);
    else
        i->tcodes = strdup(termcap);

    /* Set up the terminfo seq */ 
    if ( (terminfo = tigetstr(i->tiname)) == 0 )
        err_msg("CAPNAME (%s) is not present in this TERM's terminfo description\n", i->tiname);
    else if (terminfo == (char*)-1 )
        err_msg("CAPNAME (%s) is not a terminfo string capability\n", i->tiname);
    else
        i->ticodes = strdup(terminfo);

    return 0;
}

/* Binds all of the key sequences */
static void import_keyseqs(void) {
    int i;
    /* strings */
    for( i = 0; seqlist[i].tname != NULL; i++)
        import_keyseq(&seqlist[i]);
}

/*  input_read: Will read the next char from fd.
 *
 *  block - If non-zero then it will block with no data available, 
 *          If 0 then it will not block if no data is available.
 *
 * Returns:     -1 on error 
 *              0 if no data is ready
 *              or the char read.
 */
int input_read(int fd, int block) {
    char c;
    int ret;
    int flag;

#if defined(HAVE_SELECT)
    fd_set readfds, exceptfds;
    struct timeval timeout;
#endif

#if defined(HAVE_SELECT)
    FD_ZERO(&readfds);
    FD_ZERO (&exceptfds);
    FD_SET (fd, &readfds);
    FD_SET (fd, &exceptfds);
    
    /* Only do select if we are blocking */
    if ( !block ) {
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;   /* 0.1 seconds; it's in usec */

        ret = select (fd + 1, &readfds, (fd_set *)NULL, &exceptfds, &timeout);

        if (ret <= 0)
            return 0;   /* Nothing to read. */
    }
#endif

    /* Set nonblocking */
    if ( !block ) {
        flag = fcntl(fd, F_GETFL, 0);
        fcntl(fd, F_SETFL, flag | O_NONBLOCK);
    }

read_again:

    /* Read a char */
    ret = read(fd, &c, 1);

    if ( ret == -1 && errno == EAGAIN )
        c = 0;  /* No data available */
    else if ( ret == -1 && errno == EINTR )
        goto read_again;
    else if ( ret == -1 ) {
        c = 0; 
        err_msg("Errno(%d)\n", errno);
    } else if ( ret == 0 ) {
        c = 0; 
        ret = -1;
        err_msg("Read returned nothing\n"); 
    }

    /* Set to original state */
    if ( !block )
        fcntl(fd, F_SETFL, flag);

    if ( ret == -1 )
        return -1;

    return c;
}

/* input_get_seq:
 *
 * Returns: -1 on error
 *          -2 if no data was ready
 *          -3 if bad sequence was found
 *          > 0 if macro was found and returned.
 */
static int input_get_seq(struct input *input) {
    int i, j = 0; 
    int possible[list_size];
    int c;
    int still_possible;

    if ( (c = input_read(input->stdinfd, 0)) == 0 )
        return -2;   /* No data ready (Got esc key) */

    /* Initalize all possible esq sequences to be a possible match */
    for ( i = 0; i < list_size; i++ )
        possible[i] = 1;

    do {
        still_possible = 0;
        input->bad_esc_seq[j++] = c;
        for ( i = 0; i < list_size; i++) { /* for each mapping */
            if ( possible[i] && list[i].sequence[j] == c ) {
                if ( j + 1== strlen(list[i].sequence) ) {/* Found match */
                    /* Save last sequence found */
                    input->last_entry = &list[i];
                    return input->last_entry->macro;
                }
                still_possible = 1;
            } else
                possible[i] = 0;
        }

        if ( !still_possible )
            break;

        c = input_read(input->stdinfd, 0);

        /* Bad escape sequence */
        if ( c == 0 )
            break;
    } while ( c != -1 ); /* No data ready and no match ( return everything ) */
    
    /* Assertion: The sequence did not match anything of interest */
    input->bad_esc_seq_size = j - 1;
    input->bad_esc_seq_counter = 1;

    /* Return the first bad esc seq */
    return input->bad_esc_seq[0];
}

/* input_getch: Gets the next character or cgdb relevant escape sequence.
 *
 * i       - The context to read the next char from.
 *
 * Returns -1 on error, or key pressed, or macro for esc sequence.
 */
static int input_getch(struct input *i) {
    char c;

    /* Write out the bad esc sequence read in previously */
    if ( i->bad_esc_seq_counter != -1 ) {
        if ( i->bad_esc_seq[i->bad_esc_seq_counter] == i->bad_esc_seq_size )
            i->bad_esc_seq_counter = -1;
        else
            return i->bad_esc_seq[(i->bad_esc_seq_counter)++];
    }

    /* We use a blocking read here because we know at least one byte is ready.
     * the input library knows this because it is only called when select was 
     * triggered 
     */
    if ( (c = input_read(i->stdinfd, 1)) <= 0 ) {
        return -1;                                  /* On error */
    } else if ( c == 27 ) {
        int result;
        if ( ( result = input_get_seq(i)) == -1 )    /* On macro Esc sequence */
            return -1;
        else if ( result == -2 )
            return c;  /* Found only the esc key */
        else if ( result > 0 && i->bad_esc_seq_counter == -1)
            return i->last_entry->macro;
    }
    
    return c;                                       /* On regular key */
}

struct input *input_init(int stdinfd) {
    struct input *i = (struct input *)xmalloc(sizeof(struct input));
    i->stdinfd = stdinfd;
    i->last_entry = NULL;
    i->bad_esc_seq[0] = '\0';
    i->bad_esc_seq_counter = -1;

    import_keyseqs();
    /*print_list();*/

    store_list();
    add_keybindings();

    /*display_list();*/

    return i;
}

int input_getkey(struct input *i) {
    return input_getch(i);
}

char *input_get_last_seq(struct input *i) {
    return i->last_entry->sequence;
}

char *input_get_last_seq_name(struct input *i) {
    return i->last_entry->description;
}
