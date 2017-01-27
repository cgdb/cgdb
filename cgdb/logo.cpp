/* logo.c
 * ------
 * Data for ASCII logos.
 *
 * These logos were generated thanks to Jorg Seyfferth and his web-based
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
#include "sys_util.h"
#include "sys_win.h"
#include "cgdb.h"
#include "logo.h"
#include "highlight_groups.h"

/* Logo index */
static int logoindex = -1;

/* --------------- */
/* Data Structures */
/* --------------- */

struct Logo {
   int h;                /* Height of logo */
   int w;                /* Width of logo */
   const char *data[15]; /* Increase the array size as necessary */
};

/*
    With vim (note: check 'stty -a' for lnext if Ctrl+V doesn't work)

    :read !figlet -f colossal cgdb | toilet -f term -F metal --export ansi
    s/\\/\\\\/g     ; replace all \ with \\
    s/"/\\"/g       ; replace all " with \"
    s/^[/\\033/g    ; replace ^[ with \033, hit Ctrl+V,Ctrl+[ to get ^[
    s/^/"/          ; add " at start of lines
    s/$/",/         ; add ", at end of lines
*/
static struct Logo CGDB_LOGO[] =
{
    { 6, 31,
        { "                  .______.    ",
          "  ____   ____   __| _/\\_ |__  ",
          "_/ ___\\ / ___\\ / __ |  | __ \\ ",
          "\\  \\___/ /_/  > /_/ |  | \\_\\ \\",
          " \\___  >___  /\\____ |  |___  /",
          "     \\/_____/      \\/      \\/ "
        }
    },
    { 6, 49,
        { "_______________________________/\\/\\__/\\/\\_______",
          "___/\\/\\/\\/\\____/\\/\\/\\/\\________/\\/\\__/\\/\\_______",
          "_/\\/\\________/\\/\\__/\\/\\____/\\/\\/\\/\\__/\\/\\/\\/\\___",
          "_/\\/\\__________/\\/\\/\\/\\__/\\/\\__/\\/\\__/\\/\\__/\\/\\_",
          "___/\\/\\/\\/\\________/\\/\\____/\\/\\/\\/\\__/\\/\\/\\/\\___",
          "_____________/\\/\\/\\/\\___________________________"
        }
    },
    { 7, 38,
        { "                          _|  _|      ",
          "  _|_|_|    _|_|_|    _|_|_|  _|_|_|  ",
          "_|        _|    _|  _|    _|  _|    _|",
          "_|        _|    _|  _|    _|  _|    _|",
          "  _|_|_|    _|_|_|    _|_|_|  _|_|_|  ",
          "                _|                    ",
          "            _|_|                      "
        }
    },

    /* echo cgdb | boxes -d peek -a c -s 20x7 | toilet --gay -f term */
    { 7, 20,
        {
            "\033[0;1;35;95m  \033[0m       \033[0;1;36;96m_\033[0;1;34;94m\\|\033[0;1;35;95m/_\033[0m",
            "         \033[0;1;36;96m(\033[0;1;34;94mo\033[0m \033[0;1;35;95mo)\033[0m",
            " \033[0;1;35;95m+\033[0;1;31;91m--\033[0;1;33;93m--\033[0;1;32;92moO\033[0;1;36;96mO-\033[0;1;34;94m{_\033[0;1;35;95m}-\033[0;1;31;91mOO\033[0;1;33;93mo-\033[0;1;32;92m-+\033[0m",
            " \033[0;1;35;95m|\033[0m                 \033[0;1;32;92m|\033[0m",
            " \033[0;1;35;95m|\033[0m      \033[0;1;36;96mcg\033[0;1;34;94mdb\033[0m       \033[0;1;32;92m|\033[0m",
            " \033[0;1;35;95m|\033[0m                 \033[0;1;32;92m|\033[0m",
            " \033[0;1;35;95m+\033[0;1;31;91m--\033[0;1;33;93m--\033[0;1;32;92m--\033[0;1;36;96m--\033[0;1;34;94m--\033[0;1;35;95m--\033[0;1;31;91m--\033[0;1;33;93m--\033[0;1;32;92m*/\033[0m",
        }
    },

