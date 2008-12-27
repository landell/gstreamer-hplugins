GCC = gcc
CROSS_COMPILE ?=
CFLAGS ?= -g -Wall
LIBS = -ljpeg
CC := $(CROSS_COMPILE)$(GCC) $(CFLAGS)
OBJECTS = v4l2capture.o device.o negotiation.o hcverror.o hcvloop.o \
	crop.o jpegutils.o save.o savepipe.o hcvmemsrc.o
INSTALL_DIR = /usr/bin/
RUNTIME_DIR = /var/run/hcv

all: v4l2capture v4l2capture-client

.c.o:
	$(CC) -c -o $@ $<

v4l2capture-client: client.o
	$(CC) -o v4l2capture-client client.o

v4l2capture: $(OBJECTS)
	$(CC) -o v4l2capture $(OBJECTS) $(LIBS)

install:
	cp v4l2capture $(INSTALL_DIR)
	cp v4l2capture-client $(INSTALL_DIR)
	mkdir -p $(RUNTIME_DIR)
	chown root.video $(RUNTIME_DIR)
	chmod 775 $(RUNTIME_DIR)

clean:
	rm -f v4l2capture $(OBJECTS) v4l2capture-client client.o
