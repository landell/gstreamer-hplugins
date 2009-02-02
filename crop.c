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


#include "crop.h"
#include <string.h>
#include <stdlib.h>

ImageBuffer *
image_crop (ImageBuffer *src, crop_window_t *win)
{
	ImageBuffer *dst;
	int i;
	/* FIXME: assuming bytes per pixel is 3 */
	int bpp = 3;
	dst = malloc (sizeof (ImageBuffer));
	if (dst == NULL)
		return NULL;
	dst->fmt.height = win->bottom - win->top;
	dst->fmt.width = win->right - win->left;
	dst->fmt.pixelformat = src->fmt.pixelformat;
	dst->fmt.bytesperline = dst->fmt.width * bpp;
	dst->len = dst->fmt.height * dst->fmt.bytesperline;
	dst->data = malloc (dst->len);
	if (dst->data == NULL)
	{
		free (dst);
		return NULL;
	}
	for (i = win->top; i < win->bottom; i++)
	{
		int dline = (i - win->top) * dst->fmt.bytesperline;
		int sline = (i * src->fmt.width + win->left) * bpp;
		memcpy (dst->data + dline, src->data + sline, dst->fmt.bytesperline);
	}
	return dst; 
}
