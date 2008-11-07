/*  Copyright 2008 Samuel R. C. Vale <srcvale@holoscopio.com>

    This software was based on V4L2 Documentation, and 
    luvcview (Copyright 2005 2006 2007 Laurent Pinchart, Michel
    Xhaard, and 2006 Gabriel A. Devenyi).

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
    along with v4l2capture.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <linux/videodev2.h>
#include <asm/types.h>

#include "device.h"

#define MAX_BUFFERS 20
#define MIN_BUFFERS 5
#define MIN_HEADER 0xaf

int device_open (V4l2Device *dev)
{
	dev->buffer = NULL;
	dev->framebuffer = NULL;

	if ((dev->fd = open (dev->name, O_RDWR | O_NONBLOCK, 0)) < 0)
		return DEVICE_INVALID;

	memset (&dev->device_capability, 0, sizeof(struct v4l2_capability));
	if (ioctl (dev->fd, VIDIOC_QUERYCAP, &dev->device_capability) < 0)
	{
		close (dev->fd);
		return DEVICE_IS_NOT_V4L2;
	}

	if (!(dev->device_capability.capabilities & V4L2_CAP_VIDEO_CAPTURE))
	{
		close (dev->fd);
		return DEVICE_DONT_CAPTURE;
	}

	dev->buffersize = dev->width * dev->height * 2;
	dev->framebuffer = malloc(dev->buffersize);

	if (dev->framebuffer == NULL)
	{
		close (dev->fd);
		return DEVICE_OUT_OF_MEMORY;
	}

	return DEVICE_OK;
}

int device_init (V4l2Device *dev)
{
	struct v4l2_format image_format;
	struct v4l2_requestbuffers b_req;
	struct v4l2_streamparm setfps;
	unsigned int min;
	int b;

	memset (&image_format, 0, sizeof(struct v4l2_format));
	memset (&b_req, 0, sizeof(struct v4l2_requestbuffers));
	memset (&setfps, 0, sizeof(struct v4l2_streamparm));

	/* TODO: Test if device is already initialized. */

	if (!(dev->device_capability.capabilities & V4L2_CAP_STREAMING))
		return DEVICE_MODE_NOT_SUPPORTED;

	/* TODO: Probe for Read and Write interface, and use it if
	 * available!
	 *if (!(device_capability.capabilities & V4L2_CAP_READWRITE))
	 *	return DEVICE_MODE_NOT_SUPPORTED;
	 */

	image_format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	image_format.fmt.pix.width = dev->width; 
	image_format.fmt.pix.height = dev->height;
	image_format.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
	image_format.fmt.pix.field = V4L2_FIELD_ANY;

	if (ioctl (dev->fd, VIDIOC_S_FMT, &image_format) < 0)
		return DEVICE_INVALID_FORMAT;

	if ((image_format.fmt.pix.width != dev->width) ||
		(image_format.fmt.pix.height != dev->height)) 
		return DEVICE_INVALID_FORMAT;

        /* Buggy driver paranoia. */
	min = image_format.fmt.pix.width * 2;
        if (image_format.fmt.pix.bytesperline < min)
                image_format.fmt.pix.bytesperline = min;
        min = image_format.fmt.pix.bytesperline * image_format.fmt.pix.height;
        if (image_format.fmt.pix.sizeimage < min)
                image_format.fmt.pix.sizeimage = min;

	/* Framerate */
	if ((dev->fps < 1) || (dev->fps > 30))
		dev->fps = 15;

	setfps.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	setfps.parm.capture.timeperframe.numerator = 1;
	setfps.parm.capture.timeperframe.denominator = dev->fps;
	ioctl(dev->fd, VIDIOC_S_PARM, &setfps);

	/* Request Buffers for MMAP */
	b_req.count = MAX_BUFFERS;
	b_req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	b_req.memory = V4L2_MEMORY_MMAP;

	if (ioctl (dev->fd, VIDIOC_REQBUFS, &b_req) < 0)
	{
                if (EINVAL == errno)
			return DEVICE_MODE_NOT_SUPPORTED;
		else
			return DEVICE_ERROR; /* ? */
	}

	if (b_req.count < MIN_BUFFERS)
		return DEVICE_OUT_OF_MEMORY;

        dev->buffer = malloc (b_req.count * sizeof (DeviceBuffer));
	
	if (!dev->buffer)
		return DEVICE_OUT_OF_MEMORY;

	/* buffer mapping */
	dev->n_buffers = b_req.count;
	for (b = 0; b < dev->n_buffers; ++b)
	{
		/* TODO: remove this "buf" allocation, if useless. */
		struct v4l2_buffer buf;
		memset (&buf, 0, sizeof(struct v4l2_buffer));

		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = b;

		if (ioctl (dev->fd, VIDIOC_QUERYBUF, &buf) < 0)
			return DEVICE_BUFFER_ERROR;

		dev->buffer[b].length = buf.length;
		dev->buffer[b].start = mmap (NULL, buf.length,
			PROT_READ | PROT_WRITE,	MAP_SHARED, dev->fd, buf.m.offset);

		if (dev->buffer[b].start == MAP_FAILED)
			return DEVICE_BUFFER_ERROR;
        }

	return DEVICE_OK;
}

int device_start_capture (V4l2Device *dev)
{
	/* TODO: Teste if capture is running. */
	unsigned int i;
	enum v4l2_buf_type type;

	for (i = 0; i < dev->n_buffers; ++i)
	{
		struct v4l2_buffer buf;
		memset (&buf, 0, sizeof(struct v4l2_buffer));

		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;

		if (ioctl (dev->fd, VIDIOC_QBUF, &buf) < 0)
                	return DEVICE_BUFFER_ERROR;
	}

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (ioctl (dev->fd, VIDIOC_STREAMON, &type) < 0)
		return DEVICE_STREAM_ERROR;

	return DEVICE_OK;
}

int device_getframe (V4l2Device *dev)
{
	struct v4l2_buffer buf;

	memset (&buf, 0, sizeof(struct v4l2_buffer));

	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;

	if (ioctl (dev->fd, VIDIOC_DQBUF, &buf) < 0) 
	{
		switch (errno)
		{
			case EAGAIN:
				return DEVICE_NOT_READY;

			case EIO:
				/* Could ignore EIO, see spec. */
				/* fall through */
			default:
				return DEVICE_STREAM_ERROR;
		}
	}

	if (buf.index >= dev->n_buffers)
		return DEVICE_BUFFER_ERROR;

	/* Ignore an empty frame */
	if (buf.bytesused <= MIN_HEADER)
		return DEVICE_EMPTY_FRAME;

	memcpy (dev->framebuffer, dev->buffer[buf.index].start,
		buf.bytesused);
	dev->buffersize = buf.bytesused;

	if (ioctl (dev->fd, VIDIOC_QBUF, &buf) < 0)
		return DEVICE_BUFFER_ERROR;

	return DEVICE_OK;
}

int device_stop_capture (V4l2Device *dev)
{
	enum v4l2_buf_type type;

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (ioctl (dev->fd, VIDIOC_STREAMOFF, &type) < 0)
		return DEVICE_STREAM_ERROR;

	return DEVICE_OK;
}

int device_close (V4l2Device *dev)
{
	/* TODO: Free allocated buffers before close device. */

	if (dev->framebuffer)
		free (dev->framebuffer);
	if (dev->buffer)
	{
		int i;
		for (i = 0; i < dev->n_buffers; ++i)
			if (dev->buffer[i].start)
				munmap (dev->buffer[i].start,
				dev->buffer[i].length);
		free (dev->buffer);
	}
	return close (dev->fd);
}

