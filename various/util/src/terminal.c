#include "terminal.h"

#if HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif /* HAVE_SYS_IOCTL_H */

int 
tty_cbreak (int fd, struct termios *orig)
{
  struct termios buf;
   
  if (tcgetattr (fd, &buf) < 0)
    return -1;

  /* Save the original state, for resetting later */
  *orig = buf;
      
  buf.c_lflag &= ~(ECHO | ICANON);
  buf.c_iflag &= ~(ICRNL | INLCR);
  buf.c_cc[VMIN] = 1;
  buf.c_cc[VTIME] = 0;

#if defined (VLNEXT) && defined (_POSIX_VDISABLE)
  buf.c_cc[VLNEXT] = _POSIX_VDISABLE;
#endif

#if defined (VDSUSP) && defined (_POSIX_VDISABLE)
  buf.c_cc[VDSUSP] = _POSIX_VDISABLE;
#endif

  if (tcsetattr (fd, TCSAFLUSH, &buf) < 0)
    return -1;

  return 0;   
}

int 
tty_output_nl (int fd)
{
  struct termios buf;

  /* get attributes */
  if (tcgetattr (fd, &buf) < 0)
    return -1;

  buf.c_oflag &= ~ONLCR; /* turn off NL -> CR NL mapping */

  if (tcsetattr (fd, TCSAFLUSH, &buf) < 0)
    return -1;
    
  return 0;
}

int
tty_off_xon_xoff (int fd)
{
  struct termios buf;

  if(tcgetattr(fd, &buf) < 0)
    return -1;

  /* disable flow control; let ^S and ^Q through to pty */
  buf.c_iflag &= ~(IXON|IXOFF);
#ifdef IXANY
  buf.c_iflag &= ~IXANY;
#endif

  if(tcsetattr(fd, TCSAFLUSH, &buf) < 0)
    return -1;

  return 0;
}

int 
tty_set_echo (int fd, int echo_on)
{
  struct termios buf;

  if (tcgetattr (fd, &buf) < 0)
    return -1;

  if (echo_on == 1)
    buf.c_lflag |= ECHO;
  else if (echo_on == 0)
    buf.c_lflag &= ~ECHO;

  if (tcsetattr (fd, TCSAFLUSH, &buf) < 0)
    return -1;

  return 0;
}

int 
tty_get_attributes (int fd, struct termios *buf)
{
  if (tcgetattr (fd, buf) < 0)
    return -1;

  return 0;
}

int 
tty_set_attributes (int fd, struct termios *buf)
{
  if (tcsetattr (fd, TCSAFLUSH, buf) < 0)
    return -1;
      
  return 0;   
}
