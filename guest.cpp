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
#if 1
		uint16_t port = 0x42;

		asm("mov %0, %%rdi\n"
			"outl %1, %2"
			: /* empty */
			: "r" (mem), "a" ((uint32_t) id), "Nd" (port)
			: "memory", "rdi");
#else
		asm("mov %0, %%rdi\n"
			"outw %1, $0x42"
			: /* empty */
			: "r" (mem), "a" ((uint16_t) id)
			: "memory", "rdi");
#endif
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
	
	int __cxa_atexit(void (*f)(void*), void *objptr, void *dso);

//	void frame_dummy();
//	void __do_global_ctors_aux();

	void _start();
}

class A_CLASS_WITH_A_CONSTRUCTOR
{
public:
	A_CLASS_WITH_A_CONSTRUCTOR(int)
	{
		dispatcher d;
		sender_test st(d);
		st.puts((char*) "Constructive motherfucker\n");
//		st.putc(c);
//		st.putc('\n');
//		c = 'B';
	}

	void bla()
	{
		dispatcher d;
		sender_test st(d);
		st.puts((char*) "bla2: \"");
		st.putc(c);
		st.puts("\"\n");
	}

	char c = 'D';
};

int main();

extern uint64_t init_array;

static A_CLASS_WITH_A_CONSTRUCTOR a(42);

volatile static int abla = 42;

extern void* _data_end;
extern void* _code_end;

typedef void (*func_ptr)(void);

extern func_ptr __preinit_array_start[0], __preinit_array_end[0];
extern func_ptr __init_array_start[0], __init_array_end[0];
extern func_ptr __fini_array_start[0], __fini_array_end[0];

static void init_range(func_ptr* start, func_ptr* end)
{
	for ( func_ptr* func = start; func != end; func++ )
	{
		(*func)();
	}
}

static void custom_preinit(void) { init_range(__preinit_array_start, __preinit_array_end); }
static void custom_init(void) { init_range(__init_array_start, __init_array_end); }
static void custom_fini(void) { init_range(__fini_array_start, __fini_array_end); }
 
void __attribute__((noreturn)) __attribute__((section(".start"))) _start(void)
{
	fpu::fninit();
	uint16_t cw = fpu::fstcw();
	cw &= ~1;           // Clean IM bit: generate invalid operation exceptions
	cw &= ~(1 << 2);    // Set ZM bit: generate div-by-zero exceptions
	fpu::fldcw(cw);

	custom_preinit();
	custom_init();

	int exit_code = main();

	custom_fini();

	for (;;)
		asm("hlt" : /* empty */ : "a" (exit_code) : "memory");
}

dispatcher d;
sender_test st(d);

int main()
{
	st.puts("_code_end: ");
	st.pxint((uint64_t) &_code_end);
	st.putc('\n');

	st.puts("_data_end: ");
	st.pxint((uint64_t) &_data_end);
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
			st.puts("It's fucked\n");
			return -1;
		}
	}

//	*(int*) (2048 * 1024 * 2 + 4096 * 3 - 4) = 42;

	return 42;
}
