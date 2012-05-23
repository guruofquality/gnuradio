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

#include "gr_double_buff.h"
#include <boost/format.hpp>
#include <boost/make_shared.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <gruel/high_res_timer.h>
#include <unistd.h>

namespace ipc = boost::interprocess;

/*!
 * This routine generates an incredibly unique name for the allocation.
 *
 * Because we are using boost IPC, and it expects that we share the memory;
 * IPC allocation requires a unique name to share amongst the processes.
 * Since we are not actually using IPC, the sharing aspect isnt very useful,
 * but we still need a unique name for the shared memory allocation anyway.
 * (I would like if a empty string would suffice as an anonymous allocation)
 */
static std::string omg_so_unique(void)
{
    boost::uuids::uuid u1; // initialize uuid
    return str(boost::format("shmem-%s-%u-%llu-%u")
        % to_string(u1) % std::rand() % gruel::high_res_timer_now() % getpid());
}

class gr_double_buff_impl : public gr_double_buff
{
public:

    gr_double_buff_impl(const size_t len)
    {
        ////////////////////////////////////////////////////////////////
        // Step 0) Find an address that can be mapped across 2x length:
        // Allocate physical memory 2x the required size.
        // Map a virtual memory region across this memory.
        // Now we have a 2x length swath of virtual memory,
        // and the physical memory is freed back to the system.
        ////////////////////////////////////////////////////////////////
        {
            //std::cout << "make shmem 2x\n" << std::endl;
            ipc::shared_memory_object shm_obj_2x(
                ipc::create_only,                  //only create
                omg_so_unique().c_str(),              //name
                ipc::read_write                   //read-write mode
            );

            //std::cout << "truncate 2x\n" << std::endl;
            shm_obj_2x.truncate(2*len);

            //std::cout << "map region 0\n" << std::endl;
            ipc::mapped_region region0(
                shm_obj_2x,                    //Memory-mappable object
                ipc::read_write,               //Access mode
                0,                //Offset from the beginning of shm
                2*len        //Length of the region
            );
            //std::cout << "region0.get_address() " << size_t(region0.get_address()) << std::endl;

            _addr = (char *)region0.get_address();
        }

        ////////////////////////////////////////////////////////////////
        // Step 1) Allocate a chunk of physical memory of length bytes
        ////////////////////////////////////////////////////////////////
        //std::cout << "make shmem\n" << std::endl;
        shm_obj = ipc::shared_memory_object(
            ipc::create_only,                  //only create
            omg_so_unique().c_str(),              //name
            ipc::read_write                   //read-write mode
        );

        //std::cout << "truncate\n" << std::endl;
        shm_obj.truncate(len);

        ////////////////////////////////////////////////////////////////
        //Step 2) Remap region1 of the virtual memory space
        ////////////////////////////////////////////////////////////////
        //std::cout << "map region 1\n" << std::endl;
        region1 = ipc::mapped_region(
            shm_obj,                      //Memory-mappable object
            ipc::read_write,               //Access mode
            0,                //Offset from the beginning of shm
            len,        //Length of the region
            _addr
        );
        //std::cout << "region1.get_address() " << size_t(region1.get_address()) << std::endl;

        ////////////////////////////////////////////////////////////////
        //Step 3) Remap region2 of the virtual memory space
        ////////////////////////////////////////////////////////////////
        //std::cout << "map region 2\n" << std::endl;
        region2 = ipc::mapped_region(
            shm_obj,                      //Memory-mappable object
            ipc::read_write,               //Access mode
            0,                //Offset from the beginning of shm
            len,        //Length of the region
            _addr + len
        );

        //std::cout << "region2.get_address() " << size_t(region2.get_address()) << std::endl;
        //std::cout << "diff " << (long(region2.get_address()) - long(region1.get_address())) << std::endl;

        ////////////////////////////////////////////////////////////////
        //4) Zero out the memory for good measure
        ////////////////////////////////////////////////////////////////
        std::memset(region1.get_address(), 0, region1.get_size());
        std::memset(region2.get_address(), 0, region2.get_size());
    }

    ~gr_double_buff_impl(void)
    {
        //NOP
    }

    void *get(void)
    {
        return _addr;
    }

private:
    char *_addr;
    ipc::shared_memory_object shm_obj;
    ipc::mapped_region region1;
    ipc::mapped_region region2;
};

size_t gr_double_buff::get_page_size(void)
{
    return ipc::mapped_region::get_page_size();
}

gr_double_buff::sptr gr_double_buff::make(const size_t len)
{
    return boost::make_shared<gr_double_buff_impl>(len);
}
