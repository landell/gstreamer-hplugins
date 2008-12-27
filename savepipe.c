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



#include <stdlib.h>
#include "jpegutils.h"
#include "device.h"
#include <jpeglib.h>

static int raw_save_image (ImageBuffer *image, FILE *file)
{
	fwrite (image->data, image->len, 1, file);
	return 0;
}

static int jpegcode_save_image (ImageBuffer *image, FILE *file)
{
	struct jpeg_compress_struct compress;
	struct jpeg_error_mgr emgr;
	int i;
	JSAMPROW p;
	compress.err = jpeg_std_error (&emgr);
	jpeg_create_compress (&compress);
	compress.in_color_space = JCS_YCbCr;
	jpeg_set_defaults (&compress);
	jpeg_stdio_dest (&compress, file);
	compress.image_width = image->fmt.width;
	compress.image_height = image->fmt.height;
	compress.input_components = 3;
	jpeg_start_compress (&compress, TRUE);
	p = (JSAMPROW) image->data;
	for (i = 0; i < image->fmt.height; i++)
	{
		jpeg_write_scanlines (&compress, &p, 1);
		p += image->fmt.bytesperline;
	}
	jpeg_finish_compress (&compress);
	return 0;
}

static int yuyv_save_image (ImageBuffer *image, FILE *file)
{
	ImageBuffer *dst;
	int i;
	int height = image->fmt.height;
	int width = image->fmt.width;
	unsigned char *data = image->data;
	dst = malloc (sizeof (ImageBuffer));
	if (dst == NULL)
		return 1;
	dst->fmt.height = height;
	dst->fmt.width = width;
	dst->fmt.pixelformat = V4L2_PIX_FMT_YUYV;
	dst->fmt.bytesperline = width * 3;
	dst->len = height * width * 3;
	dst->data = malloc (dst->len);
	if (dst->data == NULL)
	{
		free (dst);
		return 1;
	}
	for (i = 0; i < height * width; i++)
	{
		/* Y */
		dst->data[3*i] = data[2*i];
		if (i % 2 == 0)
		{
			/* U */
			dst->data[3*i+1] = dst->data[3*i+4] = data[2*i+1];
			/* V */
			dst->data[3*i+2] = dst->data[3*i+5] = data[2*i+3];
		}
	}
	
	jpegcode_save_image (dst, file);

	free (dst->data);
	free (dst);

	return 0;

}

static int mjpeg_save_image (ImageBuffer *image, FILE *file)
{
	#define MIN_SIZE 128
	unsigned char *buf = image->data;
	int size = image->len;
	unsigned char *ps, *pc;
	int sizein;

	/* look for JPEG start */
	ps = buf;
	while (((ps[0] << 8 | ps[1]) != 0xffd8) && 
		(ps < (buf + size - MIN_SIZE)))
		ps++;

	if (ps >= buf + size - MIN_SIZE)
		return 1;

	size -= ps - buf;
	pc = ps;

	if (!is_huffman (ps, size))
	{
		while (((pc[0] << 8) | pc[1]) != 0xffc0)
		{
			if (pc < (ps + size - 2))
			{
				pc++;
			} else
			{
				/* corrupted file */
				return 1;
			}
		}
		sizein = pc - ps;
		fwrite (ps, sizein, 1, file);
		fwrite (dht_data, DHT_SIZE, 1, file);
		fwrite (pc, size - sizein, 1, file);
	} else
	{
		fwrite (pc, size, 1, file);
	}

	return 0;
}

void
device_savepipe (V4l2Device *dev)
{
	switch (dev->pixelformat)
	{
		case V4L2_PIX_FMT_MJPEG:
			dev->save_image = mjpeg_save_image;
			break;
		case V4L2_PIX_FMT_YUV420:
			dev->save_image = jpegcode_save_image;
			break;
		case V4L2_PIX_FMT_YUYV:
			dev->save_image = yuyv_save_image;
			break;
		default:
			dev->save_image = raw_save_image;
	}
}
