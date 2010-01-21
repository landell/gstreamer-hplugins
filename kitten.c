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
#include "crop.h"

#include <stdlib.h>
#include <string.h>

GType gst_hcv_kitten_get_type (void);

#define HCV_TYPE_KITTEN (gst_hcv_kitten_get_type ())
#define HCV_KITTEN(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), HCV_TYPE_KITTEN, GstHcvKitten))

typedef struct _GstHcvKittenPriv GstHcvKittenPriv;
typedef struct _GstHcvKitten GstHcvKitten;

enum
{
	  PROP_0,

		GST_HCV_KITTEN_LEFT,
		GST_HCV_KITTEN_RIGHT,
		GST_HCV_KITTEN_BOTTOM,
		GST_HCV_KITTEN_TOP
};

struct _GstHcvKittenPriv
{
	int left;
	int right;
	int top;
	int bottom;
};

struct _GstHcvKitten
{
	GstBaseTransform parent_instance;

	GstHcvKittenPriv *priv;
};

static gboolean 
gst_hcv_buffer_kitten (GstHcvKitten *self, GstBuffer *gbuf)
{
  GstBuffer *nbuf = gst_buffer_ref (gbuf);
  GstCaps *caps = GST_BUFFER_CAPS (nbuf);
  GstStructure *str = gst_caps_get_structure (caps, 0);
	unsigned char *d1;
	unsigned char *d2;
	int bytes_per_line;
  gint width;
  gint height;
	gint bpp = 0;
	unsigned char *data;
	int i;

  gst_structure_get_int (str, "width", &width);
  gst_structure_get_int (str, "height", &height);
	if (!gst_structure_get_int (str, "bpp", &bpp))
		bpp = 3;
	else
		bpp /= 8;
	bytes_per_line = bpp * width;
	data = GST_BUFFER_DATA(nbuf);
	//memset (GST_BUFFER_DATA(nbuf)+80000, 0, 80000);
	d1 = data + self->priv->top * bytes_per_line + self->priv->left * bpp;
	d2 = data + self->priv->top * bytes_per_line + self->priv->right * bpp;
	for (i=self->priv->top; i<self->priv->bottom; i++)
	{
		memset (d1, 0, d2-d1);
		d1 += bytes_per_line;
		d2 += bytes_per_line;

	}
  gst_buffer_unref (nbuf);
  return TRUE;
}

static GstFlowReturn
gst_hcv_kitten_transform_ip (GstBaseTransform *trans, GstBuffer *buf)
{
  gboolean res;
  res = gst_hcv_buffer_kitten (HCV_KITTEN (trans), buf);
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
gst_hcv_kitten_set_property (GObject      *object,
                             guint         property_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
	 GstHcvKitten *self = HCV_KITTEN (object);

	 switch (property_id)
	 {
		 case GST_HCV_KITTEN_LEFT:
			 self->priv->left = g_value_get_int (value);
			 g_print ("left: %d\n", self->priv->left);
			 break;

		 case GST_HCV_KITTEN_RIGHT:
			 self->priv->right = g_value_get_int (value);
			 g_print ("right: %d\n", self->priv->right);
			 break;

		 case GST_HCV_KITTEN_TOP:
			 self->priv->top = g_value_get_int (value);
			 g_print ("top: %d\n", self->priv->top);
			 break;

		 case GST_HCV_KITTEN_BOTTOM:
			 self->priv->bottom = g_value_get_int (value);
			 g_print ("bottom: %d\n", self->priv->bottom);
			 break;

		 default:
			 /* We don't have any other property... */
			 G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
			 break;
	 }
}

static void 
gst_hcv_kitten_get_property (GObject      *object,
                             guint         property_id,
                             GValue *value,
                             GParamSpec   *pspec)
{
	 GstHcvKitten *self = HCV_KITTEN (object);

	 switch (property_id)
	 {
		 case GST_HCV_KITTEN_LEFT:
			 g_value_set_int (value, self->priv->left);
			 g_print ("left: %d\n", self->priv->left);
			 break;

		 case GST_HCV_KITTEN_RIGHT:
			 g_value_set_int (value, self->priv->right);
			 g_print ("right: %d\n", self->priv->right);
			 break;

		 case GST_HCV_KITTEN_TOP:
			 g_value_set_int (value, self->priv->top);
			 g_print ("top: %d\n", self->priv->top);
			 break;

		 case GST_HCV_KITTEN_BOTTOM:
			 g_value_set_int (value, self->priv->bottom);
			 g_print ("bottom: %d\n", self->priv->bottom);
			 break;

		 default:
			 /* We don't have any other property... */
			 G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
			 break;
	 }
}

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
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	GParamSpec *pspec;

  klass->transform_ip = gst_hcv_kitten_transform_ip;
  gst_element_class_set_details (GST_ELEMENT_CLASS (klass), &kitten_details);

	gobject_class->set_property = gst_hcv_kitten_set_property;
	gobject_class->get_property = gst_hcv_kitten_get_property;

	pspec = g_param_spec_int ("window_left",
			"Left coordinate for window",
			"Set/Get left coordinate",
			0  /* minimum value */,
			G_MAXINT /* maximum value */,
			2  /* default value */,
			G_PARAM_READWRITE);
	g_object_class_install_property (gobject_class,
			GST_HCV_KITTEN_LEFT,
			pspec);
	pspec = g_param_spec_int ("window_right",
			"Right coordinate for window",
			"Set/Get right coordinate",
			0  /* minimum value */,
			G_MAXINT /* maximum value */,
			2  /* default value */,
			G_PARAM_READWRITE);
	g_object_class_install_property (gobject_class,
			GST_HCV_KITTEN_RIGHT,
			pspec);
	pspec = g_param_spec_int ("window_top",
			"Top coordinate for window",
			"Set/Get top coordinate",
			0  /* minimum value */,
			G_MAXINT /* maximum value */,
			2  /* default value */,
			G_PARAM_READWRITE);
	g_object_class_install_property (gobject_class,
			GST_HCV_KITTEN_TOP,
			pspec);
	pspec = g_param_spec_int ("window_bottom",
			"Bottom coordinate for window",
			"Set/Get bottom coordinate",
			0  /* minimum value */,
			G_MAXINT /* maximum value */,
			2  /* default value */,
			G_PARAM_READWRITE);
	g_object_class_install_property (gobject_class,
			GST_HCV_KITTEN_BOTTOM,
			pspec);
}

static void
gst_hcv_kitten_init(GstHcvKitten *trans, GstBaseTransformClass *klass G_GNUC_UNUSED)
{
	trans->priv = (GstHcvKittenPriv *) malloc(sizeof(GstHcvKittenPriv));
	trans->priv->left = 0;
	trans->priv->right = 0;
	trans->priv->top = 0;
	trans->priv->bottom = 0;
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
        (GInstanceInitFunc) gst_hcv_kitten_init,
				NULL
      };
      type = g_type_register_static (GST_TYPE_BASE_TRANSFORM,
                                     "GstHcvKittenType", &info, 0);
    }
  return type;
}

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
