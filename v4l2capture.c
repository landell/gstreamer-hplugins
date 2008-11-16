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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <errno.h>
#include <ctype.h>
#include "device.h"
#include "huffman.h"
#include <jpeglib.h>

#define TIMEOUT 1000
#define DEFAULT_FPS 20
#define DEFAULT_RETRIES 3

static int list_res;
static char *res_code = "320x240";
static char *file_prefix = "image";
static char *device_name = "/dev/video0";
static int wait_time = TIMEOUT;
static int retries = DEFAULT_RETRIES;

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

static char * get_filename (V4l2Device *dev)
{
	#define NAME_SIZE 80
	#define MIN_SIZE 128
	char *name = NULL;

	name = malloc(NAME_SIZE);
	if (!name)
		return NULL;
	snprintf (name, NAME_SIZE, "%s.jpg", dev->prefix);
	return name;
}

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

	return 0;

}

static int save_picture (V4l2Device *dev,
			 int (*save_image) (ImageBuffer *, FILE *))
{
	int r;
	FILE *file;
	char *name = get_filename (dev);
	if (!name)
		return 1;
	file = fopen (name, "wb");

	if (file == NULL)
	{
		free (name);
		return 1;
	}

	r = save_image (&dev->image, file);

	fclose (file);

	free (name);
	return r;
}

