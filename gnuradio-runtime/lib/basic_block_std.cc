/* -*- c++ -*- */
/*
 * Copyright 2006,2012-2013 Free Software Foundation, Inc.
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

#include <gnuradio/basic_block.h>
#include <gnuradio/block_registry.h>
#include <stdexcept>
#include <sstream>
#include <iostream>

namespace gr {

  //  - publish a message on a message port
  void basic_block::message_port_pub(pmt::pmt_t port_id, pmt::pmt_t msg)
  {
    if(!pmt::dict_has_key(message_subscribers, port_id)) {
      throw std::runtime_error("port does not exist");
    }
  
    pmt::pmt_t currlist = pmt::dict_ref(message_subscribers, port_id, pmt::PMT_NIL);
    // iterate through subscribers on port
    while(pmt::is_pair(currlist)) {
      pmt::pmt_t target = pmt::car(currlist);

      pmt::pmt_t block = pmt::car(target);
      pmt::pmt_t port = pmt::cdr(target);
    
      currlist = pmt::cdr(currlist);
      basic_block_sptr blk = global_block_registry.block_lookup(block);
      //blk->post(msg);
      blk->post(port, msg);
    }
  }

}
