/*
* libslack - http://libslack.org/
*
* Copyright (C) 1999-2001 raf <raf@raf.org>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
* or visit http://www.gnu.org/copyleft/gpl.html
*
* 20011109 raf <raf@raf.org>
*/

/*
 * Author: Tatu Ylonen <ylo@cs.hut.fi>
 * Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
 *                    All rights reserved
 * Allocating a pseudo-terminal, and making it the controlling tty.
 *
 * As far as I am concerned, the code I have written for this software
 * can be used freely for any purpose.  Any derived versions of this
 * software must be clearly marked as such, and if the derived work is
 * incompatible with the protocol description in the RFC file, it must be
 * called by a name other than "ssh" or "Secure Shell".
 */

/*
 * 20010930 raf <raf@raf.org>
 * Librarified the code (no calls to fatal(), return errors instead)
 * Made slave device name truncation an error (rather than ignoring it)
 * Made it thread safe on some systems (use ttyname_r/ptsname_r)
 * Added a manpage (perldoc -F pseudo.c)
 * Made pty_allocate() more like openpty() but safe (called it pty_open())
 * Added safe version of forkpty() (called it pty_fork())
 */

/*

=head1 NAME

I<libslack(pseudo)> - pseudo terminal module

=head1 SYNOPSIS

    #include <slack/pseudo.h>

    int pty_open(int *masterfd, int *slavefd, char *slavename, size_t slavenamesize, const struct termios *slave_termios, const struct winsize *slave_winsize);
    int pty_release(const char *slavename);
    int pty_set_owner(const char *slavename, uid_t uid);
    int pty_make_controlling_tty(int *slavefd, const char *slavename);
    int pty_change_window_size(int masterfd, int row, int col, int xpixel, int ypixel);
    pid_t pty_fork(int *masterfd, char *slavename, size_t slavenamesize, const struct termios *slave_termios, const struct winsize *slave_winsize);

=head1 DESCRIPTION

This module provides functions for opening pseudo terminals, changing their
ownership, making them the controlling terminal, changing their window size
and forking a new process whose standard input, output and error and
attached to a pseudo terminal which is made the controlling terminal.

=over 4

=cut

*/

#include "config.h"
#define _GNU_SOURCE /* ptsname_r() under Linux */
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#ifdef HAVE_PTY_H
#include <pty.h>
#endif
#ifdef HAVE_UTIL_H
#include <util.h>
#endif
#ifdef HAVE_LIBUTIL_H
#include <libutil.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>

#include "pseudo.h"

#ifndef TEST

/* Pty allocated with _getpty gets broken if we do I_PUSH:es to it */
#if defined(HAVE__GETPTY) || defined(HAVE_OPENPTY)
#undef HAVE_DEV_PTMX
#endif

#ifdef HAVE_PTY_H
#include <pty.h>
#endif
#if defined(HAVE_DEV_PTMX) && defined(HAVE_SYS_STROPTS_H)
#include <sys/stropts.h>
#endif

#if defined(HAVE_VHANGUP) && !defined(HAVE_DEV_PTMX)
#define USE_VHANGUP
#endif

#ifndef O_NOCTTY
#define O_NOCTTY 0
#endif

#ifndef PATH_TTY
#define PATH_TTY "/dev/tty"
#endif

#define set_errno(errnum) (errno = (errnum), -1)

/*

=item C<int pty_open(int *masterfd, int *slavefd, char *slavename, size_t slavenamesize, const struct termios *slave_termios, const struct winsize *slave_winsize)>

A safe version of I<openpty(3)>. Allocates and opens a pseudo terminal. The
new descriptor for the master side of the pseudo terminal is stored in
C<*masterfd>. The new descriptor for the slave side of the pseudo terminal
is stored in C<*slavefd>. The device name of the slave side of the pseudo
terminal is stored in the buffer pointed to by C<slavename> which must be
able to hold at least 64 characters. C<slavenamesize> is the size of the
buffer pointed to by C<slavename>. No more than C<slavenamesize> bytes will
be written into the buffer pointed to by C<slavename>, including the
terminating C<nul> byte. If C<slave_termios> is not null, it is passed to
I<tcsetattr(3)> with the command C<TCSANOW> to set the terminal attributes
of the slave device. If C<slave_winsize> is not null, it is passed to
I<ioctl(2)> with the command C<TIOCSWINSZ> to set the window size of the
slave device. On success, returns C<0>. On error, returns C<-1> with
C<errno> set appropriately.

=cut

*/

