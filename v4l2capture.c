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
#include "device.h"

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
	device.width = 320;
	device.height = 240;
	device.prefix = file_prefix;	

	if (device_open (&device) != DEVICE_OK)
	{
		g_print ("Invalid Device: %s\n", device.name);
		exit (1);	
	}	
	
	ret = device_init (&device);
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
		case DEVICE_INVALID_FORMAT:
			g_print ("Invalid image format: %s\n",
				res_code);
			break;
	}

//	if (ret == DEVICE_OK)
//		device_getframe (&device);

	if (device_close (&device) != DEVICE_OK)
	{
		g_print ("Closing device error.\n");
		exit (1);
	}
}
