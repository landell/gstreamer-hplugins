# -*- coding: utf-8 -*-
#
#  Copyright (C) 2010 Holoscopio Tecnologia
#  Author: Luciana Fujii Pontello <luciana@holoscopio.com>
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License along
#  with this program; if not, write to the Free Software Foundation, Inc.,
#  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

import gobject
import pygst
pygst.require("0.10")
import gst

class KittenSecrecy():

	def __init__ (self):

		self.player = gst.Pipeline("player")
		source = gst.element_factory_make("v4l2src", "source")
		decoder = gst.element_factory_make("ycbcrenc", "decoder")
		conv = gst.element_factory_make("facetracker", "converter")
		encoder = gst.element_factory_make("ycbcrdec", "encoder")
		colorspace = gst.element_factory_make("ffmpegcolorspace", "colorspace")
		self.kitten = gst.element_factory_make("kitten", "kitten")
		colorspace2 = gst.element_factory_make("ffmpegcolorspace", "colorspace2")
		sink = gst.element_factory_make("xvimagesink", "sink")
		self.player.add(source, decoder, conv, encoder, colorspace, self.kitten, colorspace2, sink)
		gst.element_link_many(source, decoder, conv, encoder, colorspace, self.kitten, colorspace2, sink)

		self.player.set_state(gst.STATE_PLAYING)

		bus = self.player.get_bus()
		bus.add_signal_watch()
		bus.connect("message", self.on_message)

	def on_message(self, bus, message):
		t = message.type
		if t == gst.MESSAGE_EOS:
			self.player.set_state(gst.STATE_NULL)
		elif t == gst.MESSAGE_ELEMENT:
			print "recebi crop window"
			structure = message.structure;
			if structure.get_name() != "crop_window_t":
				return;
			self.kitten.set_property("window_left",structure["left"]);
			self.kitten.set_property("window_right",structure["right"]);
			self.kitten.set_property("window_top",structure["top"]);
			self.kitten.set_property("window_bottom",structure["bottom"]);
		elif t == gst.MESSAGE_ERROR:
			self.player.set_state(gst.STATE_NULL)
			self.loop.quit()

	def run(self):
		self.loop = gobject.MainLoop()
		self.loop.run()
