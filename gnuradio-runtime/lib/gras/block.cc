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

#define GRASP_BLOCK_(p) (dynamic_cast<gras_block_wrapper *>(p->get()))
#define GRASP_BLOCK GRASP_BLOCK_(this)

#include "gras/gras_pimpl.h"
#include "gras/pmx_helper.h"
//FIXME - this is temp until private vars make it into the private implementation
#define private public
#define protected public
#include <gnuradio/block.h>
#undef private
#undef protected
#include <gnuradio/block_registry.h>
#include <gnuradio/prefs.h>
#include <gras/block.hpp>
#include <boost/foreach.hpp>
#include <iostream>

/***********************************************************************
 * The block wrapper inherits a gras block and calls into a gr block
 **********************************************************************/
struct gras_block_wrapper : gras::Block
{
    gras_block_wrapper(const std::string &name, gr::block *block_ptr):
        gras::Block(name),
        d_block_ptr(block_ptr),
        d_history(0),
        d_shared_msg_active(false)
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
        boost::mutex::scoped_lock lock(d_shared_msg_mutex);
        d_shared_msg_active = true;
        if (d_block_ptr) d_block_ptr->start();
    }

    void notify_inactive(void)
    {
        boost::mutex::scoped_lock lock(d_shared_msg_mutex);
        d_shared_msg_active = false;
        if (d_block_ptr) d_block_ptr->stop();
    }

    void notify_topology(const size_t actual_num_inputs, const size_t actual_num_outputs)
    {
        d_actual_num_inputs = actual_num_inputs;

        if (not d_block_ptr) return;
        const size_t num_inputs = GRAS_PORTS_PIMPL(d_block_ptr)->input.num_real_ports;
        const size_t num_outputs = GRAS_PORTS_PIMPL(d_block_ptr)->output.num_real_ports;

        //setup configuration on message only ports
        for (size_t i = num_inputs; i < d_actual_num_inputs; i++)
        {
            this->input_config(i).reserve_items = 0;
            this->input_config(i).item_size = 1;
        }
        for (size_t i = num_outputs; i < actual_num_outputs; i++)
        {
            this->output_config(i).item_size = 1;
        }

        //avoid calling check_topology until fully connected
        if (size_t(d_block_ptr->input_signature()->min_streams()) > num_inputs) return;
        if (size_t(d_block_ptr->output_signature()->min_streams()) > num_outputs) return;

        //this is where history is loaded into the preload
        d_history = d_block_ptr->d_history - 1;
        this->input_config(0).preload_items = d_history;
        d_num_outputs = num_outputs;
        d_ninput_items_required.resize(num_inputs);
        d_ninput_items.resize(num_inputs);
        d_input_items.resize(num_inputs);
        d_output_items.resize(num_outputs);
        d_block_ptr->check_topology(num_inputs, num_outputs);
        d_tag_blacklist.resize(num_inputs);
    }

    gras::BufferQueueSptr input_buffer_allocator(const size_t, const gras::SBufferConfig &config);
    gras::BufferQueueSptr output_buffer_allocator(const size_t, const gras::SBufferConfig &config);

    gr::block *d_block_ptr;
    gr_vector_int d_ninput_items;
    gr_vector_int d_ninput_items_required;
    gr_vector_const_void_star d_input_items;
    gr_vector_void_star d_output_items;
    size_t d_num_outputs;
    size_t d_actual_num_inputs;
    size_t d_history;
    std::vector<std::vector<gras::Tag> > d_tag_blacklist;

    bool is_tag_blacklisted(const gras::Tag &tag, const size_t i, const bool prune = false)
    {
        std::vector<gras::Tag> &blacklist = d_tag_blacklist[i];
        for (size_t i = 0; i < blacklist.size(); i++)
        {
            if (tag.offset == blacklist[i].offset and tag.object == blacklist[i].object)
            {
                if (prune) blacklist.erase(blacklist.begin()+i);
                return true;
            }
        }
        return false;
    }

    bool handle_msgs(void);

    boost::mutex d_shared_msg_mutex;
    bool d_shared_msg_active;
};

