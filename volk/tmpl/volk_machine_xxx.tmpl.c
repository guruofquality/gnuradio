//
// Copyright 2012 Free Software Foundation, Inc.
//
// VOLK is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// VOLK is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with VOLK.  If not, see <http://www.gnu.org/licenses/>.
//

#set $this_machine = $machine_dict[$args[0]]
#set $arch_names = $this_machine.arch_names

#for $arch in $this_machine.archs
#define LV_HAVE_$(arch.name.upper()) 1
#end for

#include <volk/volk_common.h>
#include "volk_machines.h"
#include <volk/volk_config_fixed.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#for $kern in $kernels
#include <volk/$(kern.name).h>
#end for

########################################################################
#def make_arch_have_list($archs)
$(' | '.join(['(1 << LV_%s)'%a.name.upper() for a in $archs]))#slurp
#end def

########################################################################
#def make_impl_name_list($impls)
{$(', '.join(['"%s"'%i.name for i in $impls]))}#slurp
#end def

########################################################################
#def make_impl_align_list($impls)
{$(', '.join(['true' if i.is_aligned else 'false' for i in $impls]))}#slurp
#end def

########################################################################
#def make_impl_deps_list($impls)
{$(', '.join([' | '.join(['(1 << LV_%s)'%d.upper() for d in i.deps]) for i in $impls]))}#slurp
#end def

########################################################################
#def make_impl_fcn_list($name, $impls)
{$(', '.join(['%s_%s'%($name, i.name) for i in $impls]))}#slurp
#end def

struct volk_machine volk_machine_$(this_machine.name) = {
    $make_arch_have_list($this_machine.archs),
    "$this_machine.name",
    $this_machine.alignment,
    #for $kern in $kernels
        #set $impls = $kern.get_impls($arch_names)
    "$kern.name",                                   ##//kernel name
    $make_impl_name_list($impls),                   ##//list of kernel implementations by name
    $make_impl_deps_list($impls),                   ##//list of arch dependencies per implementation
    $make_impl_align_list($impls),                  ##//alignment required? for each implementation
    $make_impl_fcn_list($kern.name, $impls),        ##//pointer to each implementation
    $(len($impls)),                                 ##//number of implementations listed here
    #end for
};
