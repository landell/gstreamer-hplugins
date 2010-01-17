/*
 *  Copyright (C) 2009 Holoscopio Tecnologia
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

#include <stdlib.h>
#include <string.h>

static gboolean 
gst_hcv_buffer_kitten (GstBuffer *gbuf)
{
  /*crop_window_t *window = NULL;
  ImageBuffer buf;*/
  GstBuffer *nbuf = gst_buffer_ref (gbuf);
  GstCaps *caps = GST_BUFFER_CAPS (nbuf);
  GstStructure *str = gst_caps_get_structure (caps, 0);
  gint width;
  gint height;
	unsigned char *data;
	unsigned char *buf = malloc(sizeof(unsigned char) * 10);
  gst_structure_get_int (str, "width", &width);
  gst_structure_get_int (str, "height", &height);
	data = GST_BUFFER_DATA(nbuf);
	memset (GST_BUFFER_DATA(nbuf)+80000, 0, 80000);
  /*if (window != NULL)
    {
			send_window_bus(window);
      free (window);
    }*/
  gst_buffer_unref (nbuf);
  return TRUE;
}

static GstFlowReturn
gst_hcv_kitten_transform_ip (GstBaseTransform *trans, GstBuffer *buf)
{
  gboolean res;
  res = gst_hcv_buffer_kitten (buf);
  return GST_FLOW_OK;
}

static GstElementDetails kitten_details =
GST_ELEMENT_DETAILS ("HCV Kitten Secrecy", "Filter/Kitten", "Put image in detected space",
"Luciana Fujii Pontello <luciana@holoscopio.com>");

static GstStaticPadTemplate srctemplate = 
GST_STATIC_PAD_TEMPLATE ("src", GST_PAD_SRC, GST_PAD_ALWAYS,
		GST_STATIC_CAPS ("video/x-raw-rgb"));

static GstStaticPadTemplate sinktemplate =
GST_STATIC_PAD_TEMPLATE ("sink", GST_PAD_SINK, GST_PAD_ALWAYS,
                         GST_STATIC_CAPS ("video/x-raw-rgb"));

static void
gst_hcv_kitten_base_init (GstBaseTransformClass *klass)
{
  GstPadTemplate *src;
  GstPadTemplate *sink;
  src = gst_static_pad_template_get (&srctemplate);
  sink = gst_static_pad_template_get (&sinktemplate);
  gst_element_class_add_pad_template (GST_ELEMENT_CLASS (klass), src);
  gst_element_class_add_pad_template (GST_ELEMENT_CLASS (klass), sink);
}

static void
gst_hcv_kitten_class_init (GstBaseTransformClass *klass)
{
  klass->transform_ip = gst_hcv_kitten_transform_ip;
  gst_element_class_set_details (GST_ELEMENT_CLASS (klass), &kitten_details);
}

GType
gst_hcv_kitten_get_type (void)
{
  static GType type = 0;
  if (type == 0)
    {
      static const GTypeInfo info = {
        sizeof (GstBaseTransformClass),
        (GBaseInitFunc) gst_hcv_kitten_base_init,
        NULL,
        (GClassInitFunc) gst_hcv_kitten_class_init,
        NULL,
        NULL,
        sizeof (GstBaseTransform),
        0,
        NULL
      };
      type = g_type_register_static (GST_TYPE_BASE_TRANSFORM,
                                     "GstHcvKittenType", &info, 0);
    }
  return type;
}

#define HCV_TYPE_KITTEN (gst_hcv_kitten_get_type ())

/*
static gboolean
plugin_init (GstPlugin *plugin)
{
  gboolean res;
  res = gst_element_register (plugin, "ycbcr", GST_RANK_NONE,
                              HCV_TYPE_YCBCR);
  return res;
}

#ifndef PACKAGE
#define PACKAGE "hcv"
#endif

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR, GST_VERSION_MINOR, "hcv", "", \
                   plugin_init, "0.1", "GPL", "gsthcv", \
                   "http://holoscopio.com");
*/
