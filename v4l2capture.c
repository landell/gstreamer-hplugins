/*
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

    Copyright 2008 Samuel R. C. Vale
    srcvale@holoscopio.com
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <errno.h>
#include "device.h"
#include "huffman.h"

static int list_res;
static char *res_code = "320x240";
static char *file_prefix = "image";
static char *device_name = "/dev/video0";

#define DEFAULT_FPS 20

/* JPEG Utils */

int is_huffman (unsigned char *buf, int size)
{
	unsigned char *ptbuf;
	int i = 0;
	ptbuf = buf;
	while (((ptbuf[0] << 8) | ptbuf[1]) != 0xffda)
	{
		if (i++ > 2048)
			return 0;
		if (((ptbuf[0] << 8) | ptbuf[1]) == 0xffc4)
			return 1;
		if (ptbuf == (buf + size - 2))
			return 0;
		ptbuf++;
	}
	return 0;
}

static int save_picture (unsigned char *buf, int size)
{
	#define NAME_SIZE 80
	#define MIN_SIZE 128
	FILE *file;
	unsigned char *ps, *pc;
	int sizein;
	char *name = NULL;

	name = calloc(NAME_SIZE, 1);
	if (!name)
		return 1;
	snprintf (name, NAME_SIZE, "%s.jpg", file_prefix);
	file = fopen (name, "wb");

	if (file != NULL)
	{
		// look for JPEG start
		ps = buf;
		while (((ps[0] << 8 | ps[1]) != 0xffd8) && 
			(ps < (buf + size - MIN_SIZE)))
			ps++;

		if (ps >= buf + size - MIN_SIZE)
		{
			fclose (file);
			free (name);
			return 1;
		}

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
					// corrupted file
					fclose (file);
					free (name);
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
		fclose (file);
        }

	free (name);
	return 0;
}

static int get_resolution (char *res, int *w, int *h)
{
	#define B_SIZE	6
	char w_buf[B_SIZE], h_buf[B_SIZE], *aux;

	memset (w_buf, '\0', B_SIZE);
	memset (h_buf, '\0', B_SIZE);

	aux = strchr (res, 'x');

	if (aux == NULL)
		return 1;

	strncpy (w_buf, res, (aux - res));
	strcpy (h_buf, (aux + 1));

	*w = atoi (w_buf);
	*h = atoi (h_buf);

	if ((*w == 0) || (*h == 0))
		return 1;

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
	"[-e number of retrys (3)] "
	"[-h this help]\n");
}

int main (int argc, char **argv)
{
	V4l2Device device;
	int ret;
	int c;

	while ((c = getopt (argc, argv, "lr:o:d:")) != -1)
	{
		switch (c)
		{
			case 'l':
				list_res = 1;
				break;
			case 'r':
				res_code = strdup (optarg);
				break;
			case 'o':
				file_prefix = strdup (optarg);
				break;
			case 'd':
				device_name = strdup (optarg);
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
					"Device %s don't support video capture.\n",
					device.name); 
				break;
			case DEVICE_MODE_NOT_SUPPORTED:
				fprintf (stderr,
					"Device %s don't support MMAP.\n",
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
					"Unknow or not handled error :(.\n");
				break;
		}
	} else
	{
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

                        FD_ZERO (&fdset);
                        FD_SET (device.fd, &fdset);

                        timeout.tv_sec = 2;
                        timeout.tv_usec = 0;

                        ret = select (device.fd + 1, &fdset, NULL, NULL,
				&timeout);

			if (ret == 0)
			{
				fprintf (stderr, "Timeout.\n");
			} else if (ret == -1)
			{
				fprintf (stderr, "I got an error: %d\n",
					errno);
			} else
			{
				ret = device_getframe (&device);

				switch (ret)
				{
					case DEVICE_NOT_READY:
						fprintf (stderr, "NOT READY\n");
						break;
					case DEVICE_EMPTY_FRAME:
						fprintf (stderr, 
							"EMPTY FRAME\n");
					case DEVICE_STREAM_ERROR:
						fprintf (stderr, "STREAM ERROR\n");
						break;
					case DEVICE_BUFFER_ERROR:
						fprintf (stderr, "BUFFER ERROR\n");
						break;
				}

				if (ret == DEVICE_OK)
				{
					// Now we got a pic. It must be adjusted
					// and sent to a file.
					if (!save_picture (device.framebuffer,
						device.buffersize))
						fprintf (stderr, "Done!\n");
					else
						fprintf (stderr,
							"I can not save this image!\n");
				}
			}
		}

		if (device_stop_capture (&device) != DEVICE_OK)
			fprintf (stderr, "Error on stop streaming.\n");
	}

	if (device_close (&device) != DEVICE_OK)
	{
		fprintf (stderr, "Closing device error.\n");
		exit (1);
	}

	return 0;
}
