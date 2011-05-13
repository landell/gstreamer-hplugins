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

#ifndef HC_LIVE_KEEPER_H
#define HC_LIVE_KEEPER_H

#include <gst/gst.h>

#define HC_TYPE_LIVE_KEEPER (hc_live_keeper_get_type ())
#define HC_LIVE_KEEPER(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), HC_TYPE_LIVE_KEEPER, HcLiveKeeper))
#define HC_IS_LIVE_KEEPER(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HC_TYPE_LIVE_KEEPER))
#define HC_LIVE_KEEPER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), HC_TYPE_LIVE_KEEPER, HcLiveKeeperClass))
#define HC_IS_LIVE_KEEPER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), HC_TYPE_LIVE_KEEPER))
#define HC_LIVE_KEEPER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), HC_TYPE_LIVE_KEEPER, HcLiveKeeperClass))

typedef struct _HcLiveKeeper HcLiveKeeper;
typedef struct _HcLiveKeeperClass HcLiveKeeperClass;

struct _HcLiveKeeper
{
  GstElement parent;
};

struct _HcLiveKeeperClass
{
  GstElementClass parent;
};

GType hc_live_keeper_get_type (void);

#endif