/*Copies C<src> into C<dst> (which is C<size> bytes long). The result, C<dst>,
will be no longer than C<size - 1> bytes and will be C<nul> terminated
(unless C<size> is zero). This is similar to I<strncpy()> except that it
always terminates the string with a C<nul> byte (so it's safer) and it
doesn't fill the remainder of the buffer with C<nul> bytes (so it's faster).
Returns the length of C<src> (If this is >= C<size>, truncation occurred).
Use this rather than I<strcpy(3)> or I<strncpy(3)>.  */

size_t strlcpy(char *dst, const char *src, size_t size)
{
    const char *s = src;
    char *d = dst;
    size_t n = size;

    if (n)
        while (--n && (*d++ = *s++))
        {}

    if (n == 0)
    {
        if (size)
            *d = '\0';

        while (*s++)
        {}
    }

    return s - src - 1;
}

static int groupname2gid(const char *groupname)
{
	FILE *group = fopen("/etc/group", "r");
	char line[BUFSIZ], *gid;
	int ret = -1;

	while (fgets(line, BUFSIZ, group))
	{
		if (!strncmp(line, "tty:", 4))
		{
			if ((gid = strchr(line + 4, ':')))
				ret = atoi(gid + 1);

			break;
		}
	}

	fclose(group);

	return ret;
}

static int uid2gid(uid_t uid)
{
	FILE *passwd = fopen("/etc/passwd", "r");
	char line[BUFSIZ], *ptr;
	int ret = -1;

	while (fgets(line, BUFSIZ, passwd))
	{
		if ((ptr = strchr(line, ':')) && (ptr = strchr(ptr + 1, ':')) &&
			atoi(ptr + 1) == (int)uid && (ptr = strchr(ptr + 1, ':')))
		{
			ret = atoi(ptr + 1);
			break;
		}
	}

	fclose(passwd);

	return ret;
}

