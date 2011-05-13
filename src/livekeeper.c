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

static void hc_live_keeper_init (HcLiveKeeper *);
static void hc_live_keeper_class_init (HcLiveKeeperClass *);

G_DEFINE_TYPE (HcLiveKeeper, hc_live_keeper, GST_TYPE_ELEMENT);

static void
hc_live_keeper_init (HcLiveKeeper *keeper)
{
}

static void
hc_live_keeper_class_init (HcLiveKeeperClass *kclass)
{
  GObjectClass *oclass = G_OBJECT_CLASS (kclass);
}
