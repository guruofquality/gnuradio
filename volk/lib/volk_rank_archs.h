//
// Copyright 2012 Free Software Foundation, Inc.
//
// VOLK is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// VOLK is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with VOLK.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef INCLUDED_VOLK_RANK_ARCHS_H
#define INCLUDED_VOLK_RANK_ARCHS_H

#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

int volk_get_index(
    const char *impl_names[], //list of implementations by name
    const size_t n_impls,     //number of implementations available
    const char *impl_name     //the implementation name to find
);

int volk_rank_archs(
    const char *kern_name,    //name of the kernel to rank
    const char *impl_names[], //list of implementations by name
    const int* impl_deps,     //requirement mask per implementation
    const bool* alignment,    //alignment status of each implementation
    size_t n_impls,            //number of implementations available
    const bool align          //if false, filter aligned implementations
);

#ifdef __cplusplus
}
#endif
#endif /*INCLUDED_VOLK_RANK_ARCHS_H*/
