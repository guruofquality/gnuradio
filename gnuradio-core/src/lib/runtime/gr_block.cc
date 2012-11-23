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

#include "pmx_helper.hpp"
#include <gr_block.h>
#include <boost/foreach.hpp>
#include <iostream>

gr_block::gr_block(void)
{
    //NOP
}

gr_block::gr_block(
    const std::string &name,
    gr_io_signature_sptr input_signature,
    gr_io_signature_sptr output_signature
):
    gras::Block(name)
{
    this->set_fixed_rate(false);
    this->set_output_multiple(1);
    this->set_history(1);
    this->set_relative_rate(1.0);
    this->set_tag_propagation_policy(TPP_ALL_TO_ALL);
    this->set_input_signature(input_signature);
    this->set_output_signature(output_signature);
}

void gr_block::notify_topology(const size_t num_inputs, const size_t num_outputs)
{
    _fcast_ninput_items.resize(num_inputs);
    _work_ninput_items.resize(num_inputs);
    _work_input_items.resize(num_inputs);
    _work_output_items.resize(num_outputs);
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
    size_t num_input_items = REALLY_BIG; //so big that it must std::min
    for (size_t i = 0; i < num_inputs; i++)
    {
        _work_ninput_items[i] = input_items[i].size();
        _work_input_items[i] = input_items[i].get();
        _work_io_ptr_mask |= ptrdiff_t(_work_input_items[i]);
        size_t items = input_items[i].size();
        if (_enable_fixed_rate)
        {
            if (items <= _input_history_items)
            {
                return this->mark_input_fail(i);
            }
            items -= _input_history_items;
        }

        num_input_items = std::min(num_input_items, items);
    }

    //------------------------------------------------------------------
    //-- initialize output buffers before work
    //------------------------------------------------------------------
    size_t num_output_items = REALLY_BIG; //so big that it must std::min
    for (size_t i = 0; i < num_outputs; i++)
    {
        _work_output_items[i] = output_items[i].get();
        _work_io_ptr_mask |= ptrdiff_t(_work_output_items[i]);
        size_t items = output_items[i].size();
        items /= _output_multiple_items;
        items *= _output_multiple_items;
        num_output_items = std::min(num_output_items, items);
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
            if (_fcast_ninput_items[i] <= _work_ninput_items[i]) continue;

            //handle the case of forecast failing
            if (work_noutput_items <= _output_multiple_items)
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
        _work_input_items,
        _work_output_items
    );

    if (work_ret > 0) for (size_t i = 0; i < num_outputs; i++)
    {
        this->produce(i, work_ret);
    }

    if (work_ret == -1) this->mark_done();
}

static inline unsigned long long myullround(const double x)
{
    return (unsigned long long)(x + 0.5);
}

void gr_block::propagate_tags(const size_t which_input, const gras::TagIter &iter)
{
    const size_t num_outputs = _work_output_items.size();

    switch (_tag_prop_policy)
    {
    case TPP_DONT: break; //well that was ez
    case TPP_ALL_TO_ALL:
        for (size_t out_i = 0; out_i < num_outputs; out_i++)
        {
            BOOST_FOREACH(gras::Tag t, iter)
            {
                t.offset = myullround(t.offset * _relative_rate);
                this->post_output_tag(out_i, t);
            }
        }
        break;
    case TPP_ONE_TO_ONE:
        if (which_input < num_outputs)
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

void gr_block::consume_each(const int how_many_items)
{
    if (how_many_items < 0) return;
    gras::Block::consume(size_t(how_many_items));
}

void gr_block::consume(const size_t i, const int how_many_items)
{
    if (how_many_items < 0) return;
    gras::Block::consume(i, size_t(how_many_items));
}

void gr_block::produce(const size_t o, const int how_many_items)
{
    if (how_many_items < 0) return;
    gras::Block::produce(o, size_t(how_many_items));
}

uint64_t gr_block::nitems_read(const size_t which_input)
{
    return Block::get_consumed(which_input);
}

uint64_t gr_block::nitems_written(const size_t which_output)
{
    return Block::get_produced(which_output);
}

void gr_block::set_alignment(const size_t)
{
    //TODO
    //probably dont need this since buffers always start aligned
    //and therefore alignment is always re-acheived
}

bool gr_block::is_unaligned(void)
{
    //TODO
    //probably dont need this since volk dispatcher checks alignment
    //32 byte aligned is good enough for you
    return (_work_io_ptr_mask & ptrdiff_t(GRAS_MAX_ALIGNMENT-1)) != 0;
}

size_t gr_block::fixed_rate_noutput_to_ninput(const size_t noutput_items)
{
    if (this->fixed_rate())
    {
        return size_t(0.5 + (noutput_items/this->relative_rate())) + this->history() - 1;
    }
    else
    {
        return noutput_items + this->history() - 1;
    }
}

size_t gr_block::interpolation(void) const
{
    return size_t(1.0*this->relative_rate());
}

void gr_block::set_interpolation(const size_t interp)
{
    this->set_relative_rate(1.0*interp);
    this->set_output_multiple(interp);
}

size_t gr_block::decimation(void) const
{
    return size_t(1.0/this->relative_rate());
}

void gr_block::set_decimation(const size_t decim)
{
    this->set_relative_rate(1.0/decim);
}

unsigned gr_block::history(void) const
{
    //implement off-by-one history compat
    return _input_history_items+1;
}

void gr_block::set_history(unsigned history)
{
    gras::InputPortConfig config = this->get_input_config(0);
    //implement off-by-one history compat
    if (history == 0) history++;
    _input_history_items = history-1;
    config.lookahead_items = _input_history_items;
    this->set_input_config(0, config);
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
        gras::InputPortConfig config = this->get_input_config(0);
        config.reserve_items = size_t(0.5 + _output_multiple_items/_relative_rate);
        if (config.reserve_items) this->set_input_config(0, config);
    }
}

void gr_block::set_output_multiple(const size_t multiple)
{
    _output_multiple_items = multiple;
    gras::OutputPortConfig config = this->get_output_config(0);
    config.reserve_items = multiple;
    this->set_output_config(0, config);
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
    return this->get_output_config(0).maximum_items;
}

void gr_block::set_max_noutput_items(int max_items)
{
    gras::OutputPortConfig config = this->get_output_config(0);
    config.maximum_items = max_items;
    this->set_output_config(0, config);
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
    t.key = pmt::pmc_to_pmt(tag.key);
    t.value = pmt::pmc_to_pmt(tag.value);
    t.srcid = pmt::pmc_to_pmt(tag.srcid);
    return t;
}

static gras::Tag gr_tag2Tag(const gr_tag_t &tag)
{
    return gras::Tag
    (
        tag.offset,
        pmt::pmt_to_pmc(tag.key),
        pmt::pmt_to_pmc(tag.value),
        pmt::pmt_to_pmc(tag.srcid)
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
            if (key or pmt::pmt_equal(t.key, key)) tags.push_back(t);
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
