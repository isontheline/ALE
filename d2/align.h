// Copyright 2002, 2004, 2007 David Hilvert <dhilvert@auricle.dyndns.org>,
//                                          <dhilvert@ugcs.caltech.edu>

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

/*
 * align.h: Handle alignment of frames.
 */

#ifndef __d2align_h__
#define __d2align_h__

#include "filter.h"
#include "transformation.h"
#include "image.h"
#include "point.h"
#include "render.h"
#include "tfile.h"
#include "image_rw.h"

class align {
private:

	/*
	 * Private data members
	 */

	static ale_pos scale_factor;

	/*
	 * Original frame transformation
	 */
	static transformation orig_t;

	/*
	 * Keep data older than latest
	 */
	static int _keep;
	static transformation *kept_t;
	static int *kept_ok;

	/*
	 * Transformation file handlers
	 */

	static tload_t *tload;
	static tsave_t *tsave;

	/*
	 * Control point variables
	 */

	static const point **cp_array;
	static unsigned int cp_count;

	/*
	 * Reference rendering to align against
	 */

	static render *reference;
	static filter::scaled_filter *interpolant;
	static ale_image reference_image;

 	/*
	 * Per-pixel alignment weight map
	 */

	static ale_image weight_map;

	/*
	 * Frequency-dependent alignment weights
	 */

	static double horiz_freq_cut;
	static double vert_freq_cut;
	static double avg_freq_cut;
	static const char *fw_output;

	/*
	 * Algorithmic alignment weighting
	 */

	static const char *wmx_exec;
	static const char *wmx_file;
	static const char *wmx_defs;

	/*
	 * Non-certainty alignment weights
	 */

	static ale_image alignment_weights;

	/*
	 * Latest transformation.
	 */

	static transformation latest_t;

	/*
	 * Flag indicating whether the latest transformation 
	 * resulted in a match.
	 */

	static int latest_ok;

	/*
	 * Frame number most recently aligned.
	 */

	static int latest;

	/*
	 * Exposure registration
	 *
	 * 0. Preserve the original exposure of images.
	 *
	 * 1. Match exposure between images.
	 *
	 * 2. Use only image metadata for registering exposure.
	 */

	static int _exp_register;

	/*
	 * Alignment class.
	 *
	 * 0. Translation only.  Only adjust the x and y position of images.
	 * Do not rotate input images or perform projective transformations.
	 * 
	 * 1. Euclidean transformations only.  Adjust the x and y position
	 * of images and the orientation of the image about the image center.
	 *
	 * 2. Perform general projective transformations.  See the file gpt.h
	 * for more information about general projective transformations.
	 */

	static int alignment_class;

	/*
	 * Default initial alignment type.
	 *
	 * 0. Identity transformation.
	 *
	 * 1. Most recently accepted frame's final transformation.
	 */

	static int default_initial_alignment_type;

	/*
	 * Projective group behavior
	 *
	 * 0. Perturb in output coordinates.
	 *
	 * 1. Perturb in source coordinates
	 */

	static int perturb_type;

	/*
	 * Alignment state
	 *
	 * This structure contains alignment state information.  The change
	 * between the non-default old initial alignment and old final
	 * alignment is used to adjust the non-default current initial
	 * alignment.  If either the old or new initial alignment is a default
	 * alignment, the old --follow semantics are preserved.
	 */

	class astate_t {
		transformation old_initial_alignment;
		transformation old_final_alignment;
		transformation default_initial_alignment;
		int old_is_default;
		std::vector<int> is_default;
		const image *input_frame;

	public:
		astate_t() : 
				old_initial_alignment(transformation::eu_identity()),
				old_final_alignment(transformation::eu_identity()),
				default_initial_alignment(transformation::eu_identity()),
				is_default(1) {

			input_frame = NULL;
			is_default[0] = 1;
			old_is_default = 1;
		}

		const image *get_input_frame() const {
			return input_frame;
		}

		void set_is_default(unsigned int index, int value) {

			/*
			 * Expand the array, if necessary.
			 */
			if (index == is_default.size());
				is_default.resize(index + 1);

			assert (index < is_default.size());
			is_default[index] = value;
		}

		int get_is_default(unsigned int index) {
			assert (index < is_default.size());
			return is_default[index];
		}

		transformation get_default() {
			return default_initial_alignment;
		}

		void set_default(transformation t) {
			default_initial_alignment = t;
		}
		
		void default_set_original_bounds(const image *i) {
			default_initial_alignment.set_original_bounds(i);
		}

		void set_final(transformation t) {
			old_final_alignment = t;
		}

		void set_input_frame(const image *i) {
			input_frame = i;
		}

		/*
		 * Implement new delta --follow semantics.
		 *
		 * If we have a transformation T such that
		 *
		 * 	prev_final == T(prev_init)
		 *
		 * Then we also have
		 *
		 * 	current_init_follow == T(current_init)
		 *
		 * We can calculate T as follows:
		 *
		 * 	T == prev_final(prev_init^-1)
		 *
		 * Where ^-1 is the inverse operator.
		 */
		static trans_single follow(trans_single a, trans_single b, trans_single c) {
			trans_single cc = c;

			if (alignment_class == 0) {
				/*
				 * Translational transformations
				 */
	
				ale_pos t0 = -a.eu_get(0) +  b.eu_get(0);
				ale_pos t1 = -a.eu_get(1) +  b.eu_get(1);

				cc.eu_modify(0, t0);
				cc.eu_modify(1, t1);

			} else if (alignment_class == 1) {
				/*
				 * Euclidean transformations
				 */

				ale_pos t2 = -a.eu_get(2) +  b.eu_get(2);

				cc.eu_modify(2, t2);

				point p( c.scaled_height()/2 + c.eu_get(0) - a.eu_get(0),
					 c.scaled_width()/2 + c.eu_get(1) - a.eu_get(1) );

				p = b.transform_scaled(p);

				cc.eu_modify(0, p[0] - c.scaled_height()/2 - c.eu_get(0));
				cc.eu_modify(1, p[1] - c.scaled_width()/2 - c.eu_get(1));
				
			} else if (alignment_class == 2) {
				/*
				 * Projective transformations
				 */

				point p[4];

				p[0] = b.transform_scaled(a
				     . scaled_inverse_transform(c.transform_scaled(point(      0        ,       0       ))));
				p[1] = b.transform_scaled(a
				     . scaled_inverse_transform(c.transform_scaled(point(c.scaled_height(),       0       ))));
				p[2] = b.transform_scaled(a
				     . scaled_inverse_transform(c.transform_scaled(point(c.scaled_height(), c.scaled_width()))));
				p[3] = b.transform_scaled(a
				     . scaled_inverse_transform(c.transform_scaled(point(      0        , c.scaled_width()))));

				cc.gpt_set(p);
			}

			return cc;
		}

		/* 
		 * For multi-alignment following, we use the following approach, not
		 * guaranteed to work with large changes in scene or perspective, but
		 * which should be somewhat flexible:
		 *
		 * For 
		 *
		 * 	t[][] calculated final alignments 
		 *	s[][] alignments as loaded from file
		 * 	previous frame n
		 * 	current frame n+1
		 *	fundamental (primary) 0
		 *	non-fundamental (non-primary) m!=0
		 *	parent element m'
		 *	follow(a, b, c) applying the (a, b) delta T=b(a^-1) to c
		 *
		 * following in the case of missing file data might be generated by 
		 *
		 * 	t[n+1][0] = t[n][0]
		 * 	t[n+1][m!=0] = follow(t[n][m'], t[n+1][m'], t[n][m])
		 *
		 * cases with all noted file data present might be generated by 
		 *
		 * 	t[n+1][0] = follow(s[n][0], t[n][0], s[n+1][0])
		 * 	t[n+1][m!=0] = follow(s[n+1][m'], t[n+1][m'], s[n+1][m])
		 *
		 * For non-following behavior, or where assigning the above is
		 * impossible, we assign the following default
		 *
		 * 	t[n+1][0] = Identity
		 * 	t[n+1][m!=0] = t[n+1][m']
		 */

		void init_frame_alignment_primary(transformation *offset, int lod, ale_pos perturb) {

			if (perturb > 0 && !old_is_default && !get_is_default(0)
			 && default_initial_alignment_type == 1) {

				/*
				 * Apply following logic for the primary element.
				 */

				ui::get()->following();

				trans_single new_offset = follow(old_initial_alignment.get_element(0),
								old_final_alignment.get_element(0), 
								offset->get_element(0));

				old_initial_alignment = *offset;

				offset->set_element(0, new_offset);

				ui::get()->set_offset(new_offset);
			} else {
				old_initial_alignment = *offset;
			}

			is_default.resize(old_initial_alignment.stack_depth());
		}

		void init_frame_alignment_nonprimary(transformation *offset, 
				int lod, ale_pos perturb, unsigned int index) {

			assert (index > 0);

			unsigned int parent_index = offset->parent_index(index);

			if (perturb > 0 
			 && !get_is_default(parent_index)
			 && !get_is_default(index)
			 && default_initial_alignment_type == 1) {

				/*
				 * Apply file-based following logic for the
				 * given element.
				 */

				ui::get()->following();

				trans_single new_offset = follow(old_initial_alignment.get_element(parent_index),
								offset->get_element(parent_index), 
								offset->get_element(index));

				old_initial_alignment.set_element(index, offset->get_element(index));
				offset->set_element(index, new_offset);

				ui::get()->set_offset(new_offset);

				return;
			}

			offset->get_coordinate(parent_index);


			if (perturb > 0
			 && old_final_alignment.exists(offset->get_coordinate(parent_index))
			 && old_final_alignment.exists(offset->get_current_coordinate())
			 && default_initial_alignment_type == 1) {

				/*
				 * Apply nonfile-based following logic for
				 * the given element.
				 */

				ui::get()->following();

			 	/*
				 * XXX: Although it is different, the below
				 * should be equivalent to the comment
				 * description.
				 */

			 	trans_single a = old_final_alignment.get_element(offset->get_coordinate(parent_index));
				trans_single b = old_final_alignment.get_element(offset->get_current_coordinate());
				trans_single c = offset->get_element(parent_index);
				
				trans_single new_offset = follow(a, b, c);

				offset->set_element(index, new_offset);
				ui::get()->set_offset(new_offset);

				return;
			}

			/*
			 * Handle other cases.
			 */

			if (get_is_default(index)) {
				offset->set_element(index, offset->get_element(parent_index));
				ui::get()->set_offset(offset->get_element(index));
			}
		}

