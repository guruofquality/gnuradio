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

#ifndef INCLUDED_GNURADIO_GR_SYNC_BLOCK_H
#define INCLUDED_GNURADIO_GR_SYNC_BLOCK_H

#include <gr_block.h>

struct GR_CORE_API gr_sync_block : public gr_block
{
    gr_sync_block(void);

    gr_sync_block(
        const std::string &name,
        gr_io_signature_sptr input_signature,
        gr_io_signature_sptr output_signature
    );

    virtual ~gr_sync_block(void);

    //! implements work -> calls work
    int general_work(
        int noutput_items,
        gr_vector_int &ninput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items
    );

   /*!
    * \brief just like gr_block::general_work, only this arranges to call consume_each for you
    *
    * The user must override work to define the signal processing code
    */
    virtual int work(
        int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items
    );

};

GRAS_FORCE_INLINE int gr_sync_block::general_work(
    int noutput_items,
    gr_vector_int &ninput_items,
    gr_vector_const_void_star &input_items,
    gr_vector_void_star &output_items
){
    const int work_ret = this->work(noutput_items, input_items, output_items);
    if (work_ret > 0)
    {
        this->consume_each((decimation()*size_t(work_ret))/interpolation());
    }
    return work_ret;
}

#endif /*INCLUDED_GNURADIO_GR_SYNC_BLOCK_H*/
