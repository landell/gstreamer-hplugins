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

#ifndef HCV_CROP_H
#define HCV_CROP_H

#include "device.h"

typedef struct {
	int left;
	int top;
	int right;
	int bottom;
} crop_window_t;

/* left, top, right, bottom */
/* X-------
 * |      |
 * -------X
 */
ImageBuffer * image_mark (ImageBuffer *src, crop_window_t *win);
ImageBuffer * image_crop (ImageBuffer *src, crop_window_t *win);
int crop_format_3x4 (crop_window_t *win, int w_max, int h_max);

#endif
