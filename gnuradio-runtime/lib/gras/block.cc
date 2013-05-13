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
#include <iostream>

/***********************************************************************
 * The block wrapper inherits a gras block and calls into a gr block
 **********************************************************************/
struct gras_block_wrapper : gras::Block
{
    gras_block_wrapper(const std::string &name, gr::block *block_ptr):
        gras::Block(name), d_block_ptr(block_ptr), d_history(0)
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
        if (d_block_ptr) d_block_ptr->start();
    }

    void notify_inactive(void)
    {
        if (d_block_ptr) d_block_ptr->stop();
    }

    void notify_topology(const size_t num_inputs, const size_t num_outputs)
    {
        if (not d_block_ptr) return;

        //this is where history is loaded into the preload
        d_history = d_block_ptr->d_history - 1;
        this->input_config(0).preload_items = d_history;
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
    size_t d_history;
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
    if (d_block_ptr->d_fixed_rate) num_input_items -= d_history;
    for (size_t i = 0; i < num_inputs; i++)
    {
        d_work_ninput_items[i] = input_items[i].size();
        work_io_ptr_mask |= ptrdiff_t(input_items.vec()[i]);
        if GRAS_UNLIKELY(d_block_ptr->d_fixed_rate and input_items[i].size() <= d_history)
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

gras::BufferQueueSptr gras_block_wrapper::input_buffer_allocator(const size_t which, const gras::SBufferConfig &config)
{
    if (d_history != 0)
    {
        return gras::BufferQueue::make_circ(config, 32/*many*/);
    }
    return gras::Block::input_buffer_allocator(which, config);
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
): basic_block(name, input_signature, output_signature),
      d_output_multiple (1),
      d_output_multiple_set(false),
      d_unaligned(0),
      d_is_unaligned(false),
      d_relative_rate (1.0),
      d_history(1),
      d_fixed_rate(false),
      d_max_noutput_items_set(false),
      d_max_noutput_items(0),
      d_min_noutput_items(0),
      d_tag_propagation_policy(TPP_ALL_TO_ALL),
      d_pc_rpc_set(false),
      d_max_output_buffer(std::max(output_signature->max_streams(),1), -1),
      d_min_output_buffer(std::max(output_signature->max_streams(),1), -1)
{
    GRASP_INIT();
    GRASP.block.reset(new gras_block_wrapper(name, this));

    for (size_t i = 0; i < input_signature->sizeof_stream_items().size(); i++)
    {
        GRASP.block->input_config(i).item_size = input_signature->sizeof_stream_items().at(i);
    }
    for (size_t i = 0; i < output_signature->sizeof_stream_items().size(); i++)
    {
        GRASP.block->output_config(i).item_size = output_signature->sizeof_stream_items().at(i);
    }
}

gr::block::~block(void)
{
    boost::static_pointer_cast<gras_block_wrapper>(GRASP.block)->d_block_ptr = NULL;
    pimpl.reset();
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

void gr::block::forecast(int noutput_items, std::vector<int> &ninputs_req)
{
    for (size_t i = 0; i < ninputs_req.size(); i++)
    {
        ninputs_req[i] = fixed_rate_noutput_to_ninput(noutput_items);
    }
}

int gr::block::fixed_rate_ninput_to_noutput(int ninput)
{
    throw std::runtime_error("Unimplemented");
}

int gr::block::fixed_rate_noutput_to_ninput(int noutput)
{
    throw std::runtime_error("Unimplemented");
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

gr::block::tag_propagation_policy_t gr::block::tag_propagation_policy()
{
    return d_tag_propagation_policy;
}

void gr::block::set_tag_propagation_policy(tag_propagation_policy_t p)
{
    d_tag_propagation_policy = p;
}

bool gr::block::is_set_max_noutput_items()
{
    return d_max_noutput_items_set;
}

int gr::block::max_noutput_items()
{
    return d_max_noutput_items;
}

void gr::block::unset_max_noutput_items()
{
    d_max_noutput_items_set = false;
}

void gr::block::set_max_noutput_items(int max_items)
{
    d_max_noutput_items = max_items;
    GRASP.block->output_config(0).maximum_items = max_items;
}

void gr::block::set_unaligned(int na)
{
    // unaligned value must be less than 0 and it doesn't make sense
    // that it's larger than the alignment value.
    if((na < 0) || (na > d_output_multiple))
      throw std::invalid_argument("block::set_unaligned");

    d_unaligned = na;
}

void gr::block::set_is_unaligned(bool u)
{
    d_is_unaligned = u;
}

void gr::block::set_alignment(int multiple)
{
    //NOP
}

void gr::block::set_relative_rate(double relative_rate)
{
    if(relative_rate < 0.0)
      throw std::invalid_argument("block::set_relative_rate");

    d_relative_rate = relative_rate;
}

void gr::block::set_output_multiple(int multiple)
{
    if(multiple < 1)
      throw std::invalid_argument("block::set_output_multiple");

    d_output_multiple_set = true;
    d_output_multiple = multiple;
    GRASP.block->output_config(0).reserve_items = multiple;
}

void gr::block::set_processor_affinity(const std::vector<int> &mask)
{
    //TODO theron does not dynamically change affinities
    //this is done at thread pool setup time see gras/thread_pool.h
    d_affinity = mask;
}

void gr::block::unset_processor_affinity()
{
    //TODO theron does not dynamically change affinities
    //this is done at thread pool setup time see gras/thread_pool.h
    d_affinity.clear();
}

//holy shit, howabout a struct and a single function call

float gr::block::pc_noutput_items(){throw std::runtime_error("pc no");}
float gr::block::pc_noutput_items_avg(){throw std::runtime_error("pc no");}
float gr::block::pc_noutput_items_var(){throw std::runtime_error("pc no");}
float gr::block::pc_nproduced(){throw std::runtime_error("pc no");}
float gr::block::pc_nproduced_avg(){throw std::runtime_error("pc no");}
float gr::block::pc_nproduced_var(){throw std::runtime_error("pc no");}
float gr::block::pc_input_buffers_full(int which){throw std::runtime_error("pc no");}
float gr::block::pc_input_buffers_full_avg(int which){throw std::runtime_error("pc no");}
float gr::block::pc_input_buffers_full_var(int which){throw std::runtime_error("pc no");}
std::vector<float> gr::block::pc_input_buffers_full(){throw std::runtime_error("pc no");}
std::vector<float> gr::block::pc_input_buffers_full_avg(){throw std::runtime_error("pc no");}
std::vector<float> gr::block::pc_input_buffers_full_var(){throw std::runtime_error("pc no");}
float gr::block::pc_output_buffers_full(int which){throw std::runtime_error("pc no");}
float gr::block::pc_output_buffers_full_avg(int which){throw std::runtime_error("pc no");}
float gr::block::pc_output_buffers_full_var(int which){throw std::runtime_error("pc no");}
std::vector<float> gr::block::pc_output_buffers_full(){throw std::runtime_error("pc no");}
std::vector<float> gr::block::pc_output_buffers_full_avg(){throw std::runtime_error("pc no");}
std::vector<float> gr::block::pc_output_buffers_full_var(){throw std::runtime_error("pc no");}
float gr::block::pc_work_time(){throw std::runtime_error("pc no");}
float gr::block::pc_work_time_avg(){throw std::runtime_error("pc no");}
float gr::block::pc_work_time_var(){throw std::runtime_error("pc no");}
void gr::block::reset_perf_counters(){throw std::runtime_error("pc no");}
void gr::block::setup_pc_rpc(){throw std::runtime_error("pc no");}
