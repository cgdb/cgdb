#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <term.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdarg.h>
#include <ctype.h>

#define MAXLINE 4096

/* The data structure used at runtime to find multi-character sequences. */
struct term_entry {
    char *name;         /* The terminfo name of the key */
    char *description;  /* A human readable description for debugging */
    char *sequence;     /* The multi-char escape sequence for this key */
} *list = NULL;
static int list_size = 0;

/* A temporary variable used for clearing the screen after a number of 
 * writes to it. It is used for debugging the input library
 */
static int counter = 0;

/* This is the enumerated list of multi-char escape sequences that 
 * this library is interested in capturing. Append to the end to add more.
 */
static struct istr { 
    const char *const terminfo;
    const char *const termcap;
    const char *const description;
} istrnames[] = {
    /* Function keys */
    { "kf1",        "k1",       "F1" },
    { "kf2",        "k2",       "F2" },
    { "kf3",        "k3",       "F3" },
    { "kf4",        "k4",       "F4" },
    { "kf5",        "k5",       "F5" },
    { "kf6",        "k6",       "F6" },
    { "kf7",        "k7",       "F7" },
    { "kf8",        "k8",       "F8" },
    { "kf9",        "k9",       "F9" },
    { "kf10",       "k10",      "F10" },
    { "kf11",       "k11",      "F11" },
    { "kf12",       "k12",      "F12" },

    /* insert, delete, home, end, page up, page down */ 
    { "kich1",      "kI",       "INSERT"},
    { "kdch1",      "kD",       "DELETE"},
    { "khome",      "kh",       "HOME"},
    { "kend",       "@7",       "END"},
    { "kpp",        "kP",       "PAGE UP"},
    { "knp",        "kN",       "PAGE DOWN"},

    /* Arrow keys */
    { "cuu1",       "ku",       "CURSOR UP"},
    { "cuf1",       "nd",       "CURSOR RIGHT"},
    { "cub",        "LE",       "CURSOR LEFT"},
    { "cud",        "DO",       "CURSOR DOWN"},

    /* End of list :) */
    { NULL,         NULL,       NULL }
};

/* display_message: This is purly for debugging the library. 
 *                  It writes a message to the screen.
 */
void display_message(const char *fmt, ...) {
    va_list ap;
    char va_buf[MAXLINE];

    /* Get the buffer with format */
    va_start(ap, fmt);
#ifdef   HAVE_VSNPRINTF
    vsnprintf(va_buf, sizeof(va_buf), fmt, ap);  /* this is safe */
#else
    vsprintf(va_buf, fmt, ap);             /* this is not safe */
#endif
    va_end(ap);

    if ( counter > 50 ) {
        mvprintw(counter++, 0, "Hit a key to clear and continue");
        getch();
        clear();
        refresh();
        counter = 0;
    }

    mvprintw(counter++, 0, va_buf);
    refresh();
}

/* display_list: displays the list of accepted multi-char escape sequences.
 *               This function is used for debugging the library.
 */
static void display_list(void) {
    int i, j, length;
    display_message("DISPLAYING LIST\n");
    for ( i = 0; i < list_size; i++ ) {
        display_message("TERMINFO:(%s) DESC(%s) CODE[", list[i].name, list[i].description );

        length = strlen(list[i].sequence);
        for(j = 0; j < length; j++)
            display_message("(%c:%d)", list[i].sequence[j], list[i].sequence[j]);
        display_message("]\n");
        refresh();
    }
}

/* I don't care about the format, I just want to parse it */
static const char *parse_format(const char *s) {
    bool done = FALSE;
    bool allowminus = FALSE;

    while (*s != '\0' && !done) {
        switch (*s) {
        case 'c':       /* FALLTHRU */
        case 'd':       /* FALLTHRU */
        case 'o':       /* FALLTHRU */
        case 'x':       /* FALLTHRU */
        case 'X':       /* FALLTHRU */
        case 's':
            break;
        case '.':
        case '#':
        case ' ':
            s++;
            done = TRUE;
            break;
        case ':':
            s++;
            allowminus = TRUE;
            break;
        case '-':
            if (allowminus)
                s++;
            else 
                done = TRUE;
            break;
        default:
            if (isdigit(((unsigned char)(*s))))
                s++;
            else 
                done = TRUE;
        }
    }

    return s;
}

