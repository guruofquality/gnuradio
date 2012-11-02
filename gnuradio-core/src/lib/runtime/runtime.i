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
#define GRAS_API

//not here to fight you swig, reference() is ambigi with shared ptr, but whatevs
%ignore gri_agc_cc::reference();
%ignore gri_agc2_ff::reference();
%ignore gri_agc2_cc::reference();

%{

#include <gras/thread_pool.hpp>
#include <gras/element.hpp>
#include <gras/block.hpp>
#include <gras/hier_block.hpp>
#include <gras/top_block.hpp>
#include <gras/io_signature.hpp>
#include <gras/tags.hpp>
#include <gr_io_signature.h>
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

%include <gr_message.i>
%include <gr_msg_handler.i>
%include <gr_msg_queue.i>
%include <gr_swig_block_magic.i>

#ifdef SW_RUNTIME

%rename(io_signature)  gr_make_io_signature;
%rename(io_signature2) gr_make_io_signature2;
%rename(io_signature3) gr_make_io_signature3;
%rename(io_signaturev) gr_make_io_signaturev;

//const size types used by blocks in python
%constant int sizeof_char       = sizeof(char);
%constant int sizeof_short      = sizeof(short);
%constant int sizeof_int        = sizeof(int);
%constant int sizeof_float      = sizeof(float);
%constant int sizeof_double     = sizeof(double);
%constant int sizeof_gr_complex = sizeof(gr_complex);

%ignore gras::Block::input_buffer_allocator;
%ignore gras::Block::output_buffer_allocator;

%include <gras/element.hpp>
%include <gras/io_signature.i>
%include <gras/tags.hpp>
%include <gras/block.hpp>

%include <gr_io_signature.h>
%include <gr_block.h>
%include <gr_hier_block2.h>
%include <gr_top_block.h>
%include <gr_sync_block.h>
%include <gr_sync_decimator.h>
%include <gr_sync_interpolator.h>

#else

//the bare minimum block inheritance interface to make things work but keep swig cxx file size down
%include <gras/element.hpp>
namespace gras
{
    struct Block : Element{};
    struct HierBlock : Element{};
}
struct gr_hier_block2 : gras::HierBlock{};
struct gr_block : gras::Block{};
struct gr_sync_block : gr_block{};
struct gr_sync_interpolator : gr_sync_block{};
struct gr_sync_decimator : gr_sync_block{};

#endif
