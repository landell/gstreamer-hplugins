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

#include <stdlib.h>
#include <string.h>

static GstFlowReturn
gst_hcv_ycbcr_enc_transform (GstBaseTransform *trans G_GNUC_UNUSED, GstBuffer *inbuf,
                             GstBuffer *outbuf)
{
  unsigned int i;
  guchar * in = GST_BUFFER_DATA (inbuf);
  guchar * out = GST_BUFFER_DATA (outbuf);
  for (i = 0; i < GST_BUFFER_SIZE (inbuf) >> 1; i+=2)
    {
      out[0] = in[0];
      out[3] = in[2];
      out[1] = out[4] = in[1];
      out[2] = out[5] = in[3];
      out += 6;
      in += 4;
    }
  return GST_FLOW_OK;
}

static GstFlowReturn
gst_hcv_ycbcr_dec_transform (GstBaseTransform *trans G_GNUC_UNUSED, GstBuffer *inbuf,
                             GstBuffer *outbuf)
{
  unsigned int i;
  guchar * in = GST_BUFFER_DATA (inbuf);
  guchar * out = GST_BUFFER_DATA (outbuf);
  for (i = 0; i < GST_BUFFER_SIZE (outbuf) >> 1; i+=2)
    {
      out[0] = in[0];
      out[2] = in[3];
      out[1] = in[1];
      out[3] = in[2];
      out += 4;
      in += 6;
    }
  return GST_FLOW_OK;
}

static gboolean
gst_hcv_ycbcr_get_unit_size (GstBaseTransform *trans G_GNUC_UNUSED, GstCaps *caps,
                                 guint *size)
{
  GstStructure *str = gst_caps_get_structure (caps, 0);
  gint width;
  gint height;
  guint32 fmt;
  gint mult;
  gst_structure_get_fourcc (str, "format", &fmt);
  gst_structure_get_int (str, "width", &width);
  gst_structure_get_int (str, "height", &height);
  if (fmt == GST_MAKE_FOURCC ('Y', 'U', 'Y', '2'))
    mult = 2;
  else if (fmt == GST_MAKE_FOURCC ('Y', 'C', 'b', 'r'))
    mult = 3;
  else
    {
      return FALSE;
    }
  *size = width * height * mult;
  return TRUE;
}

GstCaps *
gst_hcv_ycbcr_enc_transform_caps (GstBaseTransform *trans G_GNUC_UNUSED, GstPadDirection dir,
                                  GstCaps *caps)
{
  GstCaps *outcaps = gst_caps_copy (caps);
  gst_caps_set_simple (outcaps, "format", GST_TYPE_FOURCC,
                       GST_MAKE_FOURCC ('Y', 'C', 'b', 'r'), NULL);
  if (dir == GST_PAD_SRC)
    gst_caps_set_simple (outcaps, "format", GST_TYPE_FOURCC,
                         GST_MAKE_FOURCC ('Y', 'U', 'Y', '2'), NULL);
  return outcaps;
}

GstCaps *
gst_hcv_ycbcr_dec_transform_caps (GstBaseTransform *trans G_GNUC_UNUSED, GstPadDirection dir,
                                  GstCaps *caps)
{
  GstCaps *outcaps = gst_caps_copy (caps);
  gst_caps_set_simple (outcaps, "format", GST_TYPE_FOURCC,
                       GST_MAKE_FOURCC ('Y', 'U', 'Y', '2'), NULL);
  if (dir == GST_PAD_SRC)
    gst_caps_set_simple (outcaps, "format", GST_TYPE_FOURCC,
                         GST_MAKE_FOURCC ('Y', 'C', 'b', 'r'), NULL);
  return outcaps;
}


static GstElementDetails enc_details =
GST_ELEMENT_DETAILS ("HCV YCbCR Dec", "Filter/YCbCrEnc", "Converts to YCbCR",
"Thadeu Lima de Souza Cascardo <cascardo@holoscopio.com>");

static GstElementDetails dec_details =
GST_ELEMENT_DETAILS ("HCV YCbCR Enc", "Filter/YCbCrDec", "Converts from YCbCR",
"Thadeu Lima de Souza Cascardo <cascardo@holoscopio.com>");


static GstStaticPadTemplate enc_srctemplate = 
GST_STATIC_PAD_TEMPLATE ("src", GST_PAD_SRC, GST_PAD_ALWAYS,
                         GST_STATIC_CAPS ("video/x-raw-yuv,format=(fourcc)YCbr"));
