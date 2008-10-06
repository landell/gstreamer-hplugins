#ifndef _H_INTERFACE_
#define _H_INTERFACE_

#include <stdio.h>

typedef enum {
	DEVICE_OK = 0,
	DEVICE_ERROR,
	DEVICE_INVALID,
	DEVICE_IS_NOT_V4L2,
	DEVICE_DONT_CAPTURE,
	DEVICE_MODE_NOT_SUPPORTED,
	DEVICE_FEATURE_NOT_SUPPORTED,
	DEVICE_FORMAT_INVALID
} DeviceErrors;


int device_open (char *device_name, int *fd);
int device_init(int fd);
int device_getframe(void);
int device_close(int fd);

#endif
