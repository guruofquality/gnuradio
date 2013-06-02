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

#include "pmx_helper.hpp"
#include <gr_block.h>
#include <boost/foreach.hpp>
#include <iostream>
#include <boost/detail/atomic_count.hpp>

static boost::detail::atomic_count unique_id_pool(0);

gr_block::gr_block(void)
{
    //NOP
}

gr_block::gr_block(
    const std::string &name,
    gr_io_signature_sptr input_signature,
    gr_io_signature_sptr output_signature
):
    gras::Block(name),
    _unique_id(++unique_id_pool),
    _name(name)
{
    //this initializes private vars, order matters
    this->set_fixed_rate(false);
    this->set_output_multiple(1);
    this->set_history(1);
    this->set_relative_rate(1.0);
    this->set_decimation(1);
    this->set_interpolation(1);
    this->set_tag_propagation_policy(TPP_ALL_TO_ALL);
    this->set_input_signature(input_signature);
    this->set_output_signature(output_signature);
}

gr_io_signature_sptr gr_block::input_signature(void) const
{
    return _in_sig;
}

gr_io_signature_sptr gr_block::output_signature(void) const
{
    return _out_sig;
}

void gr_block::set_input_signature(gr_io_signature_sptr sig)
{
    for (size_t i = 0; i < sig->sizeof_stream_items().size(); i++)
    {
        this->input_config(i).item_size = sig->sizeof_stream_items().at(i);
    }
    _in_sig = sig;
}

void gr_block::set_output_signature(gr_io_signature_sptr sig)
{
    for (size_t i = 0; i < sig->sizeof_stream_items().size(); i++)
    {
        this->output_config(i).item_size = sig->sizeof_stream_items().at(i);
    }
    _out_sig = sig;
}

gr_block::~gr_block(void)
{
    //NOP
}

void gr_block::notify_active(void)
{
    this->start();
}

bool gr_block::start(void)
{
    return true;
}

void gr_block::notify_inactive(void)
{
    this->stop();
}

bool gr_block::stop(void)
{
    return true;
}

void gr_block::notify_topology(const size_t num_inputs, const size_t num_outputs)
{
    _num_outputs = num_outputs;
    _fcast_ninput_items.resize(num_inputs);
    _work_ninput_items.resize(num_inputs);
    this->check_topology(num_inputs, num_outputs);
}

bool gr_block::check_topology(int, int)
{
    return true;
}

