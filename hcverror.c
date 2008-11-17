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


#include "device.h"
#include "hcverror.h"

char * device_error (DeviceErrors error)
{
	switch (error)
	{
		case DEVICE_OK:
			return "Device OK.";
		case DEVICE_ERROR:
			return "Device error.";
		case DEVICE_INVALID:
			return "Invalid device.";
		case DEVICE_IS_NOT_V4L2:
			return "Device is not a v4l2 device.";
		case DEVICE_DONT_CAPTURE:
			return "Device does not support video capture.";
		case DEVICE_MODE_NOT_SUPPORTED:
			return "Device does not support mode.";
		case DEVICE_FEATURE_NOT_SUPPORTED:
			return "Device does not support feature.";
		case DEVICE_INVALID_FORMAT:
			return "Invalid image format.";
		case DEVICE_OUT_OF_MEMORY:
			return "System out of memory.";
		case DEVICE_BUFFER_ERROR:
			return "Buffer error.";
		case DEVICE_STREAM_ERROR:
			return "Stream error.";
		case DEVICE_NOT_READY:
			return "Device not ready.";
		case DEVICE_EMPTY_FRAME:
			return "Empty frame.";
		default:
			return "Unknown Error.";
	}
}