int pty_open(int *masterfd, int *slavefd, char *slavename, size_t slavenamesize, const struct termios *slave_termios, const struct winsize *slave_winsize)
{
#if defined(HAVE_OPENPTY) || defined(BSD4_4)

	/* openpty(3) exists in OSF/1 and some other os'es */

#ifdef HAVE_TTYNAME_R
	char buf[64], *name = buf;
	int err;
#else
	char *name;
#endif

	if (!masterfd || !slavefd || !slavename || slavenamesize < 64)
		return set_errno(EINVAL);

	/* Open the master and slave descriptors, set ownership and permissions */

	if (openpty(masterfd, slavefd, NULL, NULL, NULL) == -1)
		return -1;

	/* Retrieve the device name of the slave */

#ifdef HAVE_TTYNAME_R
	if ((err = ttyname_r(*slavefd, buf, 64)))
	{
		close(*masterfd);
		close(*slavefd);
		return set_errno(err);
	}
#else
	if (!(name = ttyname(*slavefd)))
	{
		close(*masterfd);
		close(*slavefd);
		return set_errno(ENOTTY);
	}
#endif

	/* Return it to the caller */

	if (strlcpy(slavename, name, slavenamesize) >= slavenamesize)
	{
		close(*masterfd);
		close(*slavefd);
		return set_errno(ENOSPC);
	}

#else /* HAVE_OPENPTY */
#ifdef HAVE__GETPTY

	/*
	 * _getpty(3) exists in SGI Irix 4.x, 5.x & 6.x -- it generates more
	 * pty's automagically when needed
	 */

	char *slave;

	if (!masterfd || !slavefd || !slavename || slavenamesize < 64)
		return set_errno(EINVAL);

	/* Open the master descriptor and get the slave's device name */

	if (!(slave = _getpty(masterfd, O_RDWR, 0622, 0)))
		return -1;

	/* Return it to the caller */

	if (strlcpy(slavename, name, slavenamesize) >= slavenamesize)
	{
		close(*masterfd);
		return set_errno(ENOSPC);
	}

	/* Open the slave descriptor */

	if ((*slavefd = open(slavename, O_RDWR | O_NOCTTY)) == -1)
	{
		close(*masterfd);
		return -1;
	}

#else /* HAVE__GETPTY */
#if defined(HAVE_DEV_PTMX)

	/*
	 * This code is used e.g. on Solaris 2.x.  (Note that Solaris 2.3
	 * also has bsd-style ptys, but they simply do not work.)
	 */

#ifdef HAVE_PTSNAME_R
	char buf[64], *name = buf;
	int err;
#else
	char *name;
#endif

	if (!masterfd || !slavefd || !slavename || slavenamesize < 64)
		return set_errno(EINVAL);

	/* Open the master descriptor */

	if ((*masterfd = open("/dev/ptmx", O_RDWR | O_NOCTTY)) == -1)
		return -1;

	/* Set slave ownership and permissions to real uid of process */

	if (grantpt(*masterfd) == -1)
	{
		close(*masterfd);
		return -1;
	}

	/* Unlock the slave so it can be opened */

	if (unlockpt(*masterfd) == -1)
	{
		close(*masterfd);
		return -1;
	}

	/* Retrieve the device name of the slave */

#ifdef HAVE_PTSNAME_R
	if ((err = ptsname_r(*masterfd, buf, 64)))
	{
		close(*masterfd);
		return set_errno(err);
	}
#else
	if (!(name = ptsname(*masterfd)))
	{
		close(*masterfd);
		return set_errno(ENOTTY);
	}
#endif

	/* Return it to the caller */

	if (strlcpy(slavename, name, slavenamesize) >= slavenamesize)
	{
		close(*masterfd);
		return set_errno(ENOSPC);
	}

	/* Open the slave descriptor */

	if ((*slavefd = open(slavename, O_RDWR | O_NOCTTY)) == -1)
	{
		close(*masterfd);
		return -1;
	}

	/* Turn the slave into a terminal */

#ifndef HAVE_CYGWIN 
#ifndef HAVE_LINUX /* linux does not use the streams module */
	/*
	 * Push the appropriate streams modules, as described in Solaris pts(7).
	 * HP-UX pts(7) doesn't have ttcompat module.
	 */
	if (ioctl(*slavefd, I_PUSH, "ptem") == -1)
	{
		close(*masterfd);
		close(*slavefd);
		return -1;
	}

	if (ioctl(*slavefd, I_PUSH, "ldterm") == -1)
	{
		close(*masterfd);
		close(*slavefd);
		return -1;
	}

#ifndef __hpux
	if (ioctl(*slavefd, I_PUSH, "ttcompat") == -1)
	{
		close(*masterfd);
		close(*slavefd);
		return -1;
	}
#endif
#endif
#endif

#else /* HAVE_DEV_PTMX */
#ifdef HAVE_DEV_PTS_AND_PTC

	/* AIX-style pty code */

#ifdef HAVE_TTYNAME_R
	char buf[64], *name = buf;
	int err;
#else
	char *name;
#endif

	if (!masterfd || !slavefd || !slavename || slavenamesize < 64)
		return set_errno(EINVAL);

	/* Open the master descriptor */

	if ((*masterfd = open("/dev/ptc", O_RDWR | O_NOCTTY)) == -1)
		return -1;

	/* Retrieve the device name of the slave */

#ifdef HAVE_TTYNAME_R
	if ((err = ttyname_r(*masterfd, buf, 64)))
	{
		close(*masterfd);
		return set_errno(err);
	}
#else
	if (!(name = ttyname(*masterfd)))
	{
		close(*masterfd);
		return set_errno(ENOTTY);
	}
#endif

	/* Return it to the caller */

	if (strlcpy(slavename, name, slavenamesize) >= slavenamesize)
	{
		close(*masterfd);
		return set_errno(ENOSPC);
	}

	/* Open the slave descriptor */

	if ((*slavefd = open(name, O_RDWR | O_NOCTTY)) == -1)
	{
		close(*masterfd);
		return -1;
	}

#else /* HAVE_DEV_PTS_AND_PTC */

	/* BSD-style pty code */
	const char * const ptymajors = "pqrstuvwxyzabcdefghijklmnoABCDEFGHIJKLMNOPQRSTUVWXYZ";
	const char * const ptyminors = "0123456789abcdef";
	int num_minors = strlen(ptyminors);
	int num_ptys = strlen(ptymajors) * num_minors;
	char buf[64];
	int found = 0;
	int i;

	/* Identify the first available pty master device */

	for (i = 0; !found && i < num_ptys; i++)
	{
		snprintf(buf, sizeof buf, "/dev/pty%c%c", ptymajors[i / num_minors], ptyminors[i % num_minors]);
		snprintf(slavename, slavenamesize, "/dev/tty%c%c", ptymajors[i / num_minors], ptyminors[i % num_minors]);

		/* Open the master descriptor */

		if ((*masterfd = open(buf, O_RDWR | O_NOCTTY)) == -1)
		{
			/* Try SCO style naming */
			snprintf(buf, sizeof buf, "/dev/ptyp%d", i);
			snprintf(slavename, slavenamesize, "/dev/ttyp%d", i);

			if ((*masterfd = open(buf, O_RDWR | O_NOCTTY)) == -1)
				continue;
		}

		/* Set slave ownership and permissions to real uid of process */

		pty_set_owner(slavename, getuid());

		/* Open the slave descriptor */

		if ((*slavefd = open(slavename, O_RDWR | O_NOCTTY)) == -1)
		{
			close(*masterfd);
			return -1;
		}

		found = 1;
	}

	if (!found)
		return set_errno(ENOENT);

#endif /* HAVE_DEV_PTS_AND_PTC */
#endif /* HAVE_DEV_PTMX */
#endif /* HAVE__GETPTY */
#endif /* HAVE_OPENPTY */

	/* Set the slave's terminal attributes if requested */

	if (slave_termios && tcsetattr(*slavefd, TCSANOW, slave_termios) == -1)
	{
		close(*masterfd);
		close(*slavefd);
		return -1;
	}

	/* Set the slave's window size if required */

	if (slave_winsize && ioctl(*slavefd, TIOCSWINSZ, slave_winsize) == -1)
	{
		close(*masterfd);
		close(*slavefd);
		return -1;
	}

	return 0;
}

