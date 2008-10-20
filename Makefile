GCC = gcc
CROSS_COMPILE ?=
CFLAGS ?= -g -Wall
CC := $(CROSS_COMPILE)$(GCC) $(CFLAGS)
OBJECTS = v4l2capture.o device.o

all: v4l2capture

.c.o:
	$(CC) -c -o $@ $<

v4l2capture: $(OBJECTS)
	$(CC) -o v4l2capture $(OBJECTS)

clean:
	rm -f v4l2capture $(OBJECTS)
