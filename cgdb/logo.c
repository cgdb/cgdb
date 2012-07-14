/* logo.c
 * ------
 * Data for ASCII logos.
 *
 * These logos were generated thanks to Jörg Seyfferth and his web-based
 * ASCII-text generator:  http://www.network-science.de/ascii/
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

/* System Includes */
#if HAVE_STDLIB_H 
#include <stdlib.h>
#endif  /* HAVE_STDLIB_H */

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#if HAVE_TIME_H
#include <time.h>
#endif /* HAVE_TIME_H */

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif /* HAVE_SYS_TIME_H */

/* Local Includes */
#include "cgdb.h"
#include "logo.h"
#include "highlight_groups.h"

/* ----------- */
/* Definitions */
/* ----------- */

#define CGDB_NUM_LOGOS 4

/* --------------- */
/* Data Structures */
/* --------------- */

struct Logo{
   int h;                  /* Height of logo */
   char *data[11];         /* Increase the array size as necessary */
};

static struct Logo CGDB_LOGO[CGDB_NUM_LOGOS] =
{
   { 6,
      { "                  .______.    ",
        "  ____   ____   __| _/\\_ |__  ",
        "_/ ___\\ / ___\\ / __ |  | __ \\ ",
        "\\  \\___/ /_/  > /_/ |  | \\_\\ \\",
        " \\___  >___  /\\____ |  |___  /",
        "     \\/_____/      \\/      \\/ "
      }
   },

   { 7,
      { "                          _|  _|      ",
        "  _|_|_|    _|_|_|    _|_|_|  _|_|_|  ",
        "_|        _|    _|  _|    _|  _|    _|",
        "_|        _|    _|  _|    _|  _|    _|",
        "  _|_|_|    _|_|_|    _|_|_|  _|_|_|  ",
        "                _|                    ",
        "            _|_|                      "
      }
   },

   { 11,
      { "============================",
        "==================  ==  ====",
        "==================  ==  ====",
        "==================  ==  ====",
        "==   ====   ======  ==  ====",
        "=  =  ==  =  ===    ==    ==",
        "=  ======    ==  =  ==  =  =",
        "=  ========  ==  =  ==  =  =",
        "=  =  ==  =  ==  =  ==  =  =",
        "==   ====   ====    ==    ==",
        "============================"
      }
   },

   { 6,
      { "_______________________________/\\/\\__/\\/\\_______",
        "___/\\/\\/\\/\\____/\\/\\/\\/\\________/\\/\\__/\\/\\_______",
        "_/\\/\\________/\\/\\__/\\/\\____/\\/\\/\\/\\__/\\/\\/\\/\\___",
        "_/\\/\\__________/\\/\\/\\/\\__/\\/\\__/\\/\\__/\\/\\__/\\/\\_",
        "___/\\/\\/\\/\\________/\\/\\____/\\/\\/\\/\\__/\\/\\/\\/\\___",
        "_____________/\\/\\/\\/\\___________________________"
      }
   }
};

static char *usage[] = {
    "type  q<Enter>            to exit      ",
    "type  help<Enter>         for GDB help ",
    "type  <ESC>:help<Enter>   for CGDB help"
};

/* --------- */
/* Functions */
/* --------- */

static void center_line(WINDOW *win, int row, int width, char *data)
{
    mvwprintw(win, row, (width - strlen(data))/2, data);
}

void logo_display(WINDOW *win)
{
    static int logo = -1;                /* Logo index */
    int height, width;                   /* Dimensions of the window */
    int line;                            /* Starting line */
    int usage_height;                    /* Height of the usage message */
    int i, j;                            /* Iterators */
    int attr;

    if (hl_groups_get_attr (hl_groups_instance, HLG_LOGO, &attr) == -1)
      return;
    
    /* Get dimensions */
    getmaxyx(win, height, width);
    usage_height = sizeof(usage)/sizeof(char *);
        
    /* Clear the window */
    wmove(win, 0, 0);
    for (i = 0; i < height; i++){
       for (j = 0; j < width; j++)
          wprintw(win, " ");
       wprintw(win, "\n");
    }  
        
    /* Display cgdb logo */
    wattron(win, attr);

    /* Pick a random logo */
    if (logo == -1){
        srand(time(NULL));
        logo = rand() % CGDB_NUM_LOGOS;
    }

    /* If the logo fits on the screen, draw it */
    if (CGDB_LOGO[logo].h <= height - usage_height - 4){
        line = (height - CGDB_LOGO[logo].h - usage_height - 4)/2;
        wmove(win, line, 0);
        for(i = 0; i < CGDB_LOGO[logo].h; i++)
            center_line(win, i+line, width, CGDB_LOGO[logo].data[i]);
        center_line(win, ++i + line, width, "a curses debugger");
    }
    else{
        line = (height - usage_height - 4)/2;
        center_line(win, line, width, "CGDB: a curses debugger");
        i = 0;
    }
    center_line(win, ++i + line, width, "version " VERSION);
    i++;
    
    /* Show simple usage info */
    for (j = 0; j < sizeof(usage)/sizeof(char *); j++)
        center_line(win, ++i + line, width, usage[j]);

    wattroff(win, attr);
    curs_set(0);         /* Hide the cursor */
}
