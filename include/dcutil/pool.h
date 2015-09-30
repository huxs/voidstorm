#pragma once

#include "dcutil_helper.h"

#include <stdlib.h>
#include <stdint.h>

namespace dcutil
{
    struct PoolElement
    {
	PoolElement* m_next;
    };

    class DCUTIL_API Pool
    {
    public:
	Pool(void* mem, size_t elementSize, size_t numElements);

	void* alloc();
	void free(void* ptr);

	PoolElement* m_start;

    private:
	void initialize(size_t elementSize, size_t numElements);

	PoolElement* m_next;
    };
}
