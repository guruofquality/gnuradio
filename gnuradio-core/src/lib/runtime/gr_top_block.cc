/* -*- c++ -*- */
/*
 * Copyright 2007 Free Software Foundation, Inc.
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

#include <unistd.h>
#include <gr_top_block.h>
#include <boost/detail/atomic_count.hpp>

static boost::detail::atomic_count unique_id_pool(0);

gr_top_block::gr_top_block(void):
    //cannot make a null top block, use name constructor
    gras::TopBlock("top"),
    _unique_id(++unique_id_pool),
    _name("top")
{
    //NOP
}

gr_top_block::gr_top_block(const std::string &name):
    gras::TopBlock(name),
    _unique_id(++unique_id_pool),
    _name(name)
{
    //NOP
}

gr_top_block_sptr gr_make_top_block(const std::string &name)
{
    return gr_top_block_sptr(new gr_top_block(name));
}

void gr_top_block::start(const size_t max_items)
{
    this->set_max_noutput_items(max_items);
    this->start();
}

void gr_top_block::run(const size_t max_items)
{
    this->set_max_noutput_items(max_items);
    this->run();
}

int gr_top_block::max_noutput_items(void) const
{
    return this->global_config().maximum_output_items;
}

void gr_top_block::set_max_noutput_items(int max_items)
{
    this->global_config().maximum_output_items = max_items;
}

void gr_top_block::run(void)
{
    gras::TopBlock::run();
}

void gr_top_block::start(void)
{
    gras::TopBlock::start();
}

void gr_top_block::stop(void)
{
    gras::TopBlock::stop();
}

void gr_top_block::wait(void)
{
    gras::TopBlock::wait();
}
