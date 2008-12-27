/*  Copyright 2008 Samuel R. C. Vale <srcvale@holoscopio.com>
    Copyright 2008 Thadeu Lima de Souza Cascardo <cascardo@holoscopio.com>

    This software was based on V4L2 Documentation, and 
    luvcview (Copyright 2005 2006 2007 Laurent Pinchart, Michel
    Xhaard, and 2006 Gabriel A. Devenyi).

    This file is part of v4l2capture.

    V4l2capture is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    V4l2 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with v4l2capture.  If not, see <http://www.gnu.org/licenses/>.
*/

/* JPEG Utils */

int is_huffman (unsigned char *buf, int size)
{
	unsigned char *ptbuf;
	ptbuf = buf;
	while (((ptbuf[0] << 8) | ptbuf[1]) != 0xffda)
	{
		/* Max window we look for the magic cookie */
		if (ptbuf - buf > 2048)
			return 0;
		if (((ptbuf[0] << 8) | ptbuf[1]) == 0xffc4)
			return 1;
		if (ptbuf == (buf + size - 2))
			return 0;
		ptbuf++;
	}
	return 0;
}
