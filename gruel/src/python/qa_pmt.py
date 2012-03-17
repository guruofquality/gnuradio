#!/usr/bin/env python
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

import numpy
import unittest
import pmt

class test_gruel_pmt(unittest.TestCase):

    def test01 (self):
        a = pmt.pmt_intern("a")
        b = pmt.pmt_from_double(123765)
        d1 = pmt.pmt_make_dict()
        d2 = pmt.pmt_dict_add(d1, a, b)
        pmt.pmt_print(d2)

    def test02 (self):
        const = 123765
        x_pmt = pmt.pmt_from_double(const)
        x_int = pmt.pmt_to_double(x_pmt)
        self.assertEqual(x_int, const)

    def test03 (self):
        randints = numpy.random.randint(0, 256, 123)
        blob = pmt.pmt_make_blob(len(randints))
        blob_rw_data = pmt.pmt_blob_rw_data(blob)

        blob_rw_data[:] = randints #assign rand ints to data
        self.assertItemsEqual(blob_rw_data, randints)

        blob_ro_data = pmt.pmt_blob_ro_data(blob)
        self.assertItemsEqual(blob_ro_data, randints)

    def test04 (self):
        mgr = pmt.pmt_make_mgr()
        pmt.pmt_mgr_set(mgr, pmt.pmt_make_blob(100))
        pmt.pmt_mgr_set(mgr, pmt.pmt_make_blob(100))
        a = pmt.pmt_mgr_acquire(mgr, False)
        b = pmt.pmt_mgr_acquire(mgr, False)
        c = pmt.pmt_mgr_acquire(mgr, False)
        self.assertTrue(not pmt.pmt_is_null(a))
        self.assertTrue(not pmt.pmt_is_null(b))
        self.assertTrue(pmt.pmt_is_null(c))

if __name__ == '__main__':
    unittest.main()
