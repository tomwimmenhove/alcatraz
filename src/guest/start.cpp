#include <cstdint>

extern "C"
{
	void _start(void* data);
}

int guest_main(void* data);

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

void __attribute__((noreturn)) __attribute__((section(".start"))) _start(void* data)
{
	uint16_t fpu_contorl;
	asm volatile("fninit\n"
			     "fstcw %0\n"
				 "andw $0xfffa, %0\n" // Generate invalid operation and div-by-zero exceptions
				 "fldcw %1"
				 :"=m"(fpu_contorl)
				 : "m"(fpu_contorl));

	/* Initialize globals and calls global constructors */
	custom_preinit();
	custom_init();

	int exit_code = guest_main(data);

	/* Calls global destructors */
	custom_fini();

	/* Exit the VM */
	for (;;)
		asm("hlt" :: "a" (exit_code) : "memory");
}

