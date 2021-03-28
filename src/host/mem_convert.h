/* 
 * This file is part of the alcatraz distribution (https://github.com/tomwimmenhove/alcatraz);
 * Copyright (c) 2021 Tom Wimmenhove.
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef MEM_CONVERT_H
#define MEM_CONVERT_H

#include <cstdint>

class mem_convert
{
public:
    mem_convert(void* mem, size_t mem_size, intptr_t base_offset)
        : mem((uint64_t) mem), mem_size(mem_size), base_offset(base_offset)
    { }

    intptr_t space_at(uint64_t address)
    {
		if (address < base_offset)
		{
			return -1;
		}

        return mem_size - address + base_offset;
    }

    template<typename T>
    T data_ptr_at(uint64_t address)
    {   
        return (T) (address - base_offset + mem);
    }

    template<typename T>
	T convert_to_host(T guest_ptr)
	{
		return data_ptr_at<T>((uint64_t) guest_ptr);
	}

private:
    uint64_t mem;
    size_t mem_size;
    intptr_t base_offset;
};

#endif /* MEM_CONVERT_H */
