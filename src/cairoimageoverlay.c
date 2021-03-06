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

#include <cairo.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

GType hcv_image_overlay_get_type (void);

#define HCV_TYPE_IMAGE_OVERLAY (hcv_image_overlay_get_type ())
#define HCV_IMAGE_OVERLAY(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), HCV_TYPE_IMAGE_OVERLAY, HcvImageOverlay))

typedef struct _HcvImageOverlayPriv HcvImageOverlayPriv;
typedef struct _HcvImageOverlay HcvImageOverlay;

static GStaticMutex mutex = G_STATIC_MUTEX_INIT;

GST_DEBUG_CATEGORY (cairo_image_overlay_debug);

enum
{
	PROP_0,

	HCV_IMAGE_OVERLAY_X,
	HCV_IMAGE_OVERLAY_Y,
	HCV_IMAGE_OVERLAY_WIDTH,
	HCV_IMAGE_OVERLAY_HEIGHT,
	HCV_IMAGE_OVERLAY_IMAGE,
	HCV_IMAGE_OVERLAY_ALPHA
};

struct _HcvImageOverlayPriv
{
	int x;
	int y;
	int image_width;
	int image_height;
	int recreate;
	GString *img_path;
	float alpha_value;
	cairo_surface_t *image;
	cairo_t *scaled_context;
	cairo_surface_t *surface;
	float real_width;
	float real_height;
	cairo_format_t cairo_format;
	int buffer_width;
	int buffer_height;
};

struct _HcvImageOverlay
{
	GstBaseTransform parent_instance;

	HcvImageOverlayPriv *priv;
};

static void
hcv_image_overlay_create_surface (HcvImageOverlay *self,
		cairo_format_t cairo_format)
{
	float width_scale = 1;
	float height_scale = 1;

	self->priv->surface = cairo_image_surface_create ( cairo_format,
			self->priv->buffer_width, self->priv->buffer_height);
	self->priv->scaled_context = cairo_create (self->priv->surface);

	if (self->priv->image_width != -1)
		width_scale = (self->priv->image_width + 1) / self->priv->real_width;
	if (self->priv->image_height != -1)
		height_scale = (self->priv->image_height + 1) / self->priv->real_height;
	cairo_scale (self->priv->scaled_context, width_scale, height_scale);
	GST_DEBUG_OBJECT (self, "Status of cairo create surface: %s\n",
			cairo_status_to_string (cairo_status (self->priv->scaled_context)));
}

static gboolean
hcv_buffer_image_overlay (HcvImageOverlay *self, GstBuffer *gbuf)
{
	GstBuffer *nbuf = gst_buffer_ref (gbuf);
	cairo_t* cr;
	cairo_surface_t* surface;
	static int stride = -1;
	float alpha;

	stride = cairo_format_stride_for_width (self->priv->cairo_format,
			self->priv->buffer_width);

	if (self->priv->image_width > self->priv->buffer_width)
		self->priv->image_width = self->priv->buffer_width;
	if (self->priv->image_height > self->priv->buffer_height)
		self->priv->image_height = self->priv->buffer_height;
	g_static_mutex_lock(&mutex);
	if (self->priv->recreate)
	{
		GST_DEBUG_OBJECT (self, "Recreate flag was set\n");
		/*If there was a scaled context, destroy it to create a new one*/
		if (self->priv->scaled_context != NULL)
		{
			cairo_destroy (self->priv->scaled_context);
			if (self->priv->surface != NULL)
				cairo_surface_destroy (self->priv->surface);
		}
		hcv_image_overlay_create_surface (self, self->priv->cairo_format);

		self->priv->recreate = 0;
	}
	else
	{
		if (self->priv->scaled_context == NULL)
		{
			hcv_image_overlay_create_surface (self, self->priv->cairo_format);

			GST_DEBUG_OBJECT (self, "Cairo surface created\n");
		}
	}
	cairo_set_source_surface (self->priv->scaled_context,
			self->priv->image, 0, 0);
	GST_DEBUG_OBJECT (self, "Status of cairo_set_source_surface: %s\n",
			cairo_status_to_string (cairo_status (self->priv->scaled_context)));
	cairo_paint (self->priv->scaled_context);
	g_static_mutex_unlock(&mutex);

	surface = cairo_image_surface_create_for_data (GST_BUFFER_DATA (nbuf),
			self->priv->cairo_format,
			self->priv->buffer_width,
			self->priv->buffer_height,
			stride);
	cr = cairo_create (surface);
	GST_DEBUG_OBJECT (self, "X: %d, Y: %d, width: %d\n", self->priv->x,
			self->priv->y, self->priv->image_width);

	cairo_set_source_surface (cr, self->priv->surface, self->priv->x,
			self->priv->y);

	g_static_mutex_lock(&mutex);
	alpha = self->priv->alpha_value;
	g_static_mutex_unlock(&mutex);
	cairo_paint_with_alpha (cr, alpha);

	cairo_surface_destroy(surface);
	cairo_destroy (cr);
	gst_buffer_unref (nbuf);
	return TRUE;
}

