// Copyright 2006 David Hilvert <dhilvert@auricle.dyndns.org>,
//                              <dhilvert@ugcs.caltech.edu>

/*  This file is part of the Anti-Lamenessing Engine.

    The Anti-Lamenessing Engine is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    The Anti-Lamenessing Engine is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with the Anti-Lamenessing Engine; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/*
 * d3/focus.h: Implementation of defocusing logic.
 */

#ifndef __focus_h__
#define __focus_h__

class focus {
	struct entry {
		double start_depth;
		double end_depth;
		double focal_distance;
		double confusion_diameter;
		double dof_expansion;
		double vertical_gradient;
		double horizontal_gradient;
	};

	static unsigned int camera_index;
	static std::vector<std::vector<entry> > focus_list;

public:
	static void add_region(unsigned int ci, double sd, double ed, double fd, 
			double cc, double df, double vt, double ht) {

		if (focus_list.size() <= ci)
			focus_list.resize(ci + 1);

		entry e = { sd, ed, fd, cc, df, vt, ht };
		
		focus_list[ci].push_back(e);
	}

	static d2::image *defocus(const d2::image *defocus_prior, const d2::image *depth) {
		assert(0);

		return NULL;
	}

	static void set_camera(unsigned int c) {
		camera_index = c;
	}
};

#endif