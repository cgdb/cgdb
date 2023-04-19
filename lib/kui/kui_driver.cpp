#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_STDIO_H
#include <stdio.h>
#endif /* HAVE_STDIO_H */

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

/* Library includes */
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_REGEX_H
#include <regex.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#if HAVE_ERRNO_H
#include <errno.h>
#endif /* HAVE_ERRNO_H */

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif /* HAVE_STDLIB_H */

#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#include "sys_win.h"
#include "sys_util.h"
#include "kui_manager.h"
#include "kui_map_set.h"
#include "kui_term.h"

std::shared_ptr<kui_map_set> map;
std::unique_ptr<kui_manager> manager;

static void kui_shutdown(int use_endwin)
{
    manager.reset();

    /* Shutdown curses */
    if (use_endwin)
        swin_endwin();
    exit(0);
}

static void usage(void)
{
    printf("KUI Usage:\r\n"
            "   kui_driver [kui options]\r\n" "\r\n" "KUI Options:\r\n"
#ifdef HAVE_GETOPT_H
            "   --file      Load an rc file consisting of map and unmap commands.\r\n"
#else
            "   -f          Load an rc file consisting of map and unmap commands.\r\n"
#endif
#ifdef HAVE_GETOPT_H
            "   --help      Print help (this message) and then exit.\r\n"
#else
            "   -h          Print help (this message) and then exit.\r\n"
#endif
            "\r\nType 'q' to quit and Ctrl-z to send map or unmap command.\r\n");
    fflush(stdout);
}

static int load_map(const char *line)
{
    /* Read a complete line, and check to see if it's a 
     * map or umap command. */
    regex_t regex_map_t, regex_unmap_t;
    const char *regex_map = "map +([^ ]+) +([^ ]+)";
    const char *regex_unmap = "unmap +([^ ]+)";
    size_t nmatch = 3;
    regmatch_t pmatch[3];

    regcomp(&regex_map_t, regex_map, REG_EXTENDED);
    regcomp(&regex_unmap_t, regex_unmap, REG_EXTENDED);

    if (regexec(&regex_map_t, line, nmatch, pmatch, 0) == 0) {
        if (pmatch[0].rm_so != -1 &&
                pmatch[1].rm_so != -1 && pmatch[2].rm_so != -1) {
            int size;
            char *key, *value;

            size = pmatch[1].rm_eo - pmatch[1].rm_so;
            key = (char *) cgdb_malloc(sizeof (char) * (size) + 1);
            strncpy(key, &line[pmatch[1].rm_so], size);
            key[size] = 0;

            size = pmatch[2].rm_eo - pmatch[2].rm_so;
            value = (char *) cgdb_malloc(sizeof (char) * (size) + 1);
            strncpy(value, &line[pmatch[2].rm_so], size);
            value[size] = 0;

            if (!map->register_map(key, value)) {
                fprintf(stderr, "\r\nregestered key=%s value=%s", key, value);
                return 1;
            }
        }
    } else if (regexec(&regex_unmap_t, line, nmatch, pmatch, 0) == 0) {
        if (pmatch[0].rm_so != -1 && pmatch[1].rm_so != -1) {
            int size;
            char *key;

            size = pmatch[1].rm_eo - pmatch[1].rm_so;
            key = (char *) cgdb_malloc(sizeof (char) * (size) + 1);
            strncpy(key, &line[pmatch[1].rm_so], size);
            key[size] = 0;

            if (!map->deregister_map(key)) {
                fprintf(stderr, "\r\nderegister key=%s", key);
                return 1;
            }
        }
    }

    regfree(&regex_map_t);
    regfree(&regex_unmap_t);

    return 0;
}

static int read_mappings(const char *file)
{
    FILE *fd = fopen(file, "r");

    if (!fd) {
        fprintf(stderr, "%s:%d fopen failed\n", __FILE__, __LINE__);
        return 0;
    }

    while (!feof(fd)) {
        char line[4096];

        if (fgets(line, 4096, fd) == NULL)
            break;
        line[strlen(line) - 1] = '\0';
        load_map(line);
    }

    fclose(fd);

    return 0;
}

