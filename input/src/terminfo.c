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

enum term_entry_type {
    FLAG = 0,
    NUM,
    STR
};

struct term_entry {
    enum term_entry_type type;
    char *name;
    char *description;
    char *sequence; 
} *list = NULL;
static int list_size = 0;

static int counter = 0;

#if 0
const char *const*istrnames = strnames;
const char *const*istrcodes = strcodes;
const char *const*istrfnames = strfnames;
#else

#define SIZE 23
struct istr { 
    const char *const istrname;
    const char *const description;
} istrnames[] = {
    { "kf1", "F1" },
    { "kf2", "F2" },
    { "kf3", "F3" },
    { "kf4", "F4" },
    { "kf5", "F5" },
    { "kf6", "F6" },
    { "kf7", "F7" },
    { "kf8", "F8" },
    { "kf9", "F9" },
    { "kf10", "F10" },
    { "kf11", "F11" },
    { "kf12", "F12" },
    { "kich1", "INSERT"},
    { "kdch1", "DELETE"},
    { "khome", "HOME"},
    { "kend", "END"},
    { "kpp", "PAGE UP"},
    { "knp", "PAGE DOWN"},

    /* Arrow keys */
    { "cuu1", "CURSOR UP"},
    { "cuf1", "CURSOR RIGHT"},
    { "cub", "CURSOR LEFT"},
    { "cud", "CURSOR DOWN"},

    { NULL, NULL }
};

#endif

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
        clear();
        refresh();
        counter = 0;
    }

    mvprintw(counter++, 0, va_buf);
    refresh();
}

#if 0
static void print_single_flag(const char *terminfoname, const char *termcapname, const char *description) {
    int terminfo;
    if ( (terminfo = tigetflag(terminfoname)) == 0 ) {
        //display_message("CAPNAME (%s) is not present in this TERM's description\n", terminfoname);
        return;
    } else if (terminfo == -1 ) {
        //display_message("CAPNAME (%s) is not a boolean capability\n", terminfoname);
        return;
    }

    display_message("TERMINFO:(%s) TERMCAP(%s) DESC(%s) CODE(%d)\n", terminfoname, termcapname,description, terminfo );
}

static void print_single_num(const char *terminfoname, const char *termcapname, const char *description) {
    int terminfo;
    if ( (terminfo = tigetnum(terminfoname)) == -1 ) {
        //display_message("CAPNAME (%s) is not present in this TERM's description\n", terminfoname);
        return;
    } else if (terminfo == -2 ) {
        //display_message("CAPNAME (%s) is not a number capability\n", terminfoname);
        return;
    }

    display_message("TERMINFO:(%s) TERMCAP(%s) DESC(%s) CODE(%d)\n", terminfoname, termcapname,description, terminfo );
}

static void print_single_str(const char *terminfoname, const char *termcapname, const char *description) {
    int i, length;
    char *terminfo;
    if ( (terminfo = tigetstr(terminfoname)) == 0 ) {
        //display_message("CAPNAME (%s) is not present in this TERM's description\n", terminfoname);
        return;
    } else if (terminfo == (char*)-1 ) {
        //display_message("CAPNAME (%s) is not a string capability\n", terminfoname);
        return;
    }

    display_message("TERMINFO:(%s) TERMCAP(%s) DESC(%s) CODE[", terminfoname, termcapname,description );

    length = strlen(terminfo);
    for(i = 0; i < length; i++)
        display_message("(%d)", terminfo[i]);
    display_message("]\n");
}

static void display_item(const char * name, const char *desc, const char *codes){
    int i, length;
    display_message("TERMINFO:(%s) DESC(%s) CODE[", name, desc);

    length = strlen(codes);
    for(i = 0; i < length; i++)
        display_message("(%d)", codes[i]);
    display_message("]\n");
}

#endif

static void display_list(void) {
    int i, j, length;
    display_message("DISPLAYING LIST\n");
    for ( i = 0; i < list_size; i++ ) {
        display_message("TERMINFO:(%s) DESC(%s) CODE[", list[i].name, list[i].description );

        length = strlen(list[i].sequence);
        for(j = 0; j < length; j++)
            display_message("(%d)", list[i].sequence[j]);
        display_message("]\n");
        refresh();
    }
}

/* I don't care about the format, I just want to parse it */
static const char *parse_format(const char *s, char *format, int *len) {
    bool done = FALSE;
    bool allowminus = FALSE;
    bool err = FALSE;
    int value = 0;

    *len = 0;
    *format++ = '%';
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
        s++;
        break;
    case '#':
        s++;
        break;
    case ' ':
        s++;
        break;
    case ':':
        s++;
        allowminus = TRUE;
        break;
    case '-':
        if (allowminus)
            s++;
        break;
    default:
        if (isdigit(((unsigned char)(*s)))) {
        value = (value * 10) + (*s - '0');
        if (value > 10000)
            err = TRUE;
        s++;
        } 
    }
    }
    printw("JUST LEFT PARSE_FORMAT\n");

    return s;
}