/*

=item C<int pty_release(const char *slavename)>

Releases the slave tty device whose name is in C<slavename>. Its ownership
is returned to root, and its permissions set to C<rw-rw-rw->. Note that only
root can execute this function successfully on most systems. On success,
returns C<0>. On error, returns C<-1> with C<errno> set appropriately.

=cut

*/

int pty_release(const char *slavename)
{
	if (!slavename)
		return set_errno(EINVAL);

	if (chown(slavename, (uid_t)0, (gid_t)0) == -1)
		return -1;

	if (chmod(slavename, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) == -1)
		return -1;

	return 0;
}

/*

=item C<int pty_set_owner(const char *slavename, uid_t uid)>

Changes the ownership of the slave pty device referred to by C<slavename> to
the user id, C<uid>. Group ownership of the slave pty device will be changed
to the C<tty> group if it exists. Otherwise, it will be changed to the given
user's primary group. The slave pty device's permissions are set to
C<rw--w---->. Note that only root can execute this function successfully on
most systems. Also note that the ownership of the device is automatically
set to the real uid of the process by I<pty_open()> and I<pty_fork()>. The
permissions are also set automatically by these functions. So
I<pty_set_owner()> is only needed when the device needs to be owned by some
user other than the real user. On success, returns C<0>. On error, returns
C<-1> with C<errno> set appropriately.

=cut

*/