    /* cowsay -f eyes cgdb | toilet --gay -f term --export ansi */
    { 14, 48,
        {
            "\033[0;37m \033[0;1;35m_\033[0;1;31m__\033[0;1;33m__\033[0;1;32m_\033[0m",
            "\033[0;1;35m<\033[0;37m \033[0;1;31mcg\033[0;1;33mdb\033[0;37m \033[0;1;32m>\033[0m",
            "\033[0;37m \033[0;1;35m-\033[0;1;31m--\033[0;1;33m--\033[0;1;32m-\033[0m",
            "\033[0;37m    \033[0;1;33m\\\033[0m",
            "\033[0;37m     \033[0;1;33m\\\033[0m",
            "\033[0;37m                                   \033[0;1;34m.\033[0;1;35m::\033[0;1;31m!!\033[0;1;33m!!\033[0;1;32m!!\033[0;1;36m!:\033[0;1;34m.\033[0m",
            "\033[0;37m  \033[0;1;31m.!\033[0;1;33m!!\033[0;1;32m!!\033[0;1;36m:.\033[0;37m                        \033[0;1;34m.:\033[0;1;35m!!\033[0;1;31m!!\033[0;1;33m!!\033[0;1;32m!!\033[0;1;36m!!\033[0;1;34m!!\033[0m",
            "\033[0;37m  \033[0;1;31m~~\033[0;1;33m~~\033[0;1;32m!!\033[0;1;36m!!\033[0;1;34m!!\033[0;1;35m.\033[0;37m                 \033[0;1;32m.:\033[0;1;36m!!\033[0;1;34m!!\033[0;1;35m!!\033[0;1;31m!!\033[0;1;33m!U\033[0;1;32mWW\033[0;1;36mW$\033[0;1;34m$$\033[0;37m \033[0m",
            "\033[0;37m      \033[0;1;32m:$\033[0;1;36m$N\033[0;1;34mWX\033[0;1;35m!!\033[0;1;31m:\033[0;37m           \033[0;1;31m.:\033[0;1;33m!!\033[0;1;32m!!\033[0;1;36m!!\033[0;1;34mXU\033[0;1;35mWW\033[0;1;31m$$\033[0;1;33m$$\033[0;1;32m$$\033[0;1;36m$$\033[0;1;34m$P\033[0;37m \033[0m",
            "\033[0;37m      \033[0;1;32m$$\033[0;1;36m$$\033[0;1;34m$#\033[0;1;35m#W\033[0;1;31mX!\033[0;1;33m:\033[0;37m      \033[0;1;34m.\033[0;1;35m<!\033[0;1;31m!!\033[0;1;33m!U\033[0;1;32mW$\033[0;1;36m$$\033[0;1;34m$\"\033[0;37m  \033[0;1;31m$$\033[0;1;33m$$\033[0;1;32m$$\033[0;1;36m$$\033[0;1;34m#\033[0;37m \033[0m",
            "\033[0;37m      \033[0;1;32m$$\033[0;1;36m$$\033[0;1;34m$\033[0;37m  \033[0;1;35m$\033[0;1;31m$$\033[0;1;33mUX\033[0;37m   \033[0;1;36m:\033[0;1;34m!!\033[0;1;35mUW\033[0;1;31m$$\033[0;1;33m$$\033[0;1;32m$$\033[0;1;36m$$\033[0;1;34m$\033[0;37m   \033[0;1;31m4$\033[0;1;33m$$\033[0;1;32m$$\033[0;1;36m*\033[0;37m \033[0m",
            "\033[0;37m      \033[0;1;32m^$\033[0;1;36m$$\033[0;1;34mB\033[0;37m  \033[0;1;35m$\033[0;1;31m$$\033[0;1;33m$\\\033[0;37m     \033[0;1;34m$\033[0;1;35m$$\033[0;1;31m$$\033[0;1;33m$$\033[0;1;32m$$\033[0;1;36m$$\033[0;1;34m$\033[0;37m   \033[0;1;31md$\033[0;1;33m$R\033[0;1;32m\"\033[0;37m \033[0m",
            "\033[0;37m        \033[0;1;36m\"*\033[0;1;34m$b\033[0;1;35md$\033[0;1;31m$$\033[0;1;33m$\033[0;37m      \033[0;1;34m'\033[0;1;35m*$\033[0;1;31m$$\033[0;1;33m$$\033[0;1;32m$$\033[0;1;36m$$\033[0;1;34m$$\033[0;1;35mo+\033[0;1;31m#\"\033[0;37m \033[0m",
            "\033[0;37m             \033[0;1;35m\"\033[0;1;31m\"\"\033[0;1;33m\"\033[0;37m          \033[0;1;31m\"\033[0;1;33m\"\"\033[0;1;32m\"\"\033[0;1;36m\"\"\033[0;37m \033[0m"
        }
    },

