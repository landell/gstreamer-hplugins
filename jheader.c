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
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <jpeglib.h>

struct ctx
{
  struct jpeg_compress_struct compress;
  struct jpeg_error_mgr emgr;
};

#ifdef DEBUG
#include <assert.h>
#define ASSERT(x) assert(x)
#else
#define ASSERT(x)
#endif

#define DEFAULT_WIDTH 320
#define DEFAULT_HEIGHT 240
#define DEFAULT_TYPE 3

struct ctx *
jpegctx_new (int colorspace)
{
  struct ctx *c;
  c = malloc (sizeof (struct ctx));
  if (c == NULL)
    return NULL;
  c->compress.err = jpeg_std_error (&c->emgr);
  jpeg_create_compress (&c->compress);
  c->compress.in_color_space = colorspace;
  jpeg_set_defaults (&c->compress);
  return c;
}

int
saveto (struct ctx * c, char *filename)
{
  FILE *file;
  file = fopen (filename, "w");
  if (file == NULL)
    return 1;
  jpeg_stdio_dest (&c->compress, file);
  c->compress.image_width = 0;
  c->compress.image_height = 0;
  c->compress.input_components = 3;
  jpeg_write_tables (&c->compress);
  fclose (file);
  return 0;
}

int
main (int argc, char **argv)
{
  struct ctx *c;
  if (argc < 2)
    {
      fprintf (stderr, "convert destination\n");
      return 1;
    }
  c = jpegctx_new (JCS_YCbCr);
  if (saveto (c, argv[1]) != 0)
    {
      fprintf (stderr, "Could not save destination file\n");
      return 1;
    }
  return 0;
}
