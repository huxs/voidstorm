#pragma once

#include <stdint.h>
#include <assert.h>
#include <stdint.h>

// TODO (daniel): Aligned allocations

namespace dcutil
{
    struct AllocatorI
    {
	virtual void* alloc(size_t size) = 0;
	virtual void free(void* mem) = 0;
    };

    struct CrtAllocator : public AllocatorI
    {
	void* alloc(size_t size)
	    {
		return operator new(size);
	    }

	void free(void* mem)
	    {
		operator delete(mem);
	    }
    };

    struct StackAllocator : public AllocatorI
    {
	StackAllocator() {}
	StackAllocator(void* base, size_t size)
		: m_size(size)
	    {
	        m_base = m_ptr = base;
	    }
				
	void* alloc(size_t size)
	    {
		assert((uint8_t*)m_ptr + size < ((uint8_t*)m_base + m_size) && "Stack allocator overflow");
	
		void* ptr = m_ptr;
		m_ptr = (uint8_t*)m_ptr + size;

		return ptr;
	    }

	void free(void* mem) {}

	void reset()
	    {
		m_ptr = m_base;
	    }

        size_t getAllocatedSize() const
	    {
		return ((size_t)m_ptr - (size_t)m_base);
	    }

	void* m_base;
	void* m_ptr;
	size_t m_size;
    };

    struct PoolElement
    {
	PoolElement* m_next;
    };
    
    struct PoolAllocator : public AllocatorI
    {
	PoolAllocator() {}
	PoolAllocator(void* base, size_t elementSize, size_t numElements)
	    {
		m_start = (PoolElement*)base;
		a = 0;
		
		union 
		{
		    void* as_void;
		    char* as_char;
		    PoolElement* as_self;
		};

		as_void = m_start;
		m_next = as_self;

		PoolElement* runner = m_next;
		for(size_t i = 0; i < numElements; ++i)
		{	
		    runner->m_next = as_self;
		    runner = as_self;
		    as_char += elementSize;
		}

		runner->m_next = nullptr;
	    }

	void* alloc(size_t size)
	    {
		a++;
		assert(m_next != nullptr && "Out of elements!");
		
		PoolElement* head = m_next;
		m_next = head->m_next;
		return head;
	    }

	void free(void* mem)
	    {
		a--;
		if (mem == nullptr) return;
		
		PoolElement* head = (PoolElement*)mem;
		head->m_next = m_next;
		m_next = head;
	    }

	PoolElement* m_start;
	PoolElement* m_next;
	int a;
    };
}
