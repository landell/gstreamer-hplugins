plugin_LTLIBRARIES = libgsthcv.la

plugindir = $(libdir)/gstreamer-0.10/

libgsthcv_la_SOURCES = gstreamer.c cairoimageoverlay.c

libgsthcv_la_CFLAGS = \
					  $(GSTREAMER_CFLAGS) \
					  $(GSTREAMER_BASE_CFLAGS) \
					  $(GST_PLUGINS_BASE_CFLAGS) \
					  $(GSTVIDEO_CFLAGS) \
					  $(CAIRO_CFLAGS)


libgsthcv_la_LIBADD = \
					  $(GSTREAMER_LIBS) \
					  $(GSTREAMER_BASE_LIBS) \
					  $(GST_PLUGINS_BASE_LIBS) \
					  $(GSTVIDEO_LIBS) \
					  $(CAIRO_LIBS)
