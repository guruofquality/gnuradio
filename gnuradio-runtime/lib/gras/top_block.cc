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

#include "gras/basic_block_pimpl.h"

#include <gnuradio/top_block.h>

gr::top_block_sptr gr::make_top_block(const std::string &name)
{
    return gr::top_block_sptr(new gr::top_block(name));
}

gr::top_block::top_block(
    const std::string &name
)
{
    GRASP_INIT();
    GRASP.top_block.reset(new gras::TopBlock(name));
}

gr::top_block::~top_block(void)
{
    //NOP
}

void gr::top_block::lock(void)
{
    //dont tear down the flow graph
}

void gr::top_block::unlock(void)
{
    //thread safe commit topology changes
    GRASP.top_block->commit();
}

void gr::top_block::setup_rpc()
{
    throw std::runtime_error("top block setup_rpc not implemented");
}

void gr::top_block::start(int max_items)
{
    this->set_max_noutput_items(max_items);
    GRASP.top_block->start();
}

void gr::top_block::run(int max_items)
{
    this->set_max_noutput_items(max_items);
    GRASP.top_block->run();
}

int gr::top_block::max_noutput_items(void)
{
    return GRASP.top_block->global_config().maximum_output_items;
}

void gr::top_block::set_max_noutput_items(int max_items)
{
    GRASP.top_block->global_config().maximum_output_items = max_items;
}

void gr::top_block::stop(void)
{
    GRASP.top_block->stop();
}

void gr::top_block::wait(void)
{
    GRASP.top_block->wait();
}