bool gras_block_wrapper::handle_msgs(void)
{
    const size_t num_inputs = d_input_items.size();
    //const size_t num_outputs = d_output_items.size();
    gr::block *block = d_block_ptr;

    //look for input messages on upper message ports
    bool handled_msgs = false;
    for (size_t i = num_inputs; i < d_actual_num_inputs; i++)
    {
        PMCC msg = this->pop_input_msg(i);
        if (not msg) continue;
        handled_msgs = true;
        const std::string &port_name = GRAS_PORTS_PIMPL(block)->input.virtual_port_names[d_actual_num_inputs-i-1];
        block->insert_tail(pmt::string_to_symbol(port_name), pmt::pmc_to_pmt(msg));
    }
    if (not handled_msgs) return false;
    pmt::pmt_t msg;
    gr::prefs *p = gr::prefs::singleton();
    size_t max_nmsgs = static_cast<size_t>(p->get_long("DEFAULT", "max_messages", 100));
    BOOST_FOREACH(gr::basic_block::msg_queue_map_t::value_type &i, block->msg_queue) {
        // Check if we have a message handler attached before getting
        // any messages. This is mostly a protection for the unknown
        // startup sequence of the threads.
        if(block->has_msg_handler(i.first)) {
          while((msg = block->delete_head_nowait(i.first))) {
            block->dispatch_msg(i.first,msg);
          }
        }
        else {
          // If we don't have a handler but are building up messages,
          // prune the queue from the front to keep memory in check.
          if(block->nmsgs(i.first) > max_nmsgs)
            msg = block->delete_head_nowait(i.first);
        }
      }

//I guess this called dispatch but without the has handler filtering - makes python blocks work
     BOOST_FOREACH(gr::basic_block::msg_queue_map_t::value_type &i, block->msg_queue) {
            while((msg = block->delete_head_nowait(i.first))) {
              block->dispatch_msg(i.first, msg);
            }
          }

    //we did messages, not regular work, so leave, get called again
    return true;
}

void gr::basic_block::message_port_pub(pmt::pmt_t port_id, pmt::pmt_t msg)
{
    gr::block *block = dynamic_cast<gr::block *>(this);
    if (block == NULL) throw std::runtime_error("message_port_pub cast says no block");

    boost::mutex::scoped_lock lock(GRASP_BLOCK_(block)->d_shared_msg_mutex);
    if (not GRASP_BLOCK_(block)->d_shared_msg_active) return;

    const size_t index = GRAS_PORTS_PIMPL(block)->output.get_index(pmt::symbol_to_string(port_id));
    GRASP_BLOCK->post_output_msg(index, pmt::pmt_to_pmc(msg));
}

  inline static unsigned int
  round_up(unsigned int n, unsigned int multiple)
  {
    return ((n + multiple - 1) / multiple) * multiple;
  }

  inline static unsigned int
  round_down(unsigned int n, unsigned int multiple)
  {
    return (n / multiple) * multiple;
  }