static void insertIntoList(const char *tname, const char *tdesc, char *codes);

static void modifyInsertIntoList(const char *tname, 
                                const char *tdesc, 
                                char *codes) {
char *cp;
char temp[100];
int temp_pos = 0;
for (cp = codes; *cp!=(char)0;cp++) {
    /* The first item in the list */
    if (*cp == '%' && cp == codes) {
        printw("CAN'T BELEIVE I AM HERE\n"); refresh();
        cp++;
        cp = (char *)parse_format(cp);
    } else if ( *cp == '%' ) {

        cp++;
        switch (*cp) {
        default:
        break;

        case 'd':       /* FALLTHRU */
        case 'o':       /* FALLTHRU */
        case 'x':       /* FALLTHRU */
        case 'X':       /* FALLTHRU */
        case 'c':       /* FALLTHRU */
        case 'l':
        case 's':
        break;

        case 'p':
        case 'P':
        case 'g':
        cp++;
        break;

        case '\'':
        cp += 2;
        break;

        case '{':
        cp++; 
        while (*cp >= '0' && *cp <= '9') {
            cp++;
        }
        break;

        case '+':
        case '-':
        case '*':
        case '/':
        case 'm':
        case 'A':
        case 'O':
        case '&':
        case '|':
        case '^':
        case '=':
        case '<':
        case '>':
        case '!':
        case '~':
        case 'i':
        break;
        }
        continue;
    }
    temp[temp_pos++] = *cp;
}
if (*cp != '\0')
    cp++;

    temp[temp_pos++] = '\0';
//    printw("FOUND(%s:    :%s)\n", tname, temp);
    refresh();
insertIntoList( tname, tdesc, temp);
}

/* This should input in sorted order based on codes */
static void insertIntoList(const char *tname, const char *tdesc, char *codes) {
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
            t--;
        }
    }

    list[entry].name = (char*)malloc(sizeof(char)*((length = strlen(tname) + 1)));
    strncpy(list[entry].name, tname, length);

    list[entry].description = (char*)malloc(sizeof(char)*((length = strlen(tdesc) + 1)));
    strncpy(list[entry].description, tdesc, length);

    list[entry].sequence = (char*)malloc(sizeof(char)*((length = strlen(codes) + 1)));
    strncpy(list[entry].sequence, codes, length);
    
    list_size++;
}

/*  If block is non-zero then, it will block,
 *   if it is 0 then it will not block
 *
 * Returns 0 if no data is ready
 * or the charachter read.
 * or -1 on error 
 */
int raw_read(int fd, int block) {
   char c;
   int ret;
   int flag;

   if ( !block ) {
      flag = fcntl(fd, F_GETFL, 0);
      fcntl(fd, F_SETFL, flag | O_NONBLOCK);
   }
   ret = read(fd, &c, 1);

   if ( ret == -1 && errno == EAGAIN )
      c = 0; 
   else if ( ret == -1 ) {
      c = 0; 
      display_message("Errno(%d)\n", errno);
   } else if ( ret == 0 ) {
      c = 0; 
      ret = -1;
      display_message("Read returned nothing\n");
   }

   if ( !block )
      fcntl(fd, F_SETFL, flag);

   if ( ret == -1 )
       return -1;

   return c;
}

/* get_esc_sequence: Gets the next esc sequence from fd. This function assumes
 *                   that ESC has already been read from fd.
 *
 *  fd -> the file descriptor that will be read for the next char.
 *
 *  return -> 0     on EOF
 *            -1    on ERROR
 */
