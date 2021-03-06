// Copyright 2003, 2004 David Hilvert <dhilvert@auricle.dyndns.org>,
//                                    <dhilvert@ugcs.caltech.edu>

/*  This file is part of the Anti-Lamenessing Engine.

    The Anti-Lamenessing Engine is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    The Anti-Lamenessing Engine is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with the Anti-Lamenessing Engine; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __psf_circle_h__
#define __psf_circle_h__

#include "../../point.h"
#include "psf.h"

/*
 * Point-spread function module.
 *
 * This module implements a circular filter.
 */

class circle : public psf {
	ale_real _radius;
public:

	/*
	 * The following four functions indicate filter boundaries.  Filter
	 * support may include everything up to and including the boundaries
	 * specified here.
	 */
	ale_real min_i() const { return -_radius; }
	ale_real max_i() const { return  _radius; }
	ale_real min_j() const { return -_radius; }
	ale_real max_j() const { return  _radius; }

	/*
	 * Response function
	 *
	 * Get the response to the rectangle bounded by (top, bot, lef, rig).
	 * This function must correctly handle points which fall outside of the
	 * filter support.  The variety of the responding pixel is provided, in
	 * case response is not uniform for all pixels (e.g. some sensor arrays
	 * stagger red, green, and blue sensors).
	 */
	psf_result operator()(ale_real top, ale_real bot, ale_real lef, ale_real rig, 
			unsigned int variety) const {

		psf_result result;

		for (int k = 0; k < 3; k++)
			result.matrix(k, k) = 0;

		ale_real total = (bot - top) * (rig - lef) / (M_PI * _radius * _radius);

		for (int i = 0; i < 10; i++)
		for (int j = 0; j < 10; j++) {
			ale_real one_half = 1 / (ale_real) 2;
			ale_real r = pow(top + (bot - top) * ((i + one_half) / (ale_real) 10), 2)
				   + pow(lef + (rig - lef) * ((j + one_half) / (ale_real) 10), 2);
			if (r < _radius * _radius)
				for (int k = 0; k < 3; k++)
					result.matrix(k, k) += (total / (ale_real) 100);
		}


		return result;
	}

	circle(ale_real radius) {
		_radius = radius;
	}
};

#endif
