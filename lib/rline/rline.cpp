#include "rline.h"
#include "sys_util.h"
#include "fork_util.h"

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

/* includes {{{*/

#ifdef HAVE_LIBREADLINE

#if defined(HAVE_READLINE_READLINE_H)
#include <readline/readline.h>
#elif defined(HAVE_READLINE_H)
#include <readline.h>
#endif /* !defined(HAVE_READLINE_H) */

#endif /* HAVE_LIBREADLINE */

/* }}}*/

int rline_initialize(void)
{
    /* Don't need to initialize the whole readline state, only have it
       read in 'inputrc' to expose key mappings for rline_get_keyseq. */
    return rl_read_init_file(NULL);
}

int
rline_get_keyseq(const char *named_function, std::list<std::string> &keyseq)
{
    rl_command_func_t *func;
    char **invoking_keyseqs = NULL;
    char **invoking_keyseqs_cur = NULL;
    int len;

    func = rl_named_function(named_function);
    if (func == 0)
        return 0;

    invoking_keyseqs = rl_invoking_keyseqs(func);
    invoking_keyseqs_cur = invoking_keyseqs;

    while (invoking_keyseqs_cur && (*invoking_keyseqs_cur)) {

        std::string new_keyseq;
        new_keyseq.resize((2 * strlen(*invoking_keyseqs_cur)) + 1);
        if (rl_translate_keyseq(*invoking_keyseqs_cur, &new_keyseq[0], &len)) {
            free(*invoking_keyseqs_cur);
            /* Can't do much about readline failing, just continue on. */
            continue;
        }

        keyseq.push_back(std::move(new_keyseq));

        free(*invoking_keyseqs_cur);
        invoking_keyseqs_cur++;
    }
    free(invoking_keyseqs);

    return 0;
}

/*@}*/
/* }}}*/
