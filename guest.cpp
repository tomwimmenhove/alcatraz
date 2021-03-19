#include <stddef.h>
#include <stdint.h>

#include <memory>

static uint64_t new_p = 1024 * 1024;

void *operator new(size_t size)
{
	uint64_t p = new_p;
	new_p += size;

	return (void*) p;
}

void *operator new[](size_t size)
{
	uint64_t p = new_p;
	new_p += size;

	return (void*) p;
}

void operator delete(void *) throw() { }
void operator delete[](void *) throw() { }
void operator delete(void*, long unsigned int) { }
void operator delete [](void*, long unsigned int) { }

#include "rpcbuf/src/rpcbuf.h"
#include "rpcbuf/src/call_out_definitions.h"

class sender_test : call_sender
{
	public:
		sender_test(call_dispatcher& dispatcher)
			: call_sender(dispatcher)
		{ }

#include "call_declarations.h"
};

class dispatcher : public call_dispatcher
{
public:
	virtual ~dispatcher()  { }

protected:
	void exec(size_t id, void* mem, size_t, size_t)
	{
		uint16_t port = 0x4242;

		asm("mov %0, %%rdi\n"
			"outl %1, %2"
			: /* empty */
			: "r" (mem), "a" ((uint32_t) id), "Nd" (port)
			: "memory", "rdi");
	}
};

class fpu
{
	public:
		static inline uint16_t fstcw()
		{   
			uint16_t r;

			asm volatile("fstcw %0;":"=m"(r));

			return r;
		}

		static inline void fninit() { asm volatile("fninit"); }

		static inline void fldcw(uint16_t control) { asm volatile("fldcw %0;"::"m"(control)); }
};

extern "C"
{
	void _init();
	void _fini();

	void _start();
}

class A
{
public:
	A()
	{
		dispatcher d;
		sender_test st(d);
		st.puts((char*) "Constructive motherfucker\n");
	}
};

int main();

static A a;

void __attribute__((noreturn)) __attribute__((section(".start"))) _start(void)
{
	fpu::fninit();
	uint16_t cw = fpu::fstcw();
	cw &= ~1;           // Clean IM bit: generate invalid operation exceptions
	cw &= ~(1 << 2);    // Set ZM bit: generate div-by-zero exceptions
	fpu::fldcw(cw);

//	_init();

	int exit_code = main();

//	_fini();

	for (;;)
		asm("hlt" : /* empty */ : "a" (exit_code) : "memory");
}

int main()
{
	dispatcher d;

	sender_test st(d);

	st.foo(0, 1, 2);
	int x = st.foo(1, 2, 3);
	st.bar();

	st.puts((char*) "Motherfucker\n");

	return x;
}