static GstFlowReturn
hcv_image_overlay_transform_ip (GstBaseTransform *trans, GstBuffer *buf)
{
  gboolean res;
  res = hcv_buffer_image_overlay (HCV_IMAGE_OVERLAY (trans), buf);
  return GST_FLOW_OK;
}

static GstElementDetails image_overlay_details =
GST_ELEMENT_DETAILS ("HPlugins Cairo Image Overlay",
		"Filter/Editor/Video", "Put image in defined space",
		"Luciana Fujii Pontello <luciana@holoscopio.com>");

static GstStaticPadTemplate srctemplate =
GST_STATIC_PAD_TEMPLATE ("src", GST_PAD_SRC, GST_PAD_ALWAYS,
		GST_STATIC_CAPS (GST_VIDEO_CAPS_ARGB ";" GST_VIDEO_CAPS_BGRA));

static GstStaticPadTemplate sinktemplate =
GST_STATIC_PAD_TEMPLATE ("sink", GST_PAD_SINK, GST_PAD_ALWAYS,
		GST_STATIC_CAPS (GST_VIDEO_CAPS_ARGB ";" GST_VIDEO_CAPS_BGRA));

static void
hcv_image_overlay_define_image (HcvImageOverlay *self, GString *path)
{
	g_static_mutex_lock(&mutex);
	if (self->priv->image != NULL)
		cairo_surface_destroy(self->priv->image);
	if (path != NULL)
		self->priv->image = cairo_image_surface_create_from_png (path->str);
	self->priv->real_width = cairo_image_surface_get_width (self->priv->image);
	self->priv->real_height = cairo_image_surface_get_height (self->priv->image);
	self->priv->recreate = 1;
	g_static_mutex_unlock(&mutex);
}

static void
hcv_image_overlay_set_property (GObject      *object,
                             guint         property_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
	 HcvImageOverlay *self = HCV_IMAGE_OVERLAY (object);

	 switch (property_id)
	 {
		 case HCV_IMAGE_OVERLAY_X:
			 self->priv->x = g_value_get_int (value);
			 GST_DEBUG_OBJECT (object, "Property x set to %d\n", self->priv->x);
			 break;

		 case HCV_IMAGE_OVERLAY_Y:
			 self->priv->y = g_value_get_int (value);
			 GST_DEBUG_OBJECT (object, "Property y set to: %d\n",
					 self->priv->y);
			 break;

		 case HCV_IMAGE_OVERLAY_WIDTH:
			 g_static_mutex_lock(&mutex);
			 self->priv->image_width = g_value_get_int (value);
			 self->priv->recreate = 1;
			 g_static_mutex_unlock(&mutex);
			 GST_DEBUG_OBJECT (object, "Property image-width set to: %d\n",
					 self->priv->image_width);
			 break;

		 case HCV_IMAGE_OVERLAY_HEIGHT:
			 g_static_mutex_lock(&mutex);
			 self->priv->image_height = g_value_get_int (value);
			 self->priv->recreate = 1;
			 g_static_mutex_unlock(&mutex);
			 GST_DEBUG_OBJECT (object, "Property image-height set to: %d\n",
					 self->priv->image_height);
			 break;

		 case HCV_IMAGE_OVERLAY_IMAGE:
			 if (self->priv->img_path)
				 g_string_free(self->priv->img_path, TRUE);
			 self->priv->img_path = g_string_new (g_value_get_string (value));
			 hcv_image_overlay_define_image(self, self->priv->img_path);
			 GST_DEBUG_OBJECT (object, "Property location set to : %s\n",
					 self->priv->img_path->str);
			 break;

		 case HCV_IMAGE_OVERLAY_ALPHA:
			 g_static_mutex_lock(&mutex);
			 self->priv->alpha_value = g_value_get_float (value);
			 g_static_mutex_unlock(&mutex);
			 GST_DEBUG_OBJECT (object, "Property image-alpha set to: %f\n",
					 self->priv->alpha_value);
			 break;

		 default:
			 /* We don't have any other property... */
			 G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
			 break;
	 }
}

