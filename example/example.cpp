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
#include <string.h>
#include <iostream>
#include <unistd.h>

#include <thread> 
#include <mutex>
#include <condition_variable> 

#include "rpcbuf.h"
#include "call_in_definitions.h"

#include "alcatraz.h"
#include "shared_data.h"

/* Define a call_receiver which implements calls that the guest can call */
class receiver : public call_receiver
{
#include "guest_callout_declarations.h"

public:
	receiver(alcatraz* box) : CALL_OUT_INIT, box(box)
	{   
		setup();
	}

private:
	size_t write(int fd, const void *buf, size_t count)
	{
		if (fd < 1 || fd > 2)
		{
			return -1;
		}

		auto& mem_converter = box->get_mem_converter();
		intptr_t space = mem_converter.space_at((uint64_t) buf);
		if (space < 0 || space < count)
		{   
			throw std::overflow_error("VM tried tried to access out-of-bound memory");
		}

		return ::write(fd, mem_converter.convert_to_host(buf), count);
	}

	alcatraz* box;
};

/* Define a call_sender which allows the host to call functions implemented on the guest */
#include <call_out_definitions.h>
class sender : call_sender
{
public:
	sender(call_dispatcher& guest_dispatcher)
		: call_sender(guest_dispatcher)
	{ }

	#include "host_callout_declarations.h"
};

extern const unsigned char guest64[], guest64_end[];

int main()
{
	/* Setup the VM for the guest */
	const size_t mem_size = 2048 * 1024 * 10;
	alcatraz box(mem_size, guest64, guest64_end-guest64);

	/* Setup a receiver for the guest to call out to the host */
	receiver rt(&box);

	/* Setup a sender for the host to call out to the guest */
	alcatraz_dispatcher dispatcher(&box);
	sender st(dispatcher);

	/* Setup some input data to pass to the guest */
	input_data args;
	args.a = 42;
	args.b = 43;

	/* Start the guest in a separate thread */
	int ret_value = 0;
	std::thread box_thread([&](){
			ret_value = box.run(args, &rt, &dispatcher);
			});

	/* Wait until the message pump is running on the guest */
	dispatcher.wait_pump_ready();

	/* Call a function on the guest */
	double a = 1.2;
	double b = st.square(a);
	std::cout << "The square of " << a << " = " << b << '\n';

	/* Makes the guest break out of the call pump loop */
	st.exit(0);

	/* Wait for the guest to exit */
	box_thread.join();

	return ret_value;
}
