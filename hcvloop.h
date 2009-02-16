/*
 *  Copyright (C) 2008  Thadeu Lima de Souza Cascardo <cascardo@holoscopio.com>
 *  Copyright (C) 2009  Samuel R. C. Vale <srcvale@holoscopio.com>
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


#ifndef HCVLOOP_H
#define HCVLOOP_H

#include <stdio.h>
#include "device.h"

#define MAX_QUEUE_SIZE 20

typedef struct {
	char *prefix;
	char *prefix_full;
	int nframes;
	unsigned long flags;
} FieldOptions;

#define FO_DAEMON	0x0001
#define FO_CROP		0x0002
#define FO_MARK		0x0004
#define FO_3X4		0x0008
#define FO_GRAY		0x0010

int save_image (ImageBuffer *, char *, FieldOptions *);
int jpeg_save_image (ImageBuffer *, FILE *, int);
int device_serie (V4l2Device *, FieldOptions *);
int device_loop (V4l2Device *, FieldOptions *);
int device_shot (V4l2Device *, FieldOptions *);
ImageBuffer* image_convert_format (ImageBuffer *);
ImageBuffer* image_convert_grayscale (ImageBuffer *);

#endif
