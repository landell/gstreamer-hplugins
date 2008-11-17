GCC = gcc
CROSS_COMPILE ?=
CFLAGS ?= -g -Wall
LIBS = -ljpeg
CC := $(CROSS_COMPILE)$(GCC) $(CFLAGS)
OBJECTS = v4l2capture.o device.o negotiation.o hcverror.o

all: v4l2capture

.c.o:
	$(CC) -c -o $@ $<

v4l2capture: $(OBJECTS)
	$(CC) -o v4l2capture $(OBJECTS) $(LIBS)

install:
	cp v4l2capture /usr/bin/

clean:
	rm -f v4l2capture $(OBJECTS)
