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
#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdio.h>
#include <cmath>

#define GUEST_CUSTOM_WRITE
#include <guest.h>

#include "../shared_data.h"

/* This macro expands the the implementations or read/write/close etc, unless a custom functionis defined */
DEFINE_GUEST_DUMMY_CALLS

/* Define a call_sender which allows the guest to call functions implemented on the host */
#include <call_out_definitions.h>
class sender_test : call_sender
{
public:
	sender_test(call_dispatcher& guest_dispatcher)
		: call_sender(guest_dispatcher)
	{ }

	#include "../guest_callout_declarations.h"
};

static guest_dispatcher d;
sender_test st(d);

/* Define a call_receiver which implements calls that the host can call */
#include "call_in_definitions.h"
class receiver : public call_receiver
{
#include "../host_callout_declarations.h"

public:
	receiver() : CALL_OUT_INIT
	{
		setup();
		reset();
	}

	void reset()
	{
		stopped = false;
		exit_code = 0;
	}

	bool is_stopped() { return stopped; }
	int get_exit_code() { return exit_code; }

private:
	/* Methods called from the host */
	double square(double x) { return x * x; }

	void exit(int code)
	{
		exit_code = code;
		stopped = true;
	}

private:
	bool stopped = true;
	int exit_code = 0;
};

/* Custom write() function */
extern "C" {
	int64_t write(int fd, const void *buf, size_t count)
	{
		return st.write(fd, buf, count);
	}
}

int guest_main(void* data)
{
	input_data* args = (input_data*) data;

	puts("Hello world!");

	printf("args->a: %d\n", args->a);
	printf("args->b: %d\n", args->b);

	float x = 42;
	printf("The squre root of %g is %g\n", x, sqrt(x));

	/* Create a call pump */
	receiver rec;
	guest_call_pump pump(&rec);

	/* Run the call pump until the host calls the stop() method on it */
	while (!rec.is_stopped())
	{
		pump.do_events();
	}

	return rec.get_exit_code();
}
