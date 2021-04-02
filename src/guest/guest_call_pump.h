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
#ifndef GUEST_CALL_PUMP_H
#define GUEST_CALL_PUMP_H

#include <rpcbuf.h>
#include <cstdint>

class guest_call_pump
{
public:
	guest_call_pump(call_receiver* receiver);

	void do_events();

	virtual ~guest_call_pump() { }

private:
	uint32_t wait_for_request(uint8_t* data);
	void nudge_host();

private:
	call_receiver* receiver;
};

#endif /* GUEST_CALL_PUMP_H */