		void init_default() {

			if (default_initial_alignment_type == 0) {
				
				/*
				 * Follow the transformation of the original frame,
				 * setting new image dimensions.
				 */

				// astate->default_initial_alignment = orig_t;
				default_initial_alignment.set_current_element(orig_t.get_element(0));
				default_initial_alignment.set_dimensions(input_frame);

			} else if (default_initial_alignment_type == 1)

				/*
				 * Follow previous transformation, setting new image
				 * dimensions.
				 */

				default_initial_alignment.set_dimensions(input_frame);

			else
				assert(0);

			old_is_default = get_is_default(0);
		}
	};

	/*
	 * Alignment for failed frames -- default or optimal?
	 *
	 * A frame that does not meet the match threshold can be assigned the
	 * best alignment found, or can be assigned its alignment default.
	 */

	static int is_fail_default;

	/*
	 * Alignment code.
	 * 
	 * 0. Align images with an error contribution from each color channel.
	 * 
	 * 1. Align images with an error contribution only from the green channel.
	 * Other color channels do not affect alignment.
	 *
	 * 2. Align images using a summation of channels.  May be useful when dealing
	 * with images that have high frequency color ripples due to color aliasing.
	 */

	static int channel_alignment_type;

	/*
	 * Error metric exponent
	 */

	static ale_real metric_exponent;

	/*
	 * Match threshold
	 */

	static float match_threshold;

	/*
	 * Perturbation lower and upper bounds.
	 */

	static ale_pos perturb_lower;
	static int perturb_lower_percent;
	static ale_pos perturb_upper;
	static int perturb_upper_percent;

	/*
	 * Preferred level-of-detail scale factor is 2^lod_preferred/perturb.
	 */

	static int lod_preferred;

	/*
	 * Minimum dimension for reduced LOD.
	 */

	static int min_dimension;

	/*
	 * Maximum rotational perturbation
	 */

	static ale_pos rot_max;

	/*
	 * Barrel distortion alignment multiplier
	 */

	static ale_pos bda_mult;

	/*
	 * Barrel distortion maximum adjustment rate
	 */

	static ale_pos bda_rate;

	/*
	 * Alignment match sum
	 */

	static ale_accum match_sum;

	/*
	 * Alignment match count.
	 */

	static int match_count;

	/*
	 * Monte Carlo parameter
	 */

	static ale_pos _mc;

	/*
	 * Certainty weight flag
	 *
	 * 0. Don't use certainty weights for alignment.
	 *
	 * 1. Use certainty weights for alignment.
	 */

	static int certainty_weights;

	/*
	 * Global search parameter
	 *
	 * 0.  Local:   Local search only.
	 * 1.  Inner:   Alignment reference image inner region
	 * 2.  Outer:   Alignment reference image outer region
	 * 3.  All:     Alignment reference image inner and outer regions.
	 * 4.  Central: Inner if possible; else, best of inner and outer.
	 * 5.  Points:  Align by control points.
	 */

	static int _gs;

	/*
	 * Minimum overlap for global searches
	 */

	static ale_accum _gs_mo;
	static int gs_mo_percent;

	/*
	 * Minimum certainty for multi-alignment element registration.
	 */

	static ale_real _ma_cert;

	/*
	 * Exclusion regions
	 */

	static exclusion *ax_parameters;
	static int ax_count;

	/*
	 * Types for scale clusters.
	 */

	struct nl_scale_cluster {
		const image *accum_max;
		const image *accum_min;
		const image *certainty_max;
		const image *certainty_min;
		const image *aweight_max;
		const image *aweight_min;
		exclusion *ax_parameters;

		ale_pos input_scale;
		const image *input_certainty_max;
		const image *input_certainty_min;
		const image *input_max;
		const image *input_min;
	};

	struct scale_cluster {
		const image *accum;
		const image *certainty;
		const image *aweight;
		exclusion *ax_parameters;

		ale_pos input_scale;
		const image *input_certainty;
		const image *input;

		nl_scale_cluster *nl_scale_clusters;
	};

	/*
	 * Check for exclusion region coverage in the reference 
	 * array.
	 */
	static int ref_excluded(int i, int j, point offset, exclusion *params, int param_count) {
		for (int idx = 0; idx < param_count; idx++)
			if (params[idx].type == exclusion::RENDER
			 && i + offset[0] >= params[idx].x[0]
			 && i + offset[0] <= params[idx].x[1]
			 && j + offset[1] >= params[idx].x[2]
			 && j + offset[1] <= params[idx].x[3])
				return 1;

		return 0;
	}

	/*
	 * Check for exclusion region coverage in the input 
	 * array.
	 */
	static int input_excluded(ale_pos ti, ale_pos tj, exclusion *params, int param_count) {
		for (int idx = 0; idx < param_count; idx++)
			if (params[idx].type == exclusion::FRAME
			 && ti >= params[idx].x[0]
			 && ti <= params[idx].x[1]
			 && tj >= params[idx].x[2]
			 && tj <= params[idx].x[3])
				return 1;

		return 0;
	}

	/*
	 * Overlap function.  Determines the number of pixels in areas where
	 * the arrays overlap.  Uses the reference array's notion of pixel
	 * positions.
	 */
	static unsigned int overlap(struct scale_cluster c, transformation t, int ax_count) {
		assert (reference_image);

		unsigned int result = 0;

		point offset = c.accum->offset();

		for (unsigned int i = 0; i < c.accum->height(); i++)
		for (unsigned int j = 0; j < c.accum->width();  j++) {

			if (ref_excluded(i, j, offset, c.ax_parameters, ax_count))
				continue;

			/*
			 * Transform
			 */

			struct point q;

			q = (c.input_scale < 1.0 && interpolant == NULL)
			  ? t.scaled_inverse_transform(
				point(i + offset[0], j + offset[1]))
			  : t.unscaled_inverse_transform(
				point(i + offset[0], j + offset[1]));

			ale_pos ti = q[0];
			ale_pos tj = q[1];

			/*
			 * Check that the transformed coordinates are within
			 * the boundaries of array c.input, and check that the
			 * weight value in the accumulated array is nonzero,
			 * unless we know it is nonzero by virtue of the fact
			 * that it falls within the region of the original
			 * frame (e.g. when we're not increasing image
			 * extents).  Also check for frame exclusion.
			 */

			if (input_excluded(ti, tj, c.ax_parameters, ax_count))
				continue;

			if (ti >= 0
			 && ti <= c.input->height() - 1
			 && tj >= 0
			 && tj <= c.input->width() - 1
			 && c.certainty->get_pixel(i, j)[0] != 0)
				result++;
		}

		return result;
	}

	/*
	 * Monte carlo iteration class.
	 *
	 * Monte Carlo alignment has been used for statistical comparisons in
	 * spatial registration, and is now used for tonal registration
	 * and final match calculation.
	 */

	/*
	 * We use a random process for which the expected number of sampled
	 * pixels is +/- .000003 from the coverage in the range [.005,.995] for
	 * an image with 100,000 pixels.  (The actual number may still deviate
	 * from the expected number by more than this amount, however.)  The
	 * method is as follows:
	 *
	 * We have coverage == USE/ALL, or (expected # pixels to use)/(# total
	 * pixels).  We derive from this SKIP/USE.
	 *
	 * SKIP/USE == (SKIP/ALL)/(USE/ALL) == (1 - (USE/ALL))/(USE/ALL)
	 *
	 * Once we have SKIP/USE, we know the expected number of pixels to skip
	 * in each iteration.  We use a random selection process that provides
	 * SKIP/USE close to this calculated value.
	 *
	 * If we can draw uniformly to select the number of pixels to skip, we
	 * do.  In this case, the maximum number of pixels to skip is twice the
	 * expected number.
	 *
	 * If we cannot draw uniformly, we still assign equal probability to
	 * each of the integer values in the interval [0, 2 * (SKIP/USE)], but
	 * assign an unequal amount to the integer value ceil(2 * SKIP/USE) +
	 * 1.
	 */

	/*
	 * When reseeding the random number generator, we want the same set of
	 * pixels to be used in cases where two alignment options are compared.
	 * If we wanted to avoid bias from repeatedly utilizing the same seed,
	 * we could seed with the number of the frame most recently aligned:
	 *
	 * 	srand(latest);
	 *
	 * However, in cursory tests, it seems okay to just use the default
	 * seed of 1, and so we do this, since it is simpler; both of these
	 * approaches to reseeding achieve better results than not reseeding.
	 * (1 is the default seed according to the GNU Manual Page for
	 * rand(3).)
	 * 
	 * For subdomain calculations, we vary the seed by adding the subdomain
	 * index.
	 */

	class mc_iterate {
		ale_pos mc_max;
		unsigned int index;
		unsigned int index_max;
		int i_min;
		int i_max;
		int j_min;
		int j_max;

		rng_t rng;

