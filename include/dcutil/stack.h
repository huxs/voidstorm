#pragma once

#include "dcutil_helper.h"

#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <stdio.h>

//TODO: Algned allocations.

namespace dcutil
{
    class DCUTIL_API Stack
    {
    public:
	Stack() {}
	Stack(void* base, size_t size)
	    : m_size(size), m_used(0)
	{
	    m_base = m_ptr = base;
	}
		
	inline void* alloc(size_t size);
	inline void reset();
	
	inline size_t getTotalSize();
	inline size_t getAllocatedSize();

    private:
	void* m_base;
	void* m_ptr;
	size_t m_size;
	size_t m_used;
    };

    void* Stack::alloc(size_t size)
    {
	assert((uint8_t*)m_ptr + size < ((uint8_t*)m_base + m_size) && "Stack allocator overflow");
	
	void* ptr = m_ptr;
	m_ptr = (uint8_t*)m_ptr + size;

	m_used += size;
	
	return ptr;
    }

    void Stack::reset()
    {
	m_ptr = m_base;
    }

    size_t Stack::getTotalSize()
    {
	return m_size;
    }

    size_t Stack::getAllocatedSize()
    {
	return ((size_t)m_ptr - (size_t)m_base);
    }
}
