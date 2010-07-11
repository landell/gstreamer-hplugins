/*
 *  Copyright (C) 2009  Thadeu Lima de Souza Cascardo <cascardo@holoscopio.com>
 *  Copyright (C) 2010  Holoscópio Tecnologia
 *  Author: Luciana Fujii Pontello <luciana@holoscopio.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <gst/gst.h>
#include <gst/base/gstbasetransform.h>

extern GType hcv_image_overlay_get_type (void);
#define HCV_TYPE_IMAGE_OVERLAY (hcv_image_overlay_get_type ())

static gboolean
plugin_init (GstPlugin *plugin)
{
  gboolean res;
  res = gst_element_register (plugin, "cairoimageoverlay", GST_RANK_NONE, HCV_TYPE_IMAGE_OVERLAY);
  return res;
}

#ifndef PACKAGE
#define PACKAGE "hplugins"
#endif

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR, GST_VERSION_MINOR, "hplugins", "", \
                   plugin_init, "0.2", "GPL", "gstcairoimageoverlay", \
                   "http://holoscopio.com");
