/* -*- c++ -*- */
/*
 * Copyright 2004 Free Software Foundation, Inc.
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

// @WARNING@

GR_SWIG_BLOCK_MAGIC(trellis,@BASE_NAME@);

@SPTR_NAME@ trellis_make_@BASE_NAME@ (
  const fsm &FSMo, int STo0, int SToK,
  const fsm &FSMi, int STi0, int STiK,
  const interleaver &INTERLEAVER,
  int blocklength,
  int repetitions,
  trellis_siso_type_t SISO_TYPE,
  int D,
  const std::vector<@I_TYPE@> &TABLE,
  gr::digital::trellis_metric_type_t METRIC_TYPE,
  float scaling
);


class @NAME@ : public gr_block
{
private:
  @NAME@ (
    const fsm &FSMo, int STo0, int SToK,
    const fsm &FSMi, int STi0, int STiK,
    const interleaver &INTERLEAVER,
    int blocklength,
    int repetitions,
    trellis_siso_type_t SISO_TYPE,
    int D,
    const std::vector<@I_TYPE@> &TABLE,
    gr::digital::trellis_metric_type_t METRIC_TYPE,
    float scaling
  );

public:
  fsm FSM1 () const { return d_FSMo; }
  fsm FSM2 () const { return d_FSMi; }
  int ST10 () const { return d_STo0; }
  int ST1K () const { return d_SToK; }
  int ST20 () const { return d_STi0; }
  int ST2K () const { return d_STiK; }
  interleaver INTERLEAVER () const { return d_INTERLEAVER; }
  int blocklength () const { return d_blocklength; }
  int repetitions () const { return d_repetitions; }
  int D () const { return d_D; }
  std::vector<@I_TYPE@> TABLE () const { return d_TABLE; }
  gr::digital::trellis_metric_type_t METRIC_TYPE () const { return d_METRIC_TYPE; }
  trellis_siso_type_t SISO_TYPE () const { return d_SISO_TYPE; }
  float scaling() const { return d_scaling; }
  void set_scaling (float scaling);
};
