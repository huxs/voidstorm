#pragma once

#include "stack.h"
#include "pool.h"
#include <stdint.h>

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
	StackAllocator(Stack* stack)
	    {
		m_stack = stack;
	    }
				
	void* alloc(size_t size)
	    {
		return m_stack->alloc(size);
	    }

	void free(void* mem)
	    {

	    }		

	Stack* m_stack;
    };

    struct PoolAllocator : public AllocatorI
    {
	PoolAllocator(Pool* pool)
	    {
		m_pool = pool;
	    }

	void* alloc(size_t size)
	    {
		return m_pool->alloc();
	    }

	void free(void* mem)
	    {
		m_pool->free(mem);
	    }
	
	Pool* m_pool;
    };
}
