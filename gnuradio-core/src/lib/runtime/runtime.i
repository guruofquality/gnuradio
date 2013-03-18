//
// Copyright 2012 Josh Blum
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#define GR_CORE_API

//not here to fight you swig, reference() is ambigi with shared ptr, but whatevs
%ignore gri_agc_cc::reference();
%ignore gri_agc2_ff::reference();
%ignore gri_agc2_cc::reference();
%ignore gr_block::d_setlock;

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
%include <gras/element.hpp>
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
};
struct gr_sync_block : gr_block{};
struct gr_sync_interpolator : gr_sync_block{};
struct gr_sync_decimator : gr_sync_block{};

#endif
