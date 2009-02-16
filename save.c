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



#include "device.h"
#include "hcvloop.h"
#include <stdlib.h>

int save_image (ImageBuffer *image, char *name, FieldOptions *opt)
{
	int r;
	FILE *file;
	file = fopen (name, "wb");

	if (file == NULL)
	{
		return 1;
	}

	r = jpeg_save_image (image, file, 0);

	fclose (file);

	return r;
}
