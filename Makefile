GCC = gcc
CROSS_COMPILE ?=
CFLAGS ?= -g -Wall -Wextra
LIBS = -ljpeg
CC := $(CROSS_COMPILE)$(GCC) $(CFLAGS) -fPIC
OBJECTS = device.o negotiation.o hcverror.o hcvloop.o \
	hcvconv.o crop.o save.o savejpeg.o hcvmemsrc.o facetracker.o
GST_OBJECTS = gstreamer.o facetracker.o crop.o ycbcr.o kitten.o
DESTDIR ?=
INSTALL_DIR := $(DESTDIR)/usr/bin/
GST_LIB_DIR := $(DESTDIR)/usr/lib/gstreamer-0.10
LIB_DIR := $(DESTDIR)/usr/lib
DATA_DIR := $(DESTDIR)/var/lib/hcv
HEADER_FILE = $(DATA_DIR)/header.jpg

all: libgsthcv.so

gstreamer.o: gstreamer.c
	$(CC) -c -o $@ $< `pkg-config --cflags gstreamer-0.10`

ycbcr.o: ycbcr.c
	$(CC) -c -o $@ $< `pkg-config --cflags gstreamer-0.10`

kitten.o: kitten.c
	$(CC) -c -o $@ $< `pkg-config --cflags gstreamer-0.10 gstreamer-plugins-base-0.10 cairo`

libgsthcv.so: $(GST_OBJECTS)
	$(CC) -o $@ -shared $(GST_OBJECTS) `pkg-config --libs gstreamer-0.10 gstreamer-base-0.10 gstreamer-plugins-base-0.10 cairo` -lgstvideo-0.10

.c.o:
	$(CC) -c -o $@ $<

install: all
	install -D kitten $(INSTALL_DIR)/kitten
	install -D libgsthcv.so $(GST_LIB_DIR)/libgsthcv.so
	install -D trackerlaunch.py $(LIB_DIR)/kitten/trackerlaunch.py

clean:
	rm -f $(OBJECTS) \
		gstreamer.o libgsthcv.so ycbcr.o kitten.o
