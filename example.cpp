#include <string.h>
#include <iostream>
#include <iomanip>
#include <sys/mman.h>

#include "kvmpp/src/kvmpp.h"
#include "kvmpp/example/cpudefs.h"

static void setup_64bit_code_segment(struct kvm_sregs *sregs)
{
	struct kvm_segment seg;
	seg.base = 0;
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

static void setup_long_mode(void *mem_p, struct kvm_sregs *sregs)
{   
	char* mem = (char*) mem_p;

	uint64_t pml4_addr = 0x2000;
	uint64_t *pml4 = (uint64_t *)(mem + pml4_addr);

	uint64_t pdpt_addr = 0x3000;
	uint64_t *pdpt = (uint64_t *)(mem + pdpt_addr);

	uint64_t pd_addr = 0x4000;
	uint64_t *pd = (uint64_t *)(mem + pd_addr);

	pml4[0] = PDE64_PRESENT | PDE64_RW | PDE64_USER | pdpt_addr;
	pdpt[0] = PDE64_PRESENT | PDE64_RW | PDE64_USER | pd_addr;
	pd[0] = PDE64_PRESENT | PDE64_RW | PDE64_USER | PDE64_PS;

	sregs->cr3 = pml4_addr;
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

#include "rpcbuf/src/rpcbuf.h"
#include "rpcbuf/src/call_in_definitions.h"

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
	void puts(const char* s) { std::cout << (&(((char*) mem)[(intptr_t) s])); }
	void dint(uint64_t x) { std::cout << "Debug int: " << std::dec << x << " (0x" << std::hex << x << ")\n"; }
	void pxint(uint64_t x) { std::cout << std::hex << x; }
	void pdint(uint64_t x) { std::cout << std::dec << x; }

	void* mem;
};

void print_reg(std::string name, uint64_t value)
{
	std::cout << name << ": 0x" << std::hex << std::setfill('0') << std::setw(16) << value << " (" << std::dec << value << ")\n";
}

void print_regs(kvm_regs& regs)
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

int main()
{
	/* Get the KVM instance */
	auto kvm = kvm::get_instance();

	/* Create a virtual machine instance */
	auto machine = kvm->create_vm();

	/* Allocate memory for the machine */
	size_t mem_size = 2048 * 1024;
	void* mem = mmap(NULL, mem_size, PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
	if (mem == MAP_FAILED)
	{
		throw std::system_error(errno, std::generic_category());
	}
	memset(mem, 0, mem_size);

	madvise(mem, mem_size, MADV_MERGEABLE);

	/* Stick it in slot 0 */
	machine->set_user_memory_region(0, 0, 0, mem_size, mem);


	/* Create a virtual CPU instance on the virtual machine */
	auto vcpu = machine->create_vcpu();

	/* Setup the special registers */
	struct kvm_sregs sregs;
	vcpu->get_sregs(sregs);
	setup_long_mode(mem, &sregs);
	vcpu->set_sregs(sregs);

	/* Set up the general registers */
	struct kvm_regs regs;
	memset(&regs, 0, sizeof(regs));
	/* Clear all FLAGS bits, except bit 1 which is always set. */
	regs.rflags = 2;
	regs.rip = 0x0000;
	/* Create stack at top of 2 MB page and grow down. */
	regs.rsp = (2 << 20) - 128000;
	vcpu->set_regs(regs);


	/* Copy the guest code */
	extern const unsigned char guest64[], guest64_end[];
	memcpy(&(((char*)mem)[regs.rip]), guest64, guest64_end-guest64);

	receiver_test rt(mem);

	/* Run the CPU */
	for (;;)
	{
		auto run = vcpu->run();

		int exit = 0;
		switch(run->exit_reason)
		{
			case KVM_EXIT_HLT:
			{
				kvm_regs regs;
				vcpu->get_regs(regs);
				std::cout << "Result: " << std::dec << regs.rax << " (" << std::hex << regs.rax << "), rip=" << ((void*) regs.rip)<< '\n';

				exit = 1;
				break;
			}

			case KVM_EXIT_IO:
			{
				if (run->io.direction == KVM_EXIT_IO_OUT
						&& run->io.port == 0x42)// && run->io.size == sizeof(uint32_t))
				{
					//uint32_t id = *(uint32_t*) ((uintptr_t) run + run->io.data_offset);
					uint32_t id;
					switch (run->io.size)
					{
						case sizeof(uint32_t):
							id = *(uint32_t*) ((uintptr_t) run + run->io.data_offset);
							break;
						case sizeof(uint16_t):
							id = *(uint16_t*) ((uintptr_t) run + run->io.data_offset);
							break;
						case sizeof(uint8_t):
							id = *(uint8_t*) ((uintptr_t) run + run->io.data_offset);
							break;
					}

					//std::cout << "IO id " << id <<  " @" << run->io.data_offset << '\n';

					kvm_regs regs;
					vcpu->get_regs(regs);

					if (regs.rdi > mem_size)
					{
						throw std::overflow_error("VM tried tried to access out-of-bound memory");
					}

					void* data = (void*) ((uint64_t) regs.rdi + (uint64_t) mem);
					size_t vm_mem_space = mem_size - regs.rdi;

					rt.exec(id, data, vm_mem_space);
				}
				else
				{
					std::cout << "Unknown IO error. Port=" << run->io.port << ", size=" << (run->io.size * 8) << "bits, direction:" << (run->io.direction == KVM_EXIT_IO_OUT ? "out" : "in") << '\n';
					break;
				}
				break;
			}

			default:
			{
				exit = 1;
				auto regs = vcpu->get_regs();
				std::cout << "VCPU exited with reason: " << run->exit_reason << ", rip=" << ((void*) regs.rip)<< '\n';

				unsigned char* ripdat = (unsigned char*) mem + regs.rip;
				std::cout << "Data at rip: ";
				for (int i = 0; i < 15; i++)
				{
					std::cout << std::hex << std::setfill('0') << std::setw(2) << ((int) ripdat[i]) << ' ';
				}
				std::cout << '\n';

				print_regs(regs);
				break;
			}
		}

		if (exit)
		{
			break;
		}
	}

	kvm->destroy();

	return 0;
}
