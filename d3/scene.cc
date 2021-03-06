// Copyright 2003, 2004, 2005 David Hilvert <dhilvert@gmail.com>,
//                                          <dhilvert@auricle.dyndns.org>,
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

#include "scene.h"

/*
 * See scene.h for details on these variables.
 */

scene::lod_images *scene::al;
ale_pos scene::front_clip = 0;
int scene::input_decimation_lower = 0;
int scene::output_decimation_preferred = 0;
int scene::primary_decimation_upper = 0;
int scene::output_clip = 0;
ale_pos scene::rear_clip = 0;
const char *scene::load_model_name = NULL;
const char *scene::save_model_name = NULL;
const ale_real scene::nearness = 0.01;

scene::spatial_info_map_t scene::spatial_info_map;

// scene::spatial_info *scene::tracked_space = NULL;

unsigned long scene::total_ambiguity = 0;
unsigned long scene::total_pixels = 0;
unsigned long scene::total_divisions = 0;
unsigned long scene::total_tsteps = 0;

double scene::occ_att = 0.50;

int scene::normalize_weights = 1;
int scene::use_filter = 1;
const char *scene::d3chain_type = "";

ale_real scene::falloff_exponent = 1;
double scene::tc_multiplier = 0;
unsigned int scene::ou_iterations = 10;
unsigned int scene::pairwise_ambiguity = 3;
const char *scene::pairwise_comparisons = "auto";
int scene::d3px_count;
double *scene::d3px_parameters;
double scene::encounter_threshold = 0;
double scene::depth_median_radius = 0;
double scene::diff_median_radius = 0;
int scene::subspace_traverse = 0;

/*
 * Precision discriminator
 *
 * For some reason, colors that should be identical are calculated differently
 * along different computational pathways, either due to a compiler idiosyncrasy,
 * or due to an as-of-yet undiscovered bug in the code.  We use the following
 * constant in an effort to winnow out these discrepancies, which can sometimes
 * cause cycles in the adjustment preference function.
 */

#define DISCRIMINATOR 1e-5

/*
 * Functions.
 */

