#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#if HAVE_ERRNO_H
#include <errno.h>
#endif /* HAVE_ERRNO_H */

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif /* HAVE_SYS_STAT_H */

#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */

#if HAVE_STDIO_H
#include <stdio.h>
#endif

#if HAVE_LIMITS_H
#include <limits.h>     /* This is for PATH_MAX */
#endif /* HAVE_LIMITS_H */

#include "rlctx.h"
#include "rlctx_main.h"
#include "sys_util.h"
#include "ibuf.h"
#include "pseudo.h"
#include "error.h"
#include "fork_util.h"
#include "fs_util.h"

struct rlctx {
    int mfd; /* Master fd */
    int cfd; /* Command fd */
    rlctx_recv_line command_callback;
    rlctx_recv_line completion_callback;
    char tty_name[SLAVE_SIZE];
    struct ibuf *rlctx_com;

    /* Should the history be saved for this context?
     * If 1 yes, if 0 no.
     */
    int save_history;
    
    /* This will be NULL if save_history is 0. 
     * Otherwise, it will point to the file to save to
     * It should be freed when the object is freed.
     */
    char *config_file;

	/* This object is a context passed from the client.
	 * When rlctx calls a callback, it passes this as the first argument.
	 */
	void *callback_param;
};

struct rlctx *rlctx_init(const char *home_dir, const char *unique_id, void* p) {
    int read_history = 0;
    struct rlctx *n = (struct rlctx *)xmalloc(sizeof(struct rlctx));
    pid_t pid;
    struct stat st;
    static char rlctx_filename[PATH_MAX];

    /* Check that file permisions are correct */
    if ( home_dir == NULL || unique_id == NULL )
        n->save_history = 0;
    else
        n->save_history = 1;

    if ( n->save_history ) {
        n->config_file = (char *)xmalloc(sizeof(char)*PATH_MAX);

        strcpy ( rlctx_filename, "readline-" );
        strcat ( rlctx_filename, unique_id );
        strcat ( rlctx_filename, ".dat" );

        fs_util_get_path ( home_dir, rlctx_filename, n->config_file );

        /* Check to see if already exists, if does not exist continue */
        if ( stat( n->config_file, &st ) == -1 && errno == ENOENT ) {
            /* Doesn't exist, make sure dir permissions are ok */
            if ( access( home_dir, R_OK | W_OK ) == -1 ) {
                err_msg("%s:%d access error", __FILE__, __LINE__);
                return NULL;
            }
        } else {
            /* Exists, read it in */
            read_history = 1;   
        }
    }

    /* Start the readline program, give it the pty name so it can open it */
    if ( (pid = invoke_pty_process_function(n-> tty_name, & n->mfd, &n->cfd, &rlctx_main)) == -1 ) { /* Command fd */
        err_msg("%s:%d invoke_process error", __FILE__, __LINE__);
        return NULL;
    }

    /* Verify that the child started, by reading a byte */
    {
        char c;
        if ( read(n->mfd, &c, 1) != 1 ) {
            /* This means that the process didn't start */
            err_msg("%s:%d read error", __FILE__, __LINE__);
            fprintf(stderr, "*************************************************\n");
            fprintf(stderr, "Error: rlctx_prog failed to start via execvp.\n");
            fprintf(stderr, "This is probably becuase it is not in your path.\n");
            fprintf(stderr, "Please do a ./configure, make and make install.\n");
            fprintf(stderr, "Then add the install directory to your path.\n");
            fprintf(stderr, "*************************************************\n");
            return NULL;
        }
    }

    n->rlctx_com = ibuf_init();
    n->command_callback     = NULL;
    n->completion_callback  = NULL;
	n->callback_param       = p;
    
    /* Read in the history */
    if ( read_history ) {
        if ( rlctx_read_history(n) == -1 ) {
            err_msg("%s:%d rlctx_read_history error", __FILE__, __LINE__);
            return NULL;
        }
    }

    return n;
}

int rlctx_close(struct rlctx *rl) {
    if ( rl && rl->save_history)
        rlctx_write_history(rl);

    /* Can only close the slavename when the process is dead */
    if ( rl && pty_free_process(&rl->mfd, rl->tty_name) == -1 ) {
        err_msg("%s:%d tgdb_util_free_tty error", __FILE__, __LINE__);
        return -1;
    }

    return 0;
}

int rlctx_get_fd(struct rlctx *rl){
    return rl->mfd;
}

