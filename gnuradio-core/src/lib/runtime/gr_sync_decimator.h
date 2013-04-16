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

#ifndef INCLUDED_GNURADIO_GR_SYNC_DECIMATOR_H
#define INCLUDED_GNURADIO_GR_SYNC_DECIMATOR_H

#include <gr_sync_block.h>

struct GR_CORE_API gr_sync_decimator : gr_sync_block
{

    gr_sync_decimator(void);

    gr_sync_decimator(
        const std::string &name,
        gr_io_signature_sptr input_signature,
        gr_io_signature_sptr output_signature,
        const size_t decim_rate
    );

    virtual ~gr_sync_decimator(void);

};

#endif /*INCLUDED_GNURADIO_GR_SYNC_DECIMATOR_H*/
