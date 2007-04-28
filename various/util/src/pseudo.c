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

#include "config.h"
#define _GNU_SOURCE /* ptsname_r() under Linux */
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
/* With out this, cgdb will crash on gentoo when built with a 64 bit machine */
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
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


/* strlcpy_local: Used instead of strcpy. Copies src into dst size bytes long.
 *
 * dst: destination
 * src: source
 * size: amount to copy from source to dest.
 *
 * NOTE: dst will be no longer than size - 1 bytes and will be nul 
 * terminated (unless size is zero). This is similar to strncpy() 
 * except that it always terminates the string with a nul byte 
 * (so it's safer) and it doesn't fill the remainder of the buffer 
 * with nul bytes (so it's faster).  Returns the length of src 
 * (If this is >= size, truncation occurred).
 */

static size_t strlcpy_local(char *dst, const char *src, size_t size) {
    const char *s = src;
    char *d = dst;
    size_t n = size;

    if (n)
        while (--n && (*d++ = *s++)) {}

    if (n == 0) {
        if (size)
            *d = '\0';

        while (*s++) {}
    }

    return s - src - 1;
}

static int groupname2gid(const char *groupname) {
	FILE *group = fopen("/etc/group", "r");
	char line[BUFSIZ], *gid;
	int ret = -1;

	while (fgets(line, BUFSIZ, group)) {
		if (!strncmp(line, "tty:", 4)) {
			if ((gid = strchr(line + 4, ':')))
				ret = atoi(gid + 1);

			break;
		}
	}

	fclose(group);

	return ret;
}

static int uid2gid(uid_t uid) {
	FILE *passwd = fopen("/etc/passwd", "r");
	char line[BUFSIZ], *ptr;
	int ret = -1;

	while (fgets(line, BUFSIZ, passwd)) {
		if ((ptr = strchr(line, ':')) && (ptr = strchr(ptr + 1, ':')) &&
			atoi(ptr + 1) == (int)uid && (ptr = strchr(ptr + 1, ':'))) {
			ret = atoi(ptr + 1);
			break;
		}
	}

	fclose(passwd);

	return ret;
}

/* pty_open: A safe version of openpty. 
 * Allocates and opens a pseudo terminal. The new descriptor for the 
 * master side of the pseudo terminal is stored in masterfd. The new 
 * descriptor for the slave side of the pseudo terminal is stored in slavefd. 
 * The device name of the slave side of the pseudo terminal is stored in the 
 * buffer pointed to by slavename which must be able to hold at least 64 
 * characters. slavenamesize is the size of the buffer pointed to by 
 * slavename. No more than slavenamesize bytes will be written into the 
 * buffer pointed to by slavename, including the terminating nul byte. 
 * If slave_termios is not null, it is passed to tcsetattr with the 
 * command TCSANOW to set the terminal attributes of the slave device. 
 * If slave_winsize is not null, it is passed to ioctl with the command 
 * TIOCSWINSZ to set the window size of the slave device. On success, 
 * returns 0. On error, returns -1 with errno set appropriately.
 */
int pty_open(int *masterfd, int *slavefd, char *slavename, size_t slavenamesize, const struct termios *slave_termios, const struct winsize *slave_winsize) {
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

	if (strlcpy_local(slavename, name, slavenamesize) >= slavenamesize)
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

	if (strlcpy_local(slavename, name, slavenamesize) >= slavenamesize)
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

	if (strlcpy_local(slavename, name, slavenamesize) >= slavenamesize)
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

	if (strlcpy_local(slavename, name, slavenamesize) >= slavenamesize)
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

/* pty_release: Releases the slave tty device whose name is in slavename.
 *
 * slavename: the device name of the slave side of the pseudo terminal
 * Returns  : 0 on success or -1 on error.
 *
 * Note: Its ownership is returned to root, and its permissions set to 
 * rw-rw-rw-. Note that only root can execute this function successfully 
 * on most systems. 
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

/* pty_set_owner: Changes the ownership of the slave pty device referred to by
 *                slavename to the user id uid.
 *
 * slavename: the device name of the slave side of the pseudo terminal
 * uid      : The new desired user id.
 * Returns  : 0 on success or -1 on error.
 *
 * NOTE: Group ownership of the slave pty device will be changed
 * to the tty group if it exists. Otherwise, it will be changed to the given
 * user's primary group. The slave pty device's permissions are set to
 * rw--w----. Note that only root can execute this function successfully on
 * most systems. Also note that the ownership of the device is automatically
 * set to the real uid of the process by pty_open() and pty_fork(). The
 * permissions are also set automatically by these functions. So
 * pty_set_owner() is only needed when the device needs to be owned by some
 * user other than the real user. 
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

/* pty_make_controlling_tty: Makes the slave pty the controlling terminal. 
 *
 * slavefd contains the descriptor for the slave side of a pseudo terminal. 
 * slavename: the device name of the slave side of the pseudo terminal
 * Returns  : 0 on success or -1 on error.
 *
 * NOTE: The descriptor of the resulting controlling terminal will be stored 
 * in slavefd. 
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

/* pty_change_window_size: Changes the window size of the pseudo terminal
 *
 * masterfd: The pseudo terminal to change
 * row:      The new number of rows.
 * col:      The new number of cols.
 * xpixel:   Number of pixels in x dir
 * ypixel:   Number of pixels in y dir
 * Returns  : 0 on success or -1 on error.
 *
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

/* pty_fork:  Creates a pseudo terminal and then calls fork. In the parent 
 * process, the slave side of the pseudo terminal is closed. In the child 
 * process, the master side of the pseudo terminal is closed and the slave 
 * side is made the controlling terminal. It is duplicated onto standard 
 * input, output and error and then closed. The master side of the pseudo 
 * terminal is stored in masterfd for the parent process. The device name 
 * of the slave side of the pseudo terminal is stored in the buffer pointed 
 * to by slavename which must be able to hold at least 64 bytes.  
 * slavenamesize is the size of the buffer pointed to by slavename. No
 * more than slavenamesize bytes will be written to slavename, including 
 * the terminating nul byte. If slave_termios is not null, it is passed to 
 * tcsetattr with the command TCSANOW to set the terminal attributes of the 
 * slave device. If slave_winsize is not null, it is passed to ioctl with 
 * the command TIOCSWINSZ to set the window size of the slave device. 
 * On success, returns 0 to the child process and returns the process 
 * id of the child process to the parent process. On error, returns -1 with 
 * errno set appropriately.
 */
pid_t pty_fork(int *masterfd, char *slavename, size_t slavenamesize, const struct termios *slave_termios, const struct winsize *slave_winsize)
{
	int slavefd = 0;
	pid_t pid = 0;

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
