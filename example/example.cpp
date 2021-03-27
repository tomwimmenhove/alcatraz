#include <string.h>
#include <iostream>

#include "rpcbuf.h"
#include "call_in_definitions.h"

#include "alcatraz.h"

#include "input_data.h"

class receiver_test : public call_receiver
{
#include "call_declarations.h"

public:
	receiver_test(void* mem) : CALL_OUT_INIT, mem(mem)
{   
	setup();
}

private:
	double foo(int x, float y, double z) { return x + y + z; }
	int foo2(int x, float y, double z) { return (int) (x + y + z); }
	void bar() { std::cout << "Hello world\n"; }
	void putc(char c) { std::cout << c; }
	void puts(const char* s) { std::cout << (&(((char*) mem)[(intptr_t) s - (2 << 20)])); }
	void dint(uint64_t x) { std::cout << "Debug int: " << std::dec << x << " (0x" << std::hex << x << ")\n"; }
	void pxint(uint64_t x) { std::cout << std::hex << x; }
	void pdint(uint64_t x) { std::cout << std::dec << x; }

	void* mem;
};

int main()
{
	extern const unsigned char guest64[], guest64_end[];
	alcatraz a(2048 * 1024 * 10, 2048 * 1024, guest64, guest64_end-guest64);

	auto rt = std::make_unique<receiver_test>(a.get_mem());

	a.set_receiver(std::move(rt));

	input_data args;
	args.a = 42;
	args.b = 43;

	a.run(&args, sizeof(input_data));

	return 0;
}
