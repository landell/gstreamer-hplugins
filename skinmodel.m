#!/usr/bin/octave -qf

# Copyright (C) 2009 Samuel Ribeiro da Costa Vale
# <srcvale@holoscopio.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
# 02110-1301 USA.

filename = 'skinmodel.h';
columns = 15;

# Skin color limits, in UV color space. Take care, this parameters are
# influenced by illumination and sensor response.
# TODO: take this parameters from sample images.
ui = 114;
us = 125;
vi = 118;
vs = 125;

u_mean = (ui + us) / 2;
u_var = (u_mean - ui) / 2;
v_mean = (vi + vs) / 2;
v_var = (v_mean - vi) / 2;

# Skin color distributions
# TODO: transform this in a bivariate normal distribution.
u_dist = normpdf ([0:255], u_mean, u_var);
v_dist = normpdf ([0:255], v_mean, v_var);

model = u_dist' * v_dist;
model = uint8 (model .* 255 ./ max (max (model)));
[h w] = size (model);

fd = fopen (filename, 'w', 'native');

header = ["/*", \
"\n Copyright (C) 2009 Samuel Ribeiro da Costa Vale", \
"\n <srcvale@holoscopio.com>",\
"\n Skin color table - Generated automatically by skinmodel.m\n", \
"\n This program is free software; you can redistribute it and/or modify", \
"\n it under the terms of the GNU General Public License as published by", \
"\n the Free Software Foundation; either version 2 of the License, or", \
"\n (at your option) any later version.", \
"\n", \
"\n This program is distributed in the hope that it will be useful,", \
"\n but WITHOUT ANY WARRANTY; without even the implied warranty of", \
"\n MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the", \
"\n GNU General Public License for more details.", \
"\n", \
"\n You should have received a copy of the GNU General Public License", \
"\n along with this program; if not, write to the Free Software", \
"\n Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA", \
"\n 02110-1301 USA.", \
"\n*/", \
"\n\n#ifndef _SKIN_MODEL_H_", \
"\n#define _SKIN_MODEL_H_", \
"\n"];

fprintf (fd, '%s', header);
fprintf (fd, '\n#define SKIN_MODEL_H %d', h);
fprintf (fd, '\n#define SKIN_MODEL_W %d', w);
fprintf (fd, '\n\nstatic unsigned char skin_model');
fprintf (fd, '[SKIN_MODEL_H * SKIN_MODEL_W] = {', h, w);

fprintf (fd, '\n   %d,', model(1));
for i = 2:((h * w) - 1)
	fprintf (fd, ' %d,', model(i));
	if (mod (i, columns) == 0)
		fprintf (fd, '\n  ');
	end
end

fprintf (fd, ' %d\n};', model(h * w));

fprintf (fd , '\n\n#endif\n');
fclose (fd);
