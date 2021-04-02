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
#ifndef GUEST_DISPATCHER_H
#define GUEST_DISPATCHER_H

//#include <rpcbuf.h>

class guest_dispatcher : public call_dispatcher
{
public:
	virtual ~guest_dispatcher()  { }

protected:
	void exec(size_t id, void* mem, size_t, size_t)
	{   
		uint16_t port = 0x42;
		__asm__ volatile ("mov %0, %%rdi\n"
				"outl %1, %2"
				: /* empty */
				: "r" (mem), "a" ((uint32_t) id), "Nd" (port)
				: "memory", "rdi");
	}
};

#endif /* GUEST_DISPATCHER_H */
