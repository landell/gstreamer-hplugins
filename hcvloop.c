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
#include "hcvloop.h"
#include "hcverror.h"
#include "device.h"

void device_loop (V4l2Device *device)
{
	DeviceErrors ret;
	fd_set fds;
	int r;
	FD_ZERO (&fds);
	FD_SET (device->fd, &fds);
	r = select (device->fd + 1, &fds, NULL, NULL, NULL);
	if (r == -1 || r == 0)
	{
		fprintf (stderr, "Error on select: %s\n", strerror (errno));
	}
	else
	{
		ret = device_getframe (device);
		if (ret != DEVICE_OK)
			fprintf (stderr, "Could not get frame: %s\n",
				device_error (ret));
		else if ((ret = save_picture (device)) != DEVICE_OK)
			fprintf (stderr, "Coult not save frame: %s\n",
				device_error (ret));
	}
}
