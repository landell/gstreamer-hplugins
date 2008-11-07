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


#include <sys/ioctl.h>
#include <errno.h>
#include <linux/videodev2.h>
#include "device.h"

int device_negotiate (V4l2Device *dev, u_int32_t *formats)
{
	struct v4l2_fmtdesc fmtdesc;
	int i;
	int min;
	errno = 0;
	fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	min = -1;
	for (fmtdesc.index = 0; errno == 0; fmtdesc.index++)
	{
		u_int32_t *fmt;
		ioctl (dev->fd, VIDIOC_ENUM_FMT, &fmtdesc);
		for (i = 0, fmt = formats; *fmt != 0; i++, fmt++)
			if (fmtdesc.pixelformat == *fmt &&
			    (min == -1 || i < min))
				min = i;
	}
	if (min >= 0)
	{
		dev->pixelformat = formats[min];
		return 0;
	}
	return 1;
}
