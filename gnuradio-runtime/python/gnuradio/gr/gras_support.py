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

import gras
import weakref

from . gras_swig_helper import basic_block_to_element
from . top_block import top_block as gr_top_block

########################################################################
## Port monitor tracks actual connection indexes for msg ports
########################################################################
class port_monitor(object):
    def __init__(self):
        self._num_real_ports = 0
        self._virtual_port_names = list()

    def get_index(self, name):
        for i, name_i in enumerate(self._virtual_port_names):
            if name_i == name: return i+self._num_real_ports
        self._virtual_port_names.append(name)
        return len(self._virtual_port_names)-1+self._num_real_ports

    def update(self, new_port_index):
        if self._virtual_port_names:
            raise Exception, "error: called update on normal connection after making virtual message ports"
        self._num_real_ports = max(self._num_real_ports, new_port_index+1)

def get_xx_monitor(block, key):
    try: setattr(block, 'to_element', lambda: basic_block_to_element(block.to_basic_block()))
    except AttributeError: pass
    if not hasattr(block, key): setattr(block, key, port_monitor())
    return getattr(block, key)
def get_src_monitor(block): return get_xx_monitor(block, '__src_port_monitor')
def get_dst_monitor(block): return get_xx_monitor(block, '__dst_port_monitor')

########################################################################
## connection helper code shared by block classes
########################################################################
def allocate_ports__(src, dst):
    """
    Perform a dummy connect and disconnect.
    This causes the internal port structures
    to 1) be allocated 2) to be tracked.
    This basically makes the msg connect work.
    """
    bs = gr_top_block("bs_block")
    try:
        bs.connect(src, bs)
        bs.disconnect(src, bs)
    except AttributeError: pass
    try:
        bs.connect(bs, dst)
        bs.disconnect(bs, dst)
    except AttributeError: pass

    #track the port connections to support msg
    get_src_monitor(src[0]).update(src[1])
    get_dst_monitor(dst[0]).update(dst[1])

def allocate_msg_ports__(src, srcport, dst, dstport):
    bs = gr_top_block("bs_block")
    bs.msg_connect(src, srcport, bs, "bs")
    bs.msg_disconnect(src, srcport, bs, "bs")
    bs.msg_connect(bs, "bs", dst, dstport)
    bs.msg_disconnect(bs, "bs", dst, dstport)

def internal_connect__(fcn, obj, *args):
    """
    1) Comprehends the arg list format
    2) call the allocate ports routine
    3) call the actually connect method
    """

    if len(args) == 1:
        elem = args[0]
        fcn(obj, elem)
        return

    for src, sink in zip(args, args[1:]):

        try: src, src_index = src
        except: src_index = 0

        try: sink, sink_index = sink
        except: sink_index = 0

        allocate_ports__((src, src_index), (sink, sink_index))
        fcn(obj, (src, src_index), (sink, sink_index))

########################################################################
## custom top block understands GRSS blocks and GRAS blocks
########################################################################
class top_block(gras.TopBlock):
    def __init__(self, name="Top"):
        gras.TopBlock.__init__(self, name)

    def lock(self):
        pass

    def unlock(self):
        self.commit()

    def connect(self, *args):
        return internal_connect__(gras.TopBlock.connect, self, *args)

    def disconnect(self, *args):
        return internal_connect__(gras.TopBlock.disconnect, self, *args)

    def msg_connect(self, src, srcport, dst, dstport):
        allocate_msg_ports__(src, srcport, dst, dstport)
        gras.TopBlock.connect(self,
            (src, get_src_monitor(src).get_index(srcport)),
            (dst, get_dst_monitor(dst).get_index(dstport)),
        )

    def msg_disconnect(self, src, srcport, dst, dstport):
        gras.TopBlock.disconnect(self,
            (src, get_src_monitor(src).get_index(srcport)),
            (dst, get_dst_monitor(dst).get_index(dstport)),
        )

########################################################################
## custom hier block understands GRSS blocks and GRAS blocks
########################################################################
class hier_block2(gras.HierBlock):
    def __init__(self, name="Hier", in_sig=None, out_sig=None):
        gras.HierBlock.__init__(self, name)

        self.__in_sig = in_sig
        self.__out_sig = out_sig

        #backwards compatible silliness
        self._hb = weakref.proxy(self)

    def lock(self):
        pass

    def unlock(self):
        self.commit()

    def input_signature(self): return self.__in_sig
    def output_signature(self): return self.__out_sig

    def connect(self, *args):
        return internal_connect__(gras.HierBlock.connect, self, *args)

    def disconnect(self, *args):
        return internal_connect__(gras.HierBlock.disconnect, self, *args)

    def msg_connect(self, src, srcport, dst, dstport):
        allocate_msg_ports__(src, srcport, dst, dstport)
        gras.HierBlock.connect(self,
            (src, get_src_monitor(src).get_index(srcport)),
            (dst, get_dst_monitor(dst).get_index(dstport)),
        )

    def msg_disconnect(self, src, srcport, dst, dstport):
        gras.HierBlock.disconnect(self,
            (src, get_src_monitor(src).get_index(srcport)),
            (dst, get_dst_monitor(dst).get_index(dstport)),
        )

__all__ = ['hier_block2', 'top_block']
