#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdio.h>

#include <guest.h>

#include "../input_data.h"

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

extern "C" {
int puts(const char* s);

int64_t read(int fd, const void *buf, size_t count) { return 0; }
int close(int) { return 0; }
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

int guest_main(void* data)
{
	input_data* args = (input_data*) data;

	puts("Hello world!");

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