static int mjpeg_save_image (ImageBuffer *image, FILE *file)
{
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

static int get_resolution (char *res, int *w, int *h)
{
	int W, H;
	char *next;

	errno = 0;
	W = strtol (res, &next, 10);
	if (errno == ERANGE)
		return 1;
	while (*next != 0 && isspace (*next)) next++;
	if (*next != 'x' && *next != 'X')
		return 1;
	next++;
	errno = 0;
	H = strtol (next, &next, 10);
	if (errno == ERANGE)
		return 1;

	if (w)
		*w = W;
	if (h)
		*h = H;

	return 0;
}

/******/

static void usage ()
{
  fprintf (stderr, "v4l2capture - Grab images from a V4L2 Device\n"
	"[-r resolution (320x240)] "
	"[-o output_prefix (image)] "
	"[-d device_name (/dev/video0)] "
	"[-t timeout per frame in ms (1000)] "
	"[-e number of retries (3)] "
	"[-h this help]\n");
}

int main (int argc, char **argv)
{
	int (*save_image) (ImageBuffer *, FILE *);
	V4l2Device device;
	int ret;
	int c;
	u_int32_t formats[] =
	{
		V4L2_PIX_FMT_MJPEG,
		V4L2_PIX_FMT_YUYV,
		0
	};

	while ((c = getopt (argc, argv, "l:r:o:d:t:e:h")) != -1)
	{
		switch (c)
		{
			case 'l':
				list_res = 1;
				break;
			case 'r':
				res_code = optarg;
				break;
			case 'o':
				file_prefix = optarg;
				break;
			case 'd':
				device_name = optarg;
				break;
			case 'e':
				retries = strtol (optarg, NULL, 10);
				break;
			case 't':
				wait_time = strtol (optarg, NULL, 10);
				break;
			case '?':
			case 'h':
			default:
				usage ();
				exit (1);
		}
	}

	device.name = device_name;
	device.prefix = file_prefix;
	device.fps = DEFAULT_FPS;

	if (wait_time == 0)
		wait_time = TIMEOUT;

	if (retries == 0)
		retries = DEFAULT_RETRIES;

	if (get_resolution (res_code, &device.width, &device.height))
	{
		fprintf (stderr, "Incorrect image format input: %s\n",
			res_code);
		exit (1);
	}

	fprintf (stderr, "Image resolution: %dx%d\n", device.width,
		device.height);

	if (device_open (&device) != DEVICE_OK)
	{
		fprintf (stderr, "Invalid Device: %s\n", device.name);
		exit (1);	
	}	

	if (device_negotiate (&device, formats) == 0)
	{
		fprintf (stderr, "Selected pixel format: %c%c%c%c\n",
			device.pixelformat,
			device.pixelformat >> 8,
			device.pixelformat >> 16,
			device.pixelformat >> 24);
	}
	else
	{
		fprintf (stderr, "Could not select any supported format\n");
		exit (1);
	}

	switch (device.pixelformat)
	{
		case V4L2_PIX_FMT_MJPEG:
			save_image = mjpeg_save_image;
			break;
		case V4L2_PIX_FMT_YUV420:
			save_image = jpegcode_save_image;
			break;
		case V4L2_PIX_FMT_YUYV:
			save_image = yuyv_save_image;
			break;
		default:
			save_image = raw_save_image;
	}
	
	ret = device_init (&device);
	if (ret != DEVICE_OK)
	{	
		switch (ret)
		{
			case DEVICE_IS_NOT_V4L2:
				fprintf (stderr,
					"Device %s is not a V4l2 device.\n",
					device.name);
				break;
			case DEVICE_DONT_CAPTURE:
				fprintf (stderr,
					"Device %s does not support video capture.\n",
					device.name); 
				break;
			case DEVICE_MODE_NOT_SUPPORTED:
				fprintf (stderr,
					"Device %s does not support MMAP.\n",
					device.name);
				break;
			case DEVICE_INVALID_FORMAT:
				fprintf (stderr, "Invalid image format: %s\n",
					res_code);
				break;
			case DEVICE_OUT_OF_MEMORY:
				fprintf (stderr, "Out of memory!\n");
				break;
			default:
				fprintf (stderr,
					"Unknown or not handled error.\n");
				break;
		}

		if (device_close (&device) != DEVICE_OK)
		{
			fprintf (stderr, "Closing device error.\n");
			exit (1);
		}

	}

	ret = device_start_capture (&device);

	switch (ret)
	{
		case DEVICE_BUFFER_ERROR:
			fprintf (stderr,
				"Could not start the stream.\n");
			break;
		case DEVICE_STREAM_ERROR:
			fprintf (stderr,
				"Error on start streaming.\n");
			break;
	} 

	if (ret == DEVICE_OK)
	{		
		fprintf (stderr, "Taking a picture...\n");

		fd_set fdset;
		struct timeval timeout;
		int i; 

		FD_ZERO (&fdset);
		FD_SET (device.fd, &fdset);

		timeout.tv_sec = 0;
		timeout.tv_usec = wait_time * 1000;

		for (i = 0; i < retries; ++i)
		{
			ret = select (device.fd + 1, &fdset, NULL, NULL,
				&timeout);

			if (ret == 0)
			{
				fprintf (stderr,
					"[%d] Timeout.\n", i);
			} else if (ret == -1)
			{
				fprintf (stderr,
					"[%d] I got an error: %s\n",
					i, strerror (errno));
			} else
			{
				ret = device_getframe (&device);

				switch (ret)
				{
					case DEVICE_NOT_READY:
						fprintf (stderr,
							"NOT READY\n");
						break;
					case DEVICE_EMPTY_FRAME:
						fprintf (stderr, 
							"EMPTY FRAME\n");
						break;
					case DEVICE_STREAM_ERROR:
						fprintf (stderr,
							"STREAM ERROR\n");
						break;
					case DEVICE_BUFFER_ERROR:
						fprintf (stderr,
						"BUFFER ERROR\n");
						break;
				}

				if (ret == DEVICE_OK)
				{
					/* Now we got a pic. It must be adjusted
					 * and sent to a file.
					 */
					if (!save_picture (&device, save_image))
					{
						fprintf (stderr,
							"[%d] Done!\n",
							i);
						break;
					} else
						fprintf (stderr,
							"[%d] I cannot save this image!\n",
							i);
				}
			}
		}
	}

	if (device_stop_capture (&device) != DEVICE_OK)
		fprintf (stderr, "Error on stop streaming.\n");

	if (device_close (&device) != DEVICE_OK)
	{
		fprintf (stderr, "Closing device error.\n");
		exit (1);
	}

	return 0;
}
