#include "rlctx.h"
#include "util.h"
#include "types.h"
#include "string.h"

/* Library includes */
#include <string.h>
#include <errno.h>


struct rlctx {
    int mfd; /* Master fd */
    rlctx_recv_line command_callback;
    char tty_name[SLAVE_SIZE];
};

static struct string *rlctx_com;

struct rlctx *rlctx_init(void) {
    struct rlctx *n = (struct rlctx *)xmalloc(sizeof(struct rlctx));
    pid_t pid;
    char **argv;
    char *progname = "rlctx_prog";

    /* Start the readline program, give it the pty name so it can open it */
    if ( (pid = invoke_pty_process(progname, 0, argv, n-> tty_name, & n->mfd)) == -1 ) {
        err_msg("%s:%d invoke_process error", __FILE__, __LINE__);
        return NULL;
    }

    rlctx_com = string_init();
    
    return n;
}

int rlctx_close(struct rlctx *rl) {
    /* Can only close the slavename when the process is dead */
    if ( tgdb_util_pty_free_process(&rl->mfd, rl->tty_name) == -1 ) {
        err_msg("%s:%d tgdb_util_free_tty error", __FILE__, __LINE__);
        return -1;
    }
}

int rlctx_get_fd(struct rlctx *rl){
    return rl->mfd;
}

int rlctx_send_char(struct rlctx *rl, char c) {
    int result;
    if ( ( result = write(rl->mfd, &c, 1 )) == -1 ){ 
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
    static enum rlctx_com_state { DATA, COMMAND } rlctx_com_type = DATA;
    static unsigned short state = 0;
    
    for ( i = 0; i < size; i++) {
        /* Change states */
        if ( c[i] == '\032' ) {
            if ( rlctx_com_type == DATA )
                rlctx_com_type = COMMAND;
            else { /* Found a full command, call callback */
                rlctx_com_type = DATA;
                (rl->command_callback)(string_get(rlctx_com));
                string_clear(rlctx_com);
            }
            /* Don't want to add \032 to command */
            continue;
        }

        if ( rlctx_com_type == DATA )
            c[j++] = c[i]; 
        else
            string_addchar(rlctx_com, c[i]);
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

int rlctx_register_callback(struct rlctx *rl, rlctx_recv_line func ) {
    if ( ! func ) {
        err_msg("%s:%d NULL func error", __FILE__, __LINE__);
        return -1;
    }

    rl->command_callback = func;
    
    return 0;
}
