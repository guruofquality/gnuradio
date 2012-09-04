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

#include <volk/volk_common.h>
#include <volk/volk_typedefs.h>
#include "volk_machines.h"

struct volk_machine *volk_machines[] = {
#for $machine in $machines
#ifdef LV_MACHINE_$(machine.name.upper())
&volk_machine_$(machine.name),
#endif
#end for
};

unsigned int n_volk_machines = sizeof(volk_machines)/sizeof(*volk_machines);
