
all:
	gcc -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -lglib-2.0 -o v4l2capture v4l2capture.c device.c
clean:
	rm -f v4l2capture *.o
