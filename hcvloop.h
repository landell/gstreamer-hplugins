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
	int nframes;
	union {
		unsigned long flags;
		struct {
			unsigned daemon :1;
			unsigned facetracker :1;
			unsigned crop :1;
			unsigned facemark :1;
			unsigned force_3x4 :1;
		};
	};
} FieldOptions;

int save_picture (V4l2Device *);
int save_image (ImageBuffer *, char *);
int device_serie (V4l2Device *, FieldOptions *);
int device_loop (V4l2Device *, FieldOptions *);
int device_shot (V4l2Device *, FieldOptions *);
ImageBuffer* image_convert_format (ImageBuffer *);

#endif
