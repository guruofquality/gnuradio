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
#include "gras/pmx_helper.h"

//FIXME - this is temp until private vars make it into the private implementation
#define private public
#include <gnuradio/block.h>
#undef private

#include <boost/foreach.hpp>

/***********************************************************************
 * The block wrapper inherits a gras block and calls into a gr block
 **********************************************************************/
struct gras_block_wrapper : gras::Block
{
    gras_block_wrapper(const std::string &name, gr::block *block_ptr):
        gras::Block(name), d_block_ptr(block_ptr)
    {
        //NOP
    }

    void work(
        const InputItems &input_items,
        const OutputItems &output_items
    );

    void propagate_tags(const size_t which_input, const gras::TagIter &iter);

    void notify_active(void)
    {
        d_block_ptr->start();
    }

    void notify_inactive(void)
    {
        d_block_ptr->stop();
    }

    void notify_topology(const size_t num_inputs, const size_t num_outputs)
    {
        d_num_outputs = num_outputs;
        d_fcast_ninput_items.resize(num_inputs);
        d_work_ninput_items.resize(num_inputs);
        d_block_ptr->check_topology(num_inputs, num_outputs);
    }

    gras::BufferQueueSptr input_buffer_allocator(const size_t, const gras::SBufferConfig &config);
    gras::BufferQueueSptr output_buffer_allocator(const size_t, const gras::SBufferConfig &config);

    gr::block *d_block_ptr;
    gr_vector_int d_work_ninput_items;
    gr_vector_int d_fcast_ninput_items;
    size_t d_num_outputs;
};

void gras_block_wrapper::work(
    const InputItems &input_items,
    const OutputItems &output_items
)
{
    ptrdiff_t work_io_ptr_mask = 0;
    #define REALLY_BIG size_t(1 << 30)
    const size_t num_inputs = input_items.size();
    const size_t num_outputs = output_items.size();

    //------------------------------------------------------------------
    //-- initialize input buffers before work
    //------------------------------------------------------------------
    size_t num_input_items = input_items.min();
    if (d_block_ptr->d_fixed_rate) num_input_items -= (d_block_ptr->d_history - 1);
    for (size_t i = 0; i < num_inputs; i++)
    {
        d_work_ninput_items[i] = input_items[i].size();
        work_io_ptr_mask |= ptrdiff_t(input_items.vec()[i]);
        if GRAS_UNLIKELY(d_block_ptr->d_fixed_rate and input_items[i].size() <= (d_block_ptr->d_history - 1))
        {
            return this->mark_input_fail(i);
        }
    }

    //------------------------------------------------------------------
    //-- initialize output buffers before work
    //------------------------------------------------------------------
    size_t num_output_items = output_items.min();
    num_output_items /= d_block_ptr->d_output_multiple;
    num_output_items *= d_block_ptr->d_output_multiple;
    for (size_t i = 0; i < num_outputs; i++)
    {
        work_io_ptr_mask |= ptrdiff_t(output_items.vec()[i]);
    }

    //------------------------------------------------------------------
    //-- calculate the work_noutput_items given:
    //-- min of num_input_items
    //-- min of num_output_items
    //-- relative rate and output multiple items
    //------------------------------------------------------------------
    size_t work_noutput_items = num_output_items;
    if (num_inputs and (d_block_ptr->d_fixed_rate or not num_outputs))
    {
        size_t calc_output_items = size_t(num_input_items*d_block_ptr->d_relative_rate);
        calc_output_items += d_block_ptr->d_output_multiple-1;
        calc_output_items /= d_block_ptr->d_output_multiple;
        calc_output_items *= d_block_ptr->d_output_multiple;
        if (calc_output_items and calc_output_items < work_noutput_items)
            work_noutput_items = calc_output_items;
    }

    //------------------------------------------------------------------
    //-- forecast
    //------------------------------------------------------------------
    if (num_inputs or num_outputs)
    {
        forecast_again_you_jerk:
        d_fcast_ninput_items = d_work_ninput_items; //init for NOP case
        d_block_ptr->forecast(work_noutput_items, d_fcast_ninput_items);
        for (size_t i = 0; i < input_items.size(); i++)
        {
            if GRAS_LIKELY(d_fcast_ninput_items[i] <= d_work_ninput_items[i]) continue;

            //handle the case of forecast failing
            if GRAS_UNLIKELY(work_noutput_items <= size_t(d_block_ptr->d_output_multiple))
            {
                return this->mark_input_fail(i);
            }

            work_noutput_items = work_noutput_items/2; //backoff regime
            work_noutput_items += d_block_ptr->d_output_multiple-1;
            work_noutput_items /= d_block_ptr->d_output_multiple;
            work_noutput_items *= d_block_ptr->d_output_multiple;
            goto forecast_again_you_jerk;
        }
    }

    //I guess this is deprecated -- so dont do this
    //TODO update d_unaligned and d_is_unaligned based off of work_io_ptr_mask
    //d_block_ptr->d_unaligned?

    const int work_ret = d_block_ptr->general_work(
        work_noutput_items,
        d_work_ninput_items,
        const_cast<InputItems &>(input_items).vec(),
        const_cast<OutputItems &>(output_items).vec()
    );

    if GRAS_LIKELY(work_ret > 0) for (size_t i = 0; i < num_outputs; i++)
    {
        this->produce(i, work_ret);
    }

    if GRAS_UNLIKELY(work_ret == -1) this->mark_done();
}

static inline unsigned long long myullround(const double x)
{
    return (unsigned long long)(x + 0.5);
}

