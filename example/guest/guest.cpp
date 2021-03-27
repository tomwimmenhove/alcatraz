#include <stdlib.h>

#include <cstddef>
#include <cstdint>
#include <memory>

#include <rpcbuf.h>
#include <klib.h>
#include <guest_dispatcher.h>

#include "../input_data.h"

extern void* heap_start;
static uintptr_t new_p;

void *operator new[](size_t size);
void *operator new(size_t size);

void operator delete(void *) throw();
void operator delete[](void *) throw();

void operator delete(void*, long unsigned int) { }
void operator delete [](void*, long unsigned int) { }

#include <call_out_definitions.h>

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

extern "C"
{
	int puts(const char* s);
}

int puts(const char* s)
{
	return st.write(1, s, strlen(s));
}

class A_CLASS_WITH_A_CONSTRUCTOR
{
public:
	A_CLASS_WITH_A_CONSTRUCTOR(int)
	{
		guest_dispatcher d;
		sender_test st(d);
		puts((char*) "Constructive motherfucker\n");
//		st.putc(c);
//		st.putc('\n');
//		c = 'B';
	}

	void bla()
	{
		guest_dispatcher d;
		sender_test st(d);
		puts((char*) "bla2: \"");
		st.putc(c);
		puts("\"\n");
	}

	char c = 'D';
};

extern uint64_t init_array;

static A_CLASS_WITH_A_CONSTRUCTOR a(42);

volatile static int abla = 42;

extern void* _data_end;
extern void* _code_end;

int guest_main(void* data)
{
	auto test = new A_CLASS_WITH_A_CONSTRUCTOR(100);

	new_p = (intptr_t) heap_start;

	input_data* args = (input_data*) data;

	puts("Hello world!\n");

	puts("args.a: ");
	st.pdint(args->a);
	st.putc('\n');
	puts("args.b: ");
	st.pdint(args->b);
	st.putc('\n');


	puts("_code_end: ");
	st.pxint((uint64_t) &_code_end);
	st.putc('\n');

	puts("_data_end: ");
	st.pxint((uint64_t) &_data_end);
	st.putc('\n');

	puts("heap_start: ");
	st.pxint((uint64_t) heap_start);
	st.putc('\n');

	puts("&a: ");
	st.pxint((uint64_t) &a);
	st.putc('\n');

	void* p1 = malloc(10000);
	void* p2 = malloc(10000);
	free(p1);
	void* p3 = malloc(10000);

	puts("p1: ");
	st.pxint((uint64_t) p1);
	st.putc('\n');

	puts("p2: ");
	st.pxint((uint64_t) p2);
	st.putc('\n');

	puts("p3: ");
	st.pxint((uint64_t) p3);
	st.putc('\n');

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
			puts("It's fucked\n");
			return -1;
		}
	}

//	*(int*) (2048 * 1024 * 2 + 4096 * 3 - 4) = 42;

	return 42;
}
