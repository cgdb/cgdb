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
#include "scroller.h"
#include "cgdb.h"

static struct scroller *top_win = NULL;

static void if_print(struct scroller *s, const char *buf ) {
    /* Print it to the scroller */
    scr_add(s, buf);
    scr_refresh(s, 1);
    /*wrefresh(src_win->win);*/
}

/* This contains all of the ESC sequences this library cares about. */
struct tlist {
    char *tname;
    char *tiname;
    char *description;
    char *tcodes;
    char *ticodes;
} seqlist[] = {
  { "@7", "kend",   "End Key",                  NULL, NULL },
  { "kh", "khome",  "Home key",                 NULL, NULL },
  { "kH", "kll",    "Home down",                NULL, NULL },
  { "cr", "cr",     "Carriage return",          NULL, NULL },
  { "dc", "dch1",   "Delete",                   NULL, NULL },
  { "kD", "kdch1",  "Delete",                   NULL, NULL },
  { "ic", "ich1",   "Insert",                   NULL, NULL },
  { "kI", "kich1",  "Insert",                   NULL, NULL },
  { "kN", "knp",    "next page",                NULL, NULL },
  { "kP", "kpp",    "previous page",            NULL, NULL },

  /* For arrow keys */
  { "kd", "kcud1",  "Down arrow key",           NULL, NULL },
  { "kl", "kcub1",  "Left arrow key",           NULL, NULL },
  { "kr", "kcuf1",  "Right arrow key",          NULL, NULL },
  { "ku", "kcuu1",  "Up arrow key",             NULL, NULL },
  { "le", "cub1",   "Move left one space",      NULL, NULL },
  { "nd", "cuf1",   "Move right one space",     NULL, NULL },
  { "up", "cuu1",   "Up one line",              NULL, NULL },

  /* Function keys */
  { "k0", "kf0",    "F0 function key",          NULL, NULL },
  { "k1", "kf1",    "F1 function key",          NULL, NULL },
  { "k2", "kf2",    "F2 function key",          NULL, NULL },
  { "k3", "kf3",    "F3 function key",          NULL, NULL },
  { "k4", "kf4",    "F4 function key",          NULL, NULL },
  { "k5", "kf5",    "F5 function key",          NULL, NULL },
  { "k6", "kf6",    "F6 function key",          NULL, NULL },
  { "k7", "kf7",    "F7 function key",          NULL, NULL },
  { "k8", "kf8",    "F8 function key",          NULL, NULL },
  { "k9", "kf9",    "F9 function key",          NULL, NULL },
  { "k;", "kf10",   "F10 function key",         NULL, NULL },
  { "F1", "kf11",   "F11 function key",         NULL, NULL },
  { "F2", "kf12",   "F12 function key",         NULL, NULL },
  { NULL, NULL,     NULL,                       NULL, NULL }
};


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

    if_print(top_win, va_buf);
}

static void print_seq(struct tlist *item ) {
                        
    int i, length;
    
    if ( item->tcodes != NULL && item->ticodes != NULL )
        display_message("DESCRIPTION(%s)", item->description);

    if ( item->tcodes != NULL ) {
        display_message("TERMCAP NAME(%s)", item->tname);
        display_message("CODE[");

        length = strlen(item->tcodes);
        for(i = 0; i < length; i++)
            display_message("(%d)", item->tcodes[i]);
        display_message("]\n");
    }

    if ( item->ticodes != NULL ) {
        display_message("TERMINFO NAME(%s)", item->tiname);
        display_message("CODE[");

        length = strlen(item->ticodes);
        for(i = 0; i < length; i++)
            display_message("(%d)", item->ticodes[i]);
        display_message("]\n");
    }
}

struct term_entry {
    char *name;
    char *description;
    char *sequence; 
} *list = NULL;
static int list_size = 0;

static void print_list_item(struct term_entry *item) {
    int i, length;
    display_message("TERMINFO NAME(%s)", item->name);
    display_message("DESCRIPTION(%s)", item->description);
    display_message("CODE[");

    length = strlen(item->sequence);
    for(i = 0; i < length; i++)
        display_message("(%d)", item->sequence[i]);
    display_message("]\n");
}

