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
#ifndef ALCATRAZ_H
#define ALCATRAZ_H

#include <memory>
#include <kvmpp.h>

#include "mem_convert.h"

class alcatraz_dispatcher;

class alcatraz
{
public:
	alcatraz(uint64_t mem_size, const void* vm_code, size_t vm_code_size);
	
	void set_receiver(std::unique_ptr<call_receiver>&& receiver);
	inline void* get_mem() { return mem; }
	inline mem_convert& get_mem_converter() { return *mem_converter; }

	std::unique_ptr<kvm_vcpu> setup_vcpu(void* data = nullptr, size_t data_len = 0,
			int cpu_id = 0, size_t drop_stack = 0);

	template<typename T>
	std::unique_ptr<kvm_vcpu> setup_vcpu(T&& data,
			int cpu_id = 0, size_t drop_stack = 0)
	{
		return setup_vcpu((void*) &data, sizeof(T), cpu_id, drop_stack);
	}

	int run(std::unique_ptr<kvm_vcpu>& vcpu, call_receiver* receiver, alcatraz_dispatcher* dispatcher);

	virtual ~alcatraz();

private:
	struct page_table
	{
		uint64_t entries[512];
	};

	void setup_64bit_code_segment(struct kvm_sregs *sregs);
	void map_page(uint64_t virt, uint64_t phys, uint64_t phys_pages_start, std::vector<page_table>& pages);
	std::vector<page_table> create_page_tables();
	void setup_long_mode(struct kvm_sregs *sregs);
	void print_reg(std::string name, uint64_t value);
	void print_regs(kvm_regs& regs);

private:
	void* mem;
	size_t mem_size;
	size_t vm_code_size;
	void* page_mem;
	size_t page_mem_size;
	const uint64_t entry_point = 0x0000000000200000; // As defined in src/guest/linker.ld
	size_t mem_pages_start;
	std::unique_ptr<kvm_machine> machine;
	std::unique_ptr<call_receiver> receiver;
	std::unique_ptr<mem_convert> mem_converter;
};

class alcatraz_dispatcher : public call_dispatcher
{
public:
	alcatraz_dispatcher(alcatraz* box)
		: box(box)
	{ }

	virtual ~alcatraz_dispatcher() { }

	/* These (wait and nudge) are coupled to the KVM I/O operations */
	size_t wait(uintptr_t guest_address);
	void nudge();

	void wait_pump_ready();

protected:
	void exec(size_t id, void* mem, size_t result_size, size_t param_size);

private:
	alcatraz* box;
	std::mutex m;
	std::condition_variable cv;
	bool message_ready = false;
	bool response_ready = false;
	bool pump_ready = false;
	intptr_t guest_address = 0;
	size_t id;
};

#endif /* ALCATRAZ_H */
