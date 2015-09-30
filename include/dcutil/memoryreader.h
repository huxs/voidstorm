#pragma once

#include "dcutil_helper.h"
#include <stdint.h>
#include <cstddef>

namespace dcutil
{
    class DCUTIL_API MemoryReader
    {
    public:
	MemoryReader(void* mem, size_t size);
	size_t read(void* data, size_t size);
    private:
	const char* m_data;
	size_t m_pos;
	size_t m_size;
    };

    template<typename T>
    inline size_t read(MemoryReader* reader, T& value)
    {
	return reader->read(&value, sizeof(value));
    }
}
