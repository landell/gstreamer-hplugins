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
#include <jpeglib.h>

int jpeg_save_image (ImageBuffer *image, FILE *file)
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