void gras_block_wrapper::work(
    const InputItems &input_items,
    const OutputItems &output_items
)
{
    //handle message only ports
    if (this->handle_msgs()) return;

    //ptrdiff_t work_io_ptr_mask = 0;
    const size_t num_inputs = d_input_items.size();
    const size_t num_outputs = d_output_items.size();

    //its just a message only blocks
    if (num_inputs == 0 and num_outputs == 0) return;
    ptrdiff_t work_io_ptr_mask = 0;

    gr::block        *m = d_block_ptr;
    int noutput_items;
    int max_items_avail;
    int max_noutput_items = round_up(m->is_set_max_noutput_items()? m->d_max_noutput_items : 100000000, m->output_multiple());
    int new_alignment = 0;
    int alignment_state = -1;

    if(num_inputs == 0) {

      // determine the minimum available output space
      noutput_items = round_down(output_items.min(), m->output_multiple());
      noutput_items = std::min(noutput_items, max_noutput_items);

      if(noutput_items == 0){		// we're output blocked
        throw std::runtime_error("exec0: noutput_items == 0");
        return;
      }

      goto setup_call_to_work;		// jump to common code
    }

    else if(num_outputs == 0) {
      
      max_items_avail = 0;
      for(size_t i = 0; i < num_inputs; i++) {
        {
          d_ninput_items[i] = input_items[i].size();
        }

        max_items_avail = std::max (max_items_avail, d_ninput_items[i]);
      }

      // take a swag at how much output we can sink
      noutput_items = (int)(max_items_avail * m->relative_rate ());
      noutput_items = round_down(noutput_items, m->output_multiple ());
      noutput_items = std::min(noutput_items, max_noutput_items);

      if(noutput_items == 0) {    // we're blocked on input
        throw std::runtime_error("exec1: noutput_items == 0");
        return;
      }

      goto try_again;     // Jump to code shared with regular case.
    }

    else {
      // do the regular thing

      max_items_avail = 0;
      for(size_t i = 0; i < num_inputs; i++) {
        {
          d_ninput_items[i] = input_items[i].size();
        }
        max_items_avail = std::max(max_items_avail, d_ninput_items[i]);
      }

      // determine the minimum available output space
      noutput_items = round_down(output_items.min(), m->output_multiple());

      if(noutput_items == 0) {		// we're output blocked
        throw std::runtime_error("exec2: noutput_items == 0");
        return;
      }

    try_again:
      if(m->fixed_rate()) {
        // try to work it forward starting with max_items_avail.
        // We want to try to consume all the input we've got.
        int reqd_noutput_items = m->fixed_rate_ninput_to_noutput(max_items_avail);

        // only test this if we specifically set the output_multiple
        if(m->output_multiple_set())
          reqd_noutput_items = round_down(reqd_noutput_items, m->output_multiple());

        if(reqd_noutput_items > 0 && reqd_noutput_items <= noutput_items)
          noutput_items = reqd_noutput_items;

        // if we need this many outputs, overrule the max_noutput_items setting
        max_noutput_items = std::max(m->output_multiple(), max_noutput_items);
      }
      noutput_items = std::min(noutput_items, max_noutput_items);

      // Check if we're still unaligned; use up items until we're
      // aligned again. Otherwise, make sure we set the alignment
      // requirement.
      if(!m->output_multiple_set()) {
        if(m->is_unaligned()) {
          // When unaligned, don't just set noutput_items to the remaining
          // samples to meet alignment; this causes too much overhead in
          // requiring a premature call back here. Set the maximum amount
          // of samples to handle unalignment and get us back aligned.
          if(noutput_items >= m->unaligned()) {
            noutput_items = round_up(noutput_items, m->alignment())	\
              - (m->alignment() - m->unaligned());
            new_alignment = 0;
          }
          else {
            new_alignment = m->unaligned() - noutput_items;
          }
          alignment_state = 0;
        }
        else if(noutput_items < m->alignment()) {
          // if we don't have enough for an aligned call, keep track of
          // misalignment, set unaligned flag, and proceed.
          new_alignment = m->alignment() - noutput_items;
          m->set_unaligned(new_alignment);
          m->set_is_unaligned(true);
          alignment_state = 1;
        }
        else {
          // enough to round down to the nearest alignment and process.
          noutput_items = round_down(noutput_items, m->alignment());
          m->set_is_unaligned(false);
          alignment_state = 2;
        }
      }

      // ask the block how much input they need to produce noutput_items
      m->forecast (noutput_items, d_ninput_items_required);

      // See if we've got sufficient input available
      size_t i;
      for(i = 0; i < num_inputs; i++)
        if(d_ninput_items_required[i] > d_ninput_items[i])	// not enough
          break;

      if(i < num_inputs) {			// not enough input on input[i]
        // if we can, try reducing the size of our output request
        if(noutput_items > m->output_multiple()) {
          noutput_items /= 2;
          noutput_items = round_up(noutput_items, m->output_multiple());
          goto try_again;
        }

        // If we were made unaligned in this round but return here without
        // processing; reset the unalignment claim before next entry.
        if(alignment_state == 1) {
          m->set_unaligned(0);
          m->set_is_unaligned(false);
        }

        this->mark_input_fail(i);
        return;
      }

      // We've got enough data on each input to produce noutput_items.
      // Finish setting up the call to work.
      for(size_t i = 0; i < num_inputs; i++)
      {
        d_input_items[i] = input_items[i].cast<const void *>();
        work_io_ptr_mask |= ptrdiff_t(d_input_items[i]);
      }

    setup_call_to_work:

      for(size_t i = 0; i < num_outputs; i++)
      {
        d_output_items[i] = output_items[i].cast<void *>();
        work_io_ptr_mask |= ptrdiff_t(d_output_items[i]);
      }

      //use ptr IO mask to determine alignment
      //this call should be replaced by VOLK dispatchers
      m->set_is_unaligned((work_io_ptr_mask & (~(GRAS_MAX_ALIGNMENT-1))) != 0);

      // Do the actual work of the block
      int n = m->general_work(noutput_items, d_ninput_items,
                              d_input_items, d_output_items);

      // Adjust number of unaligned items left to process
      if(m->is_unaligned()) {
        m->set_unaligned(new_alignment);
        m->set_is_unaligned(m->unaligned() != 0);
      }

      if(n == gr::block::WORK_DONE)
        goto were_done;

      if(n != gr::block::WORK_CALLED_PRODUCE)
      {
        //manual for loop for produce, dont use convenience produce
        //because it will cause a consume on the virtual msg ports
        for (size_t i = 0; i < d_output_items.size(); i++)
        {
            this->produce(i, n);	// advance write pointers
        }
    }

        return;

      // We didn't produce any output even though we called general_work.
      // We have (most likely) consumed some input.

      /*
      // If this is a source, it's broken.
      if(d->source_p()) {
        std::cerr << "block_executor: source " << m
                  << " produced no output.  We're marking it DONE.\n";
        // FIXME maybe we ought to raise an exception...
        goto were_done;
      }
      */

      // Have the caller try again...
      return;
    }
    assert(0);

  were_done:
    this->mark_done();





/*
    //------------------------------------------------------------------
    //-- initialize input buffers before work
    //------------------------------------------------------------------
    size_t num_input_items = input_items.min();
    if (d_block_ptr->d_fixed_rate) num_input_items -= d_history;
    for (size_t i = 0; i < num_inputs; i++)
    {
        d_input_items[i] = input_items[i].cast<const void *>();
        d_ninput_items[i] = input_items[i].size();
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
        d_output_items[i] = output_items[i].cast<void *>();
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
        size_t calc_output_items = d_block_ptr->fixed_rate_ninput_to_noutput(num_input_items);
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
        d_ninput_items_required = d_ninput_items; //init for NOP case
        d_block_ptr->forecast(work_noutput_items, d_ninput_items_required);
        for (size_t i = 0; i < input_items.size(); i++)
        {
            if GRAS_LIKELY(d_ninput_items_required[i] <= d_ninput_items[i]) continue;

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
        d_ninput_items,
        d_input_items,
        d_output_items
    );

    if GRAS_LIKELY(work_ret > 0) for (size_t i = 0; i < num_outputs; i++)
    {
        this->produce(i, work_ret);
    }

    if GRAS_UNLIKELY(work_ret == -1) this->mark_done();
    */
}

