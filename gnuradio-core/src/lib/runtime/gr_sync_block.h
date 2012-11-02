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

    //! implements work -> calls work
    inline int general_work(
        int noutput_items,
        gr_vector_int &ninput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items
    ){
        const int work_ret = this->work(noutput_items, input_items, output_items);
        if (work_ret > 0)
        {
            this->consume_each(size_t(0.5+(work_ret/this->relative_rate())));
        }
        return work_ret;
    }

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

#endif /*INCLUDED_GNURADIO_GR_SYNC_BLOCK_H*/
