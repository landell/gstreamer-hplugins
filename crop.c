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
image_mark (ImageBuffer *src, crop_window_t *win)
{
	ImageBuffer *dst;
	/* FIXME: assuming bytes per pixel is 3 */
	int bpp = 3;
	dst = malloc (sizeof (ImageBuffer));
	if (dst == NULL)
		return NULL;
	dst->fmt.height = src->fmt.height;
	dst->fmt.width = src->fmt.width;
	dst->fmt.pixelformat = src->fmt.pixelformat;
	dst->fmt.bytesperline = dst->fmt.width * bpp;
	dst->len = dst->fmt.height * dst->fmt.bytesperline;
	dst->data = malloc (dst->len);
	if (dst->data == NULL)
	{
		free (dst);
		return NULL;
	}
	memcpy (dst->data, src->data, dst->len);
	image_mark_self (dst, win);
	return dst;
}

void
image_mark_self (ImageBuffer *dst, crop_window_t *win)
{
	int i;
	unsigned char *d1;
	unsigned char *d2;
	/* FIXME: assuming bytes per pixel is 3 */
	int bpp = 3;
	/*
	d1 = dst->data + win->top * dst->fmt.bytesperline + win->left * bpp;
	d2 = dst->data + win->bottom * dst->fmt.bytesperline + win->left * bpp;
	for (i = win->left; i < win->right; i++)
	{
		d1[0] = d2[0] = 0x00;
		d1[1] = d2[1] = 0x00;
		d1[2] = d2[2] = 0x00;
		d1 += 3;
		d2 += 3;
	}
	*/
	d1 = dst->data + win->top * dst->fmt.bytesperline + win->left * bpp;
	d2 = dst->data + win->top * dst->fmt.bytesperline + win->right * bpp;
	for (i = win->top; i < win->bottom; i++)
	{
		memset (d1, 0, d2 - d1);
		/*
		d1[0] = d2[0] = 0x00;
		d1[1] = d2[1] = 0x00;
		d1[2] = d2[2] = 0x00;
		*/
		d1 += dst->fmt.bytesperline;
		d2 += dst->fmt.bytesperline;
	}
}

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
	dst->fmt.height = win->bottom - win->top + 1;
	dst->fmt.width = win->right - win->left + 1;
	dst->fmt.pixelformat = src->fmt.pixelformat;
	dst->fmt.bytesperline = dst->fmt.width * bpp;
	dst->len = dst->fmt.height * dst->fmt.bytesperline;
	dst->data = malloc (dst->len);
	if (dst->data == NULL)
	{
		free (dst);
		return NULL;
	}
	for (i = win->top; i <= win->bottom; i++)
	{
		int dline = (i - win->top) * dst->fmt.bytesperline;
		int sline = (i * src->fmt.width + win->left) * bpp;
		memcpy (dst->data + dline, src->data + sline, dst->fmt.bytesperline);
	}
	return dst; 
}

int crop_format_3x4 (crop_window_t *win, int w_max, int h_max)
{
	int w, h, fix;

	if (!win)
		return 1;
	
	w = win->right - win->left;
	h = win->bottom - win->top;

	/* Testing ratio. Ratio must be (w*4 == h*3). */
	if ((4 * w) > (3 * h))
	{
		fix = (4 * w / 3) - h;
		fix += (fix % 2) ? 0 : 1;
		win->bottom += fix / 2;
		win->top -= fix / 2;
	}
	else
	{
		fix = (3 * h / 4) - w;
		fix += (fix % 2) ? 0 : 1;
		win->right += fix / 2;
		win->left -= fix / 2;
	}
	
	/* Testing limits */
	if (win->top < 0)
	{
		win->bottom += - win->top;
		win->top = 0;
	}

	if (win->bottom >= h_max)
	{
		win->top -= (win->bottom - h_max) + 1;
		win->bottom = h_max - 1;
	}

	if (win->right < 0)
	{
		win->left += - win->right;
		win->right = 0;
	}

	if (win->left >= w_max)
	{
		win->right -= (win->left - w_max) + 1;
		win->left = w_max - 1;
	}

	/* Drop an error if any limit is reached yet */
	if ((win->top < 0) || (win->bottom >= h_max) ||
		(win->right < 0) || (win->left >= w_max))
		return 1;

	return 0;
}
