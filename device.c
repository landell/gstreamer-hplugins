/*
    This file is part of v4l2capture.

    V4l2capture is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    V4l2 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.

    Copyright 2008 Samuel R. C. Vale
    srcvale@holoscopio.com
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <linux/videodev.h>
#include "device.h"

int device_open (V4l2Device *dev)
{
	struct stat st;

	if (stat (dev->name, &st) == -1)
		return DEVICE_INVALID;

	if (!S_ISCHR (st.st_mode))
		return DEVICE_INVALID;

	if ((dev->fd = open (dev->name, O_RDWR | O_NONBLOCK, 0)) < 0)
		return DEVICE_INVALID;

	return DEVICE_OK;
}

int device_init (V4l2Device *dev)
{
	struct v4l2_capability device_capability;
	struct v4l2_cropcap device_cropcap;
	struct v4l2_crop device_crop;
	struct v4l2_format image_format;
	unsigned int min;

	if (ioctl (dev->fd, VIDIOC_QUERYCAP, &device_capability) < 0)
		return DEVICE_IS_NOT_V4L2;

	if (!(device_capability.capabilities & V4L2_CAP_VIDEO_CAPTURE))
		return DEVICE_DONT_CAPTURE;

	// TODO: Probe for Read and Write interface, and use it if
	// available!
	//if (!(device_capability.capabilities & V4L2_CAP_READWRITE))
	//	return DEVICE_MODE_NOT_SUPPORTED;

	image_format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	image_format.fmt.pix.width = dev->width; 
	image_format.fmt.pix.height = dev->height;
	image_format.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
	image_format.fmt.pix.field = V4L2_FIELD_ANY;

	if (ioctl (dev->fd, VIDIOC_S_FMT, &image_format) < 0)
		return DEVICE_FORMAT_INVALID;

        /* Buggy driver paranoia. */
/*        min = fmt.fmt.pix.width * 2;
        if (fmt.fmt.pix.bytesperline < min)
                fmt.fmt.pix.bytesperline = min;
        min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
        if (fmt.fmt.pix.sizeimage < min)
                fmt.fmt.pix.sizeimage = min;

        switch (io) {
        case IO_METHOD_READ:
                init_read (fmt.fmt.pix.sizeimage);
                break;
*/

}

int device_getframe (V4l2Device *dev)
{
	return 0;
}

int device_close (V4l2Device *dev)
{
	return close (dev->fd);
}

