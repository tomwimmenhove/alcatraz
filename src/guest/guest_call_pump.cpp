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

#include "guest_call_pump.h"

guest_call_pump::guest_call_pump(call_receiver* receiver)
	: receiver(receiver)
{ }

void guest_call_pump::do_events()
{
	size_t max_data_size = receiver->get_max_size();
	uint8_t data[max_data_size];

	/* Wait for a call from the host */
	uint32_t id = wait_for_request(data);

	/* Execute it */
	receiver->exec(id, data);

	/* Let the host know we're done */
	nudge_host();
}

/* Tell the host where our data is stored, and wait for it
 * to give us an id to execute */
uint32_t guest_call_pump::wait_for_request(uint8_t* data)
{
	uint16_t port = 0x43;
	uint32_t id;

	__asm__ volatile (
			"mov %1, %%rdi\n"
			"inl %w2, %0"
			:"=a" (id)
			:"m" (data), "Nd" (port)
			: "rdi");

	return id;
}

/* Let the host know we're done */
void guest_call_pump::nudge_host()
{
	uint16_t port = 0x43;
	uint32_t dummy = 0;

	__asm__ volatile (
			"outl %0, %1"
			: : "a" (dummy), "Nd" (port) :);
}