	public:
		mc_iterate(int _i_min, int _i_max, int _j_min, int _j_max, unsigned int subdomain) 
				: rng() {

			ale_pos coverage;

			i_min = _i_min;
			i_max = _i_max;
			j_min = _j_min;
			j_max = _j_max;

			if (i_max < i_min || j_max < j_min)
				index_max = 0;
			else 
				index_max = (i_max - i_min) * (j_max - j_min);

			if (index_max < 500 || _mc > 100 || _mc <= 0)
				coverage = 1;
			else
				coverage = _mc / 100;

			double su = (1 - coverage) / coverage;

			mc_max = (floor(2*su) * (1 + floor(2*su)) + 2*su)
			       / (2 + 2 * floor(2*su) - 2*su);

			rng.seed(1 + subdomain);

#define FIXED16 3
#if ALE_COORDINATES == FIXED16
			/*
			 * XXX: This calculation might not yield the correct
			 * expected value.
			 */
			index = -1 + (int) ceil(((ale_pos) mc_max+1) 
				   / (ale_pos) ( (1 + 0xffffff)
				                 / (1 + (rng.get() & 0xffffff))));
#else
                        index = -1 + (int) ceil((ale_accum) (mc_max+1) 
                                   * ( (1 + ((ale_accum) (rng.get())) ) 
                                     / (1 + ((ale_accum) RAND_MAX)) ));
#endif
#undef FIXED16
		}

		int get_i() {
			return index / (j_max - j_min) + i_min;
		}

		int get_j() {
			return index % (j_max - j_min) + j_min;
		}

		void operator++(int whats_this_for) {
#define FIXED16 3
#if ALE_COORDINATES == FIXED16
			index += (int) ceil(((ale_pos) mc_max+1) 
				   / (ale_pos) ( (1 + 0xffffff)
				                 / (1 + (rng.get() & 0xffffff))));
#else
                        index += (int) ceil((ale_accum) (mc_max+1) 
                               * ( (1 + ((ale_accum) (rng.get())) ) 
                                 / (1 + ((ale_accum) RAND_MAX)) ));
#endif
#undef FIXED16
		}

		int done() {
			return (index >= index_max);
		}
	};

	/*
	 * Not-quite-symmetric difference function.  Determines the difference in areas
	 * where the arrays overlap.  Uses the reference array's notion of pixel positions.
	 *
	 * For the purposes of determining the difference, this function divides each
	 * pixel value by the corresponding image's average pixel magnitude, unless we
	 * are:
	 *
	 * a) Extending the boundaries of the image, or
	 * 
	 * b) following the previous frame's transform
	 *
	 * If we are doing monte-carlo pixel sampling for alignment, we
	 * typically sample a subset of available pixels; otherwise, we sample
	 * all pixels.
	 *
	 */

	template<class diff_trans>
	class diff_stat_generic {

		transformation::elem_bounds_t elem_bounds;
	
		struct run {

			diff_trans offset;

			ale_accum result;
			ale_accum divisor;

			point max, min;
			ale_accum centroid[2], centroid_divisor;
			ale_accum de_centroid[2], de_centroid_v, de_sum;

			void init() {

				result = 0;
				divisor = 0;

				min = point::posinf();
				max = point::neginf();

				centroid[0] = 0;
				centroid[1] = 0;
				centroid_divisor = 0;

				de_centroid[0] = 0;
				de_centroid[1] = 0;

				de_centroid_v = 0;

				de_sum = 0;
			}

			void init(diff_trans _offset) {
				offset = _offset;
				init();
			}

			/*
			 * Required for STL sanity.
			 */
			run() : offset(diff_trans::eu_identity()) {
				init();
			}

			run(diff_trans _offset) : offset(_offset) {
				init(_offset);
			}

			void add(const run &_run) {
				result += _run.result;
				divisor += _run.divisor;

				for (int d = 0; d < 2; d++) {
					if (min[d] > _run.min[d])
						min[d] = _run.min[d];
					if (max[d] < _run.max[d])
						max[d] = _run.max[d];
					centroid[d] += _run.centroid[d];
					de_centroid[d] += _run.de_centroid[d];
				}

				centroid_divisor += _run.centroid_divisor;
				de_centroid_v += _run.de_centroid_v;
				de_sum += _run.de_sum;
			}

			run(const run &_run) : offset(_run.offset) {

				/*
				 * Initialize
				 */
				init(_run.offset);

				/*
				 * Add
				 */
				add(_run);
			}

			run &operator=(const run &_run) {

				/*
				 * Initialize
				 */
				init(_run.offset);

				/*
				 * Add
				 */
				add(_run);

				return *this;
			}

			~run() {
			}

			ale_accum get_error() const {
				return pow(result / divisor, 1/(ale_accum) metric_exponent);
			}

			void sample(int f, scale_cluster c, int i, int j, point t, point u,
					const run &comparison) {

				pixel pa = c.accum->get_pixel(i, j);

				ale_real this_result[2] = { 0, 0 };
				ale_real this_divisor[2] = { 0, 0 };

				pixel p[2];
				pixel weight[2];
				weight[0] = pixel(1, 1, 1);
				weight[1] = pixel(1, 1, 1);

				pixel tm = offset.get_tonal_multiplier(point(i, j) + c.accum->offset());

				if (interpolant != NULL) {
					interpolant->filtered(i, j, &p[0], &weight[0], 1, f);

					if (weight[0].min_norm() > ale_real_weight_floor) {
						p[0] /= weight[0];
					} else {
						return;
					}

				} else {
                                        p[0] = c.input->get_bl(t);
				}

				p[0] *= tm;

				if (u.defined()) {
					p[1] = c.input->get_bl(u);
					p[1] *= tm;
				}


				/*
				 * Handle certainty.
				 */

				if (certainty_weights == 1) {

					/*
					 * For speed, use arithmetic interpolation (get_bl(.))
					 * instead of geometric (get_bl(., 1))
					 */

					weight[0] *= c.input_certainty->get_bl(t);
					if (u.defined())
						weight[1] *= c.input_certainty->get_bl(u);
					weight[0] *= c.certainty->get_pixel(i, j);
					weight[1] *= c.certainty->get_pixel(i, j);
				}

				if (c.aweight != NULL) {
					weight[0] *= c.aweight->get_pixel(i, j);
					weight[1] *= c.aweight->get_pixel(i, j);
				}

				/*
				 * Update sampling area statistics
				 */

				if (min[0] > i)
					min[0] = i;
				if (min[1] > j)
					min[1] = j;
				if (max[0] < i)
					max[0] = i;
				if (max[1] < j)
					max[1] = j;

				centroid[0] += (weight[0][0] + weight[0][1] + weight[0][2]) * i;
				centroid[1] += (weight[0][0] + weight[0][1] + weight[0][2]) * j;
				centroid_divisor += (weight[0][0] + weight[0][1] + weight[0][2]);

				/*
				 * Determine alignment type.
				 */

				for (int m = 0; m < (u.defined() ? 2 : 1); m++)
				if (channel_alignment_type == 0) {
					/*
					 * Align based on all channels.
					 */


					for (int k = 0; k < 3; k++) {
						ale_real achan = pa[k];
						ale_real bchan = p[m][k];

						this_result[m] += weight[m][k] * pow(fabs(achan - bchan), metric_exponent);
						this_divisor[m] += weight[m][k] * pow(achan > bchan ? achan : bchan, metric_exponent);
					}
				} else if (channel_alignment_type == 1) {
					/*
					 * Align based on the green channel.
					 */

					ale_real achan = pa[1];
					ale_real bchan = p[m][1];

					this_result[m] = weight[m][1] * pow(fabs(achan - bchan), metric_exponent);
					this_divisor[m] = weight[m][1] * pow(achan > bchan ? achan : bchan, metric_exponent);
				} else if (channel_alignment_type == 2) {
					/*
					 * Align based on the sum of all channels.
					 */

					ale_real asum = 0;
					ale_real bsum = 0;
					ale_real wsum = 0;

					for (int k = 0; k < 3; k++) {
						asum += pa[k];
						bsum += p[m][k];
						wsum += weight[m][k] / 3;
					}

					this_result[m] = wsum * pow(fabs(asum - bsum), metric_exponent);
					this_divisor[m] = wsum * pow(asum > bsum ? asum : bsum, metric_exponent);
				}

				if (u.defined()) {
//					ale_real de = fabs(this_result[0] / this_divisor[0]
//						         - this_result[1] / this_divisor[1]);
					ale_real de = fabs(this_result[0] - this_result[1]);

					de_centroid[0] += de * (ale_real) i;
					de_centroid[1] += de * (ale_real) j;

					de_centroid_v += de * (ale_real) t.lengthto(u);

					de_sum += de;
				}

				result += (this_result[0]);
				divisor += (this_divisor[0]);
			}

			void rescale(ale_pos scale) {
				offset.rescale(scale);

				de_centroid[0] *= scale;
				de_centroid[1] *= scale;
				de_centroid_v *= scale;
			}

			point get_centroid() {
				point result = point(centroid[0] / centroid_divisor, centroid[1] / centroid_divisor);

				assert (finite(centroid[0]) 
				     && finite(centroid[1]) 
				     && (result.defined() || centroid_divisor == 0));

				return result;
			}

			point get_error_centroid() {
				point result = point(de_centroid[0] / de_sum, de_centroid[1] / de_sum);
				return result;
			}


			ale_pos get_error_perturb() {
				ale_pos result = de_centroid_v / de_sum;

				return result;
			}

		};

		/*
		 * When non-empty, runs.front() is best, runs.back() is
		 * testing.
		 */

		std::vector<run> runs;

		/*
		 * old_runs stores the latest available perturbation set for
		 * each multi-alignment element.
		 */

		typedef int run_index;
		std::map<run_index, run> old_runs;

		static void *diff_subdomain(void *args);

		struct subdomain_args {
			struct scale_cluster c;
			std::vector<run> runs;
			int ax_count;
			int f;
			int i_min, i_max, j_min, j_max;
			int subdomain;
		};

		struct scale_cluster si;
		int ax_count;
		int frame;

