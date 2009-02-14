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
#include "hcverror.h"
#include "hcvloop.h"

#define TIMEOUT 1000
#define DEFAULT_FPS 20
#define NFRAMES 10

static char *res_code = "320x240";
static char *file_prefix = "image";
static char *device_name = "/dev/video0";

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
	"Parameters:\n"
	"-r resolution	Image resolution WxH (default: 320x240)\n"
	"-o prefix	Output prefix (default: ./image)\n"
	"-d device	Path to device (default: /dev/video0)\n"
	"-n number	Number of frames to take (max: %d, default: %d)\n"
	"-s		Run as service (use v4l2capture-client to shot)\n"
	"-c		Crop detected faces\n"
	"-p		Force 3x4 ratio format to face detection\n"
	"-m		Draw a green rectangle in detected faces\n"
	"-h		Show this help\n", MAX_QUEUE_SIZE, NFRAMES);
}

int main (int argc, char **argv)
{
	V4l2Device device;
	int ret;
	int c;
	char *nframes_aux;
	FieldOptions options;
	
	options.flags = 0;
	options.nframes = NFRAMES;

	u_int32_t formats[] =
	{
		V4L2_PIX_FMT_MJPEG,
		V4L2_PIX_FMT_YUYV,
		0
	};

	while ((c = getopt (argc, argv, "r:o:d:n:hcspm")) != -1)
	{
		switch (c)
		{
			case 'r':
				res_code = optarg;
				break;
			case 'o':
				file_prefix = optarg;
				break;
			case 'd':
				device_name = optarg;
				break;
			case 'n':
				nframes_aux = optarg;
				errno = 0;
				options.nframes = strtol (nframes_aux, NULL, 10);
				if (errno == ERANGE ||
					options.nframes > MAX_QUEUE_SIZE)
				{
					usage ();
					exit(1);
				}
				break;
			case 's':
				options.flags |= FO_DAEMON;
				break;
			case 'c':
				options.flags |= FO_CROP;
				break;
			case 'p':
				options.flags |= FO_3X4;
				break;
			case 'm':
				options.flags |= FO_MARK;
				break;
			case '?':
			case 'h':
			default:
				usage ();
				exit (1);
		}
	}

	device.name = device_name;
	device.fps = DEFAULT_FPS;

	options.prefix = file_prefix;

	if (get_resolution (res_code, &device.width, &device.height))
	{
		fprintf (stderr, "Incorrect image format input: %s\n",
			res_code);
		exit (1);
	}

	fprintf (stderr, "Image resolution: %dx%d\n", device.width,
		device.height);

	if ((ret = device_open (&device)) != DEVICE_OK)
	{
		fprintf (stderr, "Could not open device: %s\n",
			device_error (ret));
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

	ret = device_init (&device);
	if (ret != DEVICE_OK)
	{	
		fprintf (stderr, "Could not initialize device: %s\n",
			device_error (ret));
		if ((ret = device_close (&device)) != DEVICE_OK)
		{
			fprintf (stderr, "Could not close device: %s\n",
				device_error (ret));
		}
		exit (1);
	}

	ret = device_start_capture (&device);
	if (ret != DEVICE_OK)
	{	
		fprintf (stderr, "Could not start capture: %s\n",
			device_error (ret));
		if ((ret = device_close (&device)) != DEVICE_OK)
		{
			fprintf (stderr, "Could not close device: %s\n",
				device_error (ret));
		}
		exit (1);
	}

	fprintf (stderr, "Taking a picture...\n");
	if (device_shot (&device, &options) != 0)
	{
		fprintf (stderr, "Error on frame capture. "
			"Do you have the correct permissions?\n");
	}

	if ((ret = device_stop_capture (&device)) != DEVICE_OK)
		fprintf (stderr, "Error on stop streaming: %s\n",
			device_error (ret));

	if (device_close (&device) != DEVICE_OK)
	{
		fprintf (stderr, "Could not close device: %s\n",
			device_error (ret));
		exit (1);
	}

	return 0;
}
