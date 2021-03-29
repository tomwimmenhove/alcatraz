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
#include <string.h>
#include <iostream>
#include <unistd.h>

#include <thread> 
#include <mutex>
#include <condition_variable> 

#include "rpcbuf.h"
#include "call_in_definitions.h"

#include "alcatraz.h"
#include "input_data.h"

std::mutex m;
std::condition_variable cv;
bool message_ready = false;
uintptr_t fn_address;
std::unique_ptr<kvm_vcpu> vcpu;
size_t payload_size = 256;
void *payload = nullptr;

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

	void set_msg_pump_ready(bool ready) { msg_pump_ready = ready; }

	void set_test_address(uintptr_t address)
	{
		fn_address = address;
	}

	void wait()
	{
		std::unique_lock<std::mutex> lk(m);
		cv.wait(lk, [&] { return message_ready; } );
		message_ready = false;

		/* Read the guest registers */
		kvm_regs regs;
		vcpu->get_regs(regs);

		/* Subtract 128 bytes (red zone) + payload size + room for all GP registers (minus the stack pointer) */
		size_t size = 128 + sizeof(intptr_t) * 16 + payload_size;
		regs.rsp -= size;

		/* Find the actuall address (on the host) of the stack on the guest */
		auto& mem_converter = box->get_mem_converter();
		intptr_t space = mem_converter.space_at(regs.rsp);
		if (space < 0 || space < size)
		{   
			throw std::overflow_error("VM tried tried to access out-of-bound memory");
		}
		uintptr_t* rsp = mem_converter.convert_to_host((uintptr_t*) regs.rsp);

		/* Stack layout:
		 *
		 * + ----------------------------+
		 * | Red zone                    |
		 * + ----------------------------+
		 * | Return pointer (Old %rip)   |
		 * +-----------------------------+
		 * | All (but %rsp) GP registers |
		 * +-----------------------------+
		 * | Payload                     |
		 * +-----------------------------+ <- %rsp
		 *
		 * So, on the guest side where %rip starts to execute, we can now safely
		 * call a user-fuction. After that we'll have to add %rdi to %rsp (as if
		 * to pop the payload back from the stack), pop all registers (in the
		 * order as they're copied below), increase %rsp with another 136 (return
		 * pointer and 128 bytes for the red-zone) and, finally, jump to the return
		 * pointer (%rsp - 136)
		 */

		/* 'push' payload onto stack */
		if (payload_size && payload)
		{
			memcpy(rsp, payload, payload_size);
		}
		rsp = (uintptr_t*)((uintptr_t) rsp + payload_size);

		/* Copy all registers to the stack */
		*rsp++ = regs.r15;	*rsp++ = regs.r14;	*rsp++ = regs.r13;	*rsp++ = regs.r12;
		*rsp++ = regs.r11;	*rsp++ = regs.r10;	*rsp++ = regs.r9;	*rsp++ = regs.r8;
		*rsp++ = regs.rdi;	*rsp++ = regs.rsi;	*rsp++ = regs.rbp;	/* rsp skipped */
		*rsp++ = regs.rbx;	*rsp++ = regs.rdx;	*rsp++ = regs.rcx;	*rsp++ = regs.rax;

		/* Set the current address as return address */
		*rsp++ = regs.rip;

		/* Finally, set up the function we're about to call */
		regs.rip = fn_address;		/* Copy the address of the function into the instruction pointer */
		regs.rdi = regs.rsp;		/* First argument points to the payload */
		regs.rsi = payload_size;	/* Second argument is the payload size */

		/* Set the altered registers on the guest */
		vcpu->set_regs(regs);
	}

	alcatraz* box;
	bool msg_pump_ready = false;
//	std::mutex mutex;
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

	vcpu = box.setup_vcpu(args);

	std::thread box_thread([&](){
			box.run(vcpu.get());
			});

	const char* s = "Hello from the host!\n";

	while (box_thread.joinable())
	{
		std::unique_lock<std::mutex> lk(m);
		message_ready = true;

		payload = (void*) s;
		payload_size = strlen(s);

		cv.notify_one();
		lk.unlock();
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}


	box_thread.join();
}