static GstStaticPadTemplate enc_sinktemplate =
GST_STATIC_PAD_TEMPLATE ("sink", GST_PAD_SINK, GST_PAD_ALWAYS,
                         GST_STATIC_CAPS ("video/x-raw-yuv,format=(fourcc)YUY2"));
static GstStaticPadTemplate dec_srctemplate = 
GST_STATIC_PAD_TEMPLATE ("src", GST_PAD_SRC, GST_PAD_ALWAYS,
                         GST_STATIC_CAPS ("video/x-raw-yuv,format=(fourcc)YUY2"));
static GstStaticPadTemplate dec_sinktemplate =
GST_STATIC_PAD_TEMPLATE ("sink", GST_PAD_SINK, GST_PAD_ALWAYS,
                         GST_STATIC_CAPS ("video/x-raw-yuv,format=(fourcc)YCbr"));


static void
gst_hcv_ycbcr_enc_base_init (GstBaseTransformClass *klass)
{
  GstPadTemplate *src;
  GstPadTemplate *sink;
  src = gst_static_pad_template_get (&enc_srctemplate);
  sink = gst_static_pad_template_get (&enc_sinktemplate);
  gst_element_class_add_pad_template (GST_ELEMENT_CLASS (klass), src);
  gst_element_class_add_pad_template (GST_ELEMENT_CLASS (klass), sink);
}

static void
gst_hcv_ycbcr_dec_base_init (GstBaseTransformClass *klass)
{
  GstPadTemplate *src;
  GstPadTemplate *sink;
  src = gst_static_pad_template_get (&dec_srctemplate);
  sink = gst_static_pad_template_get (&dec_sinktemplate);
  gst_element_class_add_pad_template (GST_ELEMENT_CLASS (klass), src);
  gst_element_class_add_pad_template (GST_ELEMENT_CLASS (klass), sink);
}

static void
gst_hcv_ycbcr_enc_class_init (GstBaseTransformClass *klass)
{
  klass->transform = gst_hcv_ycbcr_enc_transform;
  klass->transform_caps = gst_hcv_ycbcr_enc_transform_caps;
  klass->get_unit_size = gst_hcv_ycbcr_get_unit_size;
  gst_element_class_set_details (GST_ELEMENT_CLASS (klass), &enc_details);
}

static void
gst_hcv_ycbcr_dec_class_init (GstBaseTransformClass *klass)
{
  klass->transform = gst_hcv_ycbcr_dec_transform;
  klass->transform_caps = gst_hcv_ycbcr_dec_transform_caps;
  klass->get_unit_size = gst_hcv_ycbcr_get_unit_size;
  gst_element_class_set_details (GST_ELEMENT_CLASS (klass), &dec_details);
}

GType
gst_hcv_ycbcr_enc_get_type (void)
{
  static GType type = 0;
  if (type == 0)
    {
      static const GTypeInfo info = {
        sizeof (GstBaseTransformClass),
        (GBaseInitFunc) gst_hcv_ycbcr_enc_base_init,
        NULL,
        (GClassInitFunc) gst_hcv_ycbcr_enc_class_init,
        NULL,
        NULL,
        sizeof (GstBaseTransform),
        0,
        NULL,
				NULL
      };
      type = g_type_register_static (GST_TYPE_BASE_TRANSFORM,
                                     "GstHCVYCbCrEncType", &info, 0);
    }
  return type;
}

GType
gst_hcv_ycbcr_dec_get_type (void)
{
  static GType type = 0;
  if (type == 0)
    {
      static const GTypeInfo info = {
        sizeof (GstBaseTransformClass),
        (GBaseInitFunc) gst_hcv_ycbcr_dec_base_init,
        NULL,
        (GClassInitFunc) gst_hcv_ycbcr_dec_class_init,
        NULL,
        NULL,
        sizeof (GstBaseTransform),
        0,
        NULL,
				NULL
      };
      type = g_type_register_static (GST_TYPE_BASE_TRANSFORM,
                                     "GstHCVYCbCrDecType", &info, 0);
    }
  return type;
}


#define HCV_TYPE_YCBCR_ENC (gst_hcv_ycbcr_enc_get_type ())
#define HCV_TYPE_YCBCR_DEC (gst_hcv_ycbcr_dec_get_type ())

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