//static void display_item(const char * name, const char *desc, const char *codes){
//    int i, length;
//    display_message("TERMINFO:(%s) DESC(%s) CODE[", name, desc);
//
//    length = strlen(codes);
//    for(i = 0; i < length; i++)
//        display_message("(%d)", codes[i]);
//    display_message("]\n");
//}

static void display_list(void) {
    int i;
    for ( i = 0; i < list_size; i++ )
        print_list_item(&list[i]);
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

static void add_keybindings(void) {
    insertIntoList ("Up arrow",     "Up arrow",     "\033[0A");
    insertIntoList ("Left arrow",   "Left arrow",   "\033[0B");
    insertIntoList ("Right arrow",  "Right arrow",  "\033[0C");
    insertIntoList ("Down arrow",   "Down arrow",   "\033[0D");

    insertIntoList ("Up arrow",     "Up arrow",     "\033[A");
    insertIntoList ("Down arrow",   "Down arrow",   "\033[B");
    insertIntoList ("Right arrow",  "Right arrow",  "\033[C");
    insertIntoList ("Left arrow",   "Left arrow",   "\033[D");
    insertIntoList ("Home",         "Home",         "\033[H");
    insertIntoList ("End",          "End",          "\033[F");

    insertIntoList ("Up arrow",     "Up arrow",     "\033OA");
    insertIntoList ("Down arrow",   "Down arrow",   "\033OB");
    insertIntoList ("Right arrow",  "Right arrow",  "\033OC");
    insertIntoList ("Left arrow",   "Left arrow",   "\033OD");
    insertIntoList ("Home",         "Home",         "\033OH");
    insertIntoList ("End",          "End",          "\033OF");
}

/* Puts list into a searchable database */
static void store_list(void) {
    int size = 100, i = 0;

    /* Allocate for all the entries, plus 1 for the null termination */
    list = (struct term_entry *)malloc(sizeof(struct term_entry)*size + 1);

    /* Twice to save the data */
    for( i = 0; seqlist[i].tname != NULL; i++) {
        if ( seqlist[i].tcodes != NULL )
            insertIntoList(seqlist[i].tname, seqlist[i].description, seqlist[i].tcodes);

        if ( seqlist[i].ticodes != NULL )
            insertIntoList(seqlist[i].tiname, seqlist[i].description, seqlist[i].ticodes);
    }
}

/* Prints all of the key sequences */
static void print_list(void) {
    int i;
    /* strings */
    for( i = 0; seqlist[i].tname != NULL; i++)
        print_seq(&seqlist[i]);
}

/* Gets a single key sequence */
static void import_keyseq(struct tlist *i) {
    char *terminfo, *termcap;
    /* Set up the termcap seq */ 
    if ( (termcap = tgetstr(i->tname, NULL)) == 0 )
        display_message("CAPNAME (%s) is not present in this TERM's termcap description\n", i->tname);
    else if (termcap == (char*)-1 )
        display_message("CAPNAME (%s) is not a termcap string capability\n", i->tname);
    else
        i->tcodes = strdup(termcap);

    /* Set up the terminfo seq */ 
    if ( (terminfo = tigetstr(i->tiname)) == 0 )
        display_message("CAPNAME (%s) is not present in this TERM's terminfo description\n", i->tiname);
    else if (terminfo == (char*)-1 )
        display_message("CAPNAME (%s) is not a terminfo string capability\n", i->tiname);
    else
        i->ticodes = strdup(terminfo);
}

/* Binds all of the key sequences */
static void import_keyseqs(void) {
    int i;
    /* strings */
    for( i = 0; seqlist[i].tname != NULL; i++)
        import_keyseq(&seqlist[i]);
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

static int init_curses()
{
    if ( putenv("ESCDELAY=0") == -1 )
       fprintf(stderr, "(%s:%d) putenv failed\r\n", __FILE__, __LINE__);

    setbuf(stdout, NULL);
    initscr();                       /* Start curses mode */
    noecho();                        /* Do not echo characters typed by user */
    raw();                        /* Line buffering disabled */
    refresh();                       /* Refresh the initial window once */
    top_win = scr_new(0, 0, 50, 80);
    return 0;
}

int main(int argc, char **argv){
    init_curses();
    import_keyseqs();
    print_list();
    store_list();
    add_keybindings();
    display_list();
    capture_chars(0);
    echo();
    endwin();
    return 0;
}