static int rlctx_send_char_internal(int fd, char c) {
    int result;
    if ( ( result = write(fd, &c, 1 )) == -1 ){ 
        if ( errno == EBADF ) 
            err_msg("%s:%d write error1", __FILE__, __LINE__);
        else if ( errno == EINVAL )
            err_msg("%s:%d write error2", __FILE__, __LINE__);
        else if ( errno == EFBIG ) 
            err_msg("%s:%d write error3", __FILE__, __LINE__);
        else if ( errno == EPIPE ) 
            err_msg("%s:%d write error4", __FILE__, __LINE__);
        else if ( errno == EAGAIN )
            err_msg("%s:%d write error5", __FILE__, __LINE__);
        else if ( errno == EINTR )
            err_msg("%s:%d write error6", __FILE__, __LINE__);
        else if ( errno == ENOSPC )
            err_msg("%s:%d write error7", __FILE__, __LINE__);
        else if ( errno == EIO ) 
            err_msg("%s:%d write error8", __FILE__, __LINE__);
        else
            err_msg("%s:%d write error9", __FILE__, __LINE__);
        return -1;
    } else if ( result != 1 ) {
        err_msg("%s:%d write error no 1", __FILE__, __LINE__);
        return -1;
    }

    return 0;

}

int rlctx_send_char(struct rlctx *rl, char c) {
    /* This is important.
     * Readline will not run a command entered during reverse-search-history
     * if '\n' is sent to it. It will only select the command. 
     *
     * Readline has to recieve '\r' to select and run the command. If a command
     * is not run then libtgdb does not function properly.
     */
    if ( c == '\n' )
        c = '\r';

    return rlctx_send_char_internal(rl->mfd, c);
}

int rlctx_change_prompt(struct rlctx *rl, const char *prompt) {
    int i; 
    int length = strlen(prompt);

    /* Send init char */
    if ( rlctx_send_char_internal(rl->cfd, '\032') == -1 ) {
        err_msg("%s:%d rlctx_send_char_internal error", __FILE__, __LINE__);
        return -1;
    }

    /* Send call char */
    if ( rlctx_send_char_internal(rl->cfd, 'P') == -1 ) {
        err_msg("%s:%d rlctx_send_char_internal error", __FILE__, __LINE__);
        return -1;
    }

    for ( i = 0; i < length; i++ ) {
        if ( rlctx_send_char_internal(rl->cfd, prompt[i]) == -1 ) {
            err_msg("%s:%d rlctx_send_char_internal error", __FILE__, __LINE__);
            return -1;
        }
    }

    /* Send terminate char */
    if ( rlctx_send_char_internal(rl->cfd, '\032') == -1 ) {
        err_msg("%s:%d rlctx_send_char_internal error", __FILE__, __LINE__);
        return -1;
    }

    return 0;
}

int rlctx_redisplay(struct rlctx *rl) {

    /* Send init char */
    if ( rlctx_send_char_internal(rl->cfd, '\032') == -1 ) {
        err_msg("%s:%d rlctx_send_char_internal error", __FILE__, __LINE__);
        return -1;
    }

    /* Send call char */
    if ( rlctx_send_char_internal(rl->cfd, 'T') == -1 ) {
        err_msg("%s:%d rlctx_send_char_internal error", __FILE__, __LINE__);
        return -1;
    }

    /* Send terminate char */
    if ( rlctx_send_char_internal(rl->cfd, '\032') == -1 ) {
        err_msg("%s:%d rlctx_send_char_internal error", __FILE__, __LINE__);
        return -1;
    }
    
    return 0;
}

int rlctx_read_history(struct rlctx *rl) {
    int i; 
    int length = strlen(rl->config_file);
    
    /* Send init char */
    if ( rlctx_send_char_internal(rl->cfd, '\032') == -1 ) {
        err_msg("%s:%d rlctx_send_char_internal error", __FILE__, __LINE__);
        return -1;
    }

    /* Send call char */
    if ( rlctx_send_char_internal(rl->cfd, 'R') == -1 ) {
        err_msg("%s:%d rlctx_send_char_internal error", __FILE__, __LINE__);
        return -1;
    }

    for ( i = 0; i < length; i++ ) {
        if ( rlctx_send_char_internal(rl->cfd, (rl->config_file)[i]) == -1 ) {
            err_msg("%s:%d rlctx_send_char_internal error", __FILE__, __LINE__);
            return -1;
        }
    }

    /* Send terminate char */
    if ( rlctx_send_char_internal(rl->cfd, '\032') == -1 ) {
        err_msg("%s:%d rlctx_send_char_internal error", __FILE__, __LINE__);
        return -1;
    }

    return 0;
} 

