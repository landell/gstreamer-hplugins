plugin_LTLIBRARIES = libgstcairoimageoverlay.la

plugindir = $(libdir)/gstreamer-0.10/

libgstcairoimageoverlay_la_SOURCES = \
	gstreamer.c \
	cairoimageoverlay.c \
	livekeeper.c \
	livekeeper.h

libgstcairoimageoverlay_la_CFLAGS = \
					  $(GSTREAMER_CFLAGS) \
					  $(GSTREAMER_BASE_CFLAGS) \
					  $(GST_PLUGINS_BASE_CFLAGS) \
					  $(GSTVIDEO_CFLAGS) \
					  $(CAIRO_CFLAGS)


libgstcairoimageoverlay_la_LIBADD = \
					  $(GSTREAMER_LIBS) \
					  $(GSTREAMER_BASE_LIBS) \
					  $(GST_PLUGINS_BASE_LIBS) \
					  $(GSTVIDEO_LIBS) \
					  $(CAIRO_LIBS)

libgstcairoimageoverlay_la_LDFLAGS = -module -avoid-version
