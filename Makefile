GCC = gcc
CROSS_COMPILE ?=
CFLAGS ?= -g -Wall -Wextra
LIBS = -ljpeg
OBJECTS = hcverror.o
GST_OBJECTS = gstreamer.o cairoimageoverlay.o
DESTDIR ?=
PREFIX ?= /usr/local
INSTALL_DIR := $(PREFIX)/bin/
GST_LIB_DIR := $(PREFIX)/lib/gstreamer-0.10
LIB_DIR := $(PREFIX)/lib
DATADIR := $(PREFIX)/share
CFLAGS += -DDATADIR="\"$(DATADIR)\""
CC := $(CROSS_COMPILE)$(GCC) $(CFLAGS) -fPIC

all: libgsthcv.so

gstreamer.o: gstreamer.c
	$(CC) -c -o $@ $< `pkg-config --cflags gstreamer-0.10`

cairoimageoverlay.o: cairoimageoverlay.c
	$(CC) -c -o $@ $< `pkg-config --cflags gstreamer-0.10 gstreamer-plugins-base-0.10 cairo`

libgsthcv.so: $(GST_OBJECTS)
	$(CC) -o $@ -shared $(GST_OBJECTS) `pkg-config --libs gstreamer-0.10 gstreamer-base-0.10 gstreamer-plugins-base-0.10 cairo` -lgstvideo-0.10

.c.o:
	$(CC) -c -o $@ $<

install: all
	install -D --mode=644 libgsthcv.so $(DESTDIR)$(GST_LIB_DIR)/libgsthcv.so
	install -D --mode=644 img/kitten.png $(DESTDIR)$(DATADIR)/hcv/kitten.png

clean:
	rm -f $(OBJECTS) \
		gstreamer.o libgsthcv.so ycbcr.o cairoimageoverlay.o