static inline unsigned long long myullround(const double x)
{
    return (unsigned long long)(x);
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
                if (is_tag_blacklisted(t, which_input, true)) continue;
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
                if (is_tag_blacklisted(t, which_input, true)) continue;
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
    gras_ports_pimpl_alloc(this);
    this->reset(new gras_block_wrapper(name, this));

    for (size_t i = 0; i < input_signature->sizeof_stream_items().size(); i++)
    {
        GRASP_BLOCK->input_config(i).item_size = input_signature->sizeof_stream_items().at(i);
    }
    for (size_t i = 0; i < output_signature->sizeof_stream_items().size(); i++)
    {
        GRASP_BLOCK->output_config(i).item_size = output_signature->sizeof_stream_items().at(i);
    }
}

gr::block::~block(void)
{
    //global_block_registry.unregister_primitive(alias());
    GRASP_BLOCK->d_block_ptr = NULL;
    this->reset();
    gras_ports_pimpl_free(this);
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

void gr::block::forecast(int noutput_items, std::vector<int> &ninput_items_required)
{
    unsigned ninputs = ninput_items_required.size ();
    for(unsigned i = 0; i < ninputs; i++)
      ninput_items_required[i] = noutput_items + history() - 1;
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

    //manual for loop for consume, dont use convenience consume
    //because it will cause a consume on the virtual msg ports
    for (size_t i = 0; i < GRASP_BLOCK->d_input_items.size(); i++)
    {
        GRASP_BLOCK->consume(i, size_t(how_many_items));
    }
}

void gr::block::consume(const int i, const int how_many_items)
{
    if GRAS_UNLIKELY(how_many_items < 0) return;
    GRASP_BLOCK->consume(i, size_t(how_many_items));
}

void gr::block::produce(const int o, const int how_many_items)
{
    if GRAS_UNLIKELY(how_many_items < 0) return;
    GRASP_BLOCK->produce(o, size_t(how_many_items));
}

uint64_t gr::block::nitems_read(unsigned int which_input)
{
    return GRASP_BLOCK->get_consumed(which_input);
}

uint64_t gr::block::nitems_written(unsigned int which_output)
{
    return GRASP_BLOCK->get_produced(which_output);
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
    GRASP_BLOCK->post_output_tag(which_output, gr_tag2Tag(tag));
}

void gr::block::get_tags_in_range(
    std::vector<gr::tag_t> &tags,
    unsigned int which_input,
    uint64_t abs_start,
    uint64_t abs_end,
    const pmt::pmt_t &key
){
    tags.clear();
    BOOST_FOREACH(const gras::Tag &tag, GRASP_BLOCK->get_input_tags(which_input))
    {
        if (tag.offset >= abs_start and tag.offset < abs_end)
        {
            if (not tag.object.is<gras::StreamTag>()) continue; //gr wrapper only supports stream tag
            if (GRASP_BLOCK->is_tag_blacklisted(tag, which_input)) continue;
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
    return this->get_tags_in_range(tags, which_input, abs_start, abs_end, pmt::pmt_t());
}

void gr::block::remove_item_tag(unsigned int which_input, const gr::tag_t &tag)
{
    BOOST_FOREACH(const gras::Tag &tag_i, GRASP_BLOCK->get_input_tags(which_input))
    {
        if (GRASP_BLOCK->is_tag_blacklisted(tag_i, which_input)) continue;
        const gras::Tag my_t = gr_tag2Tag(tag);
        if (
            tag_i.offset == my_t.offset and
            tag_i.object.as<gras::StreamTag>() == my_t.object.as<gras::StreamTag>()
        )
        {
            GRASP_BLOCK->d_tag_blacklist[which_input].push_back(tag_i);
            return;
        }
    }
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
    GRASP_BLOCK->output_config(0).maximum_items = max_items;
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
    if(multiple < 1)
      throw std::invalid_argument("block::set_alignment_multiple");

    d_output_multiple = multiple;
}

static void update_input_reserve(gr::block *p)
{
    /*!
     * Set an input reserve for fixed rate blocks.
     *
     * FIXME: Also do this when output multiple is large,
     * This makes gr-trellis pass under conditions where not fixed rate set,
     * but the output multiple is so large that default input isnt sufficient.
     */
    if (p->d_fixed_rate or p->d_output_multiple > 1024)
    {
        size_t reserve = 0;
        //use the fixed_rate_noutput_to_ninput if overloaded
        try
        {
            reserve = p->fixed_rate_noutput_to_ninput(p->d_output_multiple);
        }
        //otherwise just use relative rate for this calculation
        catch(...)
        {
            reserve = size_t(0.5 + p->d_output_multiple/p->d_relative_rate);
        }
        if (reserve) GRASP_BLOCK_(p)->input_config(0).reserve_items = reserve;
    }
}

void gr::block::set_relative_rate(double relative_rate)
{
    if(relative_rate < 0.0)
      throw std::invalid_argument("block::set_relative_rate");

    d_relative_rate = relative_rate;
    update_input_reserve(this);
}

void gr::block::set_output_multiple(int multiple)
{
    if(multiple < 1)
      throw std::invalid_argument("block::set_output_multiple");

    d_output_multiple_set = true;
    d_output_multiple = multiple;
    GRASP_BLOCK->output_config(0).reserve_items = multiple;
    update_input_reserve(this);
}

static gras::ThreadPool make_new_tp(const std::vector<int> &mask, int prio)
{
    gras::ThreadPoolConfig config;
    config.thread_count = 1;

    //determine mask bits and set if non zero
    long int_mask = 0;
    for (size_t i = 0; i < mask.size(); i++)
    {
        int_mask |= (1 << mask[i]);
    }
    if (int_mask != 0) config.processor_mask = int_mask;

    //determine 1.0 scale prio and set if non zero
    float float_prio = (prio/100.0f);
    if (float_prio != 0.0f) config.thread_priority = float_prio;

    return gras::ThreadPool(config);
}

void gr::block::set_processor_affinity(const std::vector<int> &mask)
{
    d_affinity = mask;
    GRASP_BLOCK->set_thread_pool(make_new_tp(d_affinity, d_priority));
}

void gr::block::unset_processor_affinity()
{
    d_affinity.clear();
    GRASP_BLOCK->set_thread_pool(make_new_tp(d_affinity, d_priority));
}

int 
  gr::block::active_thread_priority()
  {
    if (gras::ThreadPool::test_thread_priority(d_priority/100.0f)) return d_priority;
    return -1;
  }

  int 
  gr::block::thread_priority()
  {
    return d_priority;
  }

  int 
  gr::block::set_thread_priority(int priority)
  {
    d_priority = priority;
    GRASP_BLOCK->set_thread_pool(make_new_tp(d_affinity, d_priority));
    return d_priority;
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
