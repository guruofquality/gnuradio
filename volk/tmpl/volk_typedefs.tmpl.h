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

#ifndef INCLUDED_VOLK_TYPEDEFS
#define INCLUDED_VOLK_TYPEDEFS

#include <inttypes.h>
#include <volk/volk_complex.h>

#for $kern in $kernels
typedef void (*$(kern.pname))($kern.arglist_types);
#end for

#endif /*INCLUDED_VOLK_TYPEDEFS*/
