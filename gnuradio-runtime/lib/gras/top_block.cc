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

#define GRASP_TOP_BLOCK (boost::static_pointer_cast<gras::TopBlock>(this->block_pimpl))

#include "gras/gras_pimpl.h"
#include <gnuradio/top_block.h>
#include <gras/top_block.hpp>
#include <iostream>

gr::top_block_sptr gr::make_top_block(const std::string &name)
{
    return gr::top_block_sptr(new gr::top_block(name));
}

gr::top_block::top_block(
    const std::string &name
)
    : hier_block2(name,
        io_signature::make(0,0,0),
        io_signature::make(0,0,0))
{
    GRAS_PORTS_PIMPL_INIT();
    block_pimpl.reset(new gras::TopBlock(name));
}

gr::top_block::~top_block(void)
{
    block_pimpl.reset();
}

void gr::top_block::lock(void)
{
    //dont tear down the flow graph
}

void gr::top_block::unlock(void)
{
    //thread safe commit topology changes
    GRASP_TOP_BLOCK->commit();
}

void gr::top_block::setup_rpc()
{
    throw std::runtime_error("top block setup_rpc not implemented");
}

void gr::top_block::start(int max_items)
{
    this->set_max_noutput_items(max_items);
    GRASP_TOP_BLOCK->start();
}

void gr::top_block::run(int max_items)
{
    this->set_max_noutput_items(max_items);
    GRASP_TOP_BLOCK->run();
}

int gr::top_block::max_noutput_items(void)
{
    return GRASP_TOP_BLOCK->global_config().maximum_output_items;
}

void gr::top_block::set_max_noutput_items(int max_items)
{
    GRASP_TOP_BLOCK->global_config().maximum_output_items = max_items;
}

void gr::top_block::stop(void)
{
    GRASP_TOP_BLOCK->stop();
}

void gr::top_block::wait(void)
{
    GRASP_TOP_BLOCK->wait();
}

std::string gr::top_block::edge_list()
{
    return "edge list";
}

void gr::top_block::dump()
{
    //NOP
}

gr::top_block_sptr gr::top_block::to_top_block()
{
    return boost::static_pointer_cast<gr::top_block>(shared_from_this());
}
