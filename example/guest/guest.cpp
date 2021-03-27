#include <cstddef>
#include <cstdint>
#include <memory>

#include <rpcbuf.h>
#include <klib.h>

#include "../input_data.h"

extern void* heap_start;
static uintptr_t new_p;

void *operator new(size_t size)
{
	uintptr_t p = new_p;
	new_p += size;

	return (void*) p;
}

void *operator new[](size_t size)
{
	uintptr_t p = new_p;
	new_p += size;

	return (void*) p;
}

void operator delete(void *) throw() { }
void operator delete[](void *) throw() { }
void operator delete(void*, long unsigned int) { }
void operator delete [](void*, long unsigned int) { }

#include <call_out_definitions.h>

class sender_test : call_sender
{
	public:
		sender_test(call_dispatcher& dispatcher)
			: call_sender(dispatcher)
		{ }

	#include "../call_declarations.h"
};

class dispatcher : public call_dispatcher
{
public:
	virtual ~dispatcher()  { }

protected:
	void exec(size_t id, void* mem, size_t, size_t)
	{
		uint16_t port = 0x42;
		asm("mov %0, %%rdi\n"
			"outl %1, %2"
			: /* empty */
			: "r" (mem), "a" ((uint32_t) id), "Nd" (port)
			: "memory", "rdi");
	}
};

static dispatcher d;
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
		dispatcher d;
		sender_test st(d);
		puts((char*) "Constructive motherfucker\n");
//		st.putc(c);
//		st.putc('\n');
//		c = 'B';
	}

	void bla()
	{
		dispatcher d;
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
