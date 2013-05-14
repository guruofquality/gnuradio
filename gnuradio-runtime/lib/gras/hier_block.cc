/*
 * Copyright 2013 Free Software Foundation, Inc.
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

#define GRASP_HIER_BLOCK (boost::static_pointer_cast<gras::HierBlock>(this->block_pimpl))

#include "gras/gras_pimpl.h"
#include <gnuradio/hier_block2.h>
#include <gras/hier_block.hpp>

gr::hier_block2_sptr gr::make_hier_block2(
    const std::string &name,
    gr::io_signature::sptr input_signature,
    gr::io_signature::sptr output_signature
)
{
    return gr::hier_block2_sptr(new gr::hier_block2(name, input_signature, output_signature));
}

gr::hier_block2::hier_block2(
    const std::string &name,
    gr::io_signature::sptr input_signature,
    gr::io_signature::sptr output_signature
)
{
    GRAS_PORTS_PIMPL_INIT();
    block_pimpl.reset(new gras::HierBlock(name));
}

gr::hier_block2::~hier_block2(void)
{
    block_pimpl.reset();
}

void gr::hier_block2::lock(void)
{
    //dont tear down the flow graph
}

void gr::hier_block2::unlock(void)
{
    //thread safe commit topology changes
    GRASP_HIER_BLOCK->commit();
}

gr::flat_flowgraph_sptr gr::hier_block2::flatten() const
{
    return flat_flowgraph_sptr(); //nothing to do
}

gr::hier_block2::opaque_self gr::hier_block2::self()
{
    //hide hier self in this opaque_self
    //dont need sptr magic hack, just connect w/ *this
    return boost::static_pointer_cast<opaque_self::element_type>(block_pimpl);
}

static gr::basic_block *resolve_basic_block(gr::basic_block_sptr block, gr::basic_block *self)
{
    //check if hier is hidden in an opaque_self
    if (size_t(block.get()) == size_t(self->block_pimpl.get())) return self;

    //otherwise pick out the initialized element
    else return block.get();
}

static gras::Element &get_elem_sptr(gr::basic_block_sptr block, gr::basic_block *self)
{
    boost::shared_ptr<gras::Element> element;
    gr::basic_block *bb_ptr = resolve_basic_block(block, self);

    //store container for safety
    element = boost::static_pointer_cast<gras::Element>(bb_ptr->block_pimpl);
    element->set_container(new gras::WeakContainerSharedPtr(block));

    return *element;
}

void gr::hier_block2::connect(gr::basic_block_sptr block)
{
    GRASP_HIER_BLOCK->connect(get_elem_sptr(block, this));
}

void gr::hier_block2::connect(
    gr::basic_block_sptr src, int src_port,
    gr::basic_block_sptr dst, int dst_port
)
{
    GRAS_PORTS_PIMPL(resolve_basic_block(src, this))->output.update(src_port);
    GRAS_PORTS_PIMPL(resolve_basic_block(dst, this))->input.update(dst_port);

    GRASP_HIER_BLOCK->connect(
        get_elem_sptr(src, this), src_port,
        get_elem_sptr(dst, this), dst_port
    );
}

void gr::hier_block2::disconnect(gr::basic_block_sptr block)
{
    GRASP_HIER_BLOCK->disconnect(get_elem_sptr(block, this));
}

void gr::hier_block2::disconnect(
    gr::basic_block_sptr src, int src_port,
    gr::basic_block_sptr dst, int dst_port
)
{
    GRASP_HIER_BLOCK->disconnect(
        get_elem_sptr(src, this), src_port,
        get_elem_sptr(dst, this), dst_port
    );
}

void gr::hier_block2::disconnect_all()
{
    GRASP_HIER_BLOCK->disconnect_all();
}

void gr::hier_block2::msg_connect(basic_block_sptr src, pmt::pmt_t srcport,
                 basic_block_sptr dst, pmt::pmt_t dstport)
{
    return this->msg_connect(src, pmt::symbol_to_string(srcport), dst, pmt::symbol_to_string(dstport));
}
void gr::hier_block2::msg_connect(basic_block_sptr src, std::string srcport,
                 basic_block_sptr dst, std::string dstport)
{
    const size_t src_port = GRAS_PORTS_PIMPL(resolve_basic_block(src, this))->output.get_index(srcport);
    const size_t dst_port = GRAS_PORTS_PIMPL(resolve_basic_block(dst, this))->input.get_index(dstport);
    GRASP_HIER_BLOCK->connect(
        get_elem_sptr(src, this), src_port,
        get_elem_sptr(dst, this), dst_port
    );
}
void gr::hier_block2::msg_disconnect(basic_block_sptr src, pmt::pmt_t srcport,
                 basic_block_sptr dst, pmt::pmt_t dstport)
{
    return this->msg_disconnect(src, pmt::symbol_to_string(srcport), dst, pmt::symbol_to_string(dstport));
}
void gr::hier_block2::msg_disconnect(basic_block_sptr src, std::string srcport,
                 basic_block_sptr dst, std::string dstport)
{
    const size_t src_port = GRAS_PORTS_PIMPL(resolve_basic_block(src, this))->output.get_index(srcport);
    const size_t dst_port = GRAS_PORTS_PIMPL(resolve_basic_block(dst, this))->input.get_index(dstport);
    GRASP_HIER_BLOCK->disconnect(
        get_elem_sptr(src, this), src_port,
        get_elem_sptr(dst, this), dst_port
    );
}

gr::hier_block2_sptr gr::hier_block2::to_hier_block2()
{
    return boost::static_pointer_cast<gr::hier_block2>(shared_from_this());
}
