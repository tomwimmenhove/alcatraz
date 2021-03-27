#ifndef GUEST_DISPATCHER_H
#define GUEST_DISPATCHER_H

#include <rpcbuf.h>

class guest_dispatcher : public call_dispatcher
{
public:
	virtual ~guest_dispatcher()  { }

protected:
	void exec(size_t id, void* mem, size_t, size_t)
	{   
		uint16_t port = 0x42;
		asm("mov %0, %%rdi\n"
				"outl %1, %2"
				: /* empty */
				: "r" (mem), "a" ((uint32_t) id), "Nd" (port)
				: "memory", "rdi");
	}
};

#endif /* GUEST_DISPATCHER_H */