		std::vector<ale_pos> perturb_multipliers;

public:
		void diff(struct scale_cluster c, const diff_trans &t, 
				int _ax_count, int f) {

			if (runs.size() == 2)
				runs.pop_back();

			runs.push_back(run(t));

			si = c;
			ax_count = _ax_count;
			frame = f;

			ui::get()->d2_align_sample_start();

			if (interpolant != NULL) {

				/*
				 * XXX: This has not been tested, and may be completely
				 * wrong.
				 */

				transformation tt = transformation::eu_identity();
				tt.set_current_element(t);
				interpolant->set_parameters(tt, c.input, c.accum->offset());
			}

			int N;
#ifdef USE_PTHREAD
			N = thread::count();

			pthread_t *threads = (pthread_t *) malloc(sizeof(pthread_t) * N);
			pthread_attr_t *thread_attr = (pthread_attr_t *) malloc(sizeof(pthread_attr_t) * N);

#else
			N = 1;
#endif

			subdomain_args *args = new subdomain_args[N];

			transformation::elem_bounds_int_t b = elem_bounds.scale_to_bounds(c.accum->height(), c.accum->width());

// 			fprintf(stdout, "[%d %d] [%d %d]\n", 
// 				global_i_min, global_i_max, global_j_min, global_j_max);

			for (int ti = 0; ti < N; ti++) {
				args[ti].c = c;
				args[ti].runs = runs;
				args[ti].ax_count = ax_count;
				args[ti].f = f;
				args[ti].i_min = b.imin + ((b.imax - b.imin) * ti) / N;
				args[ti].i_max = b.imin + ((b.imax - b.imin) * (ti + 1)) / N;
				args[ti].j_min = b.jmin;
				args[ti].j_max = b.jmax;
				args[ti].subdomain = ti;
#ifdef USE_PTHREAD
				pthread_attr_init(&thread_attr[ti]);
				pthread_attr_setdetachstate(&thread_attr[ti], PTHREAD_CREATE_JOINABLE);
				pthread_create(&threads[ti], &thread_attr[ti], diff_subdomain, &args[ti]);
#else
				diff_subdomain(&args[ti]);
#endif
			}

			for (int ti = 0; ti < N; ti++) {
#ifdef USE_PTHREAD
				pthread_join(threads[ti], NULL);
#endif
				runs.back().add(args[ti].runs.back());
			}

#ifdef USE_PTHREAD
			free(threads);
			free(thread_attr);
#endif

			delete[] args;

			ui::get()->d2_align_sample_stop();

		}

	private:
		void rediff() {
			std::vector<diff_trans> t_array;

			for (unsigned int r = 0; r < runs.size(); r++) {
				t_array.push_back(runs[r].offset);
			}

			runs.clear();

			for (unsigned int r = 0; r < t_array.size(); r++)
				diff(si, t_array[r], ax_count, frame);
		}


	public:
		int better() {
			assert(runs.size() >= 2);
			assert(runs[0].offset.scale() == runs[1].offset.scale());

			return (runs[1].get_error() < runs[0].get_error() 
			     || (!finite(runs[0].get_error()) && finite(runs[1].get_error())));
		}

		int better_defined() {
			assert(runs.size() >= 2);
			assert(runs[0].offset.scale() == runs[1].offset.scale());

			return (runs[1].get_error() < runs[0].get_error()); 
		}

		diff_stat_generic(transformation::elem_bounds_t e) 
				: runs(), old_runs(), perturb_multipliers() {
			elem_bounds = e;
		}

		run_index get_run_index(unsigned int perturb_index) {
			return perturb_index;
		}

		run &get_run(unsigned int perturb_index) {
			run_index index = get_run_index(perturb_index);

			assert(old_runs.count(index));
			return old_runs[index];
		}

		void rescale(ale_pos scale, scale_cluster _si) {
			assert(runs.size() == 1);

			si = _si;

			runs[0].rescale(scale);
			
			rediff();
		}

		~diff_stat_generic() {
		}

		diff_stat_generic &operator=(const diff_stat_generic &dst) {
			/*
			 * Copy run information.
			 */
			runs = dst.runs;
			old_runs = dst.old_runs;

			/*
			 * Copy diff variables
			 */
			si = dst.si;
			ax_count = dst.ax_count;
			frame = dst.frame;
			perturb_multipliers = dst.perturb_multipliers;
			elem_bounds = dst.elem_bounds;

			return *this;
		}

		diff_stat_generic(const diff_stat_generic &dst) : runs(), old_runs(),
		                                      perturb_multipliers() {
			operator=(dst);
		}

		void set_elem_bounds(transformation::elem_bounds_t e) {
			elem_bounds = e;
		}

		ale_accum get_result() {
			assert(runs.size() == 1);
			return runs[0].result;
		}

		ale_accum get_divisor() {
			assert(runs.size() == 1);
			return runs[0].divisor;
		}

		diff_trans get_offset() {
			assert(runs.size() == 1);
			return runs[0].offset;
		}

		int operator!=(diff_stat_generic &param) {
			return (get_error() != param.get_error());
		}

		int operator==(diff_stat_generic &param) {
			return !(operator!=(param));
		}

		ale_pos get_error_perturb() {
			assert(runs.size() == 1);
			return runs[0].get_error_perturb();
		}

		ale_accum get_error() const {
			assert(runs.size() == 1);
			return runs[0].get_error();
		}

	public:
		/*
		 * Get the set of transformations produced by a given perturbation
		 */
		void get_perturb_set(std::vector<diff_trans> *set, 
				ale_pos adj_p, ale_pos adj_o, ale_pos adj_b, 
				ale_pos *current_bd, ale_pos *modified_bd,
				std::vector<ale_pos> multipliers = std::vector<ale_pos>()) {

			assert(runs.size() == 1);

			diff_trans test_t(diff_trans::eu_identity());

			/* 
			 * Translational or euclidean transformation
			 */

			for (unsigned int i = 0; i < 2; i++)
			for (unsigned int s = 0; s < 2; s++) {

				if (!multipliers.size())
					multipliers.push_back(1);

				assert(finite(multipliers[0]));

				test_t = get_offset();

				// test_t.eu_modify(i, (s ? -adj_p : adj_p) * multipliers[0]);
				test_t.translate((i ? point(1, 0) : point(0, 1))
				               * (s ? -adj_p : adj_p)
					       * multipliers[0]);
				
				test_t.snap(adj_p / 2);

				set->push_back(test_t);
				multipliers.erase(multipliers.begin());

			}

			if (alignment_class > 0)
			for (unsigned int s = 0; s < 2; s++) {

				if (!multipliers.size())
					multipliers.push_back(1);

				assert(multipliers.size());
				assert(finite(multipliers[0]));

				if (!(adj_o * multipliers[0] < rot_max))
					return;

				ale_pos adj_s = (s ? 1 : -1) * adj_o * multipliers[0];

				test_t = get_offset();

				run_index ori = get_run_index(set->size());
				point centroid = point::undefined();

				if (!old_runs.count(ori))
					ori = get_run_index(0);

				if (!centroid.finite() && old_runs.count(ori)) {
					centroid = old_runs[ori].get_error_centroid();

					if (!centroid.finite())
						centroid = old_runs[ori].get_centroid();

					centroid *= test_t.scale() 
					          / old_runs[ori].offset.scale();
				}

				if (!centroid.finite() && !test_t.is_projective()) {
					test_t.eu_modify(2, adj_s);
				} else if (!centroid.finite()) {
					centroid = point(si.input->height() / 2, 
					                 si.input->width() / 2);

					test_t.rotate(centroid + si.accum->offset(),
							adj_s);
				} else {
					test_t.rotate(centroid + si.accum->offset(), 
							adj_s);
				}

				test_t.snap(adj_p / 2);

				set->push_back(test_t);
				multipliers.erase(multipliers.begin());
			}

			if (alignment_class == 2) {

				/*
				 * Projective transformation
				 */

				for (unsigned int i = 0; i < 4; i++)
				for (unsigned int j = 0; j < 2; j++)
				for (unsigned int s = 0; s < 2; s++) {

					if (!multipliers.size())
						multipliers.push_back(1);

					assert(multipliers.size());
					assert(finite(multipliers[0]));

					ale_pos adj_s = (s ? -1 : 1) * adj_p * multipliers [0];

					test_t = get_offset();

					if (perturb_type == 0)
						test_t.gpt_modify_bounded(j, i, adj_s, elem_bounds.scale_to_bounds(si.accum->height(), si.accum->width()));
					else if (perturb_type == 1)
						test_t.gr_modify(j, i, adj_s);
					else
						assert(0);

					test_t.snap(adj_p / 2);

					set->push_back(test_t);
					multipliers.erase(multipliers.begin());
				}

			}

			/*
			 * Barrel distortion
			 */

			if (bda_mult != 0 && adj_b != 0) {

				for (unsigned int d = 0; d < get_offset().bd_count(); d++)
				for (unsigned int s = 0; s < 2; s++) {

					if (!multipliers.size())
						multipliers.push_back(1);

					assert (multipliers.size());
					assert (finite(multipliers[0]));

					ale_pos adj_s = (s ? -1 : 1) * adj_b * multipliers[0];

					if (bda_rate > 0 && fabs(modified_bd[d] + adj_s - current_bd[d]) > bda_rate)
						continue;
				
					diff_trans test_t = get_offset();

					test_t.bd_modify(d, adj_s);

					set->push_back(test_t);
				}
			}
		}

		void confirm() {
			assert(runs.size() == 2);
			runs[0] = runs[1];
			runs.pop_back();
		}

		void discard() {
			assert(runs.size() == 2);
			runs.pop_back();
		}

