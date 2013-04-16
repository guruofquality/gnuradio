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

#define GR_CORE_API

//not here to fight you swig, reference() is ambigi with shared ptr, but whatevs
%ignore gri_agc_cc::reference();
%ignore gri_agc2_ff::reference();
%ignore gri_agc2_cc::reference();

//someone forgot about d_detail
%ignore gr_block::d_setlock;

//dont export work overloads
%ignore general_work;
%ignore work;
%ignore forecast;

%{

#include <gras/block.hpp>
#include <gras/hier_block.hpp>
#include <gras/top_block.hpp>
#include <gr_block.h>
#include <gr_top_block.h>
#include <gr_hier_block2.h>
#include <gr_message.h>
#include <gr_msg_handler.h>
#include <gr_msg_queue.h>
#include <gr_sync_block.h>
#include <gr_sync_decimator.h>
#include <gr_sync_interpolator.h>

%}

//const size types used by blocks in python
%constant int sizeof_char       = sizeof(char);
%constant int sizeof_short      = sizeof(short);
%constant int sizeof_int        = sizeof(int);
%constant int sizeof_float      = sizeof(float);
%constant int sizeof_double     = sizeof(double);
%constant int sizeof_gr_complex = sizeof(gr_complex);

%include <gr_message.i>
%include <gr_msg_handler.i>
%include <gr_msg_queue.i>
%include <gr_swig_block_magic.i>
%include <gr_io_signature.i>

#ifdef SW_RUNTIME

%import <gras/block.i>
%include <gr_block.h>
%include <gr_hier_block2.h>
%include <gr_top_block.h>
%include <gr_sync_block.h>
%include <gr_sync_decimator.h>
%include <gr_sync_interpolator.h>

#else

//the bare minimum block inheritance interface to make things work but keep swig cxx file size down
%include <gras/gras.hpp>
%import <gras/element.i>
namespace gras
{
    struct Block : gras::Element{};
    struct HierBlock : gras::Element{};
}
struct gr_hier_block2 : gras::HierBlock{};
struct gr_block : gras::Block
{
    gr_io_signature_sptr input_signature(void) const;
    gr_io_signature_sptr output_signature(void) const;

    unsigned history () const;

    int  output_multiple () const;
    double relative_rate () const;

    bool start();
    bool stop();

    uint64_t nitems_read(unsigned int which_input);
    uint64_t nitems_written(unsigned int which_output);

    // Methods to manage the block's max_noutput_items size.
    int max_noutput_items();
    void set_max_noutput_items(int m);
    void unset_max_noutput_items();
    bool is_set_max_noutput_items();

    // Methods to manage block's min/max buffer sizes.
    long max_output_buffer(int i);
    void set_max_output_buffer(long max_output_buffer);
    void set_max_output_buffer(int port, long max_output_buffer);
    long min_output_buffer(int i);
    void set_min_output_buffer(long min_output_buffer);
    void set_min_output_buffer(int port, long min_output_buffer);
};
struct gr_sync_block : gr_block{};
struct gr_sync_interpolator : gr_sync_block{};
struct gr_sync_decimator : gr_sync_block{};

#endif