int pty_set_owner(const char *slavename, uid_t uid)
{
	mode_t mode = S_IRUSR | S_IWUSR | S_IWGRP;
	struct stat status[1];
	int gid;

	if (stat(slavename, status) == -1)
		return -1;

	if ((gid = groupname2gid("tty")) == -1)
	{
		gid = uid2gid(uid);
		mode |= S_IWOTH;
	}

	if (status->st_uid != uid || status->st_gid != (unsigned int)gid)
		if (chown(slavename, uid, gid) == -1)
			if (errno != EROFS || status->st_uid != uid)
				return -1;

	if ((status->st_mode & (S_IRWXU | S_IRWXG | S_IRWXO)) != mode)
		if (chmod(slavename, mode) == -1)
			if (errno != EROFS || (status->st_mode & (S_IRGRP | S_IROTH)))
				return -1;

	return 0;
}

/*

=item C<int pty_make_controlling_tty(int *slavefd, const char *slavename)>

Makes the slave pty the controlling terminal. C<*slavefd> contains the
descriptor for the slave side of a pseudo terminal. The descriptor of the
resulting controlling terminal will be stored in C<*slavefd>. C<slavename>
is the device name of the slave side of the pseudo terminal. On success,
returns C<0>. On error, returns C<-1> with C<errno> set appropriately.

=cut

*/

int pty_make_controlling_tty(int *slavefd, const char *slavename)
{
	int fd;
#ifdef USE_VHANGUP
	void (*old)(int);
#endif /* USE_VHANGUP */

	if (!slavefd || *slavefd < 0 || !slavename)
		return set_errno(EINVAL);

	/* First disconnect from the old controlling tty */
#ifdef TIOCNOTTY
	if ((fd = open(PATH_TTY, O_RDWR | O_NOCTTY)) >= 0)
	{
		ioctl(fd, TIOCNOTTY, NULL);
		close(fd);
	}
#endif /* TIOCNOTTY */

	setsid();

	/*
	 * Verify that we are successfully disconnected from the controlling
	 * tty.
	 */
#if 0
	if ((fd = open(PATH_TTY, O_RDWR | O_NOCTTY)) >= 0)
	{
		close(fd);
		return set_errno(ENXIO);
	}
#endif

	/* Make it our controlling tty */
#ifdef TIOCSCTTY
	if (ioctl(*slavefd, TIOCSCTTY, NULL) == -1)
		return -1;
#endif /* TIOCSCTTY */
#ifdef HAVE_NEWS4
	setpgrp(0, 0);
#endif /* HAVE_NEWS4 */
#ifdef USE_VHANGUP
	old = signal(SIGHUP, SIG_IGN);
	vhangup();
	signal(SIGHUP, old);
#endif /* USE_VHANGUP */
	/* Why do this? */
	if ((fd = open(slavename, O_RDWR)) >= 0)
	{
#ifdef USE_VHANGUP
	close(*slavefd);
	*slavefd = fd;
#else /* USE_VHANGUP */
	close(fd);
#endif /* USE_VHANGUP */
	}

	/* Verify that we now have a controlling tty */
	if ((fd = open(PATH_TTY, O_RDWR)) == -1)
		return -1;

	close(fd);

	return 0;
}

/*

=item C<int pty_change_window_size(int masterfd, int row, int col, int xpixel, int ypixel)>

Changes the window size associated with the pseudo terminal referred to by
C<masterfd>. The C<row>, C<col>, C<xpixel> and C<ypixel> specify the new
window size. On success, returns C<0>. On error, returns C<-1> with C<errno>
set appropriately.

=cut

*/

int pty_change_window_size(int masterfd, int row, int col, int xpixel, int ypixel)
{
	struct winsize win;

	if (masterfd < 0 || row < 0 || col < 0 || xpixel < 0 || ypixel < 0)
		return set_errno(EINVAL);

	win.ws_row = row;
	win.ws_col = col;
	win.ws_xpixel = xpixel;
	win.ws_ypixel = ypixel;

	return ioctl(masterfd, TIOCSWINSZ, &win);
}

