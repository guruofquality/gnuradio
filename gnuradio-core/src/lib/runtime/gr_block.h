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

#ifndef INCLUDED_GNURADIO_GR_BLOCK_H
#define INCLUDED_GNURADIO_GR_BLOCK_H

#include <gr_core_api.h>
#include <gras/block.hpp>
#include <gr_io_signature.h>
#include <gr_types.h>
#include <gr_tags.h>
#include <string>
#include <deque>
#include <map>
#include <boost/foreach.hpp>
#include <gruel/thread.h>
#include <gr_sptr_magic.h>

struct GR_CORE_API gr_block : gras::Block
{

    gr_block(void);

    gr_block(
        const std::string &name,
        gr_io_signature_sptr input_signature,
        gr_io_signature_sptr output_signature
    );

    long unique_id(void) const{return _unique_id;}
    std::string name(void) const{return _name;}
    long _unique_id;
    std::string _name;

    virtual ~gr_block(void);

    gr_io_signature_sptr input_signature(void) const;
    gr_io_signature_sptr output_signature(void) const;

    void set_input_signature(gr_io_signature_sptr sig);
    void set_output_signature(gr_io_signature_sptr sig);

    virtual bool check_topology(int ninputs, int noutputs);

    //! Overload me! I am the forecast
    virtual void forecast(int, std::vector<int> &);

    //! Return options for the work call
    enum
    {
        WORK_CALLED_PRODUCE = -2,
        WORK_DONE = -1
    };

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

    virtual bool start(void);
    virtual bool stop(void);

    //! Call during work to consume items
    void consume_each(const int how_many_items);

    void consume(const size_t i, const int how_many_items);

    void produce(const size_t o, const int how_many_items);

    //! Get absolute count of all items consumed on the given input port
    uint64_t nitems_read(const size_t which_input = 0);

    //! Get absolute count of all items produced on the given output port
    uint64_t nitems_written(const size_t which_output = 0);

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

    /*******************************************************************
     * Deal with input and output port configuration
     ******************************************************************/

    unsigned history(void) const;

    void set_history(unsigned history);

    /*!
     * Enable fixed rate logic.
     * When enabled, relative rate is assumed to be set,
     * and forecast is automatically called.
     * Also, consume will be called automatically.
     */
    void set_fixed_rate(const bool fixed_rate);

    //! Get the fixed rate setting
    bool fixed_rate(void) const;

    /*!
     * The relative rate can be thought of as interpolation/decimation.
     * In other words, relative rate is the ratio of output items to input items.
     */
    void set_relative_rate(const double relative_rate);

    //! Get the relative rate setting
    double relative_rate(void) const;

    /*!
     * The output multiple setting controls work output buffer sizes.
     * Buffers will be number of items modulo rounted to the multiple.
     */
    void set_output_multiple(const size_t multiple);

    //! Get the output multiple setting
    size_t output_multiple(void) const;

    /*******************************************************************
     * Deal with tag handling and tag configuration
     ******************************************************************/

    enum tag_propagation_policy_t
    {
        TPP_DONT = 0,
        TPP_ALL_TO_ALL = 1,
        TPP_ONE_TO_ONE = 2
    };

    tag_propagation_policy_t tag_propagation_policy(void);

    void set_tag_propagation_policy(tag_propagation_policy_t p);

    ///////////// TODO //////////////////////
    void set_max_output_buffer(long){}
    void set_max_output_buffer(int, long){}
    long max_output_buffer(size_t){return 0;}
    void set_min_output_buffer(long){}
    void set_min_output_buffer(int, long){}
    long min_output_buffer(size_t){return 0;}

    ///////////// ALIAS stuff - is it used? //////////////////////
    std::string          d_symbol_alias;
    std::string          d_symbol_name;
    std::string symbol_name() const { return d_symbol_name; }
    bool alias_set() { return !d_symbol_alias.empty(); }
    std::string alias(){ return alias_set()?d_symbol_alias:symbol_name(); }
    pmt::pmt_t alias_pmt(){ return pmt::pmt_intern(alias()); }
    void set_block_alias(std::string name){d_symbol_alias = name;}

