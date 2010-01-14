GCC = gcc
CROSS_COMPILE ?=
CFLAGS ?= -g -Wall -Wextra
LIBS = -ljpeg
CC := $(CROSS_COMPILE)$(GCC) $(CFLAGS) -fPIC
OBJECTS = v4l2capture.o device.o negotiation.o hcverror.o hcvloop.o \
	hcvconv.o crop.o save.o savejpeg.o hcvmemsrc.o facetracker.o
GST_OBJECTS = gstreamer.o facetracker.o crop.o ycbcr.o kitten.o
DESTDIR ?=
INSTALL_DIR := $(DESTDIR)/usr/bin/
RUNTIME_DIR := $(DESTDIR)/var/run/hcv
DATA_DIR := $(DESTDIR)/var/lib/hcv
HEADER_FILE = $(DATA_DIR)/header.jpg
FAKEROOT = fakeroot

all: v4l2capture v4l2capture-client jheader libgsthcv.so

gstreamer.o: gstreamer.c
	$(CC) -c -o $@ $< `pkg-config --cflags gstreamer-0.10`

ycbcr.o: ycbcr.c
	$(CC) -c -o $@ $< `pkg-config --cflags gstreamer-0.10`

kitten.o: kitten.c
	$(CC) -c -o $@ $< `pkg-config --cflags gstreamer-0.10`

libgsthcv.so: $(GST_OBJECTS)
	$(CC) -o $@ -shared $(GST_OBJECTS) `pkg-config --libs gstreamer-0.10 gstreamer-base-0.10`

jheader.o: jheader.c
	$(GCC) -c -o $@ $<

.c.o:
	$(CC) -c -o $@ $<

jheader: jheader.o
	$(GCC) -o jheader jheader.o $(LIBS)

v4l2capture-client: client.o
	$(CC) -o v4l2capture-client client.o

v4l2capture: $(OBJECTS)
	$(CC) -o v4l2capture $(OBJECTS) $(LIBS)

install: all
	install -D v4l2capture $(INSTALL_DIR)/v4l2capture
	install -D v4l2capture-client $(INSTALL_DIR)/v4l2capture-client
	$(FAKEROOT) install -D -o root -g video -d $(RUNTIME_DIR)
	$(FAKEROOT) install -D -o root -g video -d $(DATA_DIR)
	./jheader $(HEADER_FILE)

clean:
	rm -f v4l2capture $(OBJECTS) v4l2capture-client client.o \
		jheader jheader.o gstreamer.o libgsthcv.so ycbcr.o kitten.o
