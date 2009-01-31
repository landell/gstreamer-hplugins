GCC = gcc
CROSS_COMPILE ?=
CFLAGS ?= -g -Wall
LIBS = -ljpeg
CC := $(CROSS_COMPILE)$(GCC) $(CFLAGS)
OBJECTS = v4l2capture.o device.o negotiation.o hcverror.o hcvloop.o \
	hcvconv.o crop.o save.o savejpeg.o hcvmemsrc.o facetracker.o
INSTALL_DIR = /usr/bin/
RUNTIME_DIR = /var/run/hcv
DATA_DIR = /var/lib/hcv
HEADER_FILE = $(DATA_DIR)/header.jpg

all: v4l2capture v4l2capture-client jheader

.c.o:
	$(CC) -c -o $@ $<

jheader: jheader.o
	$(CC) -o jheader jheader.o $(LIBS)

v4l2capture-client: client.o
	$(CC) -o v4l2capture-client client.o

v4l2capture: $(OBJECTS)
	$(CC) -o v4l2capture $(OBJECTS) $(LIBS)

install:
	cp v4l2capture $(INSTALL_DIR)
	cp v4l2capture-client $(INSTALL_DIR)
	mkdir -p $(RUNTIME_DIR) $(DATA_DIR)
	chown root.video $(RUNTIME_DIR) $(DATA_DIR)
	chmod 775 $(RUNTIME_DIR) $(DATA_DIR)
	./jheader $(HEADER_FILE)

clean:
	rm -f v4l2capture $(OBJECTS) v4l2capture-client client.o \
		jheader jheader.o