/*

=item C<pid_t pty_fork(int *masterfd, char *slavename, size_t slavenamesize, const struct termios *slave_termios, const struct winsize *slave_winsize)>

A safe version of I<forkpty(3)>. Creates a pseudo terminal and then calls
I<fork(2)>. In the parent process, the slave side of the pseudo terminal is
closed. In the child process, the master side of the pseudo terminal is
closed and the slave side is made the controlling terminal. It is duplicated
onto standard input, output and error and then closed. The master side of
the pseudo terminal is stored in C<*masterfd> for the parent process. The
device name of the slave side of the pseudo terminal is stored in the buffer
pointed to by C<slavename> which must be able to hold at least 64 bytes.
C<slavenamesize> is the size of the buffer pointed to by C<slavename>. No
more than C<slavenamesize> bytes will be written to C<slavename>, including
the terminating C<nul> byte. If C<slave_termios> is not null, it is passed
to I<tcsetattr(3)> with the command C<TCSANOW> to set the terminal
attributes of the slave device. If C<slave_winsize> is not null, it is
passed to I<ioctl(2)> with the command C<TIOCSWINSZ> to set the window size
of the slave device. On success, returns C<0> to the child process and
returns the process id of the child process to the parent process. On error,
returns C<-1> with C<errno> set appropriately.

=cut

*/

pid_t pty_fork(int *masterfd, char *slavename, size_t slavenamesize, const struct termios *slave_termios, const struct winsize *slave_winsize)
{
	int slavefd;
	pid_t pid;

	/*
	** Note: we don't use forkpty() because it closes the master in the
	** child process before making the slave the controlling terminal of the
	** child proces and this can prevent the slave from becoming the
	** controlling terminal (but I have no idea why).
	*/

	if (pty_open(masterfd, &slavefd, slavename, slavenamesize, slave_termios, slave_winsize) == -1)
		return -1;

	switch (pid = fork())
	{
		case -1:
			pty_release(slavename);
			close(slavefd);
			close(*masterfd);
			return -1;

		case 0:
		{
			/* Make the slave our controlling tty */
			if (pty_make_controlling_tty(&slavefd, slavename) == -1)
				_exit(EXIT_FAILURE);

			/* Redirect stdin, stdout and stderr from the pseudo tty */

			if (slavefd != STDIN_FILENO && dup2(slavefd, STDIN_FILENO) == -1)
				_exit(EXIT_FAILURE);

			if (slavefd != STDOUT_FILENO && dup2(slavefd, STDOUT_FILENO) == -1)
				_exit(EXIT_FAILURE);

			if (slavefd != STDERR_FILENO && dup2(slavefd, STDERR_FILENO) == -1)
				_exit(EXIT_FAILURE);

			/* Close the extra descriptor for the pseudo tty */

			if (slavefd != STDIN_FILENO && slavefd != STDOUT_FILENO && slavefd != STDERR_FILENO)
				close(slavefd);

			/* Close the master side of the pseudo tty in the child */

			close(*masterfd);

			return 0;
		}

		default:
		{
			/* Close the slave side of the pseudo tty in the parent */

			close(slavefd);

			return pid;
		}
	}
}

