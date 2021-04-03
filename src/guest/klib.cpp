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
/* Taken from https://github.com/tomwimmenhove/toyos */

#include "klib.h"

#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

extern "C" {

void __attribute__((noreturn)) panic(const char *format, ...);

void __attribute__((noreturn)) __assert_func(const char* file, int line, const char* fn, const char* assertion);
void __attribute__((noreturn)) __assert_fail(const char * assertion, const char * file, unsigned int line, const char * function);
intptr_t sbrk(ptrdiff_t heap_incr);

void __attribute__((noreturn)) _exit(int status)
{
	for (;;)
	{
		asm("hlt" :: "a" (status) : "memory");
	}
}

#ifdef DUMMY_IO
int64_t write(int fd, const void *buf, size_t count) { return 0; }
int64_t read(int fd, void *buf, size_t count) { return 0; }
int close(int fd) { return 0; }
#else
int64_t write(int fd, const void *buf, size_t count);
int64_t read(int fd, void *buf, size_t count);
int close(int fd);
#endif

int kill(int pid, int sig) { return 0; }
int getpid(void) { return 0; }
int fstat(int fd, struct stat *statbuf) { return 0; }
int isatty(int fd) { return 0; }
int64_t lseek(int fd, int64_t offset, int whence) { return 0; }

}

void *operator new(size_t size)
{
	return malloc(size);
}

void *operator new[](size_t size)
{
	return malloc(size);
}

void operator delete(void* p) throw()
{
	free(p);
}

void operator delete[](void* p) throw()
{
	free(p);
}

void operator delete(void*, long unsigned int) { }
void operator delete [](void*, long unsigned int) { }

extern void* _data_end;
intptr_t sbrk(ptrdiff_t heap_incr)
{
	static intptr_t heap_end = 0;

	intptr_t prev_heap_end;
	intptr_t new_heap_end;

	if(heap_end == 0)
	{
		heap_end = (intptr_t) &_data_end;
	}

	prev_heap_end = heap_end;
	new_heap_end = prev_heap_end + heap_incr;

	//	if(new_heap_end >= end_of_mem)
	//	{
	//		errno = ENOMEM;
	//
	//		return -1;
	//	}

	heap_end = new_heap_end;

	return prev_heap_end;
}

namespace std
{
	void __throw_bad_alloc()
	{
		panic("__throw_bad_alloc() called");
	}

	void __throw_bad_function_call()
	{
		panic("__throw_bad_function_call() callde");
	}
}

void __attribute__((noreturn)) panic(const char *format, ...)
{
	va_list args;
	va_start(args, format);

	vprintf(format, args);

	va_end(args);

	exit(1);
}

void __cxa_pure_virtual()
{
	panic("Virtual method called");
}

void __attribute__((noreturn)) __assert_func(const char* file, int line, const char* fn, const char* assertion)
{
	panic("%s: %d: %s: Assertion %s failed.\n", file, line, fn, assertion);
}

void __attribute__((noreturn)) __assert_fail(const char * assertion, const char * file, unsigned int line, const char * fn)
{
	__assert_func(file, line, fn, assertion);
}
