#include <string.h>
#include <iostream>
#include <unistd.h>

#include "rpcbuf.h"
#include "call_in_definitions.h"

#include "alcatraz.h"
#include "input_data.h"

class receiver : public call_receiver
{
#include "call_declarations.h"

public:
	receiver(alcatraz* box) : CALL_OUT_INIT, box(box)
	{   
		setup();
	}

private:
	void putc(char c) { std::cout << c; }
	void dint(uint64_t x) { std::cout << "Debug int: " << std::dec << x << " (0x" << std::hex << x << ")\n"; }
	void pxint(uint64_t x) { std::cout << std::hex << x; }
	void pdint(uint64_t x) { std::cout << std::dec << x; }
	size_t write(int fd, const void *buf, size_t count)
	{
		if (fd < 1 || fd > 2)
		{
			return -1;
		}

		auto& mem_converter = box->get_mem_converter();
		intptr_t space = mem_converter.space_at((uint64_t) buf);
		if (space < 0 || space < count)
		{   
			throw std::overflow_error("VM tried tried to access out-of-bound memory");
		}

		::write(fd, mem_converter.convert_to_host(buf), count);

		return 0;
	}

	alcatraz* box;
};

extern const unsigned char guest64[], guest64_end[];

int main()
{
	const size_t mem_size = 2048 * 1024 * 10;

	alcatraz box(mem_size, guest64, guest64_end-guest64);
	auto rt = std::make_unique<receiver>(&box);

	box.set_receiver(std::move(rt));

	input_data args;
	args.a = 42;
	args.b = 43;

	box.run(args);
}