/*

=back

=head1 ERRORS

Additional errors may be generated and returned from the underlying system
calls. See their manual pages.

=over 4

=item EINVAL

Invalid arguments were passed to one of the functions.

=item ENOTTY

I<openpty()> or I<open("/dev/ptc")> returned a slave descriptor for which
I<ttyname()> failed to return the slave device name. I<open("/dev/ptmx")>
returned a master descriptor for which I<ptsname()> failed to return the the
slave device name.

=item ENOENT

The old BSD-style pty device search failed to locate an available pseudo
terminal.

=item ENOSPC

The device name of the slave side of the pseudo terminal was too large to
fit in the C<slavename> buffer passed to I<pty_open()> or I<pty_fork()>.

=item ENXIO

I<pty_make_controlling_tty()> failed to disconnect from the controlling
terminal.

=back

=head1 EXAMPLE

A very simple pty program:

    #include <slack/std.h>
    #include <slack/pseudo.h>

    #include <sys/select.h>
    #include <sys/wait.h>

    int main(int ac, char **av)
    {
        char slavename[64];
        int masterfd;
        pid_t pid;

        if (ac == 1)
        {
            fprintf(stderr, "usage: pty command [arg...]\n");
            return EXIT_FAILURE;
        }

        switch (pid = pty_fork(&masterfd, slavename, sizeof slavename, NULL, NULL))
        {
            case -1:
                fprintf(stderr, "pty: pty_fork() failed (%s)\n", strerror(errno));
                pty_release(slavename);
                return EXIT_FAILURE;

            case 0:
                execvp(av[1], av + 1);
                return EXIT_FAILURE;

            default:
            {
                int infd = STDIN_FILENO;
                int status;

                while (masterfd != -1)
                {
                    fd_set readfds[1];
                    int maxfd;
                    char buf[BUFSIZ];
                    ssize_t bytes;
                    int n;

                    FD_ZERO(readfds);

                    if (infd != -1)
                        FD_SET(infd, readfds);

                    if (masterfd != -1)
                        FD_SET(masterfd, readfds);

                    maxfd = (masterfd > infd) ? masterfd : infd;

                    if ((n = select(maxfd + 1, readfds, NULL, NULL, NULL)) == -1 && errno != EINTR)
                        break;

                    if (n == -1 && errno == EINTR)
                        continue;

                    if (infd != -1 && FD_ISSET(infd, readfds))
                    {
                        if ((bytes = read(infd, buf, BUFSIZ)) > 0)
                        {
                            if (masterfd != -1 && write(masterfd, buf, bytes) == -1)
                                break;
                        }
                        else if (n == -1 && errno == EINTR)
                        {
                            continue;
                        }
                        else
                        {
                            infd = -1;
                            continue;
                        }
                    }

                    if (masterfd != -1 && FD_ISSET(masterfd, readfds))
                    {
                        if ((bytes = read(masterfd, buf, BUFSIZ)) > 0)
                        {
                            if (write(STDOUT_FILENO, buf, bytes) == -1)
                                break;
                        }
                        else if (n == -1 && errno == EINTR)
                        {
                            continue;
                        }
                        else
                        {
                            masterfd = -1;
                            continue;
                        }
                    }
                }

                if (waitpid(pid, &status, 0) == -1)
                {
                    fprintf(stderr, "pty: waitpid(%d) failed (%s)\n", (int)pid, strerror(errno));
                    pty_release(slavename);
                    return EXIT_FAILURE;
                }
            }
        }

        pty_release(slavename);
        close(masterfd);

        return EXIT_SUCCESS;
    }

=head1 MT-Level

MT-Safe if and only if I<ttyname_r()> or I<ptsname_r()> are available when
needed. On systems that have I<openpty()> or C<"/dev/ptc">, I<ttyname_r()>
is required, otherwise the unsafe I<ttyname()> will be used. On systems that
have C<"/dev/ptmx">, I<ptsname_r()> is required, otherwise the unsafe
I<ptsname()> will be used. On systems that have I<_getpty()>, I<pty_open()>
is unsafe because I<_getpty()> is unsafe. In short, it's MT-Safe under Linux,
Unsafe under Solaris and OpenBSD.

=head1 SEE ALSO

L<libslack(3)|libslack(3)>,
L<openpty(3)|openpty(3)>,
L<forkpty(3)|forkpty(3)>
L<open(2)|open(2)>,
L<close(2)|close(2)>,
L<grantpt(3)|grantpt(3)>,
L<unlockpt(3)|unlockpt(3)>,
L<ioctl(2)|ioctl(2)>,
L<ttyname(3)|ttyname(3)>,
L<ttyname_r(3)|ttyname_r(3)>,
L<ptsname(3)|ptsname(3)>,
L<ptsname_r(3)|ptsname_r(3)>,
L<setpgrp(2)|setpgrp(2)>,
L<vhangup(2)|vhangup(2)>,
L<setsid(2)|setsid(2)>,
L<_getpty(2)|_getpty(2)>,
L<chown(2)|chown(2)>,
L<chmod(2)|chmod(2)>,
L<tcsetattr(3)|tcsetattr(3)>,
L<setpgrp(2)|setpgrp(2)>,
L<fork(2)|fork(2)>,
L<dup2(2)|dup2(2)>

=head1 AUTHOR

1995 Tatu Ylonen <ylo@cs.hut.fi>, 2001 raf <raf@raf.org>

=cut

*/