		void perturb_test(ale_pos perturb, ale_pos adj_p, ale_pos adj_o, ale_pos adj_b, 
				ale_pos *current_bd, ale_pos *modified_bd, int stable) {

			assert(runs.size() == 1);

			std::vector<diff_trans> t_set;

			if (perturb_multipliers.size() == 0) {
				get_perturb_set(&t_set, adj_p, adj_o, adj_b, 
						current_bd, modified_bd);

				for (unsigned int i = 0; i < t_set.size(); i++) {
					diff_stat_generic test = *this;

					test.diff(si, t_set[i], ax_count, frame);

					test.confirm();

					if (finite(test.get_error_perturb()))
						perturb_multipliers.push_back(adj_p / test.get_error_perturb());
					else
						perturb_multipliers.push_back(1);

				}

				t_set.clear();
			}

			get_perturb_set(&t_set, adj_p, adj_o, adj_b, current_bd, modified_bd,
					perturb_multipliers);

			int found_unreliable = 1;
			std::vector<int> tested(t_set.size(), 0);

			for (unsigned int i = 0; i < t_set.size(); i++) {
				run_index ori = get_run_index(i);

				/*
				 * Check for stability
				 */
				if (stable
				 && old_runs.count(ori)
				 && old_runs[ori].offset == t_set[i])
					tested[i] = 1;
			}

			std::vector<ale_pos> perturb_multipliers_original = perturb_multipliers;

			while (found_unreliable) {

				found_unreliable = 0;

				for (unsigned int i = 0; i < t_set.size(); i++) {

					if (tested[i])
						continue;

					diff(si, t_set[i], ax_count, frame);

					if (!(i < perturb_multipliers.size())
					 || !finite(perturb_multipliers[i])) {

						perturb_multipliers.resize(i + 1);

						if (finite(perturb_multipliers[i])
						 && finite(adj_p)
						 && finite(adj_p / runs[1].get_error_perturb())) {
							perturb_multipliers[i] = 
								adj_p / runs[1].get_error_perturb();

							found_unreliable = 1;
						} else
							perturb_multipliers[i] = 1;

						continue;
					}

					run_index ori = get_run_index(i);

					if (old_runs.count(ori) == 0)
						old_runs.insert(std::pair<run_index, run>(ori, runs[1]));
					else 
						old_runs[ori] = runs[1];

					if (finite(perturb_multipliers_original[i])
					 && finite(runs[1].get_error_perturb())
					 && finite(adj_p)
					 && finite(perturb_multipliers_original[i] * adj_p / runs[1].get_error_perturb()))
						perturb_multipliers[i] = perturb_multipliers_original[i]
							* adj_p / runs[1].get_error_perturb();
					else
						perturb_multipliers[i] = 1;

					tested[i] = 1;

					if (better()
					 && runs[1].get_error() < runs[0].get_error()
					 && perturb_multipliers[i] 
					  / perturb_multipliers_original[i] < 2) {
						runs[0] = runs[1];
						runs.pop_back();
						return;
					}

				}

				if (runs.size() > 1)
					runs.pop_back();

				if (!found_unreliable)
					return;
			}
		}

	};

	typedef diff_stat_generic<trans_single> diff_stat_t;
	typedef diff_stat_generic<trans_multi> diff_stat_multi;


	/*
	 * Adjust exposure for an aligned frame B against reference A.
	 *
	 * Expects full-LOD images.
	 *
	 * Note: This method does not use any weighting, by certainty or
	 * otherwise, in the first exposure registration pass, as any bias of
	 * weighting according to color may also bias the exposure registration
	 * result; it does use weighting, including weighting by certainty
	 * (even if certainty weighting is not specified), in the second pass,
	 * under the assumption that weighting by certainty improves handling
	 * of out-of-range highlights, and that bias of exposure measurements
	 * according to color may generally be less harmful after spatial
	 * registration has been performed.
	 */
	class exposure_ratio_iterate : public thread::decompose_domain {
		pixel_accum *asums;
		pixel_accum *bsums;
		pixel_accum *asum;
		pixel_accum *bsum;
		struct scale_cluster c;
		const transformation &t;
		int ax_count;
		int pass_number;
	protected:
		void prepare_subdomains(unsigned int N) {
			asums = new pixel_accum[N];
			bsums = new pixel_accum[N];
		}
		void subdomain_algorithm(unsigned int thread,
				int i_min, int i_max, int j_min, int j_max) {

			point offset = c.accum->offset();

			for (mc_iterate m(i_min, i_max, j_min, j_max, thread); !m.done(); m++) {
				
				unsigned int i = (unsigned int) m.get_i();
				unsigned int j = (unsigned int) m.get_j();

				if (ref_excluded(i, j, offset, c.ax_parameters, ax_count))
					continue;

				/*
				 * Transform
				 */

				struct point q;

				q = (c.input_scale < 1.0 && interpolant == NULL)
				  ? t.scaled_inverse_transform(
					point(i + offset[0], j + offset[1]))
				  : t.unscaled_inverse_transform(
					point(i + offset[0], j + offset[1]));

				/*
				 * Check that the transformed coordinates are within
				 * the boundaries of array c.input, that they are not
				 * subject to exclusion, and that the weight value in
				 * the accumulated array is nonzero.
				 */

				if (input_excluded(q[0], q[1], c.ax_parameters, ax_count))
					continue;

				if (q[0] >= 0
				 && q[0] <= c.input->height() - 1
				 && q[1] >= 0
				 && q[1] <= c.input->width() - 1
				 && ((pixel) c.certainty->get_pixel(i, j)).minabs_norm() != 0) { 
					pixel a = c.accum->get_pixel(i, j);
					pixel b;

					b = c.input->get_bl(q);

					pixel weight = ((c.aweight && pass_number)
						      ? (pixel) c.aweight->get_pixel(i, j)
						      : pixel(1, 1, 1))
						     * (pass_number
						      ? psqrt(c.certainty->get_pixel(i, j)
						            * c.input_certainty->get_bl(q, 1))
						      : pixel(1, 1, 1));

					asums[thread] += a * weight;
					bsums[thread] += b * weight;
				}
			}
		}

		void finish_subdomains(unsigned int N) {
			for (unsigned int n = 0; n < N; n++) {
				*asum += asums[n];
				*bsum += bsums[n];
			}
			delete[] asums;
			delete[] bsums;
		}
	public:
		exposure_ratio_iterate(pixel_accum *_asum,
				       pixel_accum *_bsum,
				       struct scale_cluster _c,
				       const transformation &_t,
				       int _ax_count,
				       int _pass_number) : decompose_domain(0, _c.accum->height(), 
				                                            0, _c.accum->width()),
							   t(_t) {

			asum = _asum;
			bsum = _bsum;
			c = _c;
			ax_count = _ax_count;
			pass_number = _pass_number;
		}

		exposure_ratio_iterate(pixel_accum *_asum,
				       pixel_accum *_bsum,
				       struct scale_cluster _c,
				       const transformation &_t,
				       int _ax_count,
				       int _pass_number,
				       transformation::elem_bounds_int_t b) : decompose_domain(b.imin, b.imax, 
				                                               b.jmin, b.jmax),
							      t(_t) {

			asum = _asum;
			bsum = _bsum;
			c = _c;
			ax_count = _ax_count;
			pass_number = _pass_number;
		}
	};

	static void set_exposure_ratio(unsigned int m, struct scale_cluster c,
			const transformation &t, int ax_count, int pass_number) {

		if (_exp_register == 2) {
			/*
			 * Use metadata only.
			 */
			ale_real gain_multiplier = image_rw::exp(m).get_gain_multiplier();
			pixel multiplier = pixel(gain_multiplier, gain_multiplier, gain_multiplier);

			image_rw::exp(m).set_multiplier(multiplier);
			ui::get()->exp_multiplier(multiplier[0],
					          multiplier[1],
						  multiplier[2]);

			return;
		}

		pixel_accum asum(0, 0, 0), bsum(0, 0, 0);

		exposure_ratio_iterate eri(&asum, &bsum, c, t, ax_count, pass_number);
		eri.run();

		// std::cerr << (asum / bsum) << " ";
		
		pixel_accum new_multiplier;

		new_multiplier = asum / bsum * image_rw::exp(m).get_multiplier();

		if (finite(new_multiplier[0])
		 && finite(new_multiplier[1])
		 && finite(new_multiplier[2])) {
			image_rw::exp(m).set_multiplier(new_multiplier);
			ui::get()->exp_multiplier(new_multiplier[0],
					          new_multiplier[1],
						  new_multiplier[2]);
		}
	}

	/*
	 * Copy all ax parameters.
	 */
	static exclusion *copy_ax_parameters(int local_ax_count, exclusion *source) {

		exclusion *dest = (exclusion *) malloc(local_ax_count * sizeof(exclusion));

		assert (dest);

		if (!dest)
			ui::get()->memory_error("exclusion regions");

		for (int idx = 0; idx < local_ax_count; idx++)
			dest[idx] = source[idx];

		return dest;
	}

	/*
	 * Copy ax parameters according to frame.
	 */
	static exclusion *filter_ax_parameters(int frame, int *local_ax_count) {

		exclusion *dest = (exclusion *) malloc(ax_count * sizeof(exclusion));

		assert (dest);

		if (!dest)
			ui::get()->memory_error("exclusion regions");

		*local_ax_count = 0;

		for (int idx = 0; idx < ax_count; idx++) {
			if (ax_parameters[idx].x[4] > frame 
			 || ax_parameters[idx].x[5] < frame)
				continue;

			dest[*local_ax_count] = ax_parameters[idx];

			(*local_ax_count)++;
		}

		return dest;
	}

	static void scale_ax_parameters(int local_ax_count, exclusion *ax_parameters, 
			ale_pos ref_scale, ale_pos input_scale) {
		for (int i = 0; i < local_ax_count; i++) {
			ale_pos scale = (ax_parameters[i].type == exclusion::RENDER)
				      ? ref_scale
				      : input_scale;

			for (int n = 0; n < 6; n++) {
				ax_parameters[i].x[n] = ax_parameters[i].x[n] * scale;
			}
		}
	}

	/*
	 * Prepare the next level of detail for ordinary images.
	 */
	static const image *prepare_lod(const image *current) {
		if (current == NULL)
			return NULL;

		return current->scale_by_half("prepare_lod");
	}

	/*
	 * Prepare the next level of detail for definition maps.
	 */
	static const image *prepare_lod_def(const image *current) {
		if (current == NULL)
			return NULL;

		return current->defined_scale_by_half("prepare_lod_def"); 
	}

