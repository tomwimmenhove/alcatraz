/* Taken from https://github.com/tomwimmenhove/toyos */

#include "klib.h"

#include <malloc.h>

/* Horrible unoptimized shit. */

extern void* heap_start;
extern "C" {

void __attribute__((noreturn)) __assert_func(const char* file, int line, const char* fn, const char* assertion);
intptr_t sbrk(ptrdiff_t heap_incr);

void _exit(int status)
{
	asm("hlt" :: "a" (status) : "memory");
}

#ifdef DUMMY_IO
int64_t write(int fd, const void *buf, size_t count) { return 0; }
int64_t read(int fd, void *buf, size_t count) { return 0; }
int close(int fd) { return 0; }
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

intptr_t sbrk(ptrdiff_t heap_incr)
{
	static intptr_t heap_end = 0;

	intptr_t prev_heap_end;
	intptr_t new_heap_end;

	if(heap_end == 0)
	{
		heap_end = (intptr_t) heap_start;
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
		for(;;);
//		panic("__throw_bad_alloc() called");
	}

	void __throw_bad_function_call()
	{
		for(;;);
//		panic("__throw_bad_function_call() callde");
	}
}

#if 0
void *memset(void *s, int c, size_t n)
{
	uint8_t* p = (uint8_t*) s;

	while (n--)
		*p++ = c;

	return s;
}

void *memcpy(void *dest, const void *src, size_t n)
{
	const uint8_t* s = (const uint8_t*) src;
	uint8_t* d = (uint8_t*) dest;

	while (n--)
		*d++ = *s++;

	return dest;
}

void *memmove(void *dest, const void *src, size_t n)
{
	const uint8_t* s = (const uint8_t*) src;
	uint8_t* d = (uint8_t*) dest;

	if (dest < src)
		return memcpy(dest, src, n);

	for (size_t i = n; i > 0; i--)
		d[i - 1] = s[i - 1];

	return dest;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
	const uint8_t* p1 = (const uint8_t*) s1;
	const uint8_t* p2 = (const uint8_t*) s2;

	while(n--)
	{
		if (*p1 < *p2)
			return -1;
		if (*p1 > *p2)
			return 1;
		p1++;
		p2++;
	}

	return 0;
}

int strncmp(const char *s1, const char *s2, size_t n)
{
	while (*s1 && *s2 && n--)
	{
		if (*s1 < *s2)
			return -1;
		if (*s1 > *s2)
			return 1;

		s1++;
		s2++;
	}

	if (*s1 < *s2)
		return -1;
	if (*s1 > *s2)
		return 1;

	return 0;
}

int strcmp(const char *s1, const char *s2)
{
	return strncmp(s1, s2, 0xffffffffffffffff);
}
/*
char *strdup(const char *s)
{
	auto len = strlen(s);
	char* r = (char*) mallocator::malloc(len + 1);
	memcpy((void*) r, (void*) s, len + 1);

	return r;
}*/

char *index(const char *s, int c)
{
	while (*s)
	{
		if (*s == c)
			return (char*) s;
		s++;
	}

	if (c == 0)
		return (char*) s;

	return nullptr;
}
#endif

void __cxa_pure_virtual()
{
		for(;;);
//	panic("Virtual method called");
}

void __attribute__((noreturn)) __assert_func(const char* file, int line, const char* fn, const char* assertion)
{
		for(;;);
//	con << file << ':' << line << ':' << fn << ": Assertion '" << assertion << "' failed.\n";
//	panic("Assertion failed\n");
}

