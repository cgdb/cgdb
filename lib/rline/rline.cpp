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
    /* Master/Slave PTY used to keep readline off of stdin/stdout. */
    pty_pair_ptr pty_pair = pty_pair_create();
    if (!pty_pair) {
        return -1;
    }

    int slavefd = pty_pair_get_masterfd(pty_pair);

    rl_instream = fdopen(slavefd, "r");
    if (!rl_instream) {
        pty_pair_destroy(pty_pair);
        return -1;
    }

    rl_outstream = fdopen(slavefd, "w");
    if (!rl_outstream) {
        fclose(rl_instream);
        pty_pair_destroy(pty_pair);
        return -1;
    }

    /* Tell readline not to catch signals */
    rl_catch_signals = 0;
    rl_catch_sigwinch = 0;

    /* Initialize readline */
    rl_initialize();

    pty_pair_destroy(pty_pair);
    return 0;
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
