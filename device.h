#ifndef _H_INTERFACE_
#define _H_INTERFACE_

typedef enum {
	DEVICE_OK = 0,
	DEVICE_ERROR,
	DEVICE_INVALID,
	DEVICE_IS_NOT_V4L2,
	DEVICE_DONT_CAPTURE,
	DEVICE_MODE_NOT_SUPPORTED,
	DEVICE_FEATURE_NOT_SUPPORTED,
	DEVICE_INVALID_FORMAT,
	DEVICE_OUT_OF_MEMORY,
	DEVICE_BUFFER_ERROR
} DeviceErrors;

typedef struct {
	void *start;
	size_t length;
} DeviceBuffer;

typedef struct {
	int fd;
	char *name;
	int width;
	int height;
	char *prefix;
	DeviceBuffer *buffer;
	int n_buffers;
} V4l2Device;

int device_open (V4l2Device *dev);
int device_init(V4l2Device *dev);
int device_getframe(V4l2Device *dev);
int device_close(V4l2Device *dev);

#endif