static void insertIntoList(enum term_entry_type t, const char *tname, const char *tdesc, char *codes);

static void modifyInsertIntoList(enum term_entry_type t, 
                                const char *tname, 
                                const char *tdesc, 
                                char *codes) {
char *cp;
static char *format;
int len;
char temp[100];
int temp_pos = 0;
for (cp = codes; *cp!=(char)0;cp++) {
    /* The first item in the list */
    if (*cp == '%' && cp == codes) {
        printw("CAN'T BELEIVE I AM HERE\n"); refresh();
        cp++;
        cp = (char *)parse_format(cp, format, &len);
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
insertIntoList( t, tname, tdesc, temp);
}

/* This should input in sorted order based on codes */
static void insertIntoList(enum term_entry_type t, const char *tname, const char *tdesc, char *codes) {
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
            list[t].type = list[t - 1].type;
            list[t].name = list[t - 1].name;
            list[t].description = list[t - 1].description;
            list[t].sequence = list[t - 1].sequence;
            t--;
        }
    }
    list[entry].type = t;

    list[entry].name = (char*)malloc(sizeof(char)*((length = strlen(tname) + 1)));
    strncpy(list[entry].name, tname, length);

    list[entry].description = (char*)malloc(sizeof(char)*((length = strlen(tdesc) + 1)));
    strncpy(list[entry].description, tdesc, length);

    list[entry].sequence = (char*)malloc(sizeof(char)*((length = strlen(codes) + 1)));
    strncpy(list[entry].sequence, codes, length);
    
    list_size++;
}

#if 0

static void store_list(void) {
    int size = 0, i = 0;
    char *str;

    /* Once to get the size */
    for( i = 0; istrnames[i] != NULL; i++)
        if ( tigetstr(istrnames[i]) > 0 )
            size++;

    /* Allocate for all the entries, plus 1 for the null termination */
      list = (struct term_entry *)malloc(sizeof(struct term_entry)*size + 1);
      display_message("SIZE(%d)\n", SIZE);
    size = 0;

    /* Twice to save the data */
    for( i = 0; istrnames[i] != NULL; i++)
        if ( ( str = tigetstr(istrnames[i])) > 0 )
            modifyInsertIntoList(STR, istrnames[i], istrfnames[i], str);
}

static void print_list(void) {
    int i;

    /* strings */
    for( i = 0; istrnames[i] != NULL; i++)
        print_single_str(istrnames[i], istrcodes[i], istrfnames[i]);

    /* numbers */
    for( i = 0; numnames[i] != NULL; i++)
        print_single_num(numnames[i], numcodes[i], numfnames[i]);
    
    /* flags */
    for( i = 0; boolnames[i] != NULL; i++)
        print_single_flag(boolnames[i], boolcodes[i], boolfnames[i]);
}
#endif

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

int getEscSequence(int fd) {
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
        display_message("(%d)", bad_list[i]);
    display_message("\n");
    
    return 0;
}

int tgetch(int fd) {
    char c;

    if ( (c = raw_read(fd, 1)) == 0 ) {
        return -1;
    } else if ( c == 27 ) {
        return getEscSequence(fd);
    }
    
    return c;
}

void capture_chars(int fd) {
    char c;
    while( ( c = tgetch(fd)) != 'q' ) {
        if ( c != 0 ) {
            display_message("(%c:%d)\n", c, c);
        }
    }
}


void initialize_list(void) {
    int i;
    char *code;
    
    /* Allocating memory for list */
    list = (struct term_entry *)malloc(sizeof(struct term_entry)*SIZE + 1);
    display_message("SIZE(%d)\n", SIZE);

    for ( i = 0; i < SIZE && istrnames[i].istrname != (char *)0;  i++ ) {
        if ( (code = tigetstr(istrnames[i].istrname)) == 0 ) {
            display_message("CAPNAME (%s) is not present in this TERM's description\n", istrnames[i].istrname);
            return;
        } else if (code == (char*)-1 ) {
            display_message("CAPNAME (%s) is not a string capability\n", istrnames[i].istrname);
            return;
        }

        display_message("TERMINFO CAPNAME:(%s) DESCRIPTION(%s) successful", istrnames[i].istrname, istrnames[i].description );
        modifyInsertIntoList(STR, istrnames[i].istrname, istrnames[i].description, code);
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