    ///////////// MSG stuff not implemented //////////////////////
    typedef std::deque<pmt::pmt_t>    msg_queue_t;
    typedef std::map<pmt::pmt_t, msg_queue_t, pmt::pmt_comperator>    msg_queue_map_t;
    typedef std::map<pmt::pmt_t, msg_queue_t, pmt::pmt_comperator>::iterator msg_queue_map_itr;
    msg_queue_map_t msg_queue;
    pmt::pmt_t message_subscribers;

  typedef boost::function<void(pmt::pmt_t)> msg_handler_t;
  typedef std::map<pmt::pmt_t , msg_handler_t, pmt::pmt_comperator> d_msg_handlers_t;
  d_msg_handlers_t d_msg_handlers;

    template <typename T> void set_msg_handler(pmt::pmt_t which_port, T msg_handler){}

    void message_port_register_in(pmt::pmt_t /*port_id*/){}
    void message_port_register_out(pmt::pmt_t /*port_id*/){}
    void message_port_pub(pmt::pmt_t /*port_id*/, pmt::pmt_t /*msg*/){}
    void message_port_sub(pmt::pmt_t /*port_id*/, pmt::pmt_t /*target*/){}
    void message_port_unsub(pmt::pmt_t /*port_id*/, pmt::pmt_t /*target*/){}

    virtual bool message_port_is_hier(pmt::pmt_t port_id) { (void) port_id; /*std::cout << "is_hier\n";*/ return false; }
    virtual bool message_port_is_hier_in(pmt::pmt_t port_id) { (void) port_id; /*std::cout << "is_hier_in\n";*/ return false; }
    virtual bool message_port_is_hier_out(pmt::pmt_t port_id) { (void) port_id; /*std::cout << "is_hier_out\n";*/ return false; }

    /*!
    * \brief Get input message port names.
    *
    * Returns the available input message ports for a block. The
    * return object is a PMT vector that is filled with PMT symbols.
    */
    pmt::pmt_t message_ports_in(){return pmt::PMT_NIL;}

    /*!
    * \brief Get output message port names.
    *
    * Returns the available output message ports for a block. The
    * return object is a PMT vector that is filled with PMT symbols.
    */
    pmt::pmt_t message_ports_out(){return pmt::PMT_NIL;}

    //! is the queue empty?
    bool empty_p(pmt::pmt_t which_port) { 
        if(msg_queue.find(which_port) == msg_queue.end())
          throw std::runtime_error("port does not exist!");
        return msg_queue[which_port].empty();
    }
    bool empty_p() { 
        bool rv = true;
        BOOST_FOREACH(msg_queue_map_t::value_type &i, msg_queue){ rv &= msg_queue[i.first].empty(); }
        return rv;
    }

    //| Acquires and release the mutex
    void insert_tail( pmt::pmt_t /*which_port*/, pmt::pmt_t /*msg*/){}
    /*!
    * \returns returns pmt at head of queue or pmt_t() if empty.
    */
    pmt::pmt_t delete_head_nowait( pmt::pmt_t /*which_port*/){return pmt::PMT_NIL;}

    /*!
    * \returns returns pmt at head of queue or pmt_t() if empty.
    */
    pmt::pmt_t delete_head_blocking( pmt::pmt_t /*which_port*/){return pmt::PMT_NIL;}

    msg_queue_t::iterator get_iterator(pmt::pmt_t which_port){
        return msg_queue[which_port].begin();
    }

    void erase_msg(pmt::pmt_t which_port, msg_queue_t::iterator it){
        msg_queue[which_port].erase(it);
    }

    virtual bool has_msg_port(pmt::pmt_t which_port){
        if(msg_queue.find(which_port) != msg_queue.end()){
            return true;
        }
        if(pmt::pmt_dict_has_key(message_subscribers, which_port)){
            return true;
        }
        return false;
    }

  /*!
   * \brief Tests if there is a handler attached to port \p which_port
   */
   bool has_msg_handler(pmt::pmt_t which_port) {
     return (d_msg_handlers.find(which_port) != d_msg_handlers.end());
   }

