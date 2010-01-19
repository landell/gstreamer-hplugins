/*
 *  Copyright (C) 2009  Thadeu Lima de Souza Cascardo <cascardo@holoscopio.com>
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
#include "hcvimage.h"
#include "facetracker.h"

/*
gboolean
hcv_gst_buffer_to_image_buffer (GstBuffer *gbuf, ImageBuffer *hbuf)
{
  hbuf->data = GST_BUFFER_DATA (gbuf);
  hbuf->len = GST_BUFFER_SIZE (gbuf);
  return FALSE;
}

gboolean
hcv_image_buffer_to_gst_buffer (ImageBuffer *hbuf, GstBuffer *gbuf)
{
  GST_BUFFER_DATA (gbuf) = hbuf->data;
  GST_BUFFER_SIZE (gbuf) = hbuf->len;
  return FALSE;
}
*/

static gboolean
gst_hcv_buffer_facetracker (GstBaseTransform *trans, GstBuffer *gbuf)
{
  crop_window_t *window;
  ImageBuffer buf;
  GstBuffer *nbuf = gst_buffer_ref (gbuf);
  GstCaps *caps = GST_BUFFER_CAPS (nbuf);
  GstStructure *str = gst_caps_get_structure (caps, 0);
  guint32 fmt;
  gint width;
  gint height;
	GstBus *bus;
	GstMessage *message;
	GstStructure *window_structure = gst_structure_empty_new ("crop_window_t");

  gst_structure_get_fourcc (str, "format", &fmt);
  gst_structure_get_int (str, "width", &width);
  gst_structure_get_int (str, "height", &height);
  if (fmt != GST_MAKE_FOURCC ('Y', 'C', 'b', 'r'))
	{
      return FALSE;
	}
  buf.fmt.pixelformat = V4L2_PIX_FMT_YUV420;
  buf.fmt.width = width;
  buf.fmt.height = height;
  buf.fmt.bytesperline = width * 3;
  buf.len = GST_BUFFER_SIZE (nbuf);
  buf.data = GST_BUFFER_DATA (nbuf);
  window = image_facetracker (&buf);
  if (window != NULL)
	{
			bus = gst_element_get_bus (GST_ELEMENT (trans));
			gst_structure_set (window_structure, "left", G_TYPE_INT, window->left, NULL);
			gst_structure_set (window_structure, "right", G_TYPE_INT, window->right, NULL);
			gst_structure_set (window_structure, "top", G_TYPE_INT, window->top, NULL);
			gst_structure_set (window_structure, "bottom", G_TYPE_INT, window->bottom, NULL);
			message = gst_message_new_element (GST_OBJECT (trans), window_structure);
			gst_bus_post(bus, message);
			/*send_window_bus(window);*/
      free (window);
	}
  gst_buffer_unref (nbuf);
  return TRUE;
}


static GstFlowReturn
gst_hcv_facetracker_transform_ip (GstBaseTransform *trans, GstBuffer *buf)
{
  gboolean res;
  res = gst_hcv_buffer_facetracker (trans, buf);
  return GST_FLOW_OK;
}

static GstElementDetails details =
GST_ELEMENT_DETAILS ("HCV Facetracker", "Filter/Face", "Detects a face",
"Thadeu Lima de Souza Cascardo <cascardo@holoscopio.com>");

static GstStaticPadTemplate srctemplate = 
GST_STATIC_PAD_TEMPLATE ("src", GST_PAD_SRC, GST_PAD_ALWAYS,
                         GST_STATIC_CAPS ("video/x-raw-yuv,format=(fourcc)YCbr"));
static GstStaticPadTemplate sinktemplate =
GST_STATIC_PAD_TEMPLATE ("sink", GST_PAD_SINK, GST_PAD_ALWAYS,
                         GST_STATIC_CAPS ("video/x-raw-yuv,format=(fourcc)YCbr"));

static void
gst_hcv_facetracker_base_init (GstBaseTransformClass *klass)
{
  GstPadTemplate *src;
  GstPadTemplate *sink;
  src = gst_static_pad_template_get (&srctemplate);
  sink = gst_static_pad_template_get (&sinktemplate);
  gst_element_class_add_pad_template (GST_ELEMENT_CLASS (klass), src);
  gst_element_class_add_pad_template (GST_ELEMENT_CLASS (klass), sink);
}

static void
gst_hcv_facetracker_class_init (GstBaseTransformClass *klass)
{
  klass->transform_ip = gst_hcv_facetracker_transform_ip;
  gst_element_class_set_details (GST_ELEMENT_CLASS (klass), &details);
}

static void
gst_hcv_facetracker_init (GstBaseTransform *trans, GstBaseTransformClass *klass)
{
	gst_base_transform_set_passthrough (trans,TRUE);
}

static GType
gst_hcv_facetracker_get_type (void)
{
  static GType type = 0;
  if (type == 0)
    {
      static const GTypeInfo info = {
        sizeof (GstBaseTransformClass),
        (GBaseInitFunc) gst_hcv_facetracker_base_init,
        NULL,
        (GClassInitFunc) gst_hcv_facetracker_class_init,
        NULL,
        NULL,
        sizeof (GstBaseTransform),
        0,
        (GInstanceInitFunc)gst_hcv_facetracker_init,
      };
      type = g_type_register_static (GST_TYPE_BASE_TRANSFORM,
                                     "GstHcvFacetrackerType", &info, 0);
    }
  return type;
}

#define HCV_TYPE_FACETRACKER (gst_hcv_facetracker_get_type ())

extern GType gst_hcv_ycbcr_enc_get_type (void);
#define HCV_TYPE_YCBCR_ENC (gst_hcv_ycbcr_enc_get_type ())
extern GType gst_hcv_ycbcr_dec_get_type (void);
#define HCV_TYPE_YCBCR_DEC (gst_hcv_ycbcr_dec_get_type ())
extern GType gst_hcv_kitten_get_type (void);
#define HCV_TYPE_KITTEN (gst_hcv_kitten_get_type ())

static gboolean
plugin_init (GstPlugin *plugin)
{
  gboolean res;
  res = gst_element_register (plugin, "facetracker", GST_RANK_NONE,
                              HCV_TYPE_FACETRACKER);
  res = res && gst_element_register (plugin, "ycbcrenc", GST_RANK_NONE, HCV_TYPE_YCBCR_ENC);
  res = res && gst_element_register (plugin, "ycbcrdec", GST_RANK_NONE, HCV_TYPE_YCBCR_DEC);
  res = res && gst_element_register (plugin, "kitten", GST_RANK_NONE, HCV_TYPE_KITTEN);
  return res;
}

#ifndef PACKAGE
#define PACKAGE "hcv"
#endif

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR, GST_VERSION_MINOR, "hcv", "", \
                   plugin_init, "0.1", "GPL", "gsthcv", \
                   "http://holoscopio.com");
