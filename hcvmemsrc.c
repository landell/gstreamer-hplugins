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


#include <stdio.h>
#include <jpeglib.h>
#include "device.h"

static struct jpeg_source_mgr hcv_jpeg_source_mgr;

static void
hcv_jpeg_init_source (j_decompress_ptr cinfo)
{
	ImageBuffer *src = cinfo->client_data;
	cinfo->src->next_input_byte = src->data;
	cinfo->src->bytes_in_buffer = src->len;
}

static void
hcv_jpeg_term_source (j_decompress_ptr cinfo)
{
}

static void
hcv_jpeg_skip_input_data (j_decompress_ptr cinfo, long bytes)
{
	if (bytes <= 0)
		return;
	if ((unsigned long) bytes > cinfo->src->bytes_in_buffer)
	{
		cinfo->src->bytes_in_buffer = 0;
		cinfo->src->next_input_byte = NULL;
	}
	else
	{
		cinfo->src->next_input_byte += bytes;
		cinfo->src->bytes_in_buffer -= bytes;
	}
}

void
hcv_jpeg_membuffer_src (j_decompress_ptr cinfo, ImageBuffer *src)
{
	cinfo->client_data = src;
	cinfo->src = &hcv_jpeg_source_mgr;
	cinfo->src->init_source = hcv_jpeg_init_source;
	cinfo->src->fill_input_buffer = NULL;
	cinfo->src->skip_input_data = hcv_jpeg_skip_input_data;
	cinfo->src->resync_to_restart = jpeg_resync_to_restart;
	cinfo->src->term_source = hcv_jpeg_term_source;
}