void gr_block::work(
    const InputItems &input_items,
    const OutputItems &output_items
){
    _work_io_ptr_mask = 0;
    #define REALLY_BIG size_t(1 << 30)
    const size_t num_inputs = input_items.size();
    const size_t num_outputs = output_items.size();

    //------------------------------------------------------------------
    //-- initialize input buffers before work
    //------------------------------------------------------------------
    size_t num_input_items = input_items.min();
    if (_enable_fixed_rate) num_input_items -= _input_history_items;
    for (size_t i = 0; i < num_inputs; i++)
    {
        _work_ninput_items[i] = input_items[i].size();
        _work_io_ptr_mask |= ptrdiff_t(input_items.vec()[i]);
        if GRAS_UNLIKELY(_enable_fixed_rate and input_items[i].size() <= _input_history_items)
        {
            return this->mark_input_fail(i);
        }
    }

    //------------------------------------------------------------------
    //-- initialize output buffers before work
    //------------------------------------------------------------------
    size_t num_output_items = output_items.min();
    num_output_items /= _output_multiple_items;
    num_output_items *= _output_multiple_items;
    for (size_t i = 0; i < num_outputs; i++)
    {
        _work_io_ptr_mask |= ptrdiff_t(output_items.vec()[i]);
    }

    //------------------------------------------------------------------
    //-- calculate the work_noutput_items given:
    //-- min of num_input_items
    //-- min of num_output_items
    //-- relative rate and output multiple items
    //------------------------------------------------------------------
    size_t work_noutput_items = num_output_items;
    if (num_inputs and (_enable_fixed_rate or not num_outputs))
    {
        size_t calc_output_items = size_t(num_input_items*_relative_rate);
        calc_output_items += _output_multiple_items-1;
        calc_output_items /= _output_multiple_items;
        calc_output_items *= _output_multiple_items;
        if (calc_output_items and calc_output_items < work_noutput_items)
            work_noutput_items = calc_output_items;
    }

    //------------------------------------------------------------------
    //-- forecast
    //------------------------------------------------------------------
    if (num_inputs or num_outputs)
    {
        forecast_again_you_jerk:
        _fcast_ninput_items = _work_ninput_items; //init for NOP case
        this->forecast(work_noutput_items, _fcast_ninput_items);
        for (size_t i = 0; i < input_items.size(); i++)
        {
            if GRAS_LIKELY(_fcast_ninput_items[i] <= _work_ninput_items[i]) continue;

            //handle the case of forecast failing
            if GRAS_UNLIKELY(work_noutput_items <= _output_multiple_items)
            {
                return this->mark_input_fail(i);
            }

            work_noutput_items = work_noutput_items/2; //backoff regime
            work_noutput_items += _output_multiple_items-1;
            work_noutput_items /= _output_multiple_items;
            work_noutput_items *= _output_multiple_items;
            goto forecast_again_you_jerk;
        }
    }

    const int work_ret = this->general_work(
        work_noutput_items,
        _work_ninput_items,
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

void gr_block::propagate_tags(const size_t which_input, const gras::TagIter &iter)
{
    switch (_tag_prop_policy)
    {
    case TPP_DONT: break; //well that was ez
    case TPP_ALL_TO_ALL:
        for (size_t out_i = 0; out_i < _num_outputs; out_i++)
        {
            BOOST_FOREACH(gras::Tag t, iter)
            {
                t.offset = myullround(t.offset * _relative_rate);
                this->post_output_tag(out_i, t);
            }
        }
        break;
    case TPP_ONE_TO_ONE:
        if (which_input < _num_outputs)
        {
            BOOST_FOREACH(gras::Tag t, iter)
            {
                t.offset = myullround(t.offset * _relative_rate);
                this->post_output_tag(which_input, t);
            }
        }
        break;
    };
}

void gr_block::forecast(int noutput_items, std::vector<int> &ninputs_req)
{
    for (size_t i = 0; i < ninputs_req.size(); i++)
    {
        ninputs_req[i] = fixed_rate_noutput_to_ninput(noutput_items);
    }
}

int gr_block::general_work(
    int noutput_items,
    gr_vector_int &ninput_items,
    gr_vector_const_void_star &input_items,
    gr_vector_void_star &output_items
){
    throw std::runtime_error("gr_block subclasses must overload general_work!");
}

void gr_block::set_alignment(const size_t)
{
    //TODO
    //probably dont need this since buffers always start aligned
    //and therefore alignment is always re-acheived
}

size_t gr_block::fixed_rate_noutput_to_ninput(const size_t noutput_items)
{
    return ((decimation()*noutput_items)/interpolation()) + _input_history_items;
}

void gr_block::set_interpolation(const size_t interp)
{
    _interp = interp;
    this->set_relative_rate(1.0*interp);
    this->set_output_multiple(interp);
}

void gr_block::set_decimation(const size_t decim)
{
    _decim = decim;
    this->set_relative_rate(1.0/decim);
}

unsigned gr_block::history(void) const
{
    //implement off-by-one history compat
    return _input_history_items+1;
}

void gr_block::set_history(unsigned history)
{
    //implement off-by-one history compat
    if (history == 0) history++;
    _input_history_items = history-1;
    this->input_config(0).preload_items = _input_history_items;
    this->commit_config();
}

void gr_block::set_fixed_rate(const bool fixed_rate)
{
    _enable_fixed_rate = fixed_rate;
}

bool gr_block::fixed_rate(void) const
{
    return _enable_fixed_rate;
}

void gr_block::_update_input_reserve(void)
{
    /*!
     * Set an input reserve for fixed rate blocks.
     *
     * FIXME: Also do this when output multiple is large,
     * This makes gr-trellis pass under conditions where not fixed rate set,
     * but the output multiple is so large that default input isnt sufficient.
     */
    if (_enable_fixed_rate or _output_multiple_items > 1024)
    {
        const size_t reserve = size_t(0.5 + _output_multiple_items/_relative_rate);
        if (reserve) this->input_config(0).reserve_items = reserve;
    }
}

void gr_block::set_output_multiple(const size_t multiple)
{
    _output_multiple_items = multiple;
    this->output_config(0).reserve_items = multiple;
    this->_update_input_reserve();
}

size_t gr_block::output_multiple(void) const
{
    return _output_multiple_items;
}

void gr_block::set_relative_rate(double relative_rate)
{
    _relative_rate = relative_rate;
    this->_update_input_reserve();
}

double gr_block::relative_rate(void) const
{
    return _relative_rate;
}

int gr_block::max_noutput_items(void) const
{
    return this->output_config(0).maximum_items;
}

void gr_block::set_max_noutput_items(int max_items)
{
    this->output_config(0).maximum_items = max_items;
}

void gr_block::unset_max_noutput_items(void)
{
    this->set_max_noutput_items(0);
}

bool gr_block::is_set_max_noutput_items(void) const
{
    return this->max_noutput_items() != 0;
}

static gr_tag_t Tag2gr_tag(const gras::Tag &tag)
{
    gr_tag_t t;
    t.offset = tag.offset;
    const gras::StreamTag &st = tag.object.as<gras::StreamTag>();
    t.key = pmt::pmc_to_pmt(st.key);
    t.value = pmt::pmc_to_pmt(st.val);
    t.srcid = pmt::pmc_to_pmt(st.src);
    return t;
}

static gras::Tag gr_tag2Tag(const gr_tag_t &tag)
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

void gr_block::add_item_tag(
    const size_t which_output, const gr_tag_t &tag
){
    this->post_output_tag(which_output, gr_tag2Tag(tag));
}

void gr_block::add_item_tag(
    const size_t which_output,
    uint64_t abs_offset,
    const pmt::pmt_t &key,
    const pmt::pmt_t &value,
    const pmt::pmt_t &srcid
){
    gr_tag_t t;
    t.offset = abs_offset;
    t.key = key;
    t.value = value;
    t.srcid = srcid;
    this->add_item_tag(which_output, t);
}

void gr_block::get_tags_in_range(
    std::vector<gr_tag_t> &tags,
    const size_t which_input,
    uint64_t abs_start,
    uint64_t abs_end,
    const pmt::pmt_t &key
){
    tags.clear();
    BOOST_FOREACH(const gras::Tag &tag, this->get_input_tags(which_input))
    {
        if (tag.offset >= abs_start and tag.offset <= abs_end)
        {
            gr_tag_t t = Tag2gr_tag(tag);
            if (not key or pmt::pmt_equal(t.key, key)) tags.push_back(t);
        }
    }
}

gr_block::tag_propagation_policy_t gr_block::tag_propagation_policy(void)
{
    return _tag_prop_policy;
}

void gr_block::set_tag_propagation_policy(gr_block::tag_propagation_policy_t p)
{
    _tag_prop_policy = p;
}

gras::BufferQueueSptr gr_block::input_buffer_allocator(const size_t, const gras::SBufferConfig &config)
{
    if (_input_history_items)
    {
        return gras::BufferQueue::make_circ(config, 32/*many*/);
    }
    return gras::BufferQueueSptr();
}

gras::BufferQueueSptr gr_block::output_buffer_allocator(const size_t which, const gras::SBufferConfig &config)
{
    return gras::Block::output_buffer_allocator(which, config);
}
