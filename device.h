/*  Copyright 2008 Samuel R. C. Vale <srcvale@holoscopio.com>
    Copyright 2008 Thadeu Lima de Souza Cascardo <cascardo@holoscopio.com>

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

#ifndef _V4L2_DEVICE_H_
#define _V4L2_DEVICE_H_

#include <stdio.h>
#include <sys/types.h>
#include <linux/videodev2.h>

typedef enum {
	DEVICE_OK = 0,
	DEVICE_ERROR,
	DEVICE_INVALID,
	DEVICE_IS_NOT_V4L2,
	DEVICE_DONT_CAPTURE,
	DEVICE_MODE_NOT_SUPPORTED,
	DEVICE_FEATURE_NOT_SUPPORTED,
	DEVICE_INVALID_FORMAT,
	DEVICE_OUT_OF_MEMORY,
	DEVICE_BUFFER_ERROR,
	DEVICE_STREAM_ERROR,
	DEVICE_NOT_READY,
	DEVICE_EMPTY_FRAME
} DeviceErrors;

typedef struct {
  unsigned char * data;
  size_t len;
  struct v4l2_pix_format fmt;
} ImageBuffer;

typedef struct {
	void *start;
	size_t length;
} DeviceBuffer;

typedef struct {
	int fd;
	char *name;
	int width;
	int height;
	DeviceBuffer *buffer;
	int n_buffers;
	int fps;
	struct v4l2_capability device_capability;
	u_int32_t pixelformat;
	ImageBuffer image;
	int (*save_image) (ImageBuffer *, FILE *);
} V4l2Device;

int device_open (V4l2Device *dev);
int device_init (V4l2Device *dev);
int device_start_capture (V4l2Device *dev);
int device_stop_capture (V4l2Device *dev);
int device_getframe (V4l2Device *dev);
int device_close (V4l2Device *dev);
int device_negotiate (V4l2Device *dev, u_int32_t *formats);

#endif