int get_esc_sequence(int fd) {
    int i, j = 0; 
    int possible[list_size];
    int c;
    char bad_list[100];
    for ( i = 0; i < list_size; i++ )
        possible[i] = 1;

    if ( (c = raw_read(0, 0)) == -1 ) {/* No data ready (Got esc key) */
        display_message("GOT ESC\n");
        return 0;
    }

    do {
        bad_list[j] = c;
        j++;
        for ( i = 0; i < list_size; i++) { /* for each mapping */
            if ( possible[i] && list[i].sequence[j] == c ) {
                if ( j + 1== strlen(list[i].sequence) ) {/* Found match */
                    display_message("TERMINFO:(%s) DESC(%s)\n", list[i].name, list[i].description);
                    return 0;
                }
            } else
                possible[i] = 0;
        }
    } while ( (c = raw_read(0, 0)) != -1 ); /* No data ready and no match ( return everything ) */
    
    display_message("NO MATCH, INPUT WAS GARBAGE\n");
    for ( i = 0 ; i < j; i++ )
        display_message("(%c:%d)", bad_list[i], bad_list[i]);
    display_message("\n");
    
    return 0;
}

/* cgetch: Gets the next character the user pressed. It will translate ESC
 *         sequences and return a single define describing the char pressed.
 *
 *  fd -> the file descriptor that will be read for the next char.
 *
 *  return -> The last char read. Either literally the char, or a define 
 *            describing a multi character key. -1 on error.
 */
int cgetch(int fd) {
    char c;

    if ( (c = raw_read(fd, 1)) == 0 ) {
        return -1;
    } else if ( c == 27 ) {
        return get_esc_sequence(fd);
    }
    
    return c;
}

/* capture_chars: internal command to test the capturing of char's. 
 *                It is basically a driver to test the functionality of the lib
 */ 
void capture_chars(int fd) {
    char c;
    while( ( c = cgetch(fd)) != 'q' ) {
        if ( c != 0 ) {
            display_message("(%c:%d)\n", c, c);
        }
    }
}

/* initialize_list: Traverses the list istrnames, and puts the 
 *                  name/desc/code into the global list.
 */
void initialize_list(void) {
    int i, size = 0, length, j;
    char *code;

    for ( i = 0;  istrnames[i].terminfo != (char *)0;  i++ )
        size++;
    
    /* Allocating memory for list */
    list = (struct term_entry *)malloc(sizeof(struct term_entry)*size*2 + 1);
    display_message("size(%d)\n", size);

    for ( i = 0; i < size;  i++ ) {
        if ( (code = tigetstr(istrnames[i].terminfo)) == 0 ) {
            display_message("CAPNAME (%s) is not present in this TERM's description\n", istrnames[i].terminfo);
            continue;
        } else if (code == (char*)-1 ) {
            display_message("CAPNAME (%s) is not a string capability\n", istrnames[i].terminfo);
            continue;
        }

        display_message("TERMINFO CAPNAME:(%s) DESCRIPTION(%s) successful", istrnames[i].terminfo, istrnames[i].description );
        length = strlen(code);
        for(j = 0; j < length; j++)
            display_message("(%c:%d)", code[j], code[j]);
        display_message("]\n");
        refresh();
        modifyInsertIntoList(istrnames[i].terminfo, istrnames[i].description, code);

        if ( (code = tgetstr(istrnames[i].termcap, NULL)) == 0 ) {
            display_message("CAPNAME (%s) is not present in this TERM's description\n", istrnames[i].termcap);
            continue;
        } else if (code == (char*)-1 ) {
            display_message("CAPNAME (%s) is not a string capability\n", istrnames[i].termcap);
            continue;
        }

        display_message("TERMCAP CAPNAME:(%s) DESCRIPTION(%s) successful", istrnames[i].termcap, istrnames[i].description );
        length = strlen(code);
        for(j = 0; j < length; j++)
            display_message("(%c:%d)", code[j], code[j]);
        display_message("]\n");
        refresh();
        modifyInsertIntoList(istrnames[i].termcap, istrnames[i].description, code);
    }
    display_message("DONE");
}

int main(int argc, char **argv){
    setbuf(stdout, NULL);
    initscr();                       /* Start curses mode */
    noecho();                        /* Do not echo characters typed by user */
    raw();                        /* Line buffering disabled */
    refresh();                       /* Refresh the initial window once */
    initialize_list();
    display_list();
    capture_chars(0);
    echo();
    endwin();
    return 0;
}
