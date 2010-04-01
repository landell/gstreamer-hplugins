GCC = gcc
CROSS_COMPILE ?=
CFLAGS ?= -g -Wall -Wextra
LIBS = -ljpeg
OBJECTS = device.o negotiation.o hcverror.o hcvloop.o \
	hcvconv.o crop.o save.o savejpeg.o hcvmemsrc.o facetracker.o
GST_OBJECTS = gstreamer.o facetracker.o crop.o ycbcr.o kitten.o
DESTDIR ?=
PREFIX ?= /usr/local
INSTALL_DIR := $(PREFIX)/bin/
GST_LIB_DIR := $(PREFIX)/lib/gstreamer-0.10
LIB_DIR := $(PREFIX)/lib
DATADIR := $(PREFIX)/share
CFLAGS += -DDATADIR="\"$(DATADIR)\""
CC := $(CROSS_COMPILE)$(GCC) $(CFLAGS) -fPIC

all: libgsthcv.so
	sed 's,@LIBDIR@,$(LIB_DIR),g' < kitten.in > kitten

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
	install -D kitten $(DESTDIR)$(INSTALL_DIR)/kitten
	install -D libgsthcv.so $(DESTDIR)$(GST_LIB_DIR)/libgsthcv.so
	install -D hcv/trackerlaunch.py $(DESTDIR)$(LIB_DIR)/hcv/trackerlaunch.py
	install -D hcv/__init__.py $(DESTDIR)$(LIB_DIR)/hcv/__init__.py
	install -D img/kitten.png $(DESTDIR)$(DATADIR)/hcv/kitten.png

clean:
	rm -f $(OBJECTS) \
		gstreamer.o libgsthcv.so ycbcr.o kitten.o kitten
