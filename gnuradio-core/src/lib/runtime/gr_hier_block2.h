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

#ifndef INCLUDED_GNURADIO_GR_HIER_BLOCK2_H
#define INCLUDED_GNURADIO_GR_HIER_BLOCK2_H

#include <gr_core_api.h>
#include <gras/hier_block.hpp>
#include <gr_io_signature.h>

struct GR_CORE_API gr_hier_block2 : gras::HierBlock
{

    gr_hier_block2(void);

    gr_hier_block2(
        const std::string &name,
        gr_io_signature_sptr input_signature,
        gr_io_signature_sptr output_signature
    );

    long unique_id(void) const{return _unique_id;}
    std::string name(void) const{return _name;}
    long _unique_id;
    std::string _name;

    const gr_hier_block2 &self(void) const
    {
        return *this;
    }

    gr_io_signature_sptr input_signature(void) const;
    gr_io_signature_sptr output_signature(void) const;

    void set_input_signature(gr_io_signature_sptr sig);
    void set_output_signature(gr_io_signature_sptr sig);

    inline void lock(void){}

    inline void unlock(void){this->commit();}

    gr_io_signature_sptr _in_sig, _out_sig;
};

typedef boost::shared_ptr<gr_hier_block2> gr_hier_block2_sptr;

GR_CORE_API gr_hier_block2_sptr gr_make_hier_block2(
    const std::string &name,
    gr_io_signature_sptr input_signature,
    gr_io_signature_sptr output_signature
);

#endif /*INCLUDED_GNURADIO_GR_HIER_BLOCK2_H*/
