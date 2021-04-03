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
#include <cstdint>
#include "klib.h"

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
	__asm__ volatile("fninit\n"
			     "fstcw %0\n"
				 "andw $0xfffa, %0\n" // Generate invalid operation and div-by-zero exceptions
				 "fldcw %1"
				 :"=m"(fpu_contorl)
				 : "m"(fpu_contorl));

	/* Initialize globals and calls global constructors */
	custom_preinit();
	custom_init();

	klib_init();

	int exit_code = guest_main(data);

	/* Calls global destructors */
	custom_fini();

	/* Exit the VM */
	for (;;)
		asm("hlt" :: "a" (exit_code) : "memory");
}

