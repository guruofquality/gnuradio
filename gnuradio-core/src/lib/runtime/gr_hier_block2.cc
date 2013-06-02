/* -*- c++ -*- */
/*
 * Copyright 2006,2007,2008 Free Software Foundation, Inc.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gr_hier_block2.h>
#include <boost/detail/atomic_count.hpp>

static boost::detail::atomic_count unique_id_pool(0);


gr_hier_block2::gr_hier_block2(void)
{
    //NOP
}

gr_hier_block2::gr_hier_block2(
    const std::string &name,
    gr_io_signature_sptr input_signature,
    gr_io_signature_sptr output_signature
):
    gras::HierBlock(name),
    _unique_id(++unique_id_pool),
    _name(name)
{
    this->set_input_signature(input_signature);
    this->set_output_signature(output_signature);
}

gr_hier_block2_sptr gr_make_hier_block2(
    const std::string &name,
    gr_io_signature_sptr input_signature,
    gr_io_signature_sptr output_signature
){
    return gr_hier_block2_sptr(new gr_hier_block2(name, input_signature, output_signature));
}

gr_io_signature_sptr gr_hier_block2::input_signature(void) const
{
    return _in_sig;
}

gr_io_signature_sptr gr_hier_block2::output_signature(void) const
{
    return _out_sig;
}

void gr_hier_block2::set_input_signature(gr_io_signature_sptr sig)
{
    _in_sig = sig;
}

void gr_hier_block2::set_output_signature(gr_io_signature_sptr sig)
{
    _out_sig = sig;
}
