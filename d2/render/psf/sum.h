// Copyright 2003 David Hilvert <dhilvert@auricle.dyndns.org>,
//                              <dhilvert@ugcs.caltech.edu>

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

#ifndef __psf_sum_h__
#define __psf_sum_h__

#include "../../point.h"
#include "psf.h"

/*
 * Point-spread function module.
 *
 * This module implements the sum (f1 + f2) of point-spread functions f1 and
 * f2.  This is not a convolution.
 */

class sum : public psf {
	ale_pos _radius;
	psf *f1, *f2;
	ale_real _min_i, _max_i, _min_j, _max_j;

public:
	/*
	 * The following four functions indicate filter boundaries.  Filter
	 * support may include everything up to and including the boundaries
	 * specified here.
	 */
	ale_real min_i() const { return _min_i; }
	ale_real max_i() const { return _max_i; }
	ale_real min_j() const { return _min_j; }
	ale_real max_j() const { return _max_j; }

	/*
	 * Get the number of varieties supported by this PSF.  These usually
	 * correspond to different points in the sensor array.
	 */
	virtual unsigned int varieties() {
		return f1->varieties() * f2->varieties();
	}

	/*
	 * Select the variety appropriate for a given position in the sensor
	 * array.
	 */
	virtual unsigned int select(unsigned int i, unsigned int j) {
		return (f1->select(i, j) * f2->varieties() + f2->select(i, j));
	}

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
		psf_result r1, r2;

		r1 = (*f1)(top, bot, lef, rig, variety / f2->varieties());
		r2 = (*f2)(top, bot, lef, rig, variety % f2->varieties());

		for (int k1 = 0; k1 < 3; k1++)
		for (int k2 = 0; k2 < 3; k2++)
			result.set_matrix(k1, k2, r1.get_matrix(k1, k2) 
				                + r2.get_matrix(k1, k2));

		return result;
	}

#if 0
	/*
	 * Get the average pixel response.
	 */
	psf_result operator()(ale_real top, ale_real bot, ale_real lef, ale_real rig) {
		psf_result result;
		psf_result r1, r2;

		r1 = (*f1)(top, bot, lef, rig);
		r2 = (*f2)(top, bot, lef, rig);

		for (int k1 = 0; k1 < 3; k1++)
		for (int k2 = 0; k2 < 3; k2++)
			result.set_matrix(k1, k2, r1.get_matrix(k1, k2) 
				                + r2.get_matrix(k1, k2));

		return result;
	}
#endif

	sum(psf *f1, psf *f2) {
		this->f1 = f1;
		this->f2 = f2;

		_min_i = f1->min_i();
		_min_j = f1->min_j();
		_max_i = f1->max_i();
		_max_j = f1->max_j();

		if (_min_i > f2->min_i())
			_min_i = f2->min_i();
		if (_min_j > f2->min_j())
			_min_j = f2->min_j();
		if (_max_i < f2->max_i())
			_max_i = f2->max_i();
		if (_max_j < f2->max_j())
			_max_j = f2->max_j();
	}
};

#endif
