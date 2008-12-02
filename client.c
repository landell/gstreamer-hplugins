/*
 *  Copyright (C) 2008  Thadeu Lima de Souza Cascardo <cascardo@holoscopio.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

static int hcv_client (void)
{
	int fd;
	struct sockaddr_un saddr;
	socklen_t slen;
	fd = socket (PF_UNIX, SOCK_DGRAM, 0);
	if (fd < 0)
		return -1;
	saddr.sun_family = AF_UNIX;
	strcpy (saddr.sun_path, "/var/run/hcv/local");
	slen = sizeof (struct sockaddr) + strlen (saddr.sun_path) + 1;
	if (connect (fd, (struct sockaddr *) &saddr, slen) < 0)
	{
		close (fd);
		return -1;
	}
	return fd;
}

int main (int argc, char **argv)
{
	int sfd;
	sfd = hcv_client ();
	if (sfd < 0)
	{
		fprintf(stderr,
			"Signal not sent. Is v4l2capture running as daemon?\n");
		return 1;
	}
	write (sfd, "\n", 1);
	close (sfd);
	return 0;
}
