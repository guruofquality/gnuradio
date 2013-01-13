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

#ifndef INCLUDED_GNURADIO_GR_TOP_BLOCK_H
#define INCLUDED_GNURADIO_GR_TOP_BLOCK_H

#include <gr_core_api.h>
#include <gras/top_block.hpp>
#include <gr_hier_block2.h>

struct GR_CORE_API gr_top_block : gras::TopBlock
{

    gr_top_block(void);

    gr_top_block(const std::string &name);

};

typedef boost::shared_ptr<gr_top_block> gr_top_block_sptr;

GR_CORE_API gr_top_block_sptr gr_make_top_block(const std::string &name);

inline gr_top_block::gr_top_block(void):
    //cannot make a null top block, use name constructor
    gras::TopBlock("top")
{
    //NOP
}

inline gr_top_block::gr_top_block(const std::string &name):
    gras::TopBlock(name)
{
    //NOP
}

inline gr_top_block_sptr gr_make_top_block(const std::string &name)
{
    return gr_top_block_sptr(new gr_top_block(name));
}

#endif /*INCLUDED_GNURADIO_GR_TOP_BLOCK_H*/
