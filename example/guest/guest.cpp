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

#include "../input_data.h"

/* This macro expands the the implementations or read/write/close etc, unless a custom functionis defined */
DEFINE_GUEST_DUMMY_CALLS

class sender_test : call_sender
{
	public:
		sender_test(call_dispatcher& guest_dispatcher)
			: call_sender(guest_dispatcher)
		{ }

	#include "../call_declarations.h"
};

static guest_dispatcher d;
sender_test st(d);

/* Custom write() function */
extern "C" {
	int64_t write(int fd, const void *buf, size_t count)
	{
		return st.write(fd, buf, count);
	}
}

class construction_test_class
{
public:
	construction_test_class(int x)
		: x(x)
	{
		printf("Constructor called with arg %d\n", x);
	}

	virtual ~construction_test_class()
	{
		printf("Destructor of %d\n", x);
	}

private:
	int x;
};

std::unique_ptr<construction_test_class> a = std::make_unique<construction_test_class>(42);

extern void* _data_end;
extern void* _code_end;

int guest_main(void* data)
{
	input_data* args = (input_data*) data;

	puts("Hello world!");

	printf("args.a: %d\n", args->a);
	printf("args.b: %d\n", args->b);
	printf("_code_end: %p\n", &_code_end);
	printf("_data_end: %p\n", &_data_end);

	void* p1 = malloc(1000);
	void* p2 = malloc(1000);
	free(p1);
	void* p3 = malloc(1000);

	printf("p1: %p\n", p1);
	printf("p2: %p\n", p2);
	printf("p3: %p\n", p3);

	float x = 42;
	printf("The squre root of %g is %g\n", x, sqrt(x));

	/* Crude memcheck */
	uint64_t rsp;
	asm("mov %%rsp, %0"
			: "=r" (rsp)
			:
			:
	   );

	for (uint64_t i = (uint64_t) &_data_end + 128; i < rsp - 128; i += sizeof(uint64_t))
	{
		*((uint64_t*) i) = i * 3;
	}
	for (uint64_t i = (uint64_t) &_data_end + 128; i < rsp - 128; i += sizeof(uint64_t))
	{
		if (*((uint64_t*) i) != (uint64_t) ( i * 3))
		{
			puts("It's fucked");
			return -1;
		}
	}

	return 0;
}
