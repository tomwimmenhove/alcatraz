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
#include <iomanip>
#include <sys/mman.h>
#include <vector>
#include <mutex>
#include <condition_variable>

#include "cpudefs.h"
#include "rpcbuf.h"

#include "alcatraz.h"

alcatraz::alcatraz(uint64_t mem_size, const void* vm_code, size_t vm_code_size)
	: mem_size(mem_size), mem_pages_start(mem_size), vm_code_size(vm_code_size)
{
	/* Get the KVM instance */
	auto kvm = kvm::get_instance();

	/* Create a virtual machine instance */
	machine = kvm->create_vm();

	/* Allocate memory for the machine */
	mem = mmap(NULL, mem_size, PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
	if (mem == MAP_FAILED)
	{
		throw std::system_error(errno, std::generic_category());
	}
	memset(mem, 0, mem_size);
	mem_converter = std::make_unique<mem_convert>(mem, mem_size, entry_point);

	madvise(mem, mem_size, MADV_MERGEABLE);

	/* Stick it in slot 0 */
	machine->set_user_memory_region(0, 0, 0, mem_size, mem);

	/* Create the page tables */
	std::vector<page_table> pages = create_page_tables();

	/* Copy page table sinto mmapped memory */
	page_mem_size = pages.size() * sizeof(page_table);
	page_mem = mmap(NULL, page_mem_size, PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
	memcpy(page_mem, pages.data(), page_mem_size);

	/* Put physical page table memory in slot 1 */
	machine->set_user_memory_region(1, 0, mem_pages_start, page_mem_size, page_mem);

	/* Copy the guest code */
	memcpy(mem, vm_code, vm_code_size);
}

alcatraz::~alcatraz()
{
	kvm::get_instance()->destroy();

	munmap(page_mem, page_mem_size);
	munmap(mem, mem_size);
}

void alcatraz::set_receiver(std::unique_ptr<call_receiver>&& receiver)
{
	this->receiver = std::move(receiver);
}

void alcatraz::setup_64bit_code_segment(struct kvm_sregs *sregs)
{
	struct kvm_segment seg;
	seg.base = entry_point;
	seg.limit = 0xffffffff;
	seg.selector = 1 << 3;
	seg.present = 1;
	seg.type = 11; /* Code: execute; read; accessed */
	seg.dpl = 0;
	seg.db = 0;
	seg.s = 1; /* Code/data */
	seg.l = 1;
	seg.g = 1; /* 4KB granularity */

	sregs->cs = seg;

	seg.type = 3; /* Data: read/write, accessed */
	seg.selector = 2 << 3;
	sregs->ds = sregs->es = sregs->fs = sregs->gs = sregs->ss = seg;
}

void alcatraz::map_page(uint64_t virt, uint64_t phys, uint64_t phys_pages_start, std::vector<page_table>& pages)
{
	uint64_t pml4e = (virt >> 39) & 511;
	uint64_t pdpe = (virt >> 30) & 511;
	uint64_t pde = (virt >> 21) & 511;

	const size_t pml4e_idx = 0;

	if (pages.size() == 0)
	{
		pages.push_back(page_table{});
	}

	if (!(pages[pml4e_idx].entries[pml4e] & PDE64_PRESENT))
	{
		pages[pml4e_idx].entries[pml4e] = PDE64_PRESENT | PDE64_RW | PDE64_USER |
			((pages.size() << 12) + phys_pages_start);
		pages.push_back(page_table{});
	}
	size_t pdp_idx = (pages[pml4e_idx].entries[pml4e] - phys_pages_start) >> 12;

	if (!(pages[pdp_idx].entries[pdpe] & PDE64_PRESENT))
	{
		pages[pdp_idx].entries[pdpe] = PDE64_PRESENT | PDE64_RW | PDE64_USER |
			((pages.size() << 12) + phys_pages_start);
		pages.push_back(page_table{});
	}
	size_t pd_idx = (pages[pdp_idx].entries[pdpe] - phys_pages_start) >> 12;

	/* Finally,the actual page */
	pages[pd_idx].entries[pde] = PDE64_PRESENT | PDE64_RW | PDE64_USER | PDE64_PS | phys;
}

std::vector<alcatraz::page_table> alcatraz::create_page_tables()
{
	std::vector<page_table> pages;

	for (size_t virt = entry_point; virt < mem_size + entry_point; virt += 2 << 20)
	{
		map_page(virt, virt - entry_point, mem_pages_start, pages);
	}

	return pages;
}

void alcatraz::setup_long_mode(struct kvm_sregs *sregs)
{
	sregs->cr3 = mem_pages_start;
	sregs->cr4
		= CR4_PAE			
		| CR4_OSFXSR		// Set OSFXSR bit (Enable SSE support)	
		| CR4_OSXMMEXCPT	// Set OSXMMEXCPT bit (Enable unmasked SSE exceptions)
		| CR4_PGE;			// Set Page Global Enabled
	sregs->cr0
		= CR0_PE | CR0_MP | CR0_ET | CR0_NE | CR0_WP | CR0_AM | CR0_PG
		| CR0_NE	// Set NE bit (Native Exceptions)
		| CR0_MP;	// Set MP bit (FWAIT exempt from the TS bit)

	sregs->efer = EFER_LME | EFER_LMA;

	setup_64bit_code_segment(sregs);
}

void alcatraz::print_reg(std::string name, uint64_t value)
{
	std::cerr << name << ": 0x" << std::hex
		      << std::setfill('0') << std::setw(16) 
			  << value << " (" << std::dec << value << ")\n";
}

void alcatraz::print_regs(kvm_regs& regs)
{
	print_reg("rax", regs.rax);
	print_reg("rbx", regs.rbx);
	print_reg("rcx", regs.rcx);
	print_reg("rdx", regs.rdx);

	print_reg("rdi", regs.rdi);
	print_reg("rsi", regs.rsi);
	print_reg("rsp", regs.rsp);
	print_reg("rbp", regs.rbp);

	print_reg(" r8", regs.r8);
	print_reg(" r9", regs.r9);
	print_reg("r10", regs.r10);
	print_reg("r11", regs.r11);
	print_reg("r12", regs.r12);
	print_reg("r13", regs.r13);
	print_reg("r14", regs.r14);
	print_reg("r15", regs.r15);

	print_reg("rip", regs.rip);
	print_reg("rflags", regs.rflags);
}

std::unique_ptr<kvm_vcpu> alcatraz::setup_vcpu(void* data, size_t data_len,
		int cpu_id, size_t drop_stack)
{
	/* Create a virtual CPU instance on the virtual machine */
	auto vcpu = machine->create_vcpu(cpu_id);

	/* Setup the special registers */
	struct kvm_sregs sregs;
	vcpu->get_sregs(sregs);
	setup_long_mode(&sregs);
	vcpu->set_sregs(sregs);

	/* Set up the general registers */
	struct kvm_regs regs;
	memset(&regs, 0, sizeof(regs));
	/* Clear all FLAGS bits, except bit 1 which is always set. */
	regs.rflags = 2;
	regs.rip = entry_point; // Execution starts directly at the start of the virtual memmory

	/* Create stack right below the input data and page tables and grow down. */
	regs.rsp = entry_point + mem_size - data_len - drop_stack;
	if (data)
	{
		/* Make the first argument to _start() point to the input data structure */
		regs.rdi = regs.rsp;
		memcpy((void*) ((uintptr_t) mem + mem_size - data_len - drop_stack), data, data_len);
	}

	vcpu->set_regs(regs);

	return std::move(vcpu);
}

int alcatraz::run(std::unique_ptr<kvm_vcpu>& vcpu,
		call_receiver* receiver,
		alcatraz_dispatcher* dispatcher)
{
	/* Run the CPU */
	for (;;)
	{
		auto run = vcpu->run();

		switch(run->exit_reason)
		{
			case KVM_EXIT_HLT:
			{
				kvm_regs regs;
				vcpu->get_regs(regs);

				/* Return value is in %rax */
				return regs.rax;
			}

			case KVM_EXIT_IO:
			{
				if (run->io.direction == KVM_EXIT_IO_IN)
				{
					/* Callouts from host to guest's call pump */
					if (run->io.port == 0x43)
					{
						if (!dispatcher)
						{
							continue;
						}
						
						kvm_regs regs;
						vcpu->get_regs(regs);

						size_t id = dispatcher->wait(regs.rdi);
						vcpu->write_io_to_run((int) id);
						break;
					}
				}
				else if (run->io.direction == KVM_EXIT_IO_OUT)
				{
					/* Callout from host to guest has completed */
					if (run->io.port == 0x43)
					{
						if (!dispatcher)
						{
							continue;
						}

						dispatcher->nudge();
						break;
					}

					/* Callouts from guest to host */
					if (run->io.port == 0x42)
					{
						if (!receiver)
						{
							continue;
						}

						uint32_t id = vcpu->read_io_from_run();

						kvm_regs regs;
						vcpu->get_regs(regs);

						intptr_t space = mem_converter->space_at(regs.rdi);
						if (space < 0)
						{
							throw std::overflow_error("VM tried tried to access out-of-bound memory");
						}

						void* data = mem_converter->data_ptr_at<void*>(regs.rdi);

						receiver->exec(id, data, space);

						break;
					}
				}

				std::stringstream ss;
				ss << "Unknown IO error. Port=" << run->io.port
					<< ", size=" << (run->io.size * 8)
					<< "bits, direction:" << (run->io.direction == KVM_EXIT_IO_OUT
							? "out"
							: "in")
					<< '\n';

				throw std::invalid_argument(ss.str());
			}

			default:
			{
				auto regs = vcpu->get_regs();
#ifdef DEBUG
				print_regs(regs);

				//unsigned char* data = &((unsigned char*) mem)[regs.rip - entry_point];
				unsigned char* data = (unsigned char*) mem + 0x3b2;
				std::cout << "Data at rip: ";
				for(int i = 0; i < 15; i++)
				{
					std::cout << std::hex << std::setfill('0') << std::setw(2)
					          << (unsigned int) data[i] << " ";
				}
				std::cout << '\n';
#endif
				std::stringstream ss;
				ss << "Unexpected VCPU exist reason: " << run->exit_reason
				   << std::hex << std::setfill('0') << std::setw(2)
				   << ", rip=" << ((void*) regs.rip)<< '\n';

				throw std::invalid_argument(ss.str());
			}
		}
	}
}

/* alcatraz_dispatcher */

/* These (wait and nudge) are coupled to the KVM I/O operations */
size_t alcatraz_dispatcher::wait(uintptr_t guest_address)
{
	std::unique_lock<std::mutex> lk(m);

	if (!pump_ready)
	{
		pump_ready = true;
		cv.notify_one();
	}

	this->guest_address = guest_address;
	cv.wait(lk, [&] { return message_ready; } );
	message_ready = false;

	return id;
}

void alcatraz_dispatcher::nudge()
{   
	std::unique_lock<std::mutex> lk(m);
	response_ready = true;
	cv.notify_one();
}

void alcatraz_dispatcher::wait_pump_ready()
{
	std::unique_lock<std::mutex> lk(m);
	cv.wait(lk, [&] { return pump_ready; } );
}

void alcatraz_dispatcher::exec(size_t id, void* mem, size_t result_size, size_t param_size)
{   
	if (!guest_address)
	{   
		throw std::invalid_argument("Host called out to the guest, but no call pump was running\n");
	}

	auto& mem_converter = box->get_mem_converter();

	intptr_t space = mem_converter.space_at((intptr_t) guest_address);
	if (space < 0 || space < result_size || space < param_size)
	{   
		throw std::overflow_error("VM tried tried to access out-of-bound memory");
	}

	this->id = id;

	auto p = mem_converter.convert_to_host((void*) guest_address);

	{   
		std::unique_lock<std::mutex> lk(m);

		memcpy(p, mem, param_size);

		/* Wake up the guest's wait() */
		message_ready = true;
		cv.notify_one();
	}

	{   
		std::unique_lock<std::mutex> lk(m);

		/* Wait for the guest's nudge() */
		cv.wait(lk, [&] { return response_ready; } );
		response_ready = false;

		/* Copy the result back */
		memcpy(mem, p, result_size);
	}
}


