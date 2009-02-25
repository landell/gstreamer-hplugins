#!/usr/bin/octave -q

# Copyright (C) 2009 Samuel R. C. Vale <srcvale@holoscopio.com>
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



filename = "skinmodel.h";
columns = 15;

# Usage
function skinmodel_usage
	printf ("\nEnter a directory with some input images. Use:");
	printf ("\n$> skinmodel.m /home/user/directory-to-images\n");
end

# RGB -> YCbCr
function [Cb Cr] = skinmodel_rgb2ycbcr (image)
	R = real (image(:,:,1));
	G = real (image(:,:,2));
	B = real (image(:,:,3));
	# Y = is not necessary
	Cb =  - (0.168736 .* R) - (0.331264 .* G) + (0.5      .* B);
	Cr =    (0.5      .* R) - (0.418688 .* G) - (0.081312 .* B);
end

# Limits -> features
function [C_mean C_var] = skinmodel_limits (Ci, Cs)
	C_mean = (Ci + Cs) / 2;
	C_var = (C_mean - Ci) / 2;
end



printf ("\nV4l2capture Training Script - GNU Octave %s\n\n", version);

# Take and test command line arguments
args = argv ();

if (nargin == 0)
	skinmodel_usage();
	return;
end

# List input files for training
inputdir = args{1};
inputfile = ls ([inputdir, "/*.png"]);
[nfiles ncols] = size (inputfile);

# Skin color limits, in YCbCr color space. Take care, this parameters are
# influenced by saturated illumination and sensor response.
Cb_i = Cb_s = 128;
Cr_i = Cr_s = 128;
for i = 1:nfiles
	file = inputfile(i, :);
	printf ("%s\n", file);
	img = imread (deblank (file));
	[Cb Cr] = skinmodel_rgb2ycbcr (img);
	Cb_i = min ([(128 + min (min (Cb))), Cb_i]);
	Cb_s = max ([(128 + max (max (Cb))), Cb_s]);
	Cr_i = min ([(128 + min (min (Cr))), Cr_i]);
	Cr_s = max ([(128 + max (max (Cr))), Cr_s]);
end

[Cb_mean Cb_var] = skinmodel_limits (Cb_i, Cb_s);
[Cr_mean Cr_var] = skinmodel_limits (Cr_i, Cr_s);

# Skin color distributions
# TODO: transform this in a bivariate normal distribution.
Cb_dist = normpdf ([0:255], Cb_mean, Cb_var);
Cr_dist = normpdf ([0:255], Cr_mean, Cr_var);

model = Cb_dist' * Cr_dist;
model = uint8 (model .* 255 ./ max (max (model)));
[h w] = size (model);
model = model';

printf ("Generating model...");

fd = fopen (filename, "w", "native");

header = ["/*", \
"\n Copyright (C) 2009 Samuel R. C. Vale <srcvale@holoscopio.com>",\
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

fprintf (fd, "%s", header);
fprintf (fd, "\n#define SKIN_MODEL_H %d", h);
fprintf (fd, "\n#define SKIN_MODEL_W %d", w);
fprintf (fd, "\n\nstatic unsigned char skin_model");
fprintf (fd, "[SKIN_MODEL_H * SKIN_MODEL_W] = {", h, w);

fprintf (fd, "\n   %d,", model(1));
for i = 2:((h * w) - 1)
	fprintf (fd, " %d,", model(i));
	if (mod (i, columns) == 0)
		fprintf (fd, "\n  ");
	end
end

fprintf (fd, " %d\n};", model(h * w));

fprintf (fd , "\n\n#endif\n");
fclose (fd);

printf ("\n%s saved.\n", filename);
