#ifndef MEM_CONVERT_H
#define MEM_CONVERT_H

#include <cstdint>

class mem_convert
{
public:
    mem_convert(void* mem, size_t mem_size, intptr_t base_offset)
        : mem((uint64_t) mem), mem_size(mem_size), base_offset(base_offset)
    { }

    intptr_t space_at(uint64_t address)
    {   
        return mem_size - address + base_offset;
    }

    template<typename T>
    T data_ptr_at(uint64_t address)
    {   
        return (T) (address - base_offset + mem);
    }

private:
    uint64_t mem;
    size_t mem_size;
    intptr_t base_offset;
};

#endif /* MEM_CONVERT_H */