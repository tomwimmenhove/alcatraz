#ifndef ALCATRAZ_H
#define ALCATRAZ_H

#include <memory>

#include "kvmpp/src/kvmpp.h"

class alcatraz
{
public:
	alcatraz(uint64_t mem_size, uint64_t entry_point, const void* vm_code, size_t vm_code_size);
	
	void set_receiver(std::unique_ptr<call_receiver>&& receiver);
	void* get_mem();

	int run(void* data = nullptr, size_t data_len = 0);

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
	void* page_mem;
	size_t page_mem_size;
	uint64_t entry_point;
	size_t mem_pages_start;
	std::unique_ptr<kvm_machine> machine;
	std::unique_ptr<call_receiver> receiver;
};

#endif /* ALCATRAZ_H */
