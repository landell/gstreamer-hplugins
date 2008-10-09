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
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.

    Copyright 2008 Samuel R. C. Vale
    srcvale@holoscopio.com
*/

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <string.h>
#include "device.h"
#include "huffman.h"

static gboolean list_res;
static gchar *res_code = "320x280";
static gchar *file_prefix = "image";
static gchar *device_name = "/dev/video0";

static GOptionEntry entries[] = 
{
	{ "list-res", 'l', 0, G_OPTION_ARG_NONE, &list_res, "List available resolutions", NULL},
	{ "res", 'r', 0, G_OPTION_ARG_STRING, &res_code, "Image resolution", "{320x240 | 640x480}"},
	{ "output", 'o', 0, G_OPTION_ARG_STRING, &file_prefix, "Filename prefix", "PREFIX" },
	{ "device", 'd', 0, G_OPTION_ARG_FILENAME, &device_name, "V4L2 device", "/dev/videox" },
	{ NULL }
};

/* JPEG Utils */

int is_huffman (unsigned char *buf)
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
		ptbuf++;
	}
	return 0;
}

static void get_picture_name (char *Picture, int fmt)
{
	char temp[80];
	memset (temp, '\0', sizeof (temp));
	snprintf (temp, 26, "image.jpg");
	memcpy (Picture, temp, strlen (temp));
}

static int save_picture (unsigned char *buf, int size)
{
	FILE *file;
	unsigned char *ptdeb, *ptcur = buf;
	int sizein;
	char *name = NULL;
	name = calloc(80,1);
	get_picture_name (name, 1);
	file = fopen(name, "wb");
	if (file != NULL)
	{
		if (!is_huffman (buf))
		{
			ptdeb = ptcur = buf;
			while (((ptcur[0] << 8) | ptcur[1]) != 0xffc0)
				ptcur++;
			sizein = ptcur-ptdeb;
			fwrite (buf, sizein, 1, file);
			fwrite (dht_data, DHT_SIZE, 1, file);
			fwrite (ptcur,size-sizein, 1, file);
		} else
		{
			fwrite (ptcur, size, 1, file);
		}
		fclose (file);
        }
	if (name)
		free (name);
	return 0;
}

/******/

gint main (gint argc, gchar *argv[])
{
	GError *err = NULL;
	GOptionContext *context;
	V4l2Device device;
	gint ret;

	context = g_option_context_new ("- capture JPG image from a V4L2 device.");
	g_option_context_add_main_entries (context, entries, NULL);
	
	if (!g_option_context_parse (context, &argc, &argv, &err))
	{
     		g_print ("Option parsing failed: %s\n", err->message);
		exit (1);
	}

	device.name = device_name;
	device.width = 640;
	device.height = 480;
	device.prefix = file_prefix;	

	if (device_open (&device) != DEVICE_OK)
	{
		g_print ("Invalid Device: %s\n", device.name);
		exit (1);	
	}	
	
	ret = device_init (&device);
	if (ret != DEVICE_OK)
	{	
		switch (ret)
		{
			case DEVICE_IS_NOT_V4L2:
				g_print ("Device %s is not a V4l2 device.\n",
					device.name);
				break;
			case DEVICE_DONT_CAPTURE:
				g_print ("Device %s don't support video capture.\n",
					device.name); 
				break;
			case DEVICE_MODE_NOT_SUPPORTED:
				g_print ("Device %s don't support MMAP.\n",
					device.name);
				break;
			case DEVICE_INVALID_FORMAT:
				g_print ("Invalid image format: %s\n",
					res_code);
				break;
			case DEVICE_OUT_OF_MEMORY:
				g_print ("Out of memory!\n");
				break;
			default:
				g_print ("Unknow or not handled error :(.\n");
				break;
		}
	} else
	{
		ret = device_start_capture (&device);
		
		switch (ret)
		{
			case DEVICE_BUFFER_ERROR:
				g_print ("Could not start the stream.\n");
				break;
			case DEVICE_STREAM_ERROR:
				g_print ("Error on start streaming.\n");
				break;
		} 
		
		if (ret == DEVICE_OK)
		{		
			g_print ("Taking a picture...\n");

			// TODO: Prevent this to get in infinite loop.
			do
				ret = device_getframe (&device);
			while (ret == DEVICE_NOT_READY);

			switch (ret)
			{
				case DEVICE_STREAM_ERROR:
					g_print ("STREAM ERROR\n");
					break;
				case DEVICE_BUFFER_ERROR:
					g_print ("BUFFER ERROR\n");
					break;
			}
			
			if (ret == DEVICE_OK)
			{
				// Now we got a pic. It must be adjusted
				// and sent to a file.
				save_picture (device.framebuffer,
					device.buffersize);
				g_print ("Done!\n");
			}
		}

		if (device_stop_capture (&device) != DEVICE_OK)
			g_print ("Error on stop streaming.\n");
	}

	if (device_close (&device) != DEVICE_OK)
	{
		g_print ("Closing device error.\n");
		exit (1);
	}

	return 0;
}