void gras_block_wrapper::propagate_tags(
    const size_t which_input,
    const gras::TagIter &iter
)
{
    switch (d_block_ptr->d_tag_propagation_policy)
    {
    case gr::block::TPP_DONT: break; //well that was ez
    case gr::block::TPP_ALL_TO_ALL:
        for (size_t out_i = 0; out_i < d_num_outputs; out_i++)
        {
            BOOST_FOREACH(gras::Tag t, iter)
            {
                t.offset = myullround(t.offset * d_block_ptr->d_relative_rate);
                this->post_output_tag(out_i, t);
            }
        }
        break;
    case gr::block::TPP_ONE_TO_ONE:
        if (which_input < d_num_outputs)
        {
            BOOST_FOREACH(gras::Tag t, iter)
            {
                t.offset = myullround(t.offset * d_block_ptr->d_relative_rate);
                this->post_output_tag(which_input, t);
            }
        }
        break;
    };
}

gras::BufferQueueSptr gras_block_wrapper::input_buffer_allocator(const size_t, const gras::SBufferConfig &config)
{
    if ((d_block_ptr->d_history - 1) != 0)
    {
        return gras::BufferQueue::make_circ(config, 32/*many*/);
    }
    return gras::BufferQueueSptr();
}

gras::BufferQueueSptr gras_block_wrapper::output_buffer_allocator(const size_t which, const gras::SBufferConfig &config)
{
    return gras::Block::output_buffer_allocator(which, config);
}

/***********************************************************************
 * gr block routines below
 **********************************************************************/
gr::block::block(
    const std::string &name,
    gr::io_signature::sptr input_signature,
    gr::io_signature::sptr output_signature
)
{
    GRASP_INIT();
    GRASP.block.reset(new gras_block_wrapper(name, this));

    //this initializes private vars, order matters
    this->set_fixed_rate(false);
    this->set_output_multiple(1);
    this->set_history(1);
    this->set_relative_rate(1.0);
    this->set_input_signature(input_signature);
    this->set_output_signature(output_signature);
}

gr::block::~block(void)
{
    //NOP
}

bool gr::block::start(void)
{
    return true;
}

bool gr::block::stop(void)
{
    return true;
}

std::ostream& gr::operator << (std::ostream& os, const gr::block *m)
{
    os << "<block " << m->name() << " (" << m->unique_id() << ")>";
    return os;
}

int gr::block::general_work(int noutput_items,
                  gr_vector_int &ninput_items,
                  gr_vector_const_void_star &input_items,
                  gr_vector_void_star &output_items)
{
    throw std::runtime_error("block::general_work() not implemented");
    return 0;
}

void gr::block::consume_each(const int how_many_items)
{
    if GRAS_UNLIKELY(how_many_items < 0) return;
    GRASP.block->consume(size_t(how_many_items));
}

void gr::block::consume(const int i, const int how_many_items)
{
    if GRAS_UNLIKELY(how_many_items < 0) return;
    GRASP.block->consume(i, size_t(how_many_items));
}

void gr::block::produce(const int o, const int how_many_items)
{
    if GRAS_UNLIKELY(how_many_items < 0) return;
    GRASP.block->produce(o, size_t(how_many_items));
}

uint64_t gr::block::nitems_read(unsigned int which_input)
{
    return GRASP.block->get_consumed(which_input);
}

uint64_t gr::block::nitems_written(unsigned int which_output)
{
    return GRASP.block->get_produced(which_output);
}

//I guess this is deprecated
void gr::block::set_unaligned(int){}

static gr::tag_t Tag2gr_tag(const gras::Tag &tag)
{
    gr::tag_t t;
    t.offset = tag.offset;
    const gras::StreamTag &st = tag.object.as<gras::StreamTag>();
    t.key = pmt::pmc_to_pmt(st.key);
    t.value = pmt::pmc_to_pmt(st.val);
    t.srcid = pmt::pmc_to_pmt(st.src);
    return t;
}

static gras::Tag gr_tag2Tag(const gr::tag_t &tag)
{
    return gras::Tag
    (
        tag.offset,
        PMC_M(gras::StreamTag(
            pmt::pmt_to_pmc(tag.key),
            pmt::pmt_to_pmc(tag.value),
            pmt::pmt_to_pmc(tag.srcid)
        ))
    );
}

void gr::block::add_item_tag(
    unsigned int which_output, const gr::tag_t &tag
){
    GRASP.block->post_output_tag(which_output, gr_tag2Tag(tag));
}

void gr::block::get_tags_in_range(
    std::vector<gr::tag_t> &tags,
    unsigned int which_input,
    uint64_t abs_start,
    uint64_t abs_end,
    const pmt::pmt_t &key
){
    tags.clear();
    BOOST_FOREACH(const gras::Tag &tag, GRASP.block->get_input_tags(which_input))
    {
        if (tag.offset >= abs_start and tag.offset <= abs_end)
        {
            gr::tag_t t = Tag2gr_tag(tag);
            if (not key or pmt::equal(t.key, key)) tags.push_back(t);
        }
    }
}

void gr::block::get_tags_in_range(
    std::vector<gr::tag_t> &tags,
    unsigned int which_input,
    uint64_t abs_start,
    uint64_t abs_end
){
    return this->get_tags_in_range(tags, which_input, abs_start, abs_end, pmt::PMT_NIL);
}

void gr::block::remove_item_tag(unsigned int which_input, const tag_t &tag)
{
    //TODO is this a thing now?
    //Either add it to GRAS or filter this tag when doing propagate
}
