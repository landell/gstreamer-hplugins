/*
 *  Copyright (C) 2009  Thadeu Lima de Souza Cascardo <cascardo@holoscopio.com>
 *  Copyright (C) 2009  Samuel Ribeiro da Costa Vale <srcvale@holoscopio.com>
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


#include <string.h>
#include <stdlib.h>
#include "facetracker.h"
#include "crop.h"
#include "model.h"

/* This receives the image in YCbCr format, that is, YUV420 */
static ImageBuffer *
image_filter (ImageBuffer *src)
{
  ImageBuffer *dst;
  int i, j, k, l;
  if (src->fmt.pixelformat !=  V4L2_PIX_FMT_YUV420)
    return NULL;
  dst = malloc (sizeof (ImageBuffer));
  if (dst == NULL)
    return NULL;
  dst->fmt.height = src->fmt.height;
  dst->fmt.width = src->fmt.width;
  /* Should be only one bit per pixel, really, B/W */
  dst->fmt.pixelformat = V4L2_PIX_FMT_GREY;
  dst->fmt.bytesperline = dst->fmt.width;
  dst->len = dst->fmt.height * dst->fmt.bytesperline;
  dst->data = malloc (dst->len);
  if (dst->data == NULL)
  {
    free (dst);
    return NULL;
  }
  for (i = 0; i < dst->fmt.height; i++)
  {
    for (j = 0; j < dst->fmt.width; j++)
    {
      k = i * dst->fmt.bytesperline + j;
      l = i * src->fmt.width + j;
      dst->data[k] = model (src->data[l * 3 + 1], src->data[l * 3 + 2]);
    }
  }
  return dst;
}

/* This receives the image in greyscale format, that is, GREY */
static ImageBuffer *
image_resize_subscale (ImageBuffer *src, int scale, int th)
{
  ImageBuffer *dst;
  int i, j, k, l, m, n, sum;
  if (src->fmt.pixelformat !=  V4L2_PIX_FMT_GREY)
    return NULL;
  dst = malloc (sizeof (ImageBuffer));
  if (dst == NULL)
    return NULL;
  dst->fmt.height = src->fmt.height / scale;
  dst->fmt.width = src->fmt.width / scale;
  /* Should be only one bit per pixel, really, B/W */
  dst->fmt.pixelformat = V4L2_PIX_FMT_GREY;
  dst->fmt.bytesperline = dst->fmt.width;
  dst->len = dst->fmt.height * dst->fmt.bytesperline;
  dst->data = malloc (dst->len);
  if (dst->data == NULL)
  {
    free (dst);
    return NULL;
  }
  for (i = 0; i < dst->fmt.height; i++)
  {
    for (j = 0; j < dst->fmt.width; j++)
    {
      k = i * dst->fmt.bytesperline + j;
      l = (i * src->fmt.bytesperline + j) * scale;
      sum = 0;
      for (m = 0; m < scale; m++)
      {
        for (n = 0; n < scale; n++)
          sum += src->data[l + m * src->fmt.bytesperline + n];
      }
      sum /= (scale * scale);
      dst->data[k] = (sum > th ? sum : 0);
    }
  }
  return dst;
}

/* This receives the image in greyscale format, that is, GREY */
static int
image_search (ImageBuffer *src, int *left, int *top, int *right, int *bottom)
{
  int i, j;
  int hmin, wmin, hmax, wmax;
  if (src->fmt.pixelformat !=  V4L2_PIX_FMT_GREY)
    return 1;
  hmin = src->fmt.height;
  wmin = src->fmt.width;
  hmax = wmax = 0;
  for (i = 0; i < src->fmt.height; i++)
  {
    for (j = 0; j < src->fmt.width; j++)
    {
      if (src->data[i * src->fmt.bytesperline + j] > 0)
      {
        if (i < hmin) hmin = i;
        if (j < wmin) wmin = j;
        if (i > hmax) hmax = i;
        if (j > wmax) wmax = j;
      }
    }
  }
  if (hmin >= src->fmt.height || wmin >= src->fmt.width ||
      hmax <= 0 || wmax <= 0)
    {
      return 1;
    }
  if (left != NULL)
    *left = wmin;
  if (top != NULL)
    *top = hmin;
  if (right != NULL)
    *right = wmax;
  if (bottom != NULL)
    *bottom = hmax;
  return 0;
}

/* This receives the image in YCbCr format, that is, YUV420 */
#define MIN_AREA	900
#define MIN_SIZE	30
#define BORDER		30
#define SCALE		16
#define W_MAX		320
#define H_MAX		240
#define W_CHECK(a) { a = (a < 0 ? 0 : a); a = (a > W_MAX ? W_MAX : a);}
#define H_CHECK(a) { a = (a < 0 ? 0 : a); a = (a > H_MAX ? H_MAX : a);}

ImageBuffer *
image_facetracker (ImageBuffer *src)
{
  ImageBuffer *tmp;
  ImageBuffer *tmp2;
  int err;
  int l, t, r, b, w, h;
  tmp = image_filter (src);
  if (tmp == NULL)
    return NULL;
  tmp2 = image_resize_subscale (tmp, SCALE, 128);
  free (tmp->data);
  free (tmp);
  if (tmp2 == NULL)
    return NULL;
  err = image_search (tmp2, &l, &t, &r, &b);
  free (tmp2->data);
  free (tmp2);
  if (err)
    return NULL;
  l *= SCALE;
  t *= SCALE;
  r *= SCALE;
  b *= SCALE;
  w = r - l;
  h = b - t;
  if ((w * h < MIN_AREA) || (w < MIN_SIZE) || (h < MIN_SIZE))
    return NULL;
  l -= BORDER; W_CHECK(l);
  b += BORDER; H_CHECK(b);
  r += BORDER; W_CHECK(r);
  t -= BORDER; W_CHECK(t);
  return image_crop (src, l, t, r, b);
}
