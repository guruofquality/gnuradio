/* -*- c++ -*- */
/*
 * Copyright 2008,2009,2013 Free Software Foundation, Inc.
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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/////////// nothing but a passthrough for silliness //////////////////

#include <gnuradio/basic_block.h>
#include <gnuradio/sptr_magic.h>

namespace gnuradio {

  void
  detail::sptr_magic::create_and_stash_initial_sptr(gr::hier_block2 *)
  {
  }

  gr::basic_block_sptr
  detail::sptr_magic::fetch_initial_sptr(gr::basic_block *p)
  {
    return gr::basic_block_sptr(p);
  }
};
