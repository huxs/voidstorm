#pragma once

#include <dcutil/allocator.h>
#include <dlmalloc/dlmalloc.h>

struct HeapAllocator : public dcutil::AllocatorI
{
    HeapAllocator(mspace _ms)
	    : ms(_ms) {}
    
    void* alloc(size_t size)
	{
	    void *ptr =  mspace_malloc(ms, size);
	    return ptr;
	}

    void* realloc(void *ptr, size_t size)
	{
	    void *ret = mspace_realloc(ms, ptr, size);
	    return ret;
	}
    
    void free(void *ptr)
	{
	    mspace_free(ms, ptr);
	}

    mspace ms;
};

struct LuaAllocatorUserData
{
    dcutil::PoolAllocator *pool;
    HeapAllocator *heap;
};

struct TinyStlAllocator
{
    static void* static_allocate(size_t _bytes);
    static void static_deallocate(void* _ptr, size_t);
};