static void parse_long_options(int argc, char **argv)
{
    int opt, option_index = 0;
    const char *args = "hf:";

#ifdef HAVE_GETOPT_H
    static struct option long_options[] = {
        {"file", 1, 0, 'f'},
        {"help", 0, 0, 'h'},
        {0, 0, 0, 0}
    };
#endif

    while (1) {
#ifdef HAVE_GETOPT_H
        opt = getopt_long(argc, argv, args, long_options, &option_index);
#else
        opt = getopt(argc, argv, args);
#endif
        if (opt == -1)
            break;

        switch (opt) {
            case 'f':
                /* Load a file */
                read_mappings(optarg);
                break;
            case '?':
            case 'h':
                swin_endwin();
                usage();
                kui_shutdown(0);
            default:
                break;
        }
    }
}

void main_loop(std::unique_ptr<kui_manager>& i)
{
    int max;
    fd_set rfds;
    int result;

    max = STDIN_FILENO;

    while (1) {
        FD_ZERO(&rfds);
        FD_SET(STDIN_FILENO, &rfds);

        fprintf(stderr, "\r\n(kui) ");

        result = select(max + 1, &rfds, NULL, NULL, NULL);

        /* if the signal interrupted system call keep going */
        if (result == -1 && errno == EINTR)
            continue;
        else if (result == -1)  /* on error ... must die -> stupid OS */
            fprintf(stderr, "%s:%d select failed\n", __FILE__, __LINE__);

        if (FD_ISSET(STDIN_FILENO, &rfds)) {
            while (1) {
                int c = i->getkey();

                if (c == -1) {
                    fprintf(stderr, "kui_manager_getkey failed\n");
                    return;
                }

                if (c == 'q') {
                    fprintf(stderr, "User aborted\r\n");
                    return;
                } else if (kui_term_is_cgdb_key(c) && c == CGDB_KEY_CTRL_Z) {
                    /* Read a complete line, and check to see if it's a 
                     * map or umap command. */
                    char ch;
                    char line[4096];
                    int pos = 0;

                    fprintf(stderr, "\r\n(kui_map)");

                    do {
                        ch = fgetc(stdin);
                        line[pos++] = ch;
                        fprintf(stderr, "%c", ch);
                    } while (ch != '\r' && ch != '\n' && pos < 4095);
                    line[pos++] = 0;

                    load_map(line);
                } else {
                    if (kui_term_is_cgdb_key(c)) {
                        char *val;
                        char *sequence;

                        val = (char *) kui_term_get_string_from_key(c);

                        fprintf(stderr, "%s", val);

                        /* Print out the sequence received */
                        sequence = (char *)
                                kui_term_get_ascii_char_sequence_from_key(c);
                        while (sequence && sequence[0]) {
                            fprintf(stderr, "[%d]", sequence[0]);
                            sequence = sequence + 1;
                        }
                    } else
                        fprintf(stderr, "%c", c);
                }

                if (i->cangetkey() == 1)
                    continue;
                else
                    break;
            }
        }
    }
}

static void create_mappings(std::unique_ptr<kui_manager>& kuim)
{
    map = std::make_shared<kui_map_set>();

#if 0

#if 1

    if (!map->register_map("abc", "xyz")) {
        /* TODO: Free map and return */
        return;
    }

    if (!map->deregister_map("abc")) {
        /* TODO: Free map and return */
        return;
    }
#endif

    if (!map->register_map("abc", "xyz")) {
        /* TODO: Free map and return */
        return;
    }

    if (!map->register_map("abc", "xyp")) {
        /* TODO: Free map and return */
        return;
    }
#if 0
    if (!map->register_map("xyzd", "<F4>")) {
        /* TODO: Free map and return */
        return;
    }

    if (!map->register_map("xyzd", "<F4>")) {
        /* TODO: Free map and return */
        return;
    }

    if (!map->register_map("a<F1>", "xyz")) {
        /* TODO: Free map and return */
        return;
    }

    if (!map->register_map("a<F1><F1>", "xxx")) {
        /* TODO: Free map and return */
        return;
    }

    if (!map->register_map("a<F1><F1>", "xxx")) {
        /* TODO: Free map and return */
        return;
    }

    if (!map->register_map("<Left><Right><F1><F1>", "<F2>")) {
        /* TODO: Free map and return */
        return;
    }

    if (!map->register_map("<F6>", "p<Space>argc<CR>")) {
        /* TODO: Free map and return */
        return;
    }
#endif
#endif

    kuim->set_map_set(map);
}

/* Original terminal attributes */

int main(int argc, char **argv)
{
    /* Initialize curses */
    swin_initscr();
    swin_raw();
    swin_refresh();

    manager = kui_manager::create(STDIN_FILENO, 40, 1000);

    create_mappings(manager);

    parse_long_options(argc, argv);

    main_loop(manager);

    kui_shutdown(1);
    return 0;
}
