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

#ifndef INCLUDED_GNURADIO_GR_BLOCK_H
#define INCLUDED_GNURADIO_GR_BLOCK_H

#include <gr_core_api.h>
#include <gras/block.hpp>
#include <gr_io_signature.h>
#include <gr_types.h>
#include <gr_tags.h>
#include <string>

typedef std::vector<int> gr_vector_int;
typedef std::vector<void *> gr_vector_void_star;
typedef std::vector<const void *> gr_vector_const_void_star;

namespace gnuradio
{
//! dummy entry, just here for legacy purposes
template <typename T>
boost::shared_ptr<T> get_initial_sptr(T *p)
{
    return boost::shared_ptr<T>(p);
}
}

struct GR_CORE_API gr_block : gras::Block
{

    gr_block(void);

    gr_block(
        const std::string &name,
        gr_io_signature_sptr input_signature,
        gr_io_signature_sptr output_signature
    );

    template <typename T> void set_msg_handler(T msg_handler){/*LOL*/}

    //! implements work -> calls general work
    int work(
        const InputItems &input_items,
        const OutputItems &output_items
    );

    //! Overload me! I am the forecast
    virtual void forecast(int, std::vector<int> &);

    /*!
    * \brief compute output items from input items
    *
    * \param noutput_items	number of output items to write on each output stream
    * \param ninput_items	number of input items available on each input stream
    * \param input_items		vector of pointers to the input items, one entry per input stream
    * \param output_items	vector of pointers to the output items, one entry per output stream
    *
    * \returns number of items actually written to each output stream, or -1 on EOF.
    * It is OK to return a value less than noutput_items.  -1 <= return value <= noutput_items
    *
    * general_work must call consume or consume_each to indicate how many items
    * were consumed on each input stream.
    */
    virtual int general_work(
        int noutput_items,
        gr_vector_int &ninput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items
    );

    gr_vector_int _work_ninput_items;
    gr_vector_const_void_star _work_input_items;
    gr_vector_void_star _work_output_items;
    ptrdiff_t _work_io_ptr_mask;

    void add_item_tag(
        const size_t which_output, const gr_tag_t &tag
    );

    void add_item_tag(
        const size_t which_output,
        uint64_t abs_offset,
        const pmt::pmt_t &key,
        const pmt::pmt_t &value,
        const pmt::pmt_t &srcid=pmt::PMT_F
    );

    void get_tags_in_range(
        std::vector<gr_tag_t> &tags,
        const size_t which_input,
        uint64_t abs_start,
        uint64_t abs_end,
        const pmt::pmt_t &key = pmt::pmt_t()
    );

    unsigned history(void) const;

    void set_history(unsigned history);

    void set_alignment(const size_t alignment);

    bool is_unaligned(void);

    size_t fixed_rate_noutput_to_ninput(const size_t noutput_items);

    size_t interpolation(void) const;

    void set_interpolation(const size_t);

    size_t decimation(void) const;

    void set_decimation(const size_t);

    int max_noutput_items(void) const;

    void set_max_noutput_items(int);

    void unset_max_noutput_items(void);

    bool is_set_max_noutput_items(void) const;

    ///////////// TODO //////////////////////
    void set_max_output_buffer(long){}
    void set_max_output_buffer(int, long){}
    long max_output_buffer(size_t){return 0;}
    void set_min_output_buffer(long){}
    void set_min_output_buffer(int, long){}
    long min_output_buffer(size_t){return 0;}

};

typedef boost::shared_ptr<gr_block> gr_block_sptr;

#endif /*INCLUDED_GNURADIO_GR_BLOCK_H*/
