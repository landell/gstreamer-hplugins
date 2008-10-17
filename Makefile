
all:
	gcc -g -Wall -o v4l2capture v4l2capture.c device.c
clean:
	rm -f v4l2capture *.o
