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
 * Functions for allocating a pseudo-terminal and making it the controlling
 * tty.
 *
 * As far as I am concerned, the code I have written for this software
 * can be used freely for any purpose.  Any derived versions of this
 * software must be clearly marked as such, and if the derived work is
 * incompatible with the protocol description in the RFC file, it must be
 * called by a name other than "ssh" or "Secure Shell".
 */

#ifndef LIBSLACK_PSEUDO_H
#define LIBSLACK_PSEUDO_H

#include <termios.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#define SLAVE_SIZE 64

int pty_open(int *masterfd, int *slavefd, char *slavename, size_t slavenamesize, const struct termios *slave_termios, const struct winsize *slave_winsize);
int pty_release(const char *slavename);
int pty_set_owner(const char *slavename, uid_t uid);
int pty_make_controlling_tty(int *slavefd, const char *slavename);
int pty_change_window_size(int masterfd, int row, int col, int xpixel, int ypixel);
pid_t pty_fork(int *masterfd, char *slavename, size_t slavenamesize, const struct termios *slave_termios, const struct winsize *slave_winsize);

#endif
