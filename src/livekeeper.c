/*
 *  Copyright (C) 2011  Thadeu Lima de Souza Cascardo <cascardo@holoscopio.com>
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

#include "livekeeper.h"
#include <gst/gst.h>

#define HC_LIVE_KEEPER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), HC_TYPE_LIVE_KEEPER, HcLiveKeeperPrivate))

typedef struct _HcLiveKeeperPrivate HcLiveKeeperPrivate;

struct _HcLiveKeeperPrivate
{
  int timeout;
};

static GObjectClass *parent_class;
static void hc_live_keeper_init (HcLiveKeeper *, HcLiveKeeperClass *);
static void hc_live_keeper_finalize (HcLiveKeeper *);
static void hc_live_keeper_class_init (HcLiveKeeperClass *);
static void hc_live_keeper_base_init (HcLiveKeeperClass *);

GType
hc_live_keeper_get_type (void)
{
  static GType type = 0;
  if (type == 0)
    {
      const GTypeInfo info =
        {
          sizeof (HcLiveKeeperClass),
          (GBaseInitFunc) hc_live_keeper_base_init,
          NULL, /* hc_live_keeper_base_finalize, */
          (GClassInitFunc) hc_live_keeper_class_init,
          NULL, /* hc_live_keeper_class_finalize, */
          NULL, /* class_data */
          sizeof (HcLiveKeeper),
          0,
          (GInstanceInitFunc) hc_live_keeper_init,
        };
      type = g_type_register_static (GST_TYPE_ELEMENT,
                                     "HcLiveKeeper",
                                     &info, 0);
    }
  return type;
}

static void
hc_live_keeper_loop (GstPad *pad)
{
  HcLiveKeeper *keeper = HC_LIVE_KEEPER (GST_OBJECT_PARENT (pad));
  GstBuffer *buf = NULL;
  GTimeVal tv;
  if (keeper->lastbuf)
    {
      gint64 timeout;
      g_get_current_time (&tv);
      timeout = GST_BUFFER_DURATION (keeper->lastbuf) / GST_USECOND;
      g_time_val_add (&tv, timeout + 1);
      buf = g_async_queue_timed_pop (keeper->queue, &tv);
    }
  else
    {
      buf = g_async_queue_pop (keeper->queue);
    }
  if (buf)
    {
      gst_buffer_replace (&keeper->lastbuf, buf);
      gst_pad_push (keeper->srcpad, buf);
    }
  else if (keeper->lastbuf)
    {
      buf = gst_buffer_ref (keeper->lastbuf);
      buf = gst_buffer_make_metadata_writable (buf);
      GST_BUFFER_TIMESTAMP (buf) += GST_BUFFER_DURATION (buf);
      gst_buffer_replace (&keeper->lastbuf, buf);
      gst_pad_push (keeper->srcpad, buf);
    }
}

static GstFlowReturn
hc_live_keeper_chain (GstPad *pad, GstBuffer *buf)
{
  HcLiveKeeper *keeper = HC_LIVE_KEEPER (GST_OBJECT_PARENT (pad));
  g_async_queue_push (keeper->queue, buf);
}

static gboolean
hc_live_keeper_event (GstPad *pad, GstEvent *event)
{
  HcLiveKeeper *keeper = HC_LIVE_KEEPER (GST_OBJECT_PARENT (pad));
  switch (GST_EVENT_TYPE (event))
    {
      case GST_EVENT_EOS:
        gst_event_unref (event);
        return TRUE;
      default:
        return gst_pad_push_event (keeper->srcpad, event);
    }
}

static gboolean
hc_live_keeper_src_activate_push (GstPad *pad, gboolean active)
{
  if (active)
    {
      return gst_pad_start_task (pad,
                                 (GstTaskFunction) hc_live_keeper_loop,
                                 pad);
    }
  else
    {
      return gst_pad_stop_task (pad);
    }
}

static void
hc_live_keeper_init (HcLiveKeeper *keeper, HcLiveKeeperClass *kclass)
{

  GstElementClass *eclass = GST_ELEMENT_CLASS (kclass);
  GstPadTemplate *src_tmpl;
  GstPadTemplate *sink_tmpl;

  keeper->lastbuf = NULL;
  keeper->queue = g_async_queue_new_full ((GDestroyNotify) gst_buffer_unref);

  src_tmpl = gst_element_class_get_pad_template (eclass, "src");
  sink_tmpl = gst_element_class_get_pad_template (eclass, "sink");
  keeper->srcpad = gst_pad_new_from_template (src_tmpl, "src");
  keeper->sinkpad = gst_pad_new_from_template (sink_tmpl, "sink");

  gst_pad_set_activatepush_function (keeper->srcpad,
                                     hc_live_keeper_src_activate_push);
  gst_pad_set_chain_function (keeper->sinkpad, hc_live_keeper_chain);
  gst_pad_set_event_function (keeper->sinkpad, hc_live_keeper_event);

  gst_element_add_pad (GST_ELEMENT (keeper), keeper->srcpad);
  gst_element_add_pad (GST_ELEMENT (keeper), keeper->sinkpad);

}

static void
hc_live_keeper_finalize (HcLiveKeeper *keeper)
{
  if (keeper->lastbuf)
    gst_buffer_unref (keeper->lastbuf);
  g_async_queue_unref (keeper->queue);
  G_OBJECT_CLASS (parent_class)->finalize (G_OBJECT (keeper));
}

static void
hc_live_keeper_class_init (HcLiveKeeperClass *kclass)
{
  GObjectClass *oclass = G_OBJECT_CLASS (kclass);
  parent_class = g_type_class_peek_parent (kclass);
  oclass->finalize = (GObjectFinalizeFunc) hc_live_keeper_finalize;
}

static GstStaticPadTemplate sink_template =
GST_STATIC_PAD_TEMPLATE (
  "sink",
  GST_PAD_SINK,
  GST_PAD_ALWAYS,
  GST_STATIC_CAPS ("ANY")
);

static GstStaticPadTemplate src_template =
GST_STATIC_PAD_TEMPLATE (
  "src",
  GST_PAD_SRC,
  GST_PAD_ALWAYS,
  GST_STATIC_CAPS ("ANY")
);

static void
hc_live_keeper_base_init (HcLiveKeeperClass *kclass)
{
  GstElementClass *eclass = GST_ELEMENT_CLASS (kclass);
  gst_element_class_set_details_simple (eclass, "livekeeper",
        "Live Keeper", "keep streams live",
        "Thadeu Lima de Souza Cascardo <cascardo@holoscopio.com>");
  gst_element_class_add_pad_template (eclass,
    gst_static_pad_template_get (&src_template));
  gst_element_class_add_pad_template (eclass,
    gst_static_pad_template_get (&sink_template));
}