	/*
	 * Initialize scale cluster data structures.
	 */

	static void init_nl_cluster(struct scale_cluster *sc) {
	}

	/*
	 * Destroy the first element in the scale cluster data structure.
	 */
	static void final_clusters(struct scale_cluster *scale_clusters, ale_pos scale_factor,
			unsigned int steps) {

		if (scale_clusters[0].input_scale < 1.0) {
			delete scale_clusters[0].input;
		}

		delete scale_clusters[0].input_certainty;

		free((void *)scale_clusters[0].ax_parameters);

		for (unsigned int step = 1; step < steps; step++) {
			delete scale_clusters[step].accum;
			delete scale_clusters[step].certainty;
			delete scale_clusters[step].aweight;

			if (scale_clusters[step].input_scale < 1.0) {
				delete scale_clusters[step].input;
				delete scale_clusters[step].input_certainty;
			}

			free((void *)scale_clusters[step].ax_parameters);
		}

		free(scale_clusters);
	}

	/*
	 * Calculate the centroid of a control point for the set of frames
	 * having index lower than m.  Divide by any scaling of the output.
	 */
	static point unscaled_centroid(unsigned int m, unsigned int p) {
		assert(_keep);

		point point_sum(0, 0);
		ale_accum divisor = 0;

		for(unsigned int j = 0; j < m; j++) {
			point pp = cp_array[p][j];

			if (pp.defined()) {
				point_sum += kept_t[j].transform_unscaled(pp) 
					   / kept_t[j].scale();
				divisor += 1;
			}
		}

		if (divisor == 0)
			return point::undefined();

		return point_sum / divisor;
	}

	/*
	 * Calculate centroid of this frame, and of all previous frames,
	 * from points common to both sets.
	 */
	static void centroids(unsigned int m, point *current, point *previous) {
		/*
		 * Calculate the translation
		 */
		point other_centroid(0, 0);
		point this_centroid(0, 0);
		ale_pos divisor = 0;

		for (unsigned int i = 0; i < cp_count; i++) {
			point other_c = unscaled_centroid(m, i);
			point this_c  = cp_array[i][m];

			if (!other_c.defined() || !this_c.defined())
				continue;

			other_centroid += other_c;
			this_centroid  += this_c;
			divisor        += 1;

		}

		if (divisor == 0) {
			*current = point::undefined();
			*previous = point::undefined();
			return;
		}

		*current = this_centroid / divisor;
		*previous = other_centroid / divisor;
	}

	/*
	 * Calculate the RMS error of control points for frame m, with
	 * transformation t, against control points for earlier frames.
	 */
	static ale_pos cp_rms_error(unsigned int m, transformation t) {
		assert (_keep);

		ale_accum err = 0;
		ale_accum divisor = 0;

		for (unsigned int i = 0; i < cp_count; i++)
		for (unsigned int j = 0; j < m;        j++) {
			const point *p = cp_array[i];
			point p_ref = kept_t[j].transform_unscaled(p[j]);
			point p_cur = t.transform_unscaled(p[m]);

			if (!p_ref.defined() || !p_cur.defined())
				continue;

			err += p_ref.lengthtosq(p_cur);
			divisor += 1;
		}

		return (ale_pos) sqrt(err / divisor);
	}


	static void test_global(diff_stat_t *here, scale_cluster si, transformation t, 
			int local_ax_count, int m, ale_accum local_gs_mo, ale_pos perturb) {

		diff_stat_t test(*here);

		test.diff(si, t.get_current_element(), local_ax_count, m);

		unsigned int ovl = overlap(si, t, local_ax_count);

		if (ovl >= local_gs_mo && test.better()) {
			test.confirm();
			*here = test;
			ui::get()->set_match(here->get_error());
			ui::get()->set_offset(here->get_offset());
		} else {
			test.discard();
		}

	}

	/*
	 * Get the set of global transformations for a given density
	 */
	static void test_globals(diff_stat_t *here, 
			scale_cluster si, transformation t, int local_gs, ale_pos adj_p,
			int local_ax_count, int m, ale_accum local_gs_mo, ale_pos perturb) {

		transformation offset = t;

		point min, max;

		transformation offset_p = offset;

		if (!offset_p.is_projective())
			offset_p.eu_to_gpt();

		min = max = offset_p.gpt_get(0);
		for (int p_index = 1; p_index < 4; p_index++) {
			point p = offset_p.gpt_get(p_index);
			if (p[0] < min[0])
				min[0] = p[0];
			if (p[1] < min[1])
				min[1] = p[1];
			if (p[0] > max[0])
				max[0] = p[0];
			if (p[1] > max[1])
				max[1] = p[1];
		}

		point inner_min_t = -min;
		point inner_max_t = -max + point(si.accum->height(), si.accum->width());
		point outer_min_t = -max + point(adj_p - 1, adj_p - 1);
		point outer_max_t = point(si.accum->height(), si.accum->width()) - point(adj_p, adj_p);

		if (local_gs == 1 || local_gs == 3 || local_gs == 4 || local_gs == 6) {

			/*
			 * Inner
			 */

			for (ale_pos i = inner_min_t[0]; i <= inner_max_t[0]; i += adj_p)
			for (ale_pos j = inner_min_t[1]; j <= inner_max_t[1]; j += adj_p) {
				transformation test_t = offset;
				test_t.translate(point(i, j));
				test_global(here, si, test_t, local_ax_count, m, local_gs_mo, perturb);
			}
		} 
		
		if (local_gs == 2 || local_gs == 3 || local_gs == -1 || local_gs == 6) {

			/*
			 * Outer
			 */

			for (ale_pos i = outer_min_t[0]; i <= outer_max_t[0]; i += adj_p)
			for (ale_pos j = outer_min_t[1]; j <  inner_min_t[1]; j += adj_p) {
				transformation test_t = offset;
				test_t.translate(point(i, j));
				test_global(here, si, test_t, local_ax_count, m, local_gs_mo, perturb);
			}
			for (ale_pos i = outer_min_t[0]; i <= outer_max_t[0]; i += adj_p)
			for (ale_pos j = outer_max_t[1]; j >  inner_max_t[1]; j -= adj_p) {
				transformation test_t = offset;
				test_t.translate(point(i, j));
				test_global(here, si, test_t, local_ax_count, m, local_gs_mo, perturb);
			}
			for (ale_pos i = outer_min_t[0]; i <  inner_min_t[0]; i += adj_p)
			for (ale_pos j = outer_min_t[1]; j <= outer_max_t[1]; j += adj_p) {
				transformation test_t = offset;
				test_t.translate(point(i, j));
				test_global(here, si, test_t, local_ax_count, m, local_gs_mo, perturb);
			}
			for (ale_pos i = outer_max_t[0]; i >  inner_max_t[0]; i -= adj_p)
			for (ale_pos j = outer_min_t[1]; j <= outer_max_t[1]; j += adj_p) {
				transformation test_t = offset;
				test_t.translate(point(i, j));
				test_global(here, si, test_t, local_ax_count, m, local_gs_mo, perturb);
			}
		}
	}

	static void get_translational_set(std::vector<transformation> *set,
			transformation t, ale_pos adj_p) {

		ale_pos adj_s;

		transformation offset = t;
		transformation test_t(transformation::eu_identity());

		for (int i = 0; i < 2; i++)
		for (adj_s = -adj_p; adj_s <= adj_p; adj_s += 2 * adj_p) {

			test_t = offset;

			test_t.translate(i ? point(adj_s, 0) : point(0, adj_s));

			set->push_back(test_t);
		}
	}

	static int threshold_ok(ale_accum error) {
		if ((1 - error) * (ale_accum) 100 >= match_threshold)
			return 1;

		if (!(match_threshold >= 0))
			return 1;

		return 0;
	}

	static diff_stat_t _align_element(ale_pos perturb, ale_pos local_lower,
			scale_cluster *scale_clusters, diff_stat_t here,
			ale_pos adj_p, ale_pos adj_o, ale_pos adj_b, 
			ale_pos *current_bd, ale_pos *modified_bd,
			astate_t *astate, int lod, scale_cluster si) {

		/*
		 * Run initial tests to get perturbation multipliers and error
		 * centroids.
		 */

		std::vector<d2::trans_single> t_set;

		here.get_perturb_set(&t_set, adj_p, adj_o, adj_b, current_bd, modified_bd);

		int stable_count = 0;

		while (perturb >= local_lower) {
			
			ui::get()->alignment_dims(scale_clusters[lod].accum->height(), scale_clusters[lod].accum->width(),
			                          scale_clusters[lod].input->height(), scale_clusters[lod].input->width());

			/*
			 * Orientational adjustment value in degrees.
			 *
			 * Since rotational perturbation is now specified as an
			 * arclength, we have to convert.
			 */

			ale_pos adj_o = 2 * (double) perturb 
				          / sqrt(pow(scale_clusters[0].input->height(), 2)
					       + pow(scale_clusters[0].input->width(),  2))
					  * 180
					  / M_PI;

			/*
			 * Barrel distortion adjustment value
			 */

			ale_pos adj_b = perturb * bda_mult;

			trans_single old_offset = here.get_offset();

			here.perturb_test(perturb, adj_p, adj_o, adj_b, current_bd, modified_bd,
				stable_count);

			if (here.get_offset() == old_offset)
				stable_count++;
			else
				stable_count = 0;

			if (stable_count == 3) {

				stable_count = 0;

				perturb *= 0.5;

				if (lod > 0 
				 && lod > lrint(log(perturb) / log(2)) - lod_preferred) {

					/*
					 * Work with images twice as large
					 */

					lod--;
					si = scale_clusters[lod];

					/* 
					 * Rescale the transforms.
					 */

					ale_pos rescale_factor = (double) scale_factor
							       / (double) pow(2, lod)
							       / (double) here.get_offset().scale();

					here.rescale(rescale_factor, si);

				} else {
					adj_p = perturb / pow(2, lod);
				}

				/*
				 * Announce changes
				 */

				ui::get()->alignment_perturbation_level(perturb, lod);
			}

			ui::get()->set_match(here.get_error());
			ui::get()->set_offset(here.get_offset());
		}

		if (lod > 0) {
			ale_pos rescale_factor = (double) scale_factor
					       / (double) here.get_offset().scale();

			here.rescale(rescale_factor, scale_clusters[0]);
		}

		return here;
	}

