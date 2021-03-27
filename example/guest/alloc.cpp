#include <stdint.h>
#include <stdlib.h>

extern void* heap_start;

extern "C"
{
	intptr_t sbrk(ptrdiff_t heap_incr);
}

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

//		return -1;
//	}

	heap_end = new_heap_end;

	return prev_heap_end;
}

void *operator new(size_t size)
{   
	return malloc(size);
}

void *operator new[](size_t size)
{   
	return malloc(size);
}

void operator delete(void* p)
{
	free(p);
}

void operator delete[](void* p)
{
	free(p);
}