static void
hcv_image_overlay_dispose (GObject *object)
{
	HcvImageOverlay *self = HCV_IMAGE_OVERLAY(object);
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
	if (self->priv->img_path)
	{
		g_string_free(self->priv->img_path, TRUE);
		self->priv->img_path = NULL;
	}
	if (self->priv)
	{
		g_free(self->priv);
		self->priv = NULL;
	}
}

static void
hcv_image_overlay_get_property (GObject      *object,
                             guint         property_id,
                             GValue *value,
                             GParamSpec   *pspec)
{
	 HcvImageOverlay *self = HCV_IMAGE_OVERLAY (object);

	 switch (property_id)
	 {
		 case HCV_IMAGE_OVERLAY_X:
			 g_value_set_int (value, self->priv->x);
			 break;

		 case HCV_IMAGE_OVERLAY_Y:
			 g_value_set_int (value, self->priv->y);
			 break;

		 case HCV_IMAGE_OVERLAY_WIDTH:
			 g_value_set_int (value, self->priv->image_width);
			 break;

		 case HCV_IMAGE_OVERLAY_HEIGHT:
			 g_value_set_int (value, self->priv->image_height);
			 break;

		 case HCV_IMAGE_OVERLAY_ALPHA:
			 g_value_set_float (value, self->priv->alpha_value);
			 break;

		 default:
			 /* We don't have any other property... */
			 G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
			 break;
	 }
}

static gboolean
hcv_image_overlay_setcaps (GstPad *pad, GstCaps *caps)
{
	HcvImageOverlay *self = HCV_IMAGE_OVERLAY (GST_OBJECT_PARENT (pad));
	GstVideoFormat format;
	gint width;
	gint height;

	if (!gst_video_format_parse_caps (caps, &format, &width, &height))
	{
		GST_WARNING_OBJECT (self, "Could not parse caps");
		return FALSE;
	}
	self->priv->buffer_width = width;
	self->priv->buffer_height = height;

	if (format == GST_VIDEO_FORMAT_ARGB || format == GST_VIDEO_FORMAT_BGRA)
	{
		self->priv->cairo_format = CAIRO_FORMAT_ARGB32;
		GST_DEBUG_OBJECT (self, "Caps format set to ARGB32\n");
	}
	else
	{
		self->priv->cairo_format = CAIRO_FORMAT_RGB24;
		GST_DEBUG_OBJECT (self, "Caps format set to RGB24\n");
	}
	if (!gst_pad_set_caps (GST_BASE_TRANSFORM_SRC_PAD(self), caps))
		return FALSE;
	return TRUE;
}

static void
hcv_image_overlay_base_init (GstBaseTransformClass *klass)
{
  GstPadTemplate *src;
  GstPadTemplate *sink;
  src = gst_static_pad_template_get (&srctemplate);
  sink = gst_static_pad_template_get (&sinktemplate);
  gst_element_class_add_pad_template (GST_ELEMENT_CLASS (klass), src);
  gst_element_class_add_pad_template (GST_ELEMENT_CLASS (klass), sink);
}

static void
hcv_image_overlay_constructed (GObject *object)
{
	HcvImageOverlay *self = HCV_IMAGE_OVERLAY (object);
	self->priv->image = NULL;
	self->priv->recreate = 0;
}

