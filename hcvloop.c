/*
 *  Copyright (C) 2008  Thadeu Lima de Souza Cascardo <cascardo@holoscopio.com>
 *  Copyright (C) 2009  Samuel R. C. Vale <srcvale@holoscopio.com>
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
#include <string.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "hcvloop.h"
#include "hcverror.h"
#include "device.h"
#include "facetracker.h"
#include "crop.h"

static int queue_size;

struct
{
  ImageBuffer buffers[MAX_QUEUE_SIZE];
  int top;
} queue =
{
  .top = 0
};

#define INCQUEUE(x) (((x) + 1) % queue_size)

static void enqueue_image (ImageBuffer *image)
{
	if (queue.buffers[queue.top].data)
		free (queue.buffers[queue.top].data);
	queue.buffers[queue.top].data = malloc (image->len);
	memcpy (queue.buffers[queue.top].data, image->data, image->len);
	queue.buffers[queue.top].len = image->len;
	queue.buffers[queue.top].fmt = image->fmt;
	queue.top = INCQUEUE (queue.top);
}

static char * filenamenumber (char *prefix, int n)
{
	size_t len = strlen (prefix) + 32;
	char *name = malloc (len);
	if (!name)
		return NULL;
	snprintf (name, len, "%s-%d.jpg", prefix, n + 1);
	return name;
}

static ImageBuffer * process_image (ImageBuffer *image, FieldOptions *opt)
{
	ImageBuffer *image_aux;
	ImageBuffer *face;
	crop_window_t *window;
	int ret = 0;

	if ((face = image_aux = image_convert_format (image)) == NULL)
		return NULL;

	if (!(opt->flags & (FO_MARK | FO_CROP)))
		return face;

	if ((window = image_facetracker (image_aux)) == NULL)
		return face;

	if (opt->flags & FO_3X4)
	{
		ret = crop_format_3x4 (window,
			image_aux->fmt.width, image_aux->fmt.height);
		if (ret)
			goto out;
	}

	if (opt->flags & FO_CROP)
		face = image_crop (image_aux, window);
	else if (opt->flags & FO_MARK)
		face = image_mark (image_aux, window);

	free (image_aux->data);
	free (image_aux);
out:
	free (window);
	return face;
}

static void process_queue (V4l2Device *device, FieldOptions *opt)
{
	int i;
	char *name;
	ImageBuffer *image, *image_aux;
	fprintf (stderr, "Saving images...\n");
	for (i = queue_size - 1; i >= 0; i--)
	{
		name = filenamenumber (opt->prefix, i);
		image = &queue.buffers[(i + queue.top) % queue_size];
		if (name != NULL && image->data != NULL && image->len != 0)
		{
			if ((image_aux = process_image (image, opt)) != NULL)
			{
				save_image (image_aux, name, opt);
				free (image_aux->data);
				free (image_aux);
			}
		}
		if (name != NULL)
			free (name);
	}
}

static int hcv_server (void)
{
	int fd;
	struct sockaddr_un saddr;
	socklen_t slen;
	fd = socket (PF_UNIX, SOCK_DGRAM, 0);
	if (fd < 0)
		return -1;
	saddr.sun_family = AF_UNIX;
	strcpy (saddr.sun_path, "/var/run/hcv/local");
	slen = sizeof (struct sockaddr) + strlen (saddr.sun_path) + 1;
	unlink (saddr.sun_path);
	if (bind (fd, (struct sockaddr *) &saddr, slen) < 0)
	{
		close (fd);
		return -1;
	}
	return fd;
}

#ifndef MAX
#define MAX(x,y) (((x) > (y)) ? (x) : (y))
#endif

int device_serie (V4l2Device *device, FieldOptions *opt)
{
	DeviceErrors ret;
	fd_set fds;
	int r;
	int countdown = -1;
	queue_size = (opt->nframes >= MAX_QUEUE_SIZE ?
		MAX_QUEUE_SIZE : opt->nframes);
	countdown = queue_size;
	do {
		FD_ZERO (&fds);
		FD_SET (device->fd, &fds);
		r = select (device->fd + 1, &fds, NULL, NULL, NULL);
		if (r == -1 || r == 0)
		{
			fprintf (stderr, "Error on select: %s\n",
				strerror (errno));
		}
		if (FD_ISSET (device->fd, &fds))
		{
			ret = device_getframe (device);
			if (ret != DEVICE_OK)
				fprintf (stderr, "Could not get frame: %s\n",
					device_error (ret));
			else
				enqueue_image (&device->image);
		}
	} while (countdown-- > 0);
	process_queue (device, opt);
	return 0;
}

int device_loop (V4l2Device *device, FieldOptions *opt)
{
	char sbuf[128];
	DeviceErrors ret;
	fd_set fds;
	int sfd;
	int max_fd;
	int r;
	int countdown = -1;
	queue_size = (opt->nframes >= MAX_QUEUE_SIZE ?
		MAX_QUEUE_SIZE : opt->nframes);
	sfd = hcv_server ();
	if (sfd < 0)
		return -1;
	max_fd = MAX (sfd, device->fd);
	while (1)
	{
		FD_ZERO (&fds);
		FD_SET (device->fd, &fds);
		FD_SET (sfd, &fds);
		r = select (max_fd + 1, &fds, NULL, NULL, NULL);
		if (r == -1 || r == 0)
		{
			fprintf (stderr, "Error on select: %s\n",
				strerror (errno));
		}
		if (FD_ISSET (device->fd, &fds))
		{
			ret = device_getframe (device);
			if (ret != DEVICE_OK)
				fprintf (stderr, "Could not get frame: %s\n",
					device_error (ret));
			else
				enqueue_image (&device->image);
		}
		
		if (FD_ISSET (sfd, &fds))
		{
			read (sfd, sbuf, sizeof (sbuf));
			countdown = queue_size / 2;
		}
		if (countdown > -1 && countdown-- == 0)
		{
			process_queue (device, opt);
		}
	}
	return 0;
}

int device_shot (V4l2Device *device, FieldOptions *opt)
{
	int (* fp)(V4l2Device *, FieldOptions *);
	int ret;

	fp = ((opt->flags & FO_DAEMON) ? device_loop : device_serie);
	ret = (*fp)(device, opt);

	return ret;
}