	/*
	 * Check for satisfaction of the certainty threshold.
	 */
	static int ma_cert_satisfied(const scale_cluster &c, const transformation &t, unsigned int i) {
		transformation::elem_bounds_int_t b = t.elem_bounds().scale_to_bounds(c.accum->height(), c.accum->width());
		ale_accum sum[3] = {0, 0, 0};

		for (unsigned int ii = b.imin; ii < b.imax; ii++)
		for (unsigned int jj = b.jmin; jj < b.jmax; jj++) {
			pixel p = c.accum->get_pixel(ii, jj);
			sum[0] += p[0];
			sum[1] += p[1];
			sum[2] += p[2];
		}

		unsigned int count = (b.jmax - b.jmin) * (b.imax - b.imin);

		for (int k = 0; k < 3; k++)
			if (sum[k] / count < _ma_cert)
				return 0;

		return 1;
	}

	static diff_stat_t check_ancestor_path(const trans_multi &offset, const scale_cluster &si, diff_stat_t here, int local_ax_count, int frame) {

		if (offset.get_current_index() > 0)
		for (int i = offset.parent_index(offset.get_current_index()); i >= 0; i = (i > 0) ? (int) offset.parent_index(i) : -1) {
			trans_single t = offset.get_element(i);
			t.rescale(offset.get_current_element().scale());

			here.diff(si, t, local_ax_count, frame);

			if (here.better_defined())
				here.confirm();
			else
				here.discard();
		}

		return here;
	}

#ifdef USE_FFTW
	/*
	 * High-pass filter for frequency weights
	 */
	static void hipass(int rows, int cols, fftw_complex *inout) {
		for (int i = 0; i < rows * vert_freq_cut; i++)
		for (int j = 0; j < cols; j++)
		for (int k = 0; k < 2; k++)
			inout[i * cols + j][k] = 0;
		for (int i = 0; i < rows; i++)
		for (int j = 0; j < cols * horiz_freq_cut; j++)
		for (int k = 0; k < 2; k++)
			inout[i * cols + j][k] = 0;
		for (int i = 0; i < rows; i++)
		for (int j = 0; j < cols; j++)
		for (int k = 0; k < 2;    k++)
			if (i / (double) rows + j / (double) cols < 2 * avg_freq_cut)
				inout[i * cols + j][k] = 0;
	}
#endif


	/*
	 * Reset alignment weights
	 */
	static void reset_weights() {
		if (alignment_weights != NULL)
			ale_image_release(alignment_weights);

		alignment_weights = NULL;
	}

	/*
	 * Initialize alignment weights
	 */
	static void init_weights() {
		if (alignment_weights != NULL) 
			return;

		alignment_weights = ale_new_image(accel::context(), ALE_IMAGE_RGB, ale_image_get_type(reference_image));

		assert (alignment_weights);

		assert (!ale_resize_image(alignment_weights, 0, 0, ale_image_get_width(reference_image), ale_image_get_height(reference_image)));

		ale_image_map_0(alignment_weights, "SET_PIXEL(p, PIXEL(1, 1, 1))");
	}
	
	/*
	 * Update alignment weights with weight map
	 */
	static void map_update() {

		if (weight_map == NULL)
			return;

		init_weights();

		ale_image_map_2(alignment_weights, alignment_weights, weight_map, " \
			SET_PIXEL(p, GET_PIXEL(0, p) * GET_PIXEL_BG(1, p))");
	}

	/*
	 * Update alignment weights with algorithmic weights
	 */
	static void wmx_update() {
#ifdef USE_UNIX

		static exposure *exp_def = new exposure_default();
		static exposure *exp_bool = new exposure_boolean();

		if (wmx_file == NULL || wmx_exec == NULL || wmx_defs == NULL)
			return;

		unsigned int rows = ale_image_get_height(reference_image);
		unsigned int cols = ale_image_get_width(reference_image);

		image_rw::write_image(wmx_file, reference_image);
		image_rw::write_image(wmx_defs, reference_image, exp_bool->get_gamma(), 0, 0, 1);

		/* execute ... */
		int exit_status = 1;
		if (!fork()) {
			execlp(wmx_exec, wmx_exec, wmx_file, wmx_defs, NULL);
			ui::get()->exec_failure(wmx_exec, wmx_file, wmx_defs);
		}

		wait(&exit_status);

		if (exit_status)
			ui::get()->fork_failure("d2::align");

		ale_image wmx_weights = image_rw::read_image(wmx_file, exp_def);

		ale_image_set_x_offset(wmx_weights, ale_image_get_x_offset(reference_image));
		ale_image_set_y_offset(wmx_weights, ale_image_get_y_offset(reference_image));

		if (ale_image_get_height(wmx_weights) != rows || ale_image_get_width(wmx_weights) != cols)
			ui::get()->error("algorithmic weighting must not change image size");

		if (alignment_weights == NULL)
			alignment_weights = wmx_weights;
		else 
			ale_image_map_2(alignment_weights, alignment_weights, wmx_weights, "\
				SET_PIXEL(p, GET_PIXEL(0, p) * GET_PIXEL(1, p))");
#endif
	}

	/*
	 * Update alignment weights with frequency weights
	 */
	static void fw_update() {
#ifdef USE_FFTW
		if (horiz_freq_cut == 0
		 && vert_freq_cut  == 0
		 && avg_freq_cut   == 0)
			return;

		/*
		 * Required for correct operation of --fwshow
		 */

		assert (alignment_weights == NULL);

		int rows = reference_image->height();
		int cols = reference_image->width();
		int colors = reference_image->depth();

		alignment_weights = new_image_ale_real(rows, cols,
				colors, "alignment_weights");

		fftw_complex *inout = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * rows * cols);

		assert (inout);

		fftw_plan pf = fftw_plan_dft_2d(rows, cols,
						inout, inout, 
						FFTW_FORWARD, FFTW_ESTIMATE);

		fftw_plan pb = fftw_plan_dft_2d(rows, cols,
						inout, inout, 
						FFTW_BACKWARD, FFTW_ESTIMATE);

		for (int k = 0; k < colors; k++) {
			for (int i = 0; i < rows * cols; i++) {
				inout[i][0] = reference_image->get_pixel(i / cols, i % cols)[k];
				inout[i][1] = 0;
			}

			fftw_execute(pf);
			hipass(rows, cols, inout);
			fftw_execute(pb);

			for (int i = 0; i < rows * cols; i++) {
#if 0
				alignment_weights->pix(i / cols, i % cols)[k] = fabs(inout[i][0] / (rows * cols));
#else
				alignment_weights->set_chan(i / cols, i % cols, k,
					sqrt(pow(inout[i][0] / (rows * cols), 2)
					   + pow(inout[i][1] / (rows * cols), 2)));
#endif
			}
		}

		fftw_destroy_plan(pf);
		fftw_destroy_plan(pb);
		fftw_free(inout);

		if (fw_output != NULL)
			image_rw::write_image(fw_output, alignment_weights);
#endif
	}

	/*
	 * Update alignment to frame N.
	 */
	static void update_to(int n) {

		assert (n <= latest + 1);
		assert (n >= 0);

		static astate_t astate;

		ui::get()->set_frame_num(n);

		if (latest < 0) {

			/*
			 * Handle the initial frame
			 */

			astate.set_input_frame(image_rw::open(n));

			const image *i = astate.get_input_frame();
			int is_default;
			transformation result = alignment_class == 2
				? transformation::gpt_identity(i, scale_factor)
				: transformation::eu_identity(i, scale_factor);
			result = tload_first(tload, alignment_class == 2, result, &is_default);
			tsave_first(tsave, result, alignment_class == 2);

			if (_keep > 0) {
				kept_t = transformation::new_eu_identity_array(image_rw::count());
				kept_ok = (int *) malloc(image_rw::count()
						* sizeof(int));
				assert (kept_t);
				assert (kept_ok);

				if (!kept_t || !kept_ok)
					ui::get()->memory_error("alignment");

				kept_ok[0] = 1;
				kept_t[0] = result;
			}

			latest = 0;
			latest_ok = 1;
			latest_t = result;

			astate.set_default(result);
			orig_t = result;

			image_rw::close(n);
		}

		for (int i = latest + 1; i <= n; i++) {

			/*
			 * Handle supplemental frames.
			 */

			assert (reference != NULL);

			ui::get()->set_arender_current();
			reference->sync(i - 1);
			ui::get()->clear_arender_current();
			reference_image = reference->get_image();
			reference_defined = reference->get_defined();

			if (i == 1)
				astate.default_set_original_bounds(reference_image);

			reset_weights();
			fw_update();
			wmx_update();
			map_update();

			assert (reference_image != NULL);
			assert (reference_defined != NULL);

			astate.set_input_frame(image_rw::open(i));

			_align(i, _gs, &astate);

			image_rw::close(n);
		}
	}

