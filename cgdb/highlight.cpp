/* highlight.c:
 * ------------
 * 
 * Syntax highlighting routines.
 *
 */
#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#if HAVE_REGEX_H
#include <regex.h>
#endif /* HAVE_REGEX_H */

/* Local Includes */
#include "sys_util.h"
#include "stretchy.h"
#include "sys_win.h"
#include "cgdb.h"
#include "highlight.h"
#include "tokenizer.h"
#include "sources.h"
#include "highlight_groups.h"

struct hl_regex_info {
    regex_t t;
    int icase;
    char *regex;
};

void hl_regex_free(struct hl_regex_info **info)
{
    if (info && *info && (*info)->regex) {
        regfree(&(*info)->t);

        free((*info)->regex);
        (*info)->regex = NULL;

        free(*info);
        *info = NULL;
    }
}

int hl_regex_search(struct hl_regex_info **info, const char *line,
    const char *regex, int icase, int *start, int *end)
{
    int result;
    regmatch_t pmatch;
    int recompile = 0;

    *start = -1;
    *end = -1;

    if (!regex || !regex[0])
        return -1;
    
    if (!*info) {
        *info = (struct hl_regex_info *)cgdb_calloc(1, sizeof(struct hl_regex_info));
        recompile = 1;
    } 
    else if (!(*info)->regex)
        recompile = 1;
    else if ((*info)->regex == regex)
        recompile = 0;
    else if ((icase != -1) && (icase != (*info)->icase))
        recompile = 1;
    else if (strcmp(regex, (*info)->regex))
        recompile = 1;

    if (recompile) {
        if (*info && (*info)->regex) {
            regfree(&(*info)->t);

            free((*info)->regex);
            (*info)->regex = NULL;
        }

        /* Compile the regular expression */
        if (regcomp(&(*info)->t, regex, REG_EXTENDED | (icase ? REG_ICASE : 0)) != 0) {
            hl_regex_free(info);
            return -1;
        }

        (*info)->regex = strdup(regex);
        (*info)->icase = icase;
    }

    result = regexec(&(*info)->t, line, 1, &pmatch, 0);

    if ((result == 0) && (pmatch.rm_eo > pmatch.rm_so)) {
        *start = pmatch.rm_so;
        *end = pmatch.rm_eo;
        return 1;
    }

    return 0;
}

struct hl_line_attr *hl_regex_highlight(struct hl_regex_info **info,
        char *line, enum hl_group_kind group_kind)
{
    hl_line_attr *attrs = NULL;

    if (*info && (*info)->regex && (*info)->regex[0]) {
        int pos = 0;

        for (;;) {
            int ret;
            int len;
            int start, end;

            ret = hl_regex_search(info, line + pos, (*info)->regex, (*info)->icase, &start, &end);
            if (ret <= 0)
                break;

            len = end - start;
            pos += start;

            /* Push search attribute */
            sbpush(attrs, hl_line_attr(pos, group_kind));

            /* And the back to regular text attribute */
            sbpush(attrs, hl_line_attr(pos + len, 0));

            pos += len;
        }
    }

    return attrs;
}