#endif

#ifdef TEST

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <pwd.h>

#include <sys/types.h>
#include <sys/wait.h>

int main(int ac, char **av)
{
	int errors = 0;
	char slavename[64];
	int	masterfd, slavefd;
	uid_t euid = geteuid();
	pid_t pid;

	printf("Testing: pseudo\n");

	/* Test pty_open() */

	if (pty_open(&masterfd, &slavefd, slavename, sizeof slavename, NULL, NULL) == -1)
		++errors, printf("Test1: pty_open() failed (%s)\n", strerror(errno));
	else
	{
		/* Test pty_set_owner() if root */

		if (euid == 0 && pty_set_owner(slavename, getuid()) == -1)
			++errors, printf("Test2: pty_set_owner() failed (%s)\n", strerror(errno));

		/* Test pty_make_controlling_tty() and pty_change_window_size() */

		switch (pid = fork())
		{
			case -1:
				++errors, printf("Test3: failed to perform test: fork() failed (%s)\n", strerror(errno));
				break;

			case 0:
			{
				errors = 0;
				close(masterfd);

				if (pty_make_controlling_tty(&slavefd, slavename) == -1)
					++errors, printf("Test4: pty_make_controlling_tty() failed (%s)\n", strerror(errno));

				if (pty_change_window_size(slavefd, 80, 24, 800, 240) == -1)
					++errors, printf("Test5: pty_change_window_size() failed (%s)\n", strerror(errno));

				exit(errors);
			}

			default:
			{
				int status;

				if (waitpid(pid, &status, 0) == -1)
					++errors, printf("Test6: failed to evaluate test: waitpid() failed (%s)\n", strerror(errno));
				else if (WIFSIGNALED(status))
					++errors, printf("Test6: failed to evaluate test: child process received signal %d\n", WTERMSIG(status));
				else
					errors += WEXITSTATUS(status);

				break;
			}
		}

		/* Test pty_release() if root */

		if (euid == 0 && pty_release(slavename) == -1)
			++errors, printf("Test7: pty_release() failed (%s)\n", strerror(errno));
	}

	/* Test pty_fork() */

	switch (pid = pty_fork(&masterfd, slavename, sizeof slavename, NULL, NULL))
	{
		case -1:
			++errors, printf("Test8: pty_fork() failed (%s)\n", strerror(errno));
			break;

		case 0:
			exit(isatty(STDIN_FILENO) ? 0 : 1);
			break; /* unreached */

		default:
		{
			int status;

			if (waitpid(pid, &status, 0) == -1)
				++errors, printf("Test9: failed to evaluate test: waitpid() failed (%s)\n", strerror(errno));
			else if (WIFSIGNALED(status))
				++errors, printf("Test9: failed to evaluate test: child process received signal %d\n", WTERMSIG(status));
			else if (WEXITSTATUS(status))
				++errors, printf("Test9: pty_fork() failed to result in a tty for child\n");

			if (euid == 0 && pty_release(slavename) == -1)
				++errors, printf("Test10: pty_release() failed (%s)\n", strerror(errno));

			break;
		}
	}

	if (errors)
		printf("%d/10 tests failed\n", errors);
	else
		printf("All tests passed\n");

	if (euid)
	{
		printf("\n");
		printf("   Note: Can't test pty_set_owner() or pty_release()\n");
		printf("   On some systems (e.g. Linux) these aren't needed anyway.\n");
		printf("   Audit the code and rerun the test as root (or setuid root)\n");
	}

	return (errors == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

#endif

/* vi:set ts=4 sw=4: */
