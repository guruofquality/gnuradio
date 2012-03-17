#
# Copyright 2011 Free Software Foundation, Inc.
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

'''
The GNU Radio Utility Etcetera Library's Polymorphic Types for Python.
'''

import numpy
from pmt_swig import *
import pmt_swig as __pmt

#this function knows how to convert an address to a numpy array
def __pointer_to_ndarray(addr, nitems):
    dtype = numpy.dtype(numpy.uint8)
    class array_like:
        __array_interface__ = {
            'data' : (int(addr), False),
            'typestr' : dtype.base.str,
            'descr' : dtype.base.descr,
            'shape' : (nitems,) + dtype.shape,
            'strides' : None,
            'version' : 3
        }
    return numpy.asarray(array_like()).view(dtype.base)

#re-create the blob data functions, but yield a numpy array instead
def pmt_blob_rw_data(blob):
    return __pointer_to_ndarray(__pmt.pmt_blob_rw_data(blob), pmt_blob_length(blob))

def pmt_blob_ro_data(blob):
    return __pointer_to_ndarray(__pmt.pmt_blob_ro_data(blob), pmt_blob_length(blob))

#re-create mgr acquire by calling into python GIL-safe version
def pmt_mgr_acquire(mgr, block = True):
    return __pmt.pmt_mgr_acquire_safe(mgr, block)