    /* figlet -f colossal cgdb | toilet -f term -F metal --export ansi */
    { 11, 32,
        { "\033[0;37m                     \033[0;34m888888\033[0;37m      \033[0m",
          "\033[0;37m                     \033[0;34m888888\033[0;37m      \033[0m",
          "\033[0;37m                     \033[0;34m888888\033[0;37m      \033[0m",
          "\033[0;37m \033[0;1;34m.d8888b\033[0;37m \033[0;1;34m.d88b.\033[0;37m  \033[0;34m.d8888888888b.\033[0;37m  \033[0m",
          "\033[0;1;34md88P\"\033[0;37m   \033[0;1;34md88P\"88b\033[0;34md88\"\033[0;37m \033[0;34m888888\033[0;37m \033[0;34m\"88b\033[0;37m \033[0m",
          "\033[0;1;34m888\033[0;37m     \033[0;1;34m888\033[0;37m  \033[0;1;34m888\033[0;34m888\033[0;37m  \033[0;34m888888\033[0;37m  \033[0;34m888\033[0;37m \033[0m",
          "\033[0;1;34mY88b.\033[0;37m   \033[0;1;34mY88b\033[0;37m \033[0;1;34m888\033[0;34mY88b\033[0;37m \033[0;34m888888\033[0;37m \033[0;34md88P\033[0;37m \033[0m",
          "\033[0;37m \033[0;1;34m\"Y8888P\033[0;37m \033[0;1;34m\"Y88888\033[0;37m \033[0;34m\"Y8888888888P\"\033[0;37m  \033[0m",
          "\033[0;37m             \033[0;1;34m888\033[0;37m                 \033[0m",
          "\033[0;37m        \033[0;1;34mY8b\033[0;37m \033[0;1;34md88P\033[0;37m                 \033[0m",
          "\033[0;37m         \033[0;1;34m\"Y88P\"\033[0;37m"
        }
    },

    /* figlet -f standard cgdb | toilet -f term -F gay --export ansi */
    { 6, 23,
        { "\033[0;37m               \033[0;1;31m_\033[0;37m \033[0;1;33m_\033[0;37m     \033[0m",
          "\033[0;37m  \033[0;1;31m__\033[0;1;33m_\033[0;37m \033[0;1;32m__\033[0;37m \033[0;1;36m_\033[0;37m  \033[0;1;35m__\033[0;1;31m|\033[0;37m \033[0;1;33m|\033[0;37m \033[0;1;32m|_\033[0;1;36m_\033[0;37m  \033[0m",
          "\033[0;37m \033[0;1;35m/\033[0;37m \033[0;1;31m_\033[0;1;33m_/\033[0;37m \033[0;1;32m_\033[0;1;36m`\033[0;37m \033[0;1;34m|/\033[0;37m \033[0;1;35m_\033[0;1;31m`\033[0;37m \033[0;1;33m|\033[0;37m \033[0;1;32m'_\033[0;37m \033[0;1;36m\\\033[0;37m \033[0m",
          "\033[0;1;35m|\033[0;37m \033[0;1;31m(_\033[0;1;33m|\033[0;37m \033[0;1;32m(_\033[0;1;36m|\033[0;37m \033[0;1;34m|\033[0;37m \033[0;1;35m(_\033[0;1;31m|\033[0;37m \033[0;1;33m|\033[0;37m \033[0;1;32m|_\033[0;1;36m)\033[0;37m \033[0;1;34m|\033[0m",
          "\033[0;37m \033[0;1;35m\\\033[0;1;31m__\033[0;1;33m_\\\033[0;1;32m__\033[0;1;36m,\033[0;37m \033[0;1;34m|\\\033[0;1;35m__\033[0;1;31m,_\033[0;1;33m|_\033[0;1;32m._\033[0;1;36m_/\033[0;37m \033[0m",
          "\033[0;37m     \033[0;1;33m|\033[0;1;32m__\033[0;1;36m_/\033[0;37m             \033[0m"
        }
    },
};
#define CGDB_NUM_LOGOS (sizeof(CGDB_LOGO) / sizeof(CGDB_LOGO[0]))

