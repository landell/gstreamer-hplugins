GCC = gcc
CROSS_COMPILE ?=
CFLAGS ?= -g -Wall
LIBS = -ljpeg
CC := $(CROSS_COMPILE)$(GCC) $(CFLAGS)
OBJECTS = v4l2capture.o device.o negotiation.o hcverror.o hcvloop.o

all: v4l2capture client

.c.o:
	$(CC) -c -o $@ $<

client: client.o
	$(CC) -o v4l2capture-client client.o

v4l2capture: $(OBJECTS)
	$(CC) -o v4l2capture $(OBJECTS) $(LIBS)

install:
	cp v4l2capture /usr/bin/
	mkdir /var/run/hcv

clean:
	rm -f v4l2capture $(OBJECTS) client client.o
