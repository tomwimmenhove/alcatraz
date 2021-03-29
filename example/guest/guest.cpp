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

extern "C"{
void test_function(void* payload, size_t payload_size)
{
	write(1, payload, payload_size);
}
}

asm("asm_test_function:\n"
	/* All registers have been  pushed onto the stack already at this point */

	"push %rsi\n"

	/* call our function */
	"callq test_function\n"

	"pop %rsi\n"
	"add %rsi, %rsp\n" // Payload

	/* Restore all registers */
	"pop %r15\n"
	"pop %r14\n"
	"pop %r13\n"
	"pop %r12\n"
	"pop %r11\n"
	"pop %r10\n"
	"pop %r9\n"
	"pop %r8\n"
	"pop %rdi\n"
	"pop %rsi\n"
	"pop %rbp\n"
	/* .... rsp skipped */
	"pop %rbx\n"
	"pop %rdx\n"
	"pop %rcx\n"
	"pop %rax\n"

	/* Return */
	"add $136, %rsp\n"	// 8 bytes for the return address and 128 bytes red-zone
    "jmp *-136(%rsp)");	// Jump back to the return address


int guest_main(void* data)
{
	input_data* args = (input_data*) data;

	puts("Hello world!");

	//st.set_test_address((uintptr_t) &test_function);
	extern uintptr_t asm_test_function;
	st.set_test_address((uintptr_t) &asm_test_function);

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

	while (true)
	{
		st.wait();
//		printf("Got data from host\n");
	}

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
