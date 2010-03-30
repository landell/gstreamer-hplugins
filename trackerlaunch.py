# -*- coding: utf-8 -*-

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
