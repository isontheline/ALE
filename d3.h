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

/*
 * Top-level header file for classes treating scenes as three-dimensional data.
 */


#if defined(HASH_MAP_GNU) || defined(HASH_MAP_STD)
#include <ext/hash_map>
#endif

#include <map>
#include <set>
#include <iterator>
#include <vector>
#include <queue>
#include <stack>
#include <algorithm>
#include "time.h"
#include "d2.h"

namespace d3 {

#include "d3/align.h"
#include "d3/et.h"
#include "d3/tfile.h"
#include "d3/point.h"
#include "d3/cpf.h"
#include "d3/pt.h"
#include "d3/space.h"
#include "d3/focus.h"
#include "d3/scene.h"
	
}
