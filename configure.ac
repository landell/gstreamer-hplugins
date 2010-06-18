#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT(Gstreamer HCV Plugin, 0.1, luciana@holoscopio.com)
AM_INIT_AUTOMAKE,
AC_REVISION([$Revision: 0.1 $])
AC_CONFIG_SRCDIR([src/cairoimageoverlay.c])
AC_CONFIG_MACRO_DIR([m4])

dnl we override it here if we need to for the release candidate of new series
GST_MAJORMINOR=0.10

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL

# Checks for libraries.
# FIXME: Replace `main' with a function in `-ljpeg':
PKG_CHECK_MODULES(GSTREAMER, gstreamer-0.10)
PKG_CHECK_MODULES(GSTREAMER_BASE, gstreamer-base-0.10)
PKG_CHECK_MODULES(GST_PLUGINS_BASE, gstreamer-plugins-base-0.10)
PKG_CHECK_MODULES(GSTVIDEO, gstreamer-video-0.10)
PKG_CHECK_MODULES(CAIRO, cairo)

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h cairo.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_FUNC_MALLOC

AC_CONFIG_FILES([Makefile
                 src/Makefile])

AC_OUTPUT
