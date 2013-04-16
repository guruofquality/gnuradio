/*
 * Copyright 2012-2013 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef INCLUDED_GRNURADIO_TYPES_H
#define INCLUDED_GRNURADIO_TYPES_H

// this section is to satisfy swig includes for gras.i
// since gras.i includes gr_types.h, we only have to edit this file
#include <gras/element.hpp>
#include <gras/block.hpp>
#include <gras/top_block.hpp>
#include <gras/hier_block.hpp>

// and gnuradio apparently needs its own typedefs for stdint...
#ifdef __cplusplus

#include <boost/cstdint.hpp>
typedef boost::int16_t gr_int16;
typedef boost::int32_t gr_int32;
typedef boost::int64_t gr_int64;
typedef boost::uint16_t gr_uint16;
typedef boost::uint32_t gr_uint32;
typedef boost::uint64_t gr_uint64;

typedef std::vector<int>			gr_vector_int;
typedef std::vector<unsigned int>		gr_vector_uint;
typedef std::vector<float>			gr_vector_float;
typedef std::vector<double>			gr_vector_double;
typedef std::vector<void *>			gr_vector_void_star;
typedef std::vector<const void *>		gr_vector_const_void_star;

#include <complex>
typedef std::complex<float> gr_complex;
typedef std::complex<double> gr_complexd;

#endif

#endif /* INCLUDED_GRNURADIO_TYPES_H */
