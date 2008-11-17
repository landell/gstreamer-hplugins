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
#include "hcvloop.h"
#include "hcverror.h"
#include "device.h"

void device_loop (V4l2Device *device)
{
	DeviceErrors ret;
	while ((ret = device_getframe (device)) == DEVICE_NOT_READY);
	if (ret != DEVICE_OK)
		fprintf (stderr, "Could not get frame: %s\n",
			device_error (ret));
	else
		save_picture (device);
}
