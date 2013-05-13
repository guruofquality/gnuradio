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

#ifndef INCLUDED_LIB_GR_RUNTIME_GRAS_BASIC_BLOCK_PIMPL_H
#define INCLUDED_LIB_GR_RUNTIME_GRAS_BASIC_BLOCK_PIMPL_H

#include <gras/block.hpp>
#include <gras/hier_block.hpp>
#include <gras/top_block.hpp>

struct gras_basic_block_pimpl
{
    boost::shared_ptr<gras::Block> block;
    boost::shared_ptr<gras::HierBlock> hier_block;
    boost::shared_ptr<gras::TopBlock> top_block;
    gras::HierBlock &conn_block(void)
    {
        if (hier_block) return *hier_block;
        if (top_block) return *top_block;
        throw std::runtime_error("no conn_block");
    }
};

#define GRASP_INIT() this->pimpl.reset(new gras_basic_block_pimpl())
#define GRASP (*reinterpret_cast<gras_basic_block_pimpl *>(this->pimpl.get()))

#endif /* INCLUDED_LIB_GR_RUNTIME_GRAS_BASIC_BLOCK_PIMPL_H */
