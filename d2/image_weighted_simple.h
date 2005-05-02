// Copyright 2002, 2003, 2004 David Hilvert <dhilvert@auricle.dyndns.org>,
//                                          <dhilvert@ugcs.caltech.edu>

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
 * image_weighted_simple.h: Image representing a weighted average of inputs.
 */

#ifndef __image_weighted_simple_h__
#define __image_weighted_simple_h__

#include "image_ale_real.h"
#include "exposure/exposure.h"
#include "point.h"
#include "image.h"

class image_weighted_simple : public image {
private:
	void trigger(pixel multiplier) {
		assert(0);
	}

public:
	image_weighted_simple (unsigned int dimy, unsigned int dimx, unsigned int
			depth, char *name = "anonymous", exposure *exp = NULL) 
			: image(dimy, dimx, depth, name, exp) {
	}

	virtual ~image_weighted_simple() {
	}

	spixel &pix(unsigned int y, unsigned int x) {
		assert(0);
		static spixel foo;
		return foo;
	}

	const spixel &pix(unsigned int y, unsigned int x) const {
		assert(0);
		static spixel foo;
		return foo;
	}

	ale_real &chan(unsigned int y, unsigned int x, unsigned int k) {
		assert(0);
		static ale_real foo;
		return foo;
	}

	const ale_real &chan(unsigned int y, unsigned int x, unsigned int k) const {
		assert(0);
		static ale_real foo;
		return foo;
	}

	/*
	 * Make a new image suitable for receiving scaled values.
	 */
	virtual image *scale_generator(int height, int width, int depth, char *name) const {
		return new image_ale_real(height, width, depth, name, _exp);
	}

	/*
	 * Extend the image area to the top, bottom, left, and right,
	 * initializing the new image areas with black pixels.  Negative values
	 * shrink the image.
	 */
	void extend(int top, int bottom, int left, int right) {

		image_weighted_simple *is = new image_weighted_simple (
			height() + top  + bottom,
			 width() + left + right , depth(), name, _exp);

		assert(is);

		unsigned int min_i = (-top > 0)
			      ? -top
			      : 0;

		unsigned int min_j = (-left > 0)
			      ? -left
			      : 0;

		unsigned int max_i = (height() < is->height() - top)
			      ? height()
			      : is->height() - top;

		unsigned int max_j = (width() < is->width() - left)
			      ? width()
			      : is->width() - left;

		for (unsigned int i = min_i; i < max_i; i++)
		for (unsigned int j = min_j; j < max_j; j++) 
			is->pix(i + top, j + left)
				= get_pixel(i, j);

		delete[] _p;

		_p = is->_p;
		_dimx = is->_dimx;
		_dimy = is->_dimy;
		_depth = is->_depth;
		_offset[0] -= top;
		_offset[1] -= left;

		image_updated();

		is->_p = NULL;

		delete is;
	}

};

#endif