int rlctx_write_history(struct rlctx *rl) {
    int i; 
    int length = strlen(rl->config_file);
    
    /* Send init char */
    if ( rlctx_send_char_internal(rl->cfd, '\032') == -1 ) {
        err_msg("%s:%d rlctx_send_char_internal error", __FILE__, __LINE__);
        return -1;
    }

    /* Send call char */
    if ( rlctx_send_char_internal(rl->cfd, 'W') == -1 ) {
        err_msg("%s:%d rlctx_send_char_internal error", __FILE__, __LINE__);
        return -1;
    }

    for ( i = 0; i < length; i++ ) {
        if ( rlctx_send_char_internal(rl->cfd, (rl->config_file)[i]) == -1 ) {
            err_msg("%s:%d rlctx_send_char_internal error", __FILE__, __LINE__);
            return -1;
        }
    }

    /* Send terminate char */
    if ( rlctx_send_char_internal(rl->cfd, '\032') == -1 ) {
        err_msg("%s:%d rlctx_send_char_internal error", __FILE__, __LINE__);
        return -1;
    }

    return 0;
} 

/* rlctx_recv_data: reads from readline program.
 *      
 *      rl      - The readline context intended.
 *      c       - The buf to recieve, It will be null-terminated on way out
 *      size    - The amount of data to attempt to read
 *      
 *      Returns: amount of data read on success ( will be > 0), 
 *               -1, on error, 
 *               -2 on EOF.
 */
static int rlctx_recv_data(struct rlctx *rl, char *c, ssize_t size) {
    int result;
    if ( ( result = read(rl->mfd, c, size - 1)) == -1 ){ 
        if ( errno == EBADF ) 
            err_msg("%s:%d write error1", __FILE__, __LINE__);
        else if ( errno == EINVAL )
            err_msg("%s:%d write error2", __FILE__, __LINE__);
        else if ( errno == EAGAIN )
            err_msg("%s:%d write error5", __FILE__, __LINE__);
        else if ( errno == EINTR )
            err_msg("%s:%d write error6", __FILE__, __LINE__);
        else if ( errno == EIO ) 
            err_msg("%s:%d write error8", __FILE__, __LINE__);
        else
            err_msg("%s:%d write error9", __FILE__, __LINE__);
        return -1;
    } else if ( result == 0 )   /* EOF */
        return -2;

    return result;
}

/* rlctx_parse_command: This will parse c and remove any commands 
 *                      that the readline program sent to us. 
 *                      c will only be made smaller, from removing commands.
 *
 *      rl      - The readline context intended.
 *      c       - The buf to parse, it is assumed to be null-terminated.
 *      size    - The strlen of c
 */
static void rlctx_parse_command(struct rlctx *rl, char *c, ssize_t size) {
    int i, j = 0;
    static enum rlctx_parse_state { DATA, COMMAND } rlctx_state = DATA;
    static enum rlctx_com_type { TAB_COMPLETION, COMMAND_ENTERED } rlctx_com;
    static unsigned short command_byte = 0;
    
    for ( i = 0; i < size; i++) {
        /* Change states */
        if ( c[i] == '\032' ) {
            if ( rlctx_state == DATA ) {
                rlctx_state = COMMAND;
                command_byte = 1;
            } else { /* Found a full command, call callback */
                rlctx_state = DATA;
                if ( rlctx_com == COMMAND_ENTERED )
                    (rl->command_callback)(rl->callback_param, ibuf_get(rl->rlctx_com));
                else if ( rlctx_com == TAB_COMPLETION )
                    (rl->completion_callback)(rl->callback_param, ibuf_get(rl->rlctx_com));
                ibuf_clear(rl->rlctx_com);
            }
            /* Don't want to add \032 to command */
            continue;
        }

        /* This determines the type of command sent from rlctx_prog */
        if ( command_byte ) {
            if ( c[i] == 'C' )
                rlctx_com = COMMAND_ENTERED;
            else if ( c[i] == 'T' )
                rlctx_com = TAB_COMPLETION;
            else {
                err_msg("%s:%d Communication error", __FILE__, __LINE__);
                return;
            }
            command_byte = 0;
            continue;
        }

        if ( rlctx_state == DATA )
            c[j++] = c[i]; 
        else
            ibuf_addchar(rl->rlctx_com, c[i]);
    }

    c[j] = '\0';
}

int rlctx_recv(struct rlctx *rl, char *c, ssize_t size) {
    int result = rlctx_recv_data(rl, c, size);

    /* Return error */
    if ( result <= 0 )
        return result;

    c[result] = '\0';

    rlctx_parse_command(rl, c, result);
    return 0;
}

int rlctx_register_command_callback(struct rlctx *rl, rlctx_recv_line func ) {
    if ( ! func ) {
        err_msg("%s:%d NULL func error", __FILE__, __LINE__);
        return -1;
    }

    rl->command_callback = func;
    
    return 0;
}

int rlctx_register_completion_callback(struct rlctx *rl, rlctx_recv_line func ) {
    if ( ! func ) {
        err_msg("%s:%d NULL func error", __FILE__, __LINE__);
        return -1;
    }

    rl->completion_callback = func;
    
    return 0;
}

