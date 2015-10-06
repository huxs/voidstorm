#pragma once

#include <stdint.h>
#include <assert.h>

#define CHECK_HANDLE_LEAK(_allocator)		\
    assert(_allocator.getNumHandles() == 0);

namespace dcutil
{
    template<uint16_t size>
    class HandleAllocator
    {
    public:
	HandleAllocator()
		: m_handleCount(0), m_recCount(0)
	    {
		assert(size < UINT16_MAX);
		for (uint16_t i = 0; i < size; ++i)
		{
		    m_handles[i] = i;
		    m_recylced[i] = 0;
		}
	    }
	
	uint16_t alloc()
	    {
		if (m_recCount > 0)
		{
		    uint16_t v = m_recylced[0];
		    
		    for(int i = 1; i < m_recCount; ++i)
			m_recylced[i-1] = m_recylced[i];
		    
		    m_recCount--;
		    
		    return v;
		}

		if (m_handleCount < size)
		{
		    uint16_t index = m_handleCount++;
		    return m_handles[index];
		}
		
		assert(false && "Out of handles error.");
		return UINT16_MAX;
	    }

	void free(uint16_t handle)
	    {
		m_recylced[m_recCount] = handle;
		m_recCount++;
	    }

	uint16_t getNumHandles() { return m_handleCount - m_recCount; }
	uint16_t getMaxHandles() { return size; }

    private:
	uint16_t m_handles[size];
	uint16_t m_recylced[size];
	uint16_t m_handleCount;
	uint16_t m_recCount;
    };
}
