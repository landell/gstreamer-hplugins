/*
 *  Copyright (C) 2009 Samuel Ribeiro da Costa Vale
 *  <srcvale@@holoscopio.com>
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
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *  02110-1301 USA.
 */

#include "skinfilter.h"
#include "skinmodel.h"

/* skinfilter - given a model described by U and V components, and
 * the components U and V of desired image, this function determines the
 * probability of a pixel to represent the object of interest.
 */
void skinfilter (unsigned char *img, unsigned char *u, unsigned char *v,
	int h, int w)
{
	int i, j, k;
	unsigned char m, n;

	for (i = 0; i < h; ++i)
	{
		for (j = 0; j < w; ++j)
		{
			k = i*w + j;
			m = *(u + k);
			n = *(v + k);
			*(img + k) = *(skin_model + m*w + n);
		}
	}
}

/* skinfilter - given a model described by U and V components, and
 * the components U and V of desired image, this function determines if
 * a pixel represents the object of interest, if the probability is
 * greater than a threshold.
 */
void skinfilter_threshold (unsigned char *img, unsigned char *u, unsigned char *v,
	int h, int w, unsigned char th)
{
	int i, j, k;
	unsigned char m, n;

	for (i = 0; i < h; ++i)
	{
		for (j = 0; j < w; ++j)
		{
			k = i*w + j;
			m = *(u + k);
			n = *(v + k);
			*(img + k) = ((*(skin_model + m*w + n) > th) ? 255 : 0);
		}
	}
}
