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

#define GRASP_HIER_BLOCK (boost::static_pointer_cast<gras::HierBlock>(this->pimpl))

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
    pimpl.reset(new gras::HierBlock(name));
}

gr::hier_block2::~hier_block2(void)
{
    pimpl.reset();
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
    return boost::static_pointer_cast<opaque_self::element_type>(pimpl);
}

static gras::Element &get_elem_sptr(gr::basic_block_sptr block, boost::shared_ptr<void> self)
{
    boost::shared_ptr<gras::Element> element;

    //check if hier is hidden in an opaque_self
    if (size_t(block.get()) == size_t(self.get()))
    {
        element = boost::static_pointer_cast<gras::Element>(self);
    }

    //otherwise pick out the initialized element
    else
    {
        element = boost::static_pointer_cast<gras::Element>(block->pimpl);
        element->set_container(new gras::WeakContainerSharedPtr(block));
    }

    return *element;
}

void gr::hier_block2::connect(gr::basic_block_sptr block)
{
    GRASP_HIER_BLOCK->connect(get_elem_sptr(block, pimpl));
}

void gr::hier_block2::connect(
    gr::basic_block_sptr src, int src_port,
    gr::basic_block_sptr dst, int dst_port
)
{
    GRASP_HIER_BLOCK->connect(
        get_elem_sptr(src, pimpl), src_port,
        get_elem_sptr(dst, pimpl), dst_port
    );
}

void gr::hier_block2::disconnect(gr::basic_block_sptr block)
{
    GRASP_HIER_BLOCK->disconnect(get_elem_sptr(block, pimpl));
}

void gr::hier_block2::disconnect(
    gr::basic_block_sptr src, int src_port,
    gr::basic_block_sptr dst, int dst_port
)
{
    GRASP_HIER_BLOCK->disconnect(
        get_elem_sptr(src, pimpl), src_port,
        get_elem_sptr(dst, pimpl), dst_port
    );
}

void gr::hier_block2::disconnect_all()
{
    GRASP_HIER_BLOCK->disconnect_all();
}

//TODO -- use GRAS's builtin message passing capability

void gr::hier_block2::msg_connect(basic_block_sptr src, pmt::pmt_t srcport,
                 basic_block_sptr dst, pmt::pmt_t dstport)
{
    throw std::runtime_error("msg no");
}
void gr::hier_block2::msg_connect(basic_block_sptr src, std::string srcport,
                 basic_block_sptr dst, std::string dstport)
{
    throw std::runtime_error("msg no");
}
void gr::hier_block2::msg_disconnect(basic_block_sptr src, pmt::pmt_t srcport,
                 basic_block_sptr dst, pmt::pmt_t dstport)
{
    throw std::runtime_error("msg no");
}
void gr::hier_block2::msg_disconnect(basic_block_sptr src, std::string srcport,
                 basic_block_sptr dst, std::string dstport)
{
    throw std::runtime_error("msg no");
}

gr::hier_block2_sptr gr::hier_block2::to_hier_block2()
{
    return boost::static_pointer_cast<gr::hier_block2>(shared_from_this());
}
