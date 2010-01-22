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
#include <gst/video/video.h>
#include "crop.h"

#include <cairo.h>
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
	cairo_surface_t *image;
	cairo_t *scaled_context;
	cairo_surface_t *surface;
	float kitten_width;
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
	GstCaps *caps;
	cairo_t* cr;
	cairo_surface_t* surface;
	static gint width;
	static gint height;
	static cairo_format_t cairo_format;
	static int stride = -1;
	GstVideoFormat format;
	float desired_width = 0;
	static float previous_width = 0;
	float scale;
	int x;
	int y;


	if (stride == -1)
	{
		caps = gst_buffer_get_caps (nbuf);
		if (!gst_video_format_parse_caps (caps, &format, &width, &height)) {
			gst_caps_unref (caps);
			g_warning ("Could not parse caps");
			return FALSE;
		}

		if (format == GST_VIDEO_FORMAT_ARGB || format == GST_VIDEO_FORMAT_BGRA)
		{
			cairo_format = CAIRO_FORMAT_ARGB32;
			g_print ("Formato é ARGB32\n");
		}
		else
		{
			cairo_format = CAIRO_FORMAT_RGB24;
			g_print ("Fomato é RGB24\n");
		}
		stride = cairo_format_stride_for_width (cairo_format, width);
		gst_caps_unref (caps);
	}

	desired_width = (self->priv->right - self->priv->left)* 2.0;
	if (desired_width > width)
		desired_width = width;
	if (abs(previous_width - desired_width) > 50 )
	{
		/*If there was a scaled context, destroy it to create a new one*/
		if (self->priv->scaled_context != NULL)
		{
			cairo_destroy (self->priv->scaled_context);
			if (self->priv->surface != NULL)
				cairo_surface_destroy (self->priv->surface);
		}
		previous_width = desired_width;
		self->priv->surface = cairo_image_surface_create (cairo_format, width, height);
		self->priv->scaled_context = cairo_create (self->priv->surface);

		scale = (desired_width + 1) / self->priv->kitten_width;
		cairo_scale (self->priv->scaled_context, scale, scale);
	}
	else
	{
		if (self->priv->scaled_context == NULL)
		{
			self->priv->surface = cairo_image_surface_create (cairo_format, width, height);
			self->priv->scaled_context = cairo_create (self->priv->surface);

			scale = (desired_width + 1) / self->priv->kitten_width;
			cairo_scale (self->priv->scaled_context, scale, scale);
		}
	}
	cairo_set_source_surface (self->priv->scaled_context, self->priv->image, 0, 0);
	cairo_paint (self->priv->scaled_context);


	surface = cairo_image_surface_create_for_data (GST_BUFFER_DATA (nbuf),
			cairo_format,
			width,
			height,
			stride);
	cr = cairo_create (surface);
	x = self->priv->left - desired_width*0.25;
	if (x < 0)
		x = 0;
	y = self->priv->top - desired_width*0.20;
	if (y < 0)
		y = 0;

	cairo_set_source_surface (cr, self->priv->surface, x, y);
	cairo_paint (cr);

	cairo_destroy (cr);
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
		GST_STATIC_CAPS (GST_VIDEO_CAPS_ARGB ";" GST_VIDEO_CAPS_BGRA));

static GstStaticPadTemplate sinktemplate =
GST_STATIC_PAD_TEMPLATE ("sink", GST_PAD_SINK, GST_PAD_ALWAYS,
                         GST_STATIC_CAPS (GST_VIDEO_CAPS_ARGB ";" GST_VIDEO_CAPS_BGRA));

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
gst_hcv_kitten_dispose (GObject *object)
{
	GstHcvKitten *self = HCV_KITTEN(object);
	if (self->priv->image != NULL)
	{
		cairo_surface_destroy (self->priv->image);
		self->priv->image = NULL;
	}
	if (self->priv->scaled_context != NULL)
	{
		cairo_destroy (self->priv->scaled_context);
		self->priv->scaled_context = NULL;
	}
	if (self->priv->surface != NULL)
	{
		cairo_surface_destroy (self->priv->surface);
		self->priv->surface = NULL;
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
	gobject_class->dispose = gst_hcv_kitten_dispose;

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
gst_hcv_kitten_init (GstHcvKitten *trans, GstBaseTransformClass *klass G_GNUC_UNUSED)
{
	trans->priv = (GstHcvKittenPriv *) malloc (sizeof (GstHcvKittenPriv));
	trans->priv->left = 0;
	trans->priv->right = 0;
	trans->priv->top = 0;
	trans->priv->bottom = 0;
	trans->priv->image = cairo_image_surface_create_from_png ("img/kitten-0.6.png");
	trans->priv->kitten_width = cairo_image_surface_get_width (trans->priv->image);
	trans->priv->scaled_context = NULL;
	trans->priv->surface = NULL;
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
