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

#include "gras/gras_pimpl.h"
#include <boost/thread/mutex.hpp>
#include <map>

static boost::mutex mutex;

static std::map<void *, gras_ports_pimpl *> &get_ports_map(void)
{
    static std::map<void *, gras_ports_pimpl *> m;
    return m;
}

void gras_ports_pimpl_alloc(void *p)
{
    boost::mutex::scoped_lock lock(mutex);
    get_ports_map()[p] = new gras_ports_pimpl();
}

void gras_ports_pimpl_free(void *p)
{
    boost::mutex::scoped_lock lock(mutex);
    get_ports_map().erase(p);
}

gras_ports_pimpl *gras_ports_pimpl_get(void *p)
{
    boost::mutex::scoped_lock lock(mutex);
    return get_ports_map()[p];
}