static void
hcv_image_overlay_class_init (GstBaseTransformClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	GParamSpec *pspec;

	klass->transform_ip = GST_DEBUG_FUNCPTR(hcv_image_overlay_transform_ip);
	gst_element_class_set_details (GST_ELEMENT_CLASS (klass),
			&image_overlay_details);

	gobject_class->set_property = hcv_image_overlay_set_property;
	gobject_class->get_property = hcv_image_overlay_get_property;
	gobject_class->dispose = hcv_image_overlay_dispose;
	gobject_class->constructed = hcv_image_overlay_constructed;

	pspec = g_param_spec_int ("x",
			"X coordinate for window",
			"Set/Get x coordinate",
			0  /* minimum value */,
			G_MAXINT /* maximum value */,
			2  /* default value */,
			G_PARAM_READWRITE);
	g_object_class_install_property (gobject_class,
			HCV_IMAGE_OVERLAY_X, pspec);
	pspec = g_param_spec_int ("y",
			"Y coordinate for window",
			"Set/Get y coordinate",
			0  /* minimum value */,
			G_MAXINT /* maximum value */,
			2  /* default value */,
			G_PARAM_READWRITE);
	g_object_class_install_property (gobject_class,
			HCV_IMAGE_OVERLAY_Y, pspec);
	pspec = g_param_spec_int ("image_width",
			"Width to be set for image",
			"Set/Get image width. Value -1 keeps the original size.",
			-1  /* minimum value */,
			G_MAXINT /* maximum value */,
			-1  /* default value */,
			G_PARAM_READWRITE);
	g_object_class_install_property (gobject_class,
			HCV_IMAGE_OVERLAY_WIDTH, pspec);
	pspec = g_param_spec_int ("image_height",
			"Height to be set for image",
			"Set/Get image height. Value -1 keeps the original size.",
			-1  /* minimum value */,
			G_MAXINT /* maximum value */,
			-1  /* default value */,
			G_PARAM_READWRITE);
	g_object_class_install_property (gobject_class,
			HCV_IMAGE_OVERLAY_HEIGHT, pspec);
	pspec = g_param_spec_string ("location",
			"png image to be used",
			"Set png image filename",
			NULL,  /* default value */
			G_PARAM_WRITABLE);
	g_object_class_install_property (gobject_class,
			HCV_IMAGE_OVERLAY_IMAGE,
			pspec);
	pspec = g_param_spec_float ("image_alpha",
			"Alpha value for image",
			"Set alpha to be paint",
			0 /* minimum value */,
			1 /* maximum value */,
			1,  /* default value */
			G_PARAM_READWRITE);
	g_object_class_install_property (gobject_class,
			HCV_IMAGE_OVERLAY_ALPHA,
			pspec);
}

static void
hcv_image_overlay_init (HcvImageOverlay *trans,
		GstBaseTransformClass *klass G_GNUC_UNUSED)
{
	GstPad *sink_pad;

	GST_DEBUG_OBJECT(trans, "INIT\n");
	trans->priv = (HcvImageOverlayPriv *) malloc (sizeof (HcvImageOverlayPriv));
	trans->priv->x = 0;
	trans->priv->y = 0;
	trans->priv->image_width = -1;
	trans->priv->image_height = -1;
	trans->priv->real_width = 0;
	trans->priv->scaled_context = NULL;
	trans->priv->surface = NULL;
	trans->priv->alpha_value = 1;
	trans->priv->image = NULL;
	trans->priv->img_path = NULL;

	GST_DEBUG_CATEGORY_INIT (cairo_image_overlay_debug, "cairoimageoverlay", 0,
			"cairoimageoverlay element");

	sink_pad = GST_BASE_TRANSFORM_SINK_PAD(trans);
	gst_pad_set_setcaps_function (sink_pad, hcv_image_overlay_setcaps);
}

GType
hcv_image_overlay_get_type (void)
{
  static GType type = 0;
  if (type == 0)
    {
      static const GTypeInfo info = {
        sizeof (GstBaseTransformClass),
        (GBaseInitFunc) hcv_image_overlay_base_init,
        NULL,
        (GClassInitFunc) hcv_image_overlay_class_init,
        NULL,
        NULL,
        sizeof (GstBaseTransform),
        0,
        (GInstanceInitFunc) hcv_image_overlay_init,
				NULL
      };
      type = g_type_register_static (GST_TYPE_BASE_TRANSFORM,
                                     "HPluginsCairoImageOverlayType", &info, 0);
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