public:

	/*
	 * Set the control point count
	 */
	static void set_cp_count(unsigned int n) {
		assert (cp_array == NULL);

		cp_count = n;
		cp_array = (const point **) malloc(n * sizeof(const point *));
	}

	/*
	 * Set control points.
	 */
	static void set_cp(unsigned int i, const point *p) {
		cp_array[i] = p;
	}

	/*
	 * Register exposure
	 */
	static void exp_register() {
		_exp_register = 1;
	}

	/*
	 * Register exposure only based on metadata
	 */
	static void exp_meta_only() {
		_exp_register = 2;
	}

	/*
	 * Don't register exposure
	 */
	static void exp_noregister() {
		_exp_register = 0;
	}

	/*
	 * Set alignment class to translation only.  Only adjust the x and y
	 * position of images.  Do not rotate input images or perform
	 * projective transformations.
	 */
	static void class_translation() {
		alignment_class = 0;
	}

	/* 
	 * Set alignment class to Euclidean transformations only.  Adjust the x
	 * and y position of images and the orientation of the image about the
	 * image center.
	 */
	static void class_euclidean() {
		alignment_class = 1;
	}

	/*
	 * Set alignment class to perform general projective transformations.
	 * See the file gpt.h for more information about general projective
	 * transformations.
	 */
	static void class_projective() {
		alignment_class = 2;
	}

	/*
	 * Set the default initial alignment to the identity transformation.
	 */
	static void initial_default_identity() {
		default_initial_alignment_type = 0;
	}

	/*
	 * Set the default initial alignment to the most recently matched
	 * frame's final transformation.
	 */
	static void initial_default_follow() {
		default_initial_alignment_type = 1;
	}

	/*
	 * Perturb output coordinates.
	 */
	static void perturb_output() {
		perturb_type = 0;
	}

	/*
	 * Perturb source coordinates.
	 */
	static void perturb_source() {
		perturb_type = 1;
	}

	/*
	 * Frames under threshold align optimally
	 */
	static void fail_optimal() {
		is_fail_default = 0;
	}

	/*
	 * Frames under threshold keep their default alignments.
	 */
	static void fail_default() {
		is_fail_default = 1;
	}

	/*
	 * Align images with an error contribution from each color channel.
	 */
	static void all() {
		channel_alignment_type = 0;
	}

	/*
	 * Align images with an error contribution only from the green channel.
	 * Other color channels do not affect alignment.
	 */
	static void green() {
		channel_alignment_type = 1;
	}

	/*
	 * Align images using a summation of channels.  May be useful when
	 * dealing with images that have high frequency color ripples due to
	 * color aliasing.
	 */
	static void sum() {
		channel_alignment_type = 2;
	}

	/*
	 * Error metric exponent
	 */

	static void set_metric_exponent(float me) {
		metric_exponent = me;
	}

	/*
	 * Match threshold
	 */

	static void set_match_threshold(float mt) {
		match_threshold = mt;
	}

	/*
	 * Perturbation lower and upper bounds.
	 */

	static void set_perturb_lower(ale_pos pl, int plp) {
		perturb_lower = pl;
		perturb_lower_percent = plp;
	}

	static void set_perturb_upper(ale_pos pu, int pup) {
		perturb_upper = pu;
		perturb_upper_percent = pup;
	}

	/*
	 * Maximum rotational perturbation.
	 */

	static void set_rot_max(int rm) {

		/*
		 * Obtain the largest power of two not larger than rm.
		 */

		rot_max = pow(2, floor(log(rm) / log(2)));
	}

	/*
	 * Barrel distortion adjustment multiplier
	 */

	static void set_bda_mult(ale_pos m) {
		bda_mult = m;
	}

	/*
	 * Barrel distortion maximum rate of change
	 */

	static void set_bda_rate(ale_pos m) {
		bda_rate = m;
	}

	/*
	 * Level-of-detail
	 */

	static void set_lod_preferred(int lm) {
		lod_preferred = lm;
	}

	/*
	 * Minimum dimension for reduced level-of-detail.
	 */

	static void set_min_dimension(int md) {
		min_dimension = md;
	}

	/*
	 * Set the scale factor
	 */
	static void set_scale(ale_pos s) {
		scale_factor = s;
	}

	/*
	 * Set reference rendering to align against
	 */
	static void set_reference(render *r) {
		reference = r;
	}

	/*
	 * Set the interpolant
	 */
	static void set_interpolant(filter::scaled_filter *f) {
		interpolant = f;
	}

 	/*
	 * Set alignment weights image
	 */
	static void set_weight_map(const image *i) {
		weight_map = i;
	}

	/*
	 * Set frequency cuts
	 */
	static void set_frequency_cut(double h, double v, double a) {
		horiz_freq_cut = h;
		vert_freq_cut  = v;
		avg_freq_cut   = a;
	}

	/*
	 * Set algorithmic alignment weighting
	 */
	static void set_wmx(const char *e, const char *f, const char *d) {
		wmx_exec = e;
		wmx_file = f;
		wmx_defs = d;
	}

	/*
	 * Show frequency weights
	 */
	static void set_fl_show(const char *filename) {
		fw_output = filename;
	}

	/*
	 * Set transformation file loader.
	 */
	static void set_tload(tload_t *tl) {
		tload = tl;
	}

	/*
	 * Set transformation file saver.
	 */
	static void set_tsave(tsave_t *ts) {
		tsave = ts;
	}
	
	/*
	 * Get match statistics for frame N.
	 */
	static int match(int n) {
		update_to(n);

		if (n == latest)
			return latest_ok;
		else if (_keep)
			return kept_ok[n];
		else {
			assert(0);
			exit(1);
		}
	}

	/*
	 * Message that old alignment data should be kept.
	 */
	static void keep() {
		assert (latest == -1);
		_keep = 1;
	}

	/*
	 * Get alignment for frame N.
	 */
	static transformation of(int n) {
		update_to(n);
		if (n == latest)
			return latest_t;
		else if (_keep)
			return kept_t[n];
		else {
			assert(0);
			exit(1);
		}
	}

	/*
	 * Use Monte Carlo alignment sampling with argument N.
	 */
	static void mc(ale_pos n) {
		_mc = n;
	}

	/*
	 * Set the certainty-weighted flag.
	 */
	static void certainty_weighted(int flag) {
		certainty_weights = flag;
	}

	/*
	 * Set the global search type.
	 */
	static void gs(const char *type) {
		if (!strcmp(type, "local")) {
			_gs = 0;
		} else if (!strcmp(type, "inner")) {
			_gs = 1;
		} else if (!strcmp(type, "outer")) {
			_gs = 2;
		} else if (!strcmp(type, "all")) {
			_gs = 3;
		} else if (!strcmp(type, "central")) {
			_gs = 4;
		} else if (!strcmp(type, "defaults")) {
			_gs = 6;
		} else if (!strcmp(type, "points")) {
			_gs = 5;
			keep();
		} else {
			ui::get()->error("bad global search type");
		}
	}

	/*
	 * Set the minimum overlap for global searching
	 */
	static void gs_mo(ale_pos value, int _gs_mo_percent) {
		_gs_mo = value;
		gs_mo_percent = _gs_mo_percent;
	}

	/*
	 * Set mutli-alignment certainty lower bound.
	 */
	static void set_ma_cert(ale_real value) {
		_ma_cert = value;
	}

	/*
	 * Set alignment exclusion regions
	 */
	static void set_exclusion(exclusion *_ax_parameters, int _ax_count) {
		ax_count = _ax_count;
		ax_parameters = _ax_parameters;
	}

	/*
	 * Get match summary statistics.
	 */
	static ale_accum match_summary() {
		return match_sum / (ale_accum) match_count;
	}
};

template<class diff_trans>
void *d2::align::diff_stat_generic<diff_trans>::diff_subdomain(void *args) {

	subdomain_args *sargs = (subdomain_args *) args;

	struct scale_cluster c = sargs->c;
	std::vector<run> runs = sargs->runs;
	int ax_count = sargs->ax_count;
	int f = sargs->f;
	int i_min = sargs->i_min;
	int i_max = sargs->i_max;
	int j_min = sargs->j_min;
	int j_max = sargs->j_max;
	int subdomain = sargs->subdomain;

	assert (reference_image);

	point offset = c.accum->offset();

	for (mc_iterate m(i_min, i_max, j_min, j_max, subdomain); !m.done(); m++) {

		int i = m.get_i();
		int j = m.get_j();

		/*
		 * Check reference frame definition.
		 */

		if (!((pixel) c.accum->get_pixel(i, j)).finite()
		 || !(((pixel) c.certainty->get_pixel(i, j)).minabs_norm() > 0))
			continue;

		/*
		 * Check for exclusion in render coordinates.
		 */

		if (ref_excluded(i, j, offset, c.ax_parameters, ax_count))
			continue;

		/*
		 * Transform
		 */

		struct point q, r = point::undefined();

		q = (c.input_scale < 1.0 && interpolant == NULL)
		  ? runs.back().offset.scaled_inverse_transform(
			point(i + offset[0], j + offset[1]))
		  : runs.back().offset.unscaled_inverse_transform(
			point(i + offset[0], j + offset[1]));

		if (runs.size() == 2) {
			r = (c.input_scale < 1.0)
			  ? runs.front().offset.scaled_inverse_transform(
				point(i + offset[0], j + offset[1]))
			  : runs.front().offset.unscaled_inverse_transform(
				point(i + offset[0], j + offset[1]));
		}

		ale_pos ti = q[0];
		ale_pos tj = q[1];

		/*
		 * Check that the transformed coordinates are within
		 * the boundaries of array c.input and that they
		 * are not subject to exclusion.
		 *
		 * Also, check that the weight value in the accumulated array
		 * is nonzero, unless we know it is nonzero by virtue of the
		 * fact that it falls within the region of the original frame
		 * (e.g. when we're not increasing image extents).
		 */

		if (input_excluded(ti, tj, c.ax_parameters, ax_count))
			continue;

		if (input_excluded(r[0], r[1], c.ax_parameters, ax_count))
			r = point::undefined();

		/*
		 * Check the boundaries of the input frame
		 */

		if (!(ti >= 0
		   && ti <= c.input->height() - 1
		   && tj >= 0
		   && tj <= c.input->width() - 1))
			continue;

		if (!(r[0] >= 0
		   && r[0] <= c.input->height() - 1
		   && r[1] >= 0
		   && r[1] <= c.input->width() - 1))
			r = point::undefined();

		sargs->runs.back().sample(f, c, i, j, q, r, runs.front());
	}

	return NULL;
}

#endif
