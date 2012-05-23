/*
 * Copyright 2012 Free Software Foundation, Inc.
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

#ifndef INCLUDED_GR_DOUBLE_BUFF_H
#define INCLUDED_GR_DOUBLE_BUFF_H

#include <gr_core_api.h>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

/*!
 * Create a double mapped chunk of virtual memory.
 * Allocate one chunk of physical memory that is
 * doubly mapped across a contiguous swath of virtual memory.
 */
struct GR_CORE_API gr_double_buff : boost::noncopyable
{
    typedef boost::shared_ptr<gr_double_buff> sptr;

    //! Make a new double mapped buffer, length in bytes
    static sptr make(const size_t len);

    //! Get the page size in bytes
    static size_t get_page_size(void);

    //! Get a pointer to the virtual mem
    virtual void *get(void) = 0;

};

#endif /* INCLUDED_GR_DOUBLE_BUFF_H */
