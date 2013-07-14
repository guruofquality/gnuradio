#
# Copyright 2003-2012 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
#
# GNU Radio is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# GNU Radio is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Radio; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
#

# The presence of this file turns this directory into a Python package

# This is the main GNU Radio python module.
# We pull the swig output and the other modules into the gnuradio.gr namespace

from gnuradio_core import *
from exceptions import *
#from hier_block2 import *
#from top_block import *
from gateway import basic_block, sync_block, decim_block, interp_block
from tag_utils import tag_to_python, tag_to_pmt

import gras

RT_OK = 0
RT_NOT_IMPLEMENTED = 1
RT_NO_PRIVS = 2
RT_OTHER_ERROR = 3


def enable_realtime_scheduling():
    """
    This call is for backward compat purposes.
    See gras/thread_pool.hpp for greater options.
    """

    #any prio greater than 0 means realtime scheduling
    prio_value = 0.5

    #test that prio
    if not gras.ThreadPool.test_thread_priority(prio_value):
        return RT_NO_PRIVS

    #create a new thread pool with thread priority set
    config = gras.ThreadPoolConfig()
    config.thread_priority = prio_value
    tp = gras.ThreadPool(config)
    tp.set_active()

    return RT_OK

class top_block(gras.TopBlock):
    def __init__(self, name="Top"):
        gras.TopBlock.__init__(self, name)

    def lock(self):
        pass

    def unlock(self):
        self.commit()

    def start(self, *args):
        if args: self.global_config().maximum_output_items = args[0]
        gras.TopBlock.start(self)

    def run(self, *args):
        if args: self.global_config().maximum_output_items = args[0]
        gras.TopBlock.run(self)

class hier_block2(gras.HierBlock):
    def __init__(self, name="Hier", in_sig=None, out_sig=None):
        gras.HierBlock.__init__(self, name)

        self.__in_sig = in_sig
        self.__out_sig = out_sig

        #backwards compatible silliness
        import weakref
        self._hb = weakref.proxy(self)

    def lock(self):
        pass

    def unlock(self):
        self.commit()

    def input_signature(self): return self.__in_sig
    def output_signature(self): return self.__out_sig

# create a couple of aliases
serial_to_parallel = stream_to_vector
parallel_to_serial = vector_to_stream

# Force the preference database to be initialized
prefs = gr_prefs.singleton

#alias old gr_add_vXX and gr_multiply_vXX
add_vcc = add_cc
add_vff = add_ff
add_vii = add_ii
add_vss = add_ss
multiply_vcc = multiply_cc
multiply_vff = multiply_ff
multiply_vii = multiply_ii
multiply_vss = multiply_ss
