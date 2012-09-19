/* -*- c++ -*- */
/*
 * Copyright 2009 Free Software Foundation, Inc.
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
%module(docstring="Generic coding/decoding.") trellis

#define DIGITAL_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "trellis_swig_doc.i"

%{
#include "digital/constellation.h"
#include "digital/metric_type.h"
#include "fsm.h"
#include "interleaver.h"
#include "trellis_permutation.h"
#include "trellis_siso_f.h"
#include "trellis_siso_combined_f.h"
#include "trellis_constellation_metrics_cf.h"
%}

%include "digital/constellation.h"
%include "digital/metric_type.h"
%include "constellation.i"

// ----------------------------------------------------------------

%include "fsm.i"
%include "interleaver.i"
%include "trellis_permutation.i"
%include "trellis_siso_f.i"
%include "trellis_siso_combined_f.i"

%include "siso_type.h"

%include "trellis_constellation_metrics_cf.i"

%include "trellis_generated.i"
