#pragma once

#include <dcutil/allocator.h>
#include <dlmalloc/dlmalloc.h>
#include <mutex>

struct HeapAllocator : public dcutil::AllocatorI
{
    HeapAllocator(mspace _ms)
	    : ms(_ms) {}
    
    void* alloc(size_t size)
	{
	    mutex.lock();
	    void* ptr =  mspace_malloc(ms, size);
	    mutex.unlock();
	    return ptr;
	}

    void free(void* ptr)
	{
	    mutex.lock();
	    mspace_free(ms, ptr);
	    mutex.unlock();
	}

    std::mutex mutex;
    mspace ms;
};

struct TinyStlAllocator
{
    static void* static_allocate(size_t _bytes);
    static void static_deallocate(void* _ptr, size_t);
};

