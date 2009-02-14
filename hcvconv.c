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



#include <stdlib.h>
#include "device.h"
#include "hcvmemsrc.h"
#include <stdio.h>
#include <jpeglib.h>

static ImageBuffer* yuyv_image (ImageBuffer *image)
{
	ImageBuffer *dst;
	int i;
	int height = image->fmt.height;
	int width = image->fmt.width;
	unsigned char *data = image->data;

	dst = malloc (sizeof (ImageBuffer));
	if (dst == NULL)
		return NULL;

	dst->fmt.height = height;
	dst->fmt.width = width;
	dst->fmt.pixelformat = V4L2_PIX_FMT_YUV420;
	dst->fmt.bytesperline = width * 3;
	dst->len = height * width * 3;
	dst->data = malloc (dst->len);
	if (dst->data == NULL)
	{
		free (dst);
		return NULL;
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

	return dst;
}

static char *headerfn = "/var/lib/hcv/header.jpg";

static ImageBuffer* mjpeg_image (ImageBuffer *image)
{
	ImageBuffer *dst;
	struct jpeg_decompress_struct decompress;
	struct jpeg_error_mgr emgr;
	FILE *headerfile;
	int i;
	JSAMPROW p;
	int height;
	int width;
	unsigned char *data;

	dst = malloc (sizeof (ImageBuffer));
	if (dst == NULL)
		return NULL;

	decompress.err = jpeg_std_error (&emgr);
	jpeg_create_decompress (&decompress);
	headerfile = fopen (headerfn, "r");
	if (headerfile == NULL)
	{
		free (dst);
		jpeg_destroy_decompress (&decompress);
		return NULL;
	}
	jpeg_stdio_src (&decompress, headerfile);
	jpeg_read_header (&decompress, FALSE);
	fclose (headerfile);
	hcv_jpeg_membuffer_src (&decompress, image);
	jpeg_read_header (&decompress, TRUE);
	decompress.out_color_space = JCS_YCbCr;
	jpeg_start_decompress (&decompress);
	height = dst->fmt.height = decompress.output_height;
	width = dst->fmt.width = decompress.output_width;
	dst->fmt.pixelformat = V4L2_PIX_FMT_YUV420;
	dst->fmt.bytesperline = width * decompress.output_components;
	dst->len = height * dst->fmt.bytesperline;
	data = dst->data = malloc (dst->len);
	if (dst->data == NULL)
	{
		free (dst);
		jpeg_destroy_decompress (&decompress);
		return NULL;
	}
	p = (JSAMPROW) data;
	for (i = 0; i < dst->fmt.height; i++)
	{
		jpeg_read_scanlines (&decompress, &p, 1);
		p += dst->fmt.bytesperline;
	}
	jpeg_finish_decompress (&decompress);
	jpeg_destroy_decompress (&decompress);

	return dst;
}

ImageBuffer* image_convert_format (ImageBuffer *img)
{
	ImageBuffer *ret;
	switch (img->fmt.pixelformat)
	{
		case V4L2_PIX_FMT_MJPEG:
			ret = mjpeg_image (img);
			break;
		case V4L2_PIX_FMT_YUV420:
			/* native format */
			ret = img;
			break;
		case V4L2_PIX_FMT_YUYV:
			ret = yuyv_image (img);
			break;
		default:
			ret = NULL;
	}
	return ret;
}