static const char *usage[] = {
    "a curses debugger",
    "version " VERSION,
    "",
    "type  q<Enter>            to exit      ",
    "type  help<Enter>         for GDB help ",
    "type  <ESC>:help<Enter>   for CGDB help"
};
#define CGDB_NUM_USAGE (sizeof(usage) / sizeof(usage[0]))

/* --------- */
/* Functions */
/* --------- */

static void center_line(SWINDOW *win, int row, int width, const char *data, int datawidth, int attr)
{
    int i;
    char *line = NULL;
    int datalen = strlen(data);
    hl_line_attr *attrs = NULL;
    struct hl_line_attr line_attr;

    /* Set up default attributes at column 0 */
    line_attr.attr = attr;
    line_attr.col = 0;
    sbpush(attrs, line_attr);

    /* Parse ansi escape color codes in string */
    for (i = 0; i < datalen; i++) {
        if (data[i] == '\033') {
            int ansi_count = hl_ansi_get_color_attrs(
                    hl_groups_instance, data + i, &attr);
            if (ansi_count) {
                line_attr.col = sbcount(line);
                line_attr.attr = attr;
                sbpush(attrs, line_attr);

                i += ansi_count - 1;
                continue;
            }
        }

        sbpush(line, data[i]);
    }
    sbpush(line, 0);

    hl_printline(win, line, strlen(line), attrs, (width - datawidth) / 2, row, 0, width);

    sbfree(attrs);
    sbfree(line);
}

void logo_reset()
{
    logoindex = (logoindex + 1) % CGDB_NUM_LOGOS;
}

void logo_display(SWINDOW *win)
{
    int height, width;                 /* Dimensions of the window */
    int line;                          /* Starting line */
    int i;                             /* Iterators */
    int attr;                          /* Default logo attributes */
    int usage_height = CGDB_NUM_USAGE; /* Height of the usage message */

    /* Pick a random logoindex */
    if (logoindex == -1) {
        srand(time(NULL));
        logoindex = rand() % CGDB_NUM_LOGOS;
    }

    attr = hl_groups_get_attr(hl_groups_instance, HLG_LOGO);

    /* Get dimensions */
    height = swin_getmaxy(win);
    width = swin_getmaxx(win);

    /* Clear the window */
    swin_werase(win);

    /* If the logo fits on the screen, draw it */
    if ((CGDB_LOGO[logoindex].h <= height - usage_height - 2))
    {
        line = (height - CGDB_LOGO[logoindex].h - usage_height - 2) / 2;

        for(i = 0; i < CGDB_LOGO[logoindex].h; i++) {
            center_line(win, line++, width,
                CGDB_LOGO[logoindex].data[i], CGDB_LOGO[logoindex].w, attr);
        }
        line++;
    } else {
        line = (height - usage_height) / 2;
    }

    /* Show simple usage info */
    for (i = 0; i < usage_height; i++)
        center_line(win, line++, width, usage[i], strlen(usage[i]), attr);

    swin_curs_set(0);         /* Hide the cursor */
}