  /*
   * This function is called by the runtime system to dispatch messages.
   *
   * The thread-safety guarantees mentioned in set_msg_handler are implemented
   * by the callers of this method.
   */
  virtual void dispatch_msg(pmt::pmt_t which_port, pmt::pmt_t msg)
  {
    // AA Update this
    if(has_msg_handler(which_port)) {  // Is there a handler?
      d_msg_handlers[which_port](msg); // Yes, invoke it.
    }
  }

  /*! Used by block's setters and work functions to make
   * setting/resetting of parameters thread-safe.
   *
   * Used by calling gruel::scoped_lock l(d_setlock);
   */ 
  gruel::mutex d_setlock;

  // ----------------------------------------------------------------------------
  // Functions to handle thread affinity
  std::vector<int> d_affinity;              // thread affinity proc. mask

  /*!
   * \brief Set the thread's affinity to processor core \p n.
   *
   * \param mask a vector of unsigned ints of the core numbers available to this block.
   */
  void set_processor_affinity(const std::vector<int> &mask){d_affinity=mask;}

  /*!
   * \brief Remove processor affinity to a specific core.
   */
  void unset_processor_affinity(){}

  /*!
   * \brief Get the current processor affinity.
   */
  std::vector<int> processor_affinity() { return d_affinity; }

    ///////////////// private vars //////////////////////

    gr_vector_int _work_ninput_items;
    gr_vector_int _fcast_ninput_items;
    size_t _num_outputs;
    ptrdiff_t _work_io_ptr_mask;
    size_t _output_multiple_items;
    double _relative_rate;
    bool _enable_fixed_rate;
    size_t _input_history_items;
    tag_propagation_policy_t _tag_prop_policy;
    size_t _interp, _decim;
    gr_io_signature_sptr _in_sig, _out_sig;

    ///////////////// the Block overloads //////////////////////

    //! implements work -> calls general work
    void work(const InputItems &, const OutputItems &);

    //! notifications of new topological commits
    void notify_topology(const size_t, const size_t);

    //! start notification
    void notify_active(void);

    //! stop notification
    void notify_inactive(void);

    //! implements tag_propagation_policy()
    virtual void propagate_tags(const size_t, const gras::TagIter &);

    void _update_input_reserve(void);

    gras::BufferQueueSptr input_buffer_allocator(const size_t, const gras::SBufferConfig &);
    gras::BufferQueueSptr output_buffer_allocator(const size_t, const gras::SBufferConfig &);

};

typedef boost::shared_ptr<gr_block> gr_block_sptr;

GRAS_FORCE_INLINE void gr_block::consume_each(const int how_many_items)
{
    if GRAS_UNLIKELY(how_many_items < 0) return;
    gras::Block::consume(size_t(how_many_items));
}

GRAS_FORCE_INLINE void gr_block::consume(const size_t i, const int how_many_items)
{
    if GRAS_UNLIKELY(how_many_items < 0) return;
    gras::Block::consume(i, size_t(how_many_items));
}

GRAS_FORCE_INLINE void gr_block::produce(const size_t o, const int how_many_items)
{
    if GRAS_UNLIKELY(how_many_items < 0) return;
    gras::Block::produce(o, size_t(how_many_items));
}

GRAS_FORCE_INLINE uint64_t gr_block::nitems_read(const size_t which_input)
{
    return Block::get_consumed(which_input);
}

GRAS_FORCE_INLINE uint64_t gr_block::nitems_written(const size_t which_output)
{
    return Block::get_produced(which_output);
}

GRAS_FORCE_INLINE size_t gr_block::interpolation(void) const
{
    return _interp;
}

GRAS_FORCE_INLINE size_t gr_block::decimation(void) const
{
    return _decim;
}

GRAS_FORCE_INLINE bool gr_block::is_unaligned(void)
{
    //TODO
    //probably dont need this since volk dispatcher checks alignment
    //32 byte aligned is good enough for you
    return (_work_io_ptr_mask & ptrdiff_t(GRAS_MAX_ALIGNMENT-1)) != 0;
}

#endif /*INCLUDED_GNURADIO_GR_BLOCK_H*/
