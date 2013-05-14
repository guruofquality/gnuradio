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

#ifndef INCLUDED_LIB_GR_RUNTIME_GRAS_GRAS_PIMPL_H
#define INCLUDED_LIB_GR_RUNTIME_GRAS_GRAS_PIMPL_H

#define GRAS_PORTS_PIMPL_INIT() this->ports_pimpl.reset(new gras_ports_pimpl())
#define GRAS_PORTS_PIMPL(b) (boost::static_pointer_cast<gras_ports_pimpl>(b->ports_pimpl))

#include <cstddef>
#include <vector>
#include <string>

struct gras_ports_pimpl_monitor
{
    gras_ports_pimpl_monitor(void)
    {
        num_real_ports = 0;
    }

    size_t num_real_ports;
    std::vector<std::string> virtual_port_names;

    size_t get_index(const std::string &name)
    {
        for (size_t i = 0; i < virtual_port_names.size(); i++)
        {
            if (virtual_port_names[i] == name) return i+num_real_ports;
        }
        virtual_port_names.push_back(name);
        return virtual_port_names.size()-1+num_real_ports;
    }

    void update(const size_t new_port_index)
    {
        num_real_ports = std::max(num_real_ports, new_port_index+1);
    }
};

struct gras_ports_pimpl
{
    gras_ports_pimpl_monitor input;
    gras_ports_pimpl_monitor output;
};

#endif /*INCLUDED_LIB_GR_RUNTIME_GRAS_GRAS_PIMPL_H*/
